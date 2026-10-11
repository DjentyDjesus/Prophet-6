#pragma once

#include "Common.hpp"
#include <array>
#include <cstdint>
#include <algorithm>

namespace prophet6 {

struct SequenceStep {
    std::array<uint8_t, 6> notes{};
    uint8_t noteCount{0};
    uint8_t velocity{100};
    bool isTie{false};
    bool isRest{false};
};

// Zero-allocation, 64-step polyphonic Prophet-6 step sequencer
class StepSequencer {
public:
    static constexpr size_t MAX_STEPS = 64;

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
        currentStep_ = 0;
        recordStep_ = 0;
        recordKeyCount_ = 0;
        recordNotesDown_.fill(false);
        sampleCounter_ = 0;
        stepSamples_ = 0;
        gateSamples_ = 0;
        isGateOpen_ = false;
        activePlayingNotes_.fill(0);
        activeNoteCount_ = 0;
        transpositionSemitones_ = 0;
        rootKey_ = 60; // Middle C default root
        updateStepSamples();
    }

    void setEnabled(bool enabled) noexcept {
        if (enabled_ != enabled) {
            enabled_ = enabled;
            if (!enabled_) {
                isPlaying_ = false;
                isRecording_ = false;
            }
        }
    }
    [[nodiscard]] bool isEnabled() const noexcept { return enabled_; }

    void setPlaying(bool playing) noexcept {
        if (isPlaying_ != playing) {
            isPlaying_ = playing;
            if (isPlaying_) {
                currentStep_ = 0;
                sampleCounter_ = stepSamples_; // trigger first step immediately on first sample
                isGateOpen_ = false;
            }
        }
    }
    [[nodiscard]] bool isPlaying() const noexcept { return isPlaying_; }

    void setRecording(bool recording) noexcept {
        if (isRecording_ != recording) {
            isRecording_ = recording;
            if (isRecording_) {
                recordStep_ = 0;
                recordKeyCount_ = 0;
                recordNotesDown_.fill(false);
            }
        }
    }
    [[nodiscard]] bool isRecording() const noexcept { return isRecording_; }

    void setSequenceLength(int length) noexcept {
        sequenceLength_ = clamp(static_cast<size_t>(length), size_t{1}, MAX_STEPS);
        if (currentStep_ >= sequenceLength_) {
            currentStep_ = 0;
        }
    }
    [[nodiscard]] size_t getSequenceLength() const noexcept { return sequenceLength_; }
    void setLength(int length) noexcept { setSequenceLength(length); }
    [[nodiscard]] size_t getLength() const noexcept { return getSequenceLength(); }

    [[nodiscard]] size_t getCurrentStep() const noexcept { return currentStep_; }
    [[nodiscard]] size_t getRecordStep() const noexcept { return recordStep_; }

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

    // Step Recording Actions
    void recordRest() noexcept {
        if (!isRecording_ || recordStep_ >= MAX_STEPS) return;

        auto& step = steps_[recordStep_];
        step.noteCount = 0;
        step.isRest = true;
        step.isTie = false;

        advanceRecordStep();
    }

    void recordTie() noexcept {
        if (!isRecording_ || recordStep_ >= MAX_STEPS) return;

        auto& step = steps_[recordStep_];
        step.noteCount = 0;
        step.isRest = false;
        step.isTie = true;

        advanceRecordStep();
    }

    // Handle incoming MIDI for recording or live transposition
    void handleMidiNoteOn(int noteNumber, float velocity) noexcept {
        if (noteNumber < 0 || noteNumber > 127) return;

        if (isRecording_) {
            if (recordStep_ < MAX_STEPS) {
                auto& step = steps_[recordStep_];
                if (recordKeyCount_ == 0) {
                    step.noteCount = 0;
                    step.isRest = false;
                    step.isTie = false;
                    step.velocity = static_cast<uint8_t>(clamp(velocity * 127.0f, 1.0f, 127.0f));
                }

                if (!recordNotesDown_[static_cast<size_t>(noteNumber)]) {
                    recordNotesDown_[static_cast<size_t>(noteNumber)] = true;
                    ++recordKeyCount_;
                    if (step.noteCount < 6) {
                        step.notes[step.noteCount++] = static_cast<uint8_t>(noteNumber);
                    }
                }

                if (recordStep_ == 0 && step.noteCount == 1) {
                    rootKey_ = static_cast<uint8_t>(noteNumber);
                }
            }
        } else if (isPlaying_) {
            // Live transposition
            transpositionSemitones_ = noteNumber - static_cast<int>(rootKey_);
        }
    }

