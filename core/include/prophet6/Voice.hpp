#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include "Oscillator.hpp"
#include "LowPassFilter.hpp"
#include "HighPassFilter.hpp"
#include "PolyMod.hpp"
#include "Envelope.hpp"
#include <cmath>

namespace prophet6 {

// Single discrete polyphonic voice container
class Voice {
public:
    void init(uint32_t voiceIndex, float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        voiceIndex_ = voiceIndex;
        sampleRate_ = sampleRate;
        invSampleRate_ = 1.0f / sampleRate;

        slop_.init(voiceIndex, sampleRate);
        osc1_.init(sampleRate);
        subOsc_.init(sampleRate);
        osc2_.init(sampleRate);
        noise_.seed(123456789 + voiceIndex * 98765);
        lpf_.init(sampleRate);
        hpf_.init(sampleRate);
        filterEnv_.init(sampleRate);
        ampEnv_.init(sampleRate);

        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            invSampleRate_ = 1.0f / sampleRate;

            slop_.setSampleRate(sampleRate);
            osc1_.setSampleRate(sampleRate);
            subOsc_.setSampleRate(sampleRate);
            osc2_.setSampleRate(sampleRate);
            lpf_.setSampleRate(sampleRate);
            hpf_.setSampleRate(sampleRate);
            filterEnv_.setSampleRate(sampleRate);
            ampEnv_.setSampleRate(sampleRate);
        }
    }

    void reset() noexcept {
        currentMidiNote_ = 60.0f;
        targetMidiNote_ = 60.0f;
        currentPitch_ = 60.0f;
        velocity_ = 0.0f;
        active_ = false;
        noteNumber_ = -1;
        age_ = 0;

        osc2LastOutput_ = 0.0f;
        fadeGain_ = 1.0f;
        isFadingForSteal_ = false;
        pendingNoteNumber_ = -1;
        pendingVelocity_ = 0.0f;

        osc1_.resetPhase();
        subOsc_.resetPhase();
        osc2_.resetPhase();
        lpf_.reset();
        hpf_.reset();
        filterEnv_.reset();
        ampEnv_.reset();
    }

    // Trigger Note On (accepts fractional MIDI note for unison detune)
    void noteOn(float noteNumber, float velocity, float velSens, bool immediate = false) noexcept {
        if (!immediate && active_ && ampEnv_.getLevel() > 0.01f) {
            // Initiate 5ms click-free micro-fade before stealing/retriggering
            isFadingForSteal_ = true;
            pendingNoteNumber_ = noteNumber;
            pendingVelocity_ = velocity;
            pendingVelSens_ = velSens;
            return;
        }

        executeNoteOn(noteNumber, velocity, velSens);
    }

    // Trigger Note Off
    void noteOff(int noteNumber) noexcept {
        if (noteNumber_ >= 0 && std::lround(noteNumber_) == noteNumber) {
            filterEnv_.noteOff();
            ampEnv_.noteOff();
        }
    }

    // Force immediate release
    void release() noexcept {
        filterEnv_.noteOff();
        ampEnv_.noteOff();
    }

