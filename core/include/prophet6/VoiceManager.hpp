#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include "Voice.hpp"
#include <array>
#include <cstdint>
#include <algorithm>

namespace prophet6 {

inline constexpr size_t NUM_VOICES = 6;

// 6-Voice Polyphonic Allocator with click-free voice stealing and unison stacking
class VoiceManager {
public:
    VoiceManager() noexcept = default;

    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        for (size_t i = 0; i < NUM_VOICES; ++i) {
            voices_[i].init(static_cast<uint32_t>(i), sampleRate);
        }
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            for (auto& voice : voices_) {
                voice.setSampleRate(sampleRate);
            }
        }
    }

    void reset() noexcept {
        for (auto& voice : voices_) {
            voice.reset();
        }
        activeNoteCount_ = 0;
    }

    // Allocate note according to polyphonic or unison rules
    void noteOn(int noteNumber, float velocity, const SynthParameters& params) noexcept {
        if (params.unison) {
            // Unison Mode: Stack all 6 voices on the same fundamental note with stereo detuning
            // Detune spread in semitones across 6 voices:
            constexpr std::array<float, NUM_VOICES> unisonSpread = {
                -0.12f, 0.12f, -0.06f, 0.06f, -0.02f, 0.02f
            };
            for (size_t i = 0; i < NUM_VOICES; ++i) {
                const float detunedNote = static_cast<float>(noteNumber) + unisonSpread[i] * params.unisonDetune;
                voices_[i].noteOn(detunedNote, velocity, params.ampVelocityAmount, false);
            }
            activeNoteCount_ = 1;
            return;
        }

        const float noteFloat = static_cast<float>(noteNumber);

        // Polyphonic Mode: 6 voices
        // 1. Check if this exact note is already sounding (retrigger)
        for (auto& voice : voices_) {
            if (voice.isActive() && voice.getNoteNumber() == noteNumber) {
                voice.noteOn(noteFloat, velocity, params.ampVelocityAmount, false);
                return;
            }
        }

        // 2. Find an idle / free voice
        for (auto& voice : voices_) {
            if (!voice.isActive()) {
                voice.noteOn(noteFloat, velocity, params.ampVelocityAmount, true);
                ++activeNoteCount_;
                return;
            }
        }

        // 3. All 6 voices active: Find voice in Release state with quietest level
        int bestReleaseIdx = -1;
        float lowestReleaseLevel = 1e9f;

        for (size_t i = 0; i < NUM_VOICES; ++i) {
            if (voices_[i].isReleasing()) {
                const float lvl = voices_[i].getAmpLevel();
                if (lvl < lowestReleaseLevel) {
                    lowestReleaseLevel = lvl;
                    bestReleaseIdx = static_cast<int>(i);
                }
            }
        }

        if (bestReleaseIdx >= 0) {
            voices_[bestReleaseIdx].noteOn(noteFloat, velocity, params.ampVelocityAmount, false);
            return;
        }

        // 4. All voices currently held in sustain: Steal oldest voice (FIFO)
        size_t oldestIdx = 0;
        uint64_t maxAge = 0;
        for (size_t i = 0; i < NUM_VOICES; ++i) {
            if (voices_[i].getAge() > maxAge) {
                maxAge = voices_[i].getAge();
                oldestIdx = i;
            }
        }

        // Steal with 5ms micro-fade
        voices_[oldestIdx].noteOn(noteFloat, velocity, params.ampVelocityAmount, false);
    }

    void noteOff(int noteNumber, const SynthParameters& params) noexcept {
        if (params.unison) {
            for (auto& voice : voices_) {
                voice.release();
            }
            activeNoteCount_ = 0;
            return;
        }

        for (auto& voice : voices_) {
            if (voice.isActive() && voice.getNoteNumber() == noteNumber) {
                voice.noteOff(noteNumber);
            }
        }
    }

    void allNotesOff() noexcept {
        for (auto& voice : voices_) {
            voice.release();
        }
        activeNoteCount_ = 0;
    }

    // Render 1 stereo sample summing all 6 voices
    void renderSample(
        const SynthParameters& params,
        float globalLfo,
        float pitchBendSemitones,
        float& outL,
        float& outR) noexcept
    {
        outL = 0.0f;
        outR = 0.0f;

        // Sum 6 discrete voice cards
        for (auto& voice : voices_) {
            voice.renderSample(params, globalLfo, pitchBendSemitones, outL, outR);
        }
    }

    [[nodiscard]] std::array<Voice, NUM_VOICES>& getVoices() noexcept { return voices_; }
    [[nodiscard]] const std::array<Voice, NUM_VOICES>& getVoices() const noexcept { return voices_; }

    [[nodiscard]] size_t getActiveVoiceCount() const noexcept {
        size_t count = 0;
        for (const auto& v : voices_) {
            if (v.isActive()) ++count;
        }
        return count;
    }

    [[nodiscard]] float getLowestActiveNoteFreq() const noexcept {
        int lowestNote = 128;
        for (const auto& v : voices_) {
            if (v.isActive()) {
                const int n = v.getNoteNumber();
                if (n < lowestNote) lowestNote = n;
            }
        }
        if (lowestNote <= 127) {
            return midiNoteToFrequency(static_cast<float>(lowestNote));
        }
        return 110.0f; // Default 110 Hz (A2) if idle
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    std::array<Voice, NUM_VOICES> voices_{};
    size_t activeNoteCount_ = 0;
};

} // namespace prophet6
