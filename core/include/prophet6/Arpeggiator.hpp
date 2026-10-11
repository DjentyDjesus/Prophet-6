#pragma once

#include "Common.hpp"
#include <array>
#include <cstdint>
#include <algorithm>

namespace prophet6 {

enum class ArpMode : uint8_t {
    Up = 0,
    Down,
    UpDown,
    Random,
    Assign
};

enum class ClockDivision : uint8_t {
    Div_1_2 = 0,
    Div_1_4,
    Div_1_8,
    Div_1_8T,
    Div_1_16,
    Div_1_16T,
    Div_1_32
};

[[nodiscard]] constexpr float getClockDivisionBeats(ClockDivision div) noexcept {
    switch (div) {
        case ClockDivision::Div_1_2:   return 2.0f;
        case ClockDivision::Div_1_4:   return 1.0f;
        case ClockDivision::Div_1_8:   return 0.5f;
        case ClockDivision::Div_1_8T:  return 1.0f / 3.0f;
        case ClockDivision::Div_1_16:  return 0.25f;
        case ClockDivision::Div_1_16T: return 1.0f / 6.0f;
        case ClockDivision::Div_1_32:  return 0.125f;
        default:                       return 0.25f;
    }
}

// Zero-allocation, sample-accurate Prophet-6 Arpeggiator
class Arpeggiator {
public:
    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            updateStepSamples();
        }
    }

    void reset() noexcept {
        numHeldNotes_ = 0;
        patternLength_ = 0;
        currentPatternIndex_ = 0;
        sampleCounter_ = 0;
        stepSamples_ = 0;
        gateSamples_ = 0;
        currentlyPlayingNote_ = -1;
        isGateOpen_ = false;
        allKeysReleased_ = true;
        physicalKeyDown_.fill(false);
        numPhysicalKeysDown_ = 0;
        randomSeed_ = 987654321;
        updateStepSamples();
    }

    void setEnabled(bool enabled) noexcept {
        if (enabled_ != enabled) {
            enabled_ = enabled;
            if (!enabled_) {
                reset();
            }
        }
    }
    [[nodiscard]] bool isEnabled() const noexcept { return enabled_; }

    void setMode(ArpMode mode) noexcept {
        if (mode_ != mode) {
            mode_ = mode;
            rebuildPattern();
        }
    }
    [[nodiscard]] ArpMode getMode() const noexcept { return mode_; }

    void setOctaveRange(int octaves) noexcept {
        const int clamped = clamp(octaves, 1, 3);
        if (octaveRange_ != clamped) {
            octaveRange_ = clamped;
            rebuildPattern();
        }
    }
    [[nodiscard]] int getOctaveRange() const noexcept { return octaveRange_; }
    void setOctave(int octaves) noexcept { setOctaveRange(octaves); }
    [[nodiscard]] int getOctave() const noexcept { return getOctaveRange(); }

    void setDivision(ClockDivision div) noexcept {
        if (division_ != div) {
            division_ = div;
            updateStepSamples();
        }
    }
    [[nodiscard]] ClockDivision getDivision() const noexcept { return division_; }

    void setHold(bool hold) noexcept {
        hold_ = hold;
        if (!hold_ && allKeysReleased_) {
            numHeldNotes_ = 0;
            rebuildPattern();
        }
    }
    [[nodiscard]] bool getHold() const noexcept { return hold_; }

    void setBpm(float bpm) noexcept {
        const float clampedBpm = clamp(bpm, 30.0f, 300.0f);
        if (std::abs(bpm_ - clampedBpm) > 0.01f) {
            bpm_ = clampedBpm;
            updateStepSamples();
        }
    }
    [[nodiscard]] float getBpm() const noexcept { return bpm_; }

    void setGateRatio(float ratio) noexcept {
        gateRatio_ = clamp(ratio, 0.1f, 0.95f);
        updateStepSamples();
    }

    // Called when a physical MIDI key is pressed
    void noteOn(int noteNumber, float velocity) noexcept {
        if (noteNumber < 0 || noteNumber > 127) return;

        if (!physicalKeyDown_[static_cast<size_t>(noteNumber)]) {
            physicalKeyDown_[static_cast<size_t>(noteNumber)] = true;
            ++numPhysicalKeysDown_;
        }

        // If in hold mode and all keys were previously released, start new chord
        if (hold_ && allKeysReleased_) {
            numHeldNotes_ = 0;
        }
        allKeysReleased_ = false;

        // Add note if not already present
        for (size_t i = 0; i < numHeldNotes_; ++i) {
            if (heldNotes_[i] == static_cast<uint8_t>(noteNumber)) {
                heldVelocities_[i] = velocity;
                return;
            }
        }

        if (numHeldNotes_ < MAX_HELD) {
            heldNotes_[numHeldNotes_] = static_cast<uint8_t>(noteNumber);
            heldVelocities_[numHeldNotes_] = velocity;
            ++numHeldNotes_;
            rebuildPattern();
        }
    }

    // Called when a physical MIDI key is released
    void noteOff(int noteNumber) noexcept {
        if (noteNumber < 0 || noteNumber > 127) return;

        if (physicalKeyDown_[static_cast<size_t>(noteNumber)]) {
            physicalKeyDown_[static_cast<size_t>(noteNumber)] = false;
            if (numPhysicalKeysDown_ > 0) {
                --numPhysicalKeysDown_;
            }
        }

        if (numPhysicalKeysDown_ == 0) {
            allKeysReleased_ = true;
        }

        if (hold_) {
            // Keep playing latched notes in hold mode until new keys are pressed
            return;
        }

        for (size_t i = 0; i < numHeldNotes_; ++i) {
            if (heldNotes_[i] == static_cast<uint8_t>(noteNumber)) {
                // Shift down
                for (size_t j = i; j < numHeldNotes_ - 1; ++j) {
                    heldNotes_[j] = heldNotes_[j + 1];
                    heldVelocities_[j] = heldVelocities_[j + 1];
                }
                --numHeldNotes_;
                rebuildPattern();
                break;
            }
        }
    }

    // Callback signatures for audio engine dispatch
    template <typename NoteOnFn, typename NoteOffFn>
    void processSample(NoteOnFn&& onNoteOn, NoteOffFn&& onNoteOff) noexcept {
        if (!enabled_ || patternLength_ == 0) {
            if (currentlyPlayingNote_ >= 0) {
                onNoteOff(currentlyPlayingNote_);
                currentlyPlayingNote_ = -1;
                isGateOpen_ = false;
            }
            sampleCounter_ = 0;
            return;
        }

        // Trigger first note immediately when arp starts playing from idle
        if (currentlyPlayingNote_ < 0 && sampleCounter_ == 0) {
            currentPatternIndex_ = 0;
            const int nextNote = patternNotes_[0];
            const float nextVel = patternVelocities_[0];
            onNoteOn(nextNote, nextVel);
            currentlyPlayingNote_ = nextNote;
            isGateOpen_ = true;
        }

        // Check if gate time expired
        if (isGateOpen_ && sampleCounter_ >= gateSamples_) {
            if (currentlyPlayingNote_ >= 0) {
                onNoteOff(currentlyPlayingNote_);
                currentlyPlayingNote_ = -1;
            }
            isGateOpen_ = false;
        }

        // Check if step period expired (trigger next note)
        if (sampleCounter_ >= stepSamples_) {
            sampleCounter_ = 0;

            if (currentlyPlayingNote_ >= 0) {
                onNoteOff(currentlyPlayingNote_);
                currentlyPlayingNote_ = -1;
            }

            advancePattern();

            const int nextNote = patternNotes_[currentPatternIndex_];
            const float nextVel = patternVelocities_[currentPatternIndex_];

            onNoteOn(nextNote, nextVel);
            currentlyPlayingNote_ = nextNote;
            isGateOpen_ = true;
        }

        ++sampleCounter_;
    }

    [[nodiscard]] int getCurrentNote() const noexcept { return currentlyPlayingNote_; }
    [[nodiscard]] bool isGateOpen() const noexcept { return isGateOpen_; }
    [[nodiscard]] size_t getPatternLength() const noexcept { return patternLength_; }
    [[nodiscard]] size_t getNumHeldNotes() const noexcept { return numHeldNotes_; }