    // Render a single audio sample from this voice
    // Adds stereo contribution to outL and outR
    void renderSample(
        const SynthParameters& params,
        float globalLfo,
        float pitchBendSemitones,
        float& outL,
        float& outR) noexcept
    {
        if (!active_) {
            return;
        }

        // 1. Advance age for oldest-voice stealing logic
        ++age_;

        // 2. Handle micro-fade for click-free voice stealing
        if (isFadingForSteal_) {
            // ~5ms fade-out rate: 1.0 / (0.005 * sampleRate)
            const float fadeStep = invSampleRate_ / 0.005f;
            fadeGain_ -= fadeStep;
            if (fadeGain_ <= 0.0f) {
                fadeGain_ = 0.0f;
                isFadingForSteal_ = false;
                executeNoteOn(pendingNoteNumber_, pendingVelocity_, pendingVelSens_);
                fadeGain_ = 1.0f;
            }
        }

        // 3. Advance Slop thermal drift
        slop_.advance();

        // 4. Portamento / Pitch Glide
        if (params.glide > 0.001f) {
            const float glideCoeff = 1.0f - std::exp(-invSampleRate_ / params.glide);
            currentPitch_ += glideCoeff * (targetMidiNote_ - currentPitch_);
        } else {
            currentPitch_ = targetMidiNote_;
        }

        // 5. Envelopes
        const float filtEnv = filterEnv_.process(
            params.filtAttack, params.filtDecay, params.filtSustain, params.filtRelease);
        const float ampEnv = ampEnv_.process(
            params.ampAttack, params.ampDecay, params.ampSustain, params.ampRelease);

        // Check if voice has completely finished its release
        if (ampEnv_.isIdle() && !isFadingForSteal_) {
            active_ = false;
            noteNumber_ = -1;
            return;
        }

        // 6. Base Pitch Calculation
        const float slopPitch = slop_.getPitchDriftSemitones(params.slop);
        const float basePitch = currentPitch_ + pitchBendSemitones + slopPitch;

        // LFO Pitch modulation
        float lfoPitchOsc1 = 0.0f;
        float lfoPitchOsc2 = 0.0f;
        if (params.lfoDestOsc1Freq) {
            lfoPitchOsc1 = globalLfo * params.lfoInitialAmount * 12.0f;
        }
        if (params.lfoDestOsc2Freq) {
            lfoPitchOsc2 = globalLfo * params.lfoInitialAmount * 12.0f;
        }

        // Oscillator 2 Pitch & Low Frequency Mode
        float osc2Pitch = basePitch + static_cast<float>(params.osc2Octave) * 12.0f +
                          params.osc2FineTune + lfoPitchOsc2;
        float osc2FreqHz = 0.0f;
        if (params.osc2LowFreq) {
            // Low Frequency mode (0.05 Hz to 20 Hz)
            const float lowFreqBase = 1.0f; // 1 Hz base
            if (params.osc2KbdTrack) {
                osc2FreqHz = lowFreqBase * semitonesToRatio(currentPitch_ - 60.0f);
            } else {
                osc2FreqHz = lowFreqBase;
            }
            osc2FreqHz = clamp(osc2FreqHz * semitonesToRatio(params.osc2FineTune), 0.05f, 20.0f);
        } else {
            if (!params.osc2KbdTrack) {
                // Fixed pitch if keyboard tracking is off
                osc2Pitch = 60.0f + static_cast<float>(params.osc2Octave) * 12.0f +
                            params.osc2FineTune + lfoPitchOsc2;
            }
            osc2FreqHz = midiNoteToFrequency(osc2Pitch);
        }

        // Advance Oscillator 2 first (supplies audio sample to PolyMod)
        const bool osc2Wrapped = osc2_.advance(osc2FreqHz);
        const float osc2Audio = osc2_.render(params.osc2Shape, params.osc2PulseWidth);
        osc2LastOutput_ = osc2Audio;

        // 7. PolyMod Computation (audio-rate cross-modulation matrix)
        const PolyModModulation polyMod = polyMod_.process(
            filtEnv,
            osc2Audio,
            params.polyModFiltEnvAmt,
            params.polyModOsc2Amt,
            params.polyModDestOsc1Freq,
            params.polyModDestOsc1Shape,
            params.polyModDestOsc1PW,
            params.polyModDestLPF,
            params.polyModDestHPF
        );

        // 8. Oscillator 1 Modulation & Advance
        const float osc1Pitch = basePitch + static_cast<float>(params.osc1Octave) * 12.0f +
                                lfoPitchOsc1 + polyMod.osc1FreqSemitones;
        const float osc1FreqHz = midiNoteToFrequency(osc1Pitch);

        // Hard Sync: Osc 1 is slave to Osc 2
        if (params.osc1Sync && osc2Wrapped) {
            osc1_.hardSync(0.0f);
        }

        osc1_.advance(osc1FreqHz);
        subOsc_.advance(osc1FreqHz);

        // Dynamic waveshape & PW with PolyMod + LFO modulation
        float osc1Shape = clamp(params.osc1Shape + polyMod.osc1ShapeDelta, 0.0f, 1.0f);
        float osc1PW = params.osc1PulseWidth + polyMod.osc1PWDelta;
        if (params.lfoDestOsc1PW) {
            osc1PW += globalLfo * params.lfoInitialAmount * 0.4f;
        }
        osc1PW = clamp(osc1PW, 0.02f, 0.98f);

        // Render Osc 1, Sub-Osc, and Noise
        const float osc1Sample = osc1_.render(osc1Shape, osc1PW);
        const float subSample  = subOsc_.render();
        const float noiseSample = noise_.process();

        // 9. Audio Mixer
        const float mixed = osc1Sample  * params.osc1Level +
                            subSample   * params.osc1SubLevel +
                            osc2Audio   * params.osc2Level +
                            noiseSample * params.noiseLevel;

        // 10. 4-Pole ZDF Low-Pass Filter
        float lpfCutoffHz = params.lpfCutoff * slop_.getCutoffDriftMultiplier(params.slop);
        // Keyboard tracking
        lpfCutoffHz *= semitonesToRatio((currentPitch_ - 60.0f) * params.lpfKbdTrack);
        // Envelope modulation (up to 5 octaves)
        lpfCutoffHz *= std::pow(2.0f, filtEnv * params.lpfEnvAmount * 5.0f);
        // PolyMod modulation
        lpfCutoffHz *= std::pow(2.0f, polyMod.lpfCutoffOctaves);
        // LFO cutoff modulation
        if (params.lfoDestLPF) {
            lpfCutoffHz *= std::pow(2.0f, globalLfo * params.lfoInitialAmount * 4.0f);
        }
        const float lpfRes = clamp(params.lpfResonance + slop_.getResonanceOffset(params.slop), 0.0f, 1.0f);
        const float lpfOut = lpf_.process(mixed, lpfCutoffHz, lpfRes);

        // 11. 2-Pole ZDF High-Pass Filter (in series after LPF)
        float hpfCutoffHz = params.hpfCutoff;
        hpfCutoffHz *= semitonesToRatio((currentPitch_ - 60.0f) * params.hpfKbdTrack);
        hpfCutoffHz *= std::pow(2.0f, filtEnv * params.hpfEnvAmount * 5.0f);
        hpfCutoffHz *= std::pow(2.0f, polyMod.hpfCutoffOctaves);
        if (params.lfoDestHPF) {
            hpfCutoffHz *= std::pow(2.0f, globalLfo * params.lfoInitialAmount * 4.0f);
        }
        const float hpfOut = hpf_.process(lpfOut, hpfCutoffHz, params.hpfResonance);

        // 12. Amplifier / VCA
        float vcaGain = ampEnv;
        if (params.lfoDestAmp) {
            vcaGain *= clamp(1.0f + globalLfo * params.lfoInitialAmount * 0.5f, 0.0f, 1.5f);
        }
        const float voiceAudio = hpfOut * vcaGain * fadeGain_;

        // 13. Stereo Pan Spread
        // Voice spread: alternating L/R across the 6 voices
        // Voice indices: 0 = -1.0, 1 = +1.0, 2 = -0.6, 3 = +0.6, 4 = -0.2, 5 = +0.2
        constexpr std::array<float, 6> panPositions = {
            -0.85f, 0.85f, -0.50f, 0.50f, -0.20f, 0.20f
        };
        const float spreadPan = panPositions[voiceIndex_ % 6] * params.panSpread;
        // Equal power panning curve: theta in [0, pi/2]
        const float theta = (spreadPan + 1.0f) * (PI * 0.25f);
        const float panGainL = std::cos(theta);
        const float panGainR = std::sin(theta);

        outL += voiceAudio * panGainL;
        outR += voiceAudio * panGainR;
    }

