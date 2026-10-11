#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include "VoiceManager.hpp"
#include "LFO.hpp"
#include "Distortion.hpp"
#include "Arpeggiator.hpp"
#include "StepSequencer.hpp"
#include "Effects.hpp"
#include "Preset.hpp"
#include "MidiMap.hpp"
#include <cstdint>
#include <cstddef>
#include <algorithm>

namespace prophet6 {

// Top-Level 6-Voice Polyphonic Prophet-6 Synthesizer Engine
// Zero dynamic runtime allocations on audio thread.
class SynthEngine {
public:
    SynthEngine() noexcept = default;

    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        voiceManager_.init(sampleRate);
        globalLfo_.init(sampleRate);
        distortion_.init(sampleRate);
        arpeggiator_.init(sampleRate);
        sequencer_.init(sampleRate);
        effects_.init(sampleRate);
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            voiceManager_.setSampleRate(sampleRate);
            globalLfo_.setSampleRate(sampleRate);
            distortion_.setSampleRate(sampleRate);
            arpeggiator_.setSampleRate(sampleRate);
            sequencer_.setSampleRate(sampleRate);
            effects_.setSampleRate(sampleRate);
        }
    }

    void reset() noexcept {
        voiceManager_.reset();
        globalLfo_.reset();
        distortion_.reset();
        arpeggiator_.reset();
        sequencer_.reset();
        effects_.reset();
        pitchBendNorm_ = 0.0f;
        modWheelNorm_ = 0.0f;
    }

    void setTempo(double bpm) noexcept {
        const float b = static_cast<float>(bpm);
        arpeggiator_.setBpm(b);
        sequencer_.setBpm(b);
    }

    void setTransport(bool isPlaying, double /*ppqPosition*/) noexcept {
        if (sequencer_.isEnabled()) {
            sequencer_.setPlaying(isPlaying);
        }
    }

    // --- MIDI Message Handlers ---

    void noteOn(int noteNumber, float velocity) noexcept {
        if (noteNumber < 0 || noteNumber > 127) return;

        if (sequencer_.isRecording()) {
            sequencer_.handleMidiNoteOn(noteNumber, velocity);
            voiceManager_.noteOn(noteNumber, velocity, params_);
        } else if (sequencer_.isEnabled() && sequencer_.isPlaying()) {
            sequencer_.handleMidiNoteOn(noteNumber, velocity);
        } else if (arpeggiator_.isEnabled()) {
            arpeggiator_.noteOn(noteNumber, velocity);
        } else {
            voiceManager_.noteOn(noteNumber, velocity, params_);
            if (params_.lfoSync) {
                globalLfo_.reset(0.0f);
            }
        }
    }

    void noteOff(int noteNumber) noexcept {
        if (noteNumber < 0 || noteNumber > 127) return;

        if (sequencer_.isRecording()) {
            sequencer_.handleMidiNoteOff(noteNumber);
            voiceManager_.noteOff(noteNumber, params_);
        } else if (sequencer_.isEnabled() && sequencer_.isPlaying()) {
            // Note off during sequence playback
        } else if (arpeggiator_.isEnabled()) {
            arpeggiator_.noteOff(noteNumber);
        } else {
            voiceManager_.noteOff(noteNumber, params_);
        }
    }

    // pitchBendNormalized: -1.0f (full down) to +1.0f (full up), 0.0f = center
    void pitchBend(float pitchBendNormalized) noexcept {
        pitchBendNorm_ = clamp(pitchBendNormalized, -1.0f, 1.0f);
    }

    void controlChange(int ccNumber, int ccValue) noexcept {
        if (ccNumber == 120 || ccNumber == 123) {
            allNotesOff();
            return;
        }
        MidiMap::applyControlChange(ccNumber, ccValue, params_, modWheelNorm_);
    }

    void loadPreset(const Preset& preset) noexcept {
        params_ = preset.params;
        arpeggiator_.setEnabled(preset.arpEnabled);
        arpeggiator_.setMode(preset.arpMode);
        arpeggiator_.setOctave(preset.arpOctave);
        arpeggiator_.setDivision(preset.arpDivision);
        arpeggiator_.setHold(preset.arpHold);
        sequencer_.setEnabled(preset.seqEnabled);
        sequencer_.setLength(preset.seqLength);
        sequencer_.setBpm(preset.seqBpm);
    }

    void allNotesOff() noexcept {
        voiceManager_.allNotesOff();
        arpeggiator_.reset();
        sequencer_.reset();
    }

    // --- Audio Rendering ---

    // Render a single stereo sample frame
    void processSample(float& outL, float& outR) noexcept {
        // Step Sequencer clock & note dispatch (takes precedence over Arp when playing)
        if (sequencer_.isEnabled() && sequencer_.isPlaying()) {
            sequencer_.processSample(
                [this](int note, float vel) { voiceManager_.noteOn(note, vel, params_); },
                [this](int note)            { voiceManager_.noteOff(note, params_); }
            );
        } else if (arpeggiator_.isEnabled()) {
            arpeggiator_.processSample(
                [this](int note, float vel) { voiceManager_.noteOn(note, vel, params_); },
                [this](int note)            { voiceManager_.noteOff(note, params_); }
            );
        }

        // Calculate global LFO
        const float lfoMod = globalLfo_.process(params_.lfoFrequency, params_.lfoShape);

        // Effective LFO depth including mod wheel
        const float totalLfoDepth = clamp(params_.lfoInitialAmount + modWheelNorm_, 0.0f, 1.0f);
        const float lfoSignal = lfoMod * totalLfoDepth;

        // Pitch bend in semitones
        const float pitchBendSemitones = pitchBendNorm_ * params_.pitchBendRange;

        // Render voices to master stereo bus
        voiceManager_.renderSample(params_, lfoSignal, pitchBendSemitones, outL, outR);

        // Apply master analog distortion
        distortion_.process(outL, outR, params_.distortion);

        // Apply dual digital stereo effects directly after master distortion
        if (params_.fxOn) {
            effects_.processSample(outL, outR, params_);
        }

        // Master output volume scaling
        outL *= params_.masterVolume;
        outR *= params_.masterVolume;
    }

    // Process a block of audio samples into separate non-interleaved L and R buffers
    void process(float* outL, float* outR, size_t numSamples) noexcept {
        if (!outL || !outR || numSamples == 0) return;

        for (size_t i = 0; i < numSamples; ++i) {
            float sampleL = 0.0f;
            float sampleR = 0.0f;
            processSample(sampleL, sampleR);
            outL[i] = sampleL;
            outR[i] = sampleR;
        }
    }

    // Process interleaved stereo buffer (L, R, L, R, ...)
    void processInterleaved(float* interleaved, size_t numFrames) noexcept {
        if (!interleaved || numFrames == 0) return;

        for (size_t i = 0; i < numFrames; ++i) {
            float sampleL = 0.0f;
            float sampleR = 0.0f;
            processSample(sampleL, sampleR);
            interleaved[i * 2 + 0] = sampleL;
            interleaved[i * 2 + 1] = sampleR;
        }
    }

    // --- Module Accessors ---

    [[nodiscard]] Arpeggiator& getArpeggiator() noexcept { return arpeggiator_; }
    [[nodiscard]] const Arpeggiator& getArpeggiator() const noexcept { return arpeggiator_; }

    [[nodiscard]] StepSequencer& getSequencer() noexcept { return sequencer_; }
    [[nodiscard]] const StepSequencer& getSequencer() const noexcept { return sequencer_; }

    [[nodiscard]] DualEffectsProcessor& getEffects() noexcept { return effects_; }
    [[nodiscard]] const DualEffectsProcessor& getEffects() const noexcept { return effects_; }

    [[nodiscard]] SynthParameters& getParameters() noexcept { return params_; }
    [[nodiscard]] const SynthParameters& getParameters() const noexcept { return params_; }
    void setParameters(const SynthParameters& params) noexcept { params_ = params; }

    [[nodiscard]] VoiceManager& getVoiceManager() noexcept { return voiceManager_; }
    [[nodiscard]] const VoiceManager& getVoiceManager() const noexcept { return voiceManager_; }

    [[nodiscard]] size_t getActiveVoiceCount() const noexcept {
        return voiceManager_.getActiveVoiceCount();
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    SynthParameters params_{};
    VoiceManager voiceManager_{};
    LFO globalLfo_{};
    Distortion distortion_{};
    Arpeggiator arpeggiator_{};
    StepSequencer sequencer_{};
    DualEffectsProcessor effects_{};

    float pitchBendNorm_ = 0.0f;
    float modWheelNorm_ = 0.0f;
};

} // namespace prophet6