private:
    void updateStepSamples() noexcept {
        // Beats per second = bpm / 60
        // Seconds per beat = 60 / bpm
        const float divisionBeats = getClockDivisionBeats(division_);
        const float stepSeconds = (60.0f / bpm_) * divisionBeats;
        stepSamples_ = static_cast<uint32_t>(stepSeconds * sampleRate_);
        if (stepSamples_ < 10) stepSamples_ = 10;
        gateSamples_ = static_cast<uint32_t>(stepSamples_ * gateRatio_);
    }

    void advancePattern() noexcept {
        if (patternLength_ == 0) return;

        if (mode_ == ArpMode::Random) {
            // Xorshift32 PRNG
            randomSeed_ ^= randomSeed_ << 13;
            randomSeed_ ^= randomSeed_ >> 17;
            randomSeed_ ^= randomSeed_ << 5;
            currentPatternIndex_ = randomSeed_ % patternLength_;
        } else {
            currentPatternIndex_ = (currentPatternIndex_ + 1) % patternLength_;
        }
    }

    void rebuildPattern() noexcept {
        if (numHeldNotes_ == 0) {
            patternLength_ = 0;
            currentPatternIndex_ = 0;
            return;
        }

        // Copy held notes into temp sort buffer
        std::array<uint8_t, MAX_HELD> sortedNotes{};
        std::array<float, MAX_HELD> sortedVels{};
        for (size_t i = 0; i < numHeldNotes_; ++i) {
            sortedNotes[i] = heldNotes_[i];
            sortedVels[i]  = heldVelocities_[i];
        }

        if (mode_ == ArpMode::Up || mode_ == ArpMode::Down || mode_ == ArpMode::UpDown || mode_ == ArpMode::Random) {
            // Sort ascending by pitch
            for (size_t i = 0; i < numHeldNotes_; ++i) {
                for (size_t j = i + 1; j < numHeldNotes_; ++j) {
                    if (sortedNotes[j] < sortedNotes[i]) {
                        std::swap(sortedNotes[i], sortedNotes[j]);
                        std::swap(sortedVels[i],  sortedVels[j]);
                    }
                }
            }
        }

        size_t idx = 0;

        switch (mode_) {
            case ArpMode::Up: {
                for (int oct = 0; oct < octaveRange_; ++oct) {
                    for (size_t i = 0; i < numHeldNotes_; ++i) {
                        const int n = sortedNotes[i] + oct * 12;
                        if (n <= 127 && idx < MAX_PATTERN) {
                            patternNotes_[idx] = static_cast<uint8_t>(n);
                            patternVelocities_[idx] = sortedVels[i];
                            ++idx;
                        }
                    }
                }
                break;
            }

            case ArpMode::Down: {
                for (int oct = octaveRange_ - 1; oct >= 0; --oct) {
                    for (int i = static_cast<int>(numHeldNotes_) - 1; i >= 0; --i) {
                        const int n = sortedNotes[static_cast<size_t>(i)] + oct * 12;
                        if (n <= 127 && idx < MAX_PATTERN) {
                            patternNotes_[idx] = static_cast<uint8_t>(n);
                            patternVelocities_[idx] = sortedVels[static_cast<size_t>(i)];
                            ++idx;
                        }
                    }
                }
                break;
            }

            case ArpMode::UpDown: {
                // Ascending
                for (int oct = 0; oct < octaveRange_; ++oct) {
                    for (size_t i = 0; i < numHeldNotes_; ++i) {
                        const int n = sortedNotes[i] + oct * 12;
                        if (n <= 127 && idx < MAX_PATTERN) {
                            patternNotes_[idx] = static_cast<uint8_t>(n);
                            patternVelocities_[idx] = sortedVels[i];
                            ++idx;
                        }
                    }
                }
                // Descending (skip top note and bottom note to avoid double hits)
                if (idx > 2) {
                    for (int i = static_cast<int>(idx) - 2; i > 0; --i) {
                        if (idx < MAX_PATTERN) {
                            patternNotes_[idx] = patternNotes_[static_cast<size_t>(i)];
                            patternVelocities_[idx] = patternVelocities_[static_cast<size_t>(i)];
                            ++idx;
                        }
                    }
                }
                break;
            }

            case ArpMode::Assign: {
                // Follow exact key press arrival order
                for (int oct = 0; oct < octaveRange_; ++oct) {
                    for (size_t i = 0; i < numHeldNotes_; ++i) {
                        const int n = heldNotes_[i] + oct * 12;
                        if (n <= 127 && idx < MAX_PATTERN) {
                            patternNotes_[idx] = static_cast<uint8_t>(n);
                            patternVelocities_[idx] = heldVelocities_[i];
                            ++idx;
                        }
                    }
                }
                break;
            }

            case ArpMode::Random:
            default: {
                for (int oct = 0; oct < octaveRange_; ++oct) {
                    for (size_t i = 0; i < numHeldNotes_; ++i) {
                        const int n = sortedNotes[i] + oct * 12;
                        if (n <= 127 && idx < MAX_PATTERN) {
                            patternNotes_[idx] = static_cast<uint8_t>(n);
                            patternVelocities_[idx] = sortedVels[i];
                            ++idx;
                        }
                    }
                }
                break;
            }
        }

        patternLength_ = idx;
        if (currentPatternIndex_ >= patternLength_) {
            currentPatternIndex_ = 0;
        }
    }

    static constexpr size_t MAX_HELD = 16;
    static constexpr size_t MAX_PATTERN = 64;

    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    bool enabled_ = false;
    ArpMode mode_ = ArpMode::Up;
    int octaveRange_ = 1;
    ClockDivision division_ = ClockDivision::Div_1_16;
    bool hold_ = false;
    float bpm_ = 120.0f;
    float gateRatio_ = 0.65f;

    std::array<uint8_t, MAX_HELD> heldNotes_{};
    std::array<float, MAX_HELD> heldVelocities_{};
    size_t numHeldNotes_ = 0;
    bool allKeysReleased_ = true;
    std::array<bool, 128> physicalKeyDown_{};
    size_t numPhysicalKeysDown_ = 0;

    std::array<uint8_t, MAX_PATTERN> patternNotes_{};
    std::array<float, MAX_PATTERN> patternVelocities_{};
    size_t patternLength_ = 0;
    size_t currentPatternIndex_ = 0;

    uint32_t sampleCounter_ = 0;
    uint32_t stepSamples_ = 0;
    uint32_t gateSamples_ = 0;
    int currentlyPlayingNote_ = -1;
    bool isGateOpen_ = false;
    uint32_t randomSeed_ = 987654321;
};

} // namespace prophet6