    [[nodiscard]] bool isActive() const noexcept { return active_; }
    [[nodiscard]] int getNoteNumber() const noexcept { return static_cast<int>(std::lround(noteNumber_)); }
    [[nodiscard]] uint64_t getAge() const noexcept { return age_; }
    [[nodiscard]] float getAmpLevel() const noexcept { return ampEnv_.getLevel(); }
    [[nodiscard]] bool isReleasing() const noexcept { return ampEnv_.isRelease(); }

private:
    void executeNoteOn(float noteNumber, float velocity, float velSens) noexcept {
        noteNumber_ = noteNumber;
        velocity_ = velocity;
        targetMidiNote_ = noteNumber;
        currentPitch_ = targetMidiNote_;
        active_ = true;
        age_ = 0;
        fadeGain_ = 1.0f;
        isFadingForSteal_ = false;

        osc1_.resetPhase();
        subOsc_.resetPhase();
        osc2_.resetPhase();
        filterEnv_.noteOn(velocity, velSens);
        ampEnv_.noteOn(velocity, velSens);
    }

    uint32_t voiceIndex_ = 0;
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;

    float noteNumber_ = -1.0f;
    float velocity_ = 0.0f;
    float currentMidiNote_ = 60.0f;
    float targetMidiNote_ = 60.0f;
    float currentPitch_ = 60.0f;
    bool active_ = false;
    uint64_t age_ = 0;

    // Click-free micro-fade state
    float fadeGain_ = 1.0f;
    bool isFadingForSteal_ = false;
    float pendingNoteNumber_ = -1.0f;
    float pendingVelocity_ = 0.0f;
    float pendingVelSens_ = 0.0f;

    float osc2LastOutput_ = 0.0f;

    // Voice Modules
    SlopGenerator  slop_;
    Oscillator     osc1_;
    SubOscillator  subOsc_;
    Oscillator     osc2_;
    NoiseGenerator noise_;
    LowPassFilter  lpf_;
    HighPassFilter hpf_;
    PolyMod        polyMod_;
    Envelope       filterEnv_;
    Envelope       ampEnv_;
};

} // namespace prophet6