    void handleMidiNoteOff(int noteNumber) noexcept {
        if (noteNumber < 0 || noteNumber > 127) return;

        if (isRecording_) {
            if (recordNotesDown_[static_cast<size_t>(noteNumber)]) {
                recordNotesDown_[static_cast<size_t>(noteNumber)] = false;
                if (recordKeyCount_ > 0) {
                    --recordKeyCount_;
                }
                if (recordKeyCount_ == 0) {
                    // All keys released: advance to next step
                    advanceRecordStep();
                }
            }
        }
    }

    // Process single audio sample
    template <typename NoteOnFn, typename NoteOffFn>
    void processSample(NoteOnFn&& onNoteOn, NoteOffFn&& onNoteOff) noexcept {
        if (!enabled_ || !isPlaying_ || sequenceLength_ == 0) {
            if (activeNoteCount_ > 0) {
                for (size_t i = 0; i < activeNoteCount_; ++i) {
                    onNoteOff(activePlayingNotes_[i]);
                }
                activeNoteCount_ = 0;
                isGateOpen_ = false;
            }
            return;
        }

        // Gate cutoff (only if next step isn't a Tie)
        if (isGateOpen_ && sampleCounter_ >= gateSamples_) {
            const size_t nextStepIdx = (currentStep_ + 1) % sequenceLength_;
            if (!steps_[nextStepIdx].isTie) {
                for (size_t i = 0; i < activeNoteCount_; ++i) {
                    onNoteOff(activePlayingNotes_[i]);
                }
                activeNoteCount_ = 0;
                isGateOpen_ = false;
            }
        }

        // Advance to next step
        if (sampleCounter_ >= stepSamples_) {
            sampleCounter_ = 0;

            const auto& step = steps_[currentStep_];

            if (step.isTie) {
                // Keep playing previous notes across tie
                isGateOpen_ = true;
            } else {
                // Turn off old notes
                for (size_t i = 0; i < activeNoteCount_; ++i) {
                    onNoteOff(activePlayingNotes_[i]);
                }
                activeNoteCount_ = 0;
                isGateOpen_ = false;

                if (!step.isRest && step.noteCount > 0) {
                    // Trigger new step chord
                    const float vel = static_cast<float>(step.velocity) * (1.0f / 127.0f);
                    for (size_t i = 0; i < step.noteCount; ++i) {
                        const int transposedNote = clamp(static_cast<int>(step.notes[i]) + transpositionSemitones_, 0, 127);
                        onNoteOn(transposedNote, vel);
                        activePlayingNotes_[activeNoteCount_++] = static_cast<uint8_t>(transposedNote);
                    }
                    isGateOpen_ = true;
                }
            }

            currentStep_ = (currentStep_ + 1) % sequenceLength_;
        }

        ++sampleCounter_;
    }

    [[nodiscard]] std::array<SequenceStep, MAX_STEPS>& getSteps() noexcept { return steps_; }
    [[nodiscard]] const std::array<SequenceStep, MAX_STEPS>& getSteps() const noexcept { return steps_; }

private:
    void advanceRecordStep() noexcept {
        ++recordStep_;
        recordKeyCount_ = 0;
        if (recordStep_ >= sequenceLength_) {
            // Sequence length reached; stop recording
            isRecording_ = false;
            recordStep_ = 0;
        }
    }

    void updateStepSamples() noexcept {
        // Standard 16th note steps (0.25 beat)
        constexpr float divisionBeats = 0.25f;
        const float stepSeconds = (60.0f / bpm_) * divisionBeats;
        stepSamples_ = static_cast<uint32_t>(stepSeconds * sampleRate_);
        if (stepSamples_ < 10) stepSamples_ = 10;
        gateSamples_ = static_cast<uint32_t>(stepSamples_ * gateRatio_);
    }

    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    bool enabled_ = false;
    bool isPlaying_ = false;
    bool isRecording_ = false;

    size_t sequenceLength_ = 16;
    size_t currentStep_ = 0;
    size_t recordStep_ = 0;
    size_t recordKeyCount_ = 0;
    std::array<bool, 128> recordNotesDown_{};

    float bpm_ = 120.0f;
    float gateRatio_ = 0.75f;
    uint32_t sampleCounter_ = 0;
    uint32_t stepSamples_ = 0;
    uint32_t gateSamples_ = 0;
    bool isGateOpen_ = false;

    uint8_t rootKey_ = 60;
    int transpositionSemitones_ = 0;

    std::array<SequenceStep, MAX_STEPS> steps_{};
    std::array<uint8_t, 6> activePlayingNotes_{};
    size_t activeNoteCount_ = 0;
};

} // namespace prophet6
