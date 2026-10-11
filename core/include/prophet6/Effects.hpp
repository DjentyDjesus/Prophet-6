#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <memory>

namespace prophet6 {

enum class EffectType : int {
    BBDDelay = 0,
    DigitalDelay = 1,
    TapeDelay = 2,
    Chorus = 3,
    Flanger = 4,
    Phaser = 5,
    RingMod = 6,
    PlateReverb = 7,
    HallReverb = 8,
    Off = 9,
    NumTypes = 10
};

// Fixed circular buffer pre-allocated at initialization
// Zero dynamic runtime allocations on audio thread
template <size_t Capacity>
class DelayBuffer {
public:
    DelayBuffer() : buffer_(std::make_unique<std::array<float, Capacity>>()) {
        reset();
    }

    void reset() noexcept {
        if (buffer_) {
            buffer_->fill(0.0f);
        }
        writeIndex_ = 0;
    }

    void write(float sample) noexcept {
        (*buffer_)[writeIndex_] = sample;
        writeIndex_ = (writeIndex_ + 1) % Capacity;
    }

    [[nodiscard]] float readLinear(float delaySamples) const noexcept {
        if (delaySamples < 0.0f) delaySamples = 0.0f;
        if (delaySamples > static_cast<float>(Capacity - 2)) {
            delaySamples = static_cast<float>(Capacity - 2);
        }

        const auto intPart = static_cast<size_t>(delaySamples);
        const float frac = delaySamples - static_cast<float>(intPart);

        const size_t i0 = (writeIndex_ + Capacity - 1 - intPart) % Capacity;
        const size_t i1 = (i0 + Capacity - 1) % Capacity;

        const float s0 = (*buffer_)[i0];
        const float s1 = (*buffer_)[i1];

        return s0 + frac * (s1 - s0);
    }

    [[nodiscard]] float readCubic(float delaySamples) const noexcept {
        if (delaySamples < 1.0f) delaySamples = 1.0f;
        if (delaySamples > static_cast<float>(Capacity - 4)) {
            delaySamples = static_cast<float>(Capacity - 4);
        }

        const auto intPart = static_cast<size_t>(delaySamples);
        const float frac = delaySamples - static_cast<float>(intPart);

        const size_t i1 = (writeIndex_ + Capacity - 1 - intPart) % Capacity;
        const size_t i0 = (i1 + 1) % Capacity;
        const size_t i2 = (i1 + Capacity - 1) % Capacity;
        const size_t i3 = (i2 + Capacity - 1) % Capacity;

        const float y0 = (*buffer_)[i0];
        const float y1 = (*buffer_)[i1];
        const float y2 = (*buffer_)[i2];
        const float y3 = (*buffer_)[i3];

        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

private:
    std::unique_ptr<std::array<float, Capacity>> buffer_;
    size_t writeIndex_ = 0;
};

// 1. Analog Bucket-Brigade Delay (BBD) Emulation
// Features 1-pole low-pass filtering in feedback loop, smooth non-linear saturation, and dark repeats.
class BBDDelay {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        delayL_.reset();
        delayR_.reset();
        lpfL_ = 0.0f;
        lpfR_ = 0.0f;
        smoothDelayL_ = 0.25f * sampleRate_;
        smoothDelayR_ = 0.26f * sampleRate_;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: Delay time 0.02s to 0.85s
        const float targetTime = 0.02f + 0.83f * clamp(param1, 0.0f, 1.0f);
        const float targetDelayL = targetTime * sampleRate_;
        const float targetDelayR = targetTime * 1.05f * sampleRate_; // Stereo time offset

        // Smooth delay time (models analog clock warping without clicks)
        smoothDelayL_ += 0.001f * (targetDelayL - smoothDelayL_);
        smoothDelayR_ += 0.001f * (targetDelayR - smoothDelayR_);

        const float delOutL = delayL_.readLinear(smoothDelayL_);
        const float delOutR = delayR_.readLinear(smoothDelayR_);

        // param2: Feedback amount (0.0 to 0.94)
        const float fb = clamp(param2, 0.0f, 1.0f) * 0.94f;

        // Feedback 1-pole lowpass filter (~2000 Hz) for murky bucket-brigade high-frequency roll-off
        constexpr float cutoffHz = 2000.0f;
        const float alpha = clamp(TWO_PI * cutoffHz / sampleRate_, 0.01f, 0.85f);
        lpfL_ += alpha * (delOutL - lpfL_);
        lpfR_ += alpha * (delOutR - lpfR_);

        // Bucket-brigade analog saturation curve (Padé fast tanh)
        const float satL = fastTanh(1.25f * lpfL_);
        const float satR = fastTanh(1.25f * lpfR_);

        // Write to circular buffers
        delayL_.write(inL + fb * satL);
        delayR_.write(inR + fb * satR);

        wetL = delOutL;
        wetR = delOutR;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    DelayBuffer<65536> delayL_;
    DelayBuffer<65536> delayR_;
    float lpfL_ = 0.0f;
    float lpfR_ = 0.0f;
    float smoothDelayL_ = 11025.0f;
    float smoothDelayR_ = 11576.0f;
};

// 2. Clean Digital Delay
// Pristine stereo ping-pong / dual-tap delay with cross-feedback and DC blocking.
class DigitalDelay {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        delayL_.reset();
        delayR_.reset();
        dcBlockL_x_ = 0.0f; dcBlockL_y_ = 0.0f;
        dcBlockR_x_ = 0.0f; dcBlockR_y_ = 0.0f;
        smoothDelay_ = 0.25f * sampleRate_;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: Delay time 0.04s to 0.95s
        const float targetTime = 0.04f + 0.91f * clamp(param1, 0.0f, 1.0f);
        const float targetDelay = targetTime * sampleRate_;
        smoothDelay_ += 0.002f * (targetDelay - smoothDelay_);

        // Dotted-eighth ping-pong rhythmic subdivisions
        const float delaySamplesL = smoothDelay_;
        const float delaySamplesR = smoothDelay_ * 0.75f;

        const float outL = delayL_.readLinear(delaySamplesL);
        const float outR = delayR_.readLinear(delaySamplesR);

        // param2: Feedback amount (0.0 to 0.92)
        const float fb = clamp(param2, 0.0f, 1.0f) * 0.92f;

        // DC blocker on cross-feedback loop
        auto dcBlock = [](float in, float& xPrev, float& yPrev) {
            const float y = in - xPrev + 0.995f * yPrev;
            xPrev = in;
            yPrev = y;
            return y;
        };

        const float fbL = dcBlock(outR, dcBlockL_x_, dcBlockL_y_); // Ping-pong cross feedback
        const float fbR = dcBlock(outL, dcBlockR_x_, dcBlockR_y_);

        delayL_.write(inL + fb * fbL);
        delayR_.write(inR + fb * fbR);

        wetL = outL;
        wetR = outR;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    DelayBuffer<65536> delayL_;
    DelayBuffer<65536> delayR_;
    float dcBlockL_x_ = 0.0f, dcBlockL_y_ = 0.0f;
    float dcBlockR_x_ = 0.0f, dcBlockR_y_ = 0.0f;
    float smoothDelay_ = 11025.0f;
};

// 3. Vintage Tape Delay
// Tape read head with compound wow/flutter modulation, tape head frequency damping, and soft tape saturation.
class TapeDelay {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        delayL_.reset();
        delayR_.reset();
        wowPhase_ = 0.0f;
        flutterPhase_ = 0.0f;
        lpfL_ = 0.0f; lpfR_ = 0.0f;
        hpfL_ = 0.0f; hpfR_ = 0.0f;
        smoothDelay_ = 0.28f * sampleRate_;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: Delay time 0.05s to 0.85s
        const float targetTime = 0.05f + 0.80f * clamp(param1, 0.0f, 1.0f);
        const float targetDelay = targetTime * sampleRate_;
        smoothDelay_ += 0.001f * (targetDelay - smoothDelay_);

        // Wow (~0.6 Hz) & Flutter (~8.0 Hz) tape speed modulation
        const float wowSpeed = TWO_PI * 0.6f / sampleRate_;
        const float flutterSpeed = TWO_PI * 8.0f / sampleRate_;
        wowPhase_ += wowSpeed;
        if (wowPhase_ >= TWO_PI) wowPhase_ -= TWO_PI;
        flutterPhase_ += flutterSpeed;
        if (flutterPhase_ >= TWO_PI) flutterPhase_ -= TWO_PI;

        const float wowModL = std::sin(wowPhase_) * (0.0018f * sampleRate_);
        const float flutterModL = (std::sin(flutterPhase_) + 0.35f * std::sin(2.3f * flutterPhase_)) * (0.0005f * sampleRate_);

        // Quadrature phase on Right for organic stereo tape widening
        const float wowModR = std::cos(wowPhase_) * (0.0018f * sampleRate_);
        const float flutterModR = (std::cos(flutterPhase_) + 0.35f * std::cos(2.3f * flutterPhase_)) * (0.0005f * sampleRate_);

        const float delayL = clamp(smoothDelay_ + wowModL + flutterModL, 10.0f, static_cast<float>(65530));
        const float delayR = clamp(smoothDelay_ * 1.03f + wowModR + flutterModR, 10.0f, static_cast<float>(65530));

        const float outL = delayL_.readCubic(delayL);
        const float outR = delayR_.readCubic(delayR);

        // param2: Feedback amount & drive
        const float fb = clamp(param2, 0.0f, 1.0f) * 0.94f;

        // Tape head response: 1-pole low-pass at 4000 Hz, 1-pole high-pass at 80 Hz
        constexpr float lpCutoff = 4000.0f;
        const float lpAlpha = clamp(TWO_PI * lpCutoff / sampleRate_, 0.01f, 0.9f);
        lpfL_ += lpAlpha * (outL - lpfL_);
        lpfR_ += lpAlpha * (outR - lpfR_);

        constexpr float hpCutoff = 80.0f;
        const float hpAlpha = clamp(TWO_PI * hpCutoff / sampleRate_, 0.001f, 0.5f);
        hpfL_ += hpAlpha * (lpfL_ - hpfL_);
        hpfR_ += hpAlpha * (lpfR_ - hpfR_);
        const float shapedL = lpfL_ - hpfL_;
        const float shapedR = lpfR_ - hpfR_;

        // Tape saturation curve: soft asymmetric compression and 2nd harmonic warmth
        auto tapeSat = [](float x) {
            const float d = 1.35f * x;
            return (d / (1.0f + 0.45f * std::abs(d))) + 0.035f * d * d;
        };

        const float satL = tapeSat(shapedL);
        const float satR = tapeSat(shapedR);

        delayL_.write(inL + fb * satL);
        delayR_.write(inR + fb * satR);

        wetL = outL;
        wetR = outR;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    DelayBuffer<65536> delayL_;
    DelayBuffer<65536> delayR_;
    float wowPhase_ = 0.0f;
    float flutterPhase_ = 0.0f;
    float lpfL_ = 0.0f, lpfR_ = 0.0f;
    float hpfL_ = 0.0f, hpfR_ = 0.0f;
    float smoothDelay_ = 12348.0f;
};

// 4. Studio Chorus
// Dual-phase quadrature LFO modulated delay lines for lush 80s vintage stereo ensemble widening.
class Chorus {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        delayL_.reset();
        delayR_.reset();
        lfoPhase_ = 0.0f;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: LFO Rate (0.1 Hz to 5.0 Hz)
        const float rate = 0.1f + 4.9f * clamp(param1, 0.0f, 1.0f);
        lfoPhase_ += TWO_PI * rate / sampleRate_;
        if (lfoPhase_ >= TWO_PI) lfoPhase_ -= TWO_PI;

        // param2: Depth (0.0 to 1.0)
        const float depth = clamp(param2, 0.0f, 1.0f);

        // Base delay 18 ms, modulation +/- 4.5 ms
        const float baseDelay = 0.018f * sampleRate_;
        const float maxMod = 0.0045f * sampleRate_ * depth;

        const float modL = std::sin(lfoPhase_) * maxMod;
        const float modR = std::sin(lfoPhase_ + PI * 0.5f) * maxMod; // Quadrature 90 deg offset

        delayL_.write(inL);
        delayR_.write(inR);

        const float delOutL = delayL_.readLinear(baseDelay + modL);
        const float delOutR = delayR_.readLinear(baseDelay + modR);

        // Cross mix for vintage Prophet/Roland stereo ensemble width
        wetL = 0.85f * delOutL + 0.35f * delOutR;
        wetR = 0.85f * delOutR + 0.35f * delOutL;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    DelayBuffer<4096> delayL_;
    DelayBuffer<4096> delayR_;
    float lfoPhase_ = 0.0f;
};

// 5. Studio Flanger
// Short delay line modulated by triangle LFO with cross-feedback for jet-plane comb filtering.
class Flanger {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        delayL_.reset();
        delayR_.reset();
        lfoPhase_ = 0.0f;
        fbL_ = 0.0f;
        fbR_ = 0.0f;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: LFO Rate (0.05 Hz to 3.5 Hz)
        const float rate = 0.05f + 3.45f * clamp(param1, 0.0f, 1.0f);
        lfoPhase_ += TWO_PI * rate / sampleRate_;
        if (lfoPhase_ >= TWO_PI) lfoPhase_ -= TWO_PI;

        // Triangle LFO
        auto triangleWave = [](float phase) {
            float norm = phase / TWO_PI;
            return 2.0f * std::abs(2.0f * (norm - std::floor(norm + 0.5f))) - 1.0f;
        };

        const float triL = 0.5f * (triangleWave(lfoPhase_) + 1.0f);
        const float triR = 0.5f * (triangleWave(lfoPhase_ + PI) + 1.0f); // Inverted phase

        // Delay sweeps from 0.5 ms to 4.2 ms
        const float minDelay = 0.0005f * sampleRate_;
        const float sweepSpan = 0.0037f * sampleRate_;

        const float delayL = minDelay + triL * sweepSpan;
        const float delayR = minDelay + triR * sweepSpan;

        // param2: Feedback amount (0.0 to 0.88)
        const float fbGain = clamp(param2, 0.0f, 1.0f) * 0.88f;

        delayL_.write(inL + fbGain * fbL_);
        delayR_.write(inR + fbGain * fbR_);

        const float outL = delayL_.readLinear(delayL);
        const float outR = delayR_.readLinear(delayR);

        fbL_ = fastTanh(outL);
        fbR_ = fastTanh(outR);

        wetL = outL;
        wetR = outR;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    DelayBuffer<2048> delayL_;
    DelayBuffer<2048> delayR_;
    float lfoPhase_ = 0.0f;
    float fbL_ = 0.0f;
    float fbR_ = 0.0f;
};

// 6. 6-Stage All-Pass Filter Phaser
// Cascaded 1st-order all-pass ladder modulated by a triangle LFO with resonant feedback.
class Phaser {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        lfoPhase_ = 0.0f;
        for (int i = 0; i < 6; ++i) {
            apL_x_[i] = 0.0f; apL_y_[i] = 0.0f;
            apR_x_[i] = 0.0f; apR_y_[i] = 0.0f;
        }
        feedbackL_ = 0.0f;
        feedbackR_ = 0.0f;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: LFO Rate (0.05 Hz to 4.5 Hz)
        const float rate = 0.05f + 4.45f * clamp(param1, 0.0f, 1.0f);
        lfoPhase_ += TWO_PI * rate / sampleRate_;
        if (lfoPhase_ >= TWO_PI) lfoPhase_ -= TWO_PI;

        auto triWave = [](float phase) {
            float n = phase / TWO_PI;
            return 2.0f * std::abs(2.0f * (n - std::floor(n + 0.5f))) - 1.0f;
        };

        const float modL = 0.5f * (triWave(lfoPhase_) + 1.0f);
        const float modR = 0.5f * (triWave(lfoPhase_ + PI * 0.5f) + 1.0f); // 90 deg stereo phase

        // Frequency sweep range 180 Hz to 3500 Hz (exponential)
        const float freqL = 180.0f * std::pow(19.4f, modL);
        const float freqR = 180.0f * std::pow(19.4f, modR);

        // First-order all-pass coefficient a = (tan(pi*fc/fs) - 1) / (tan(pi*fc/fs) + 1)
        auto calcA = [this](float fc) {
            const float w = std::tan(PI * fc / sampleRate_);
            return (w - 1.0f) / (w + 1.0f);
        };

        const float aL = calcA(freqL);
        const float aR = calcA(freqR);

        // param2: Feedback amount (0.0 to 0.85)
        const float fbGain = clamp(param2, 0.0f, 1.0f) * 0.85f;

        float sigL = inL + fbGain * feedbackL_;
        float sigR = inR + fbGain * feedbackR_;

        // 6-stage cascaded allpass ladder
        for (int i = 0; i < 6; ++i) {
            const float yL = aL * sigL + apL_x_[i] - aL * apL_y_[i];
            apL_x_[i] = sigL;
            apL_y_[i] = yL;
            sigL = yL;

            const float yR = aR * sigR + apR_x_[i] - aR * apR_y_[i];
            apR_x_[i] = sigR;
            apR_y_[i] = yR;
            sigR = yR;
        }

        feedbackL_ = fastTanh(sigL);
        feedbackR_ = fastTanh(sigR);

        wetL = sigL;
        wetR = sigR;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float lfoPhase_ = 0.0f;
    std::array<float, 6> apL_x_{}, apL_y_{};
    std::array<float, 6> apR_x_{}, apR_y_{};
    float feedbackL_ = 0.0f;
    float feedbackR_ = 0.0f;
};

// 7. Ring Modulator
// Multiplies the audio bus against a tunable carrier sine oscillator for clangorous, metallic textures.
class RingModulator {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        carrierPhase_ = 0.0f;
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        // param1: Carrier frequency 20 Hz to 2800 Hz (logarithmic sweep)
        const float carrierFreq = 20.0f * std::pow(140.0f, clamp(param1, 0.0f, 1.0f));
        carrierPhase_ += TWO_PI * carrierFreq / sampleRate_;
        if (carrierPhase_ >= TWO_PI) carrierPhase_ -= TWO_PI;

        // param2: Harmonic overtone drive
        const float tone = clamp(param2, 0.0f, 1.0f);
        const float carL = (1.0f - 0.35f * tone) * std::sin(carrierPhase_) +
                           (0.35f * tone) * std::sin(3.0f * carrierPhase_);
        const float carR = (1.0f - 0.35f * tone) * std::sin(carrierPhase_ + PI * 0.25f) +
                           (0.35f * tone) * std::sin(3.0f * (carrierPhase_ + PI * 0.25f));

        wetL = inL * carL;
        wetR = inR * carR;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float carrierPhase_ = 0.0f;
};

// 8. Plate & Hall Reverb (Dattorro / Schroeder-Moorer Topology)
// Pre-delay -> Input all-pass diffusers -> 4 parallel comb filters with damping -> 2 all-pass diffusers
class SchroederReverb {
public:
    explicit SchroederReverb(bool isHall) : isHall_(isHall) {}

    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept {
        preDelayL_.reset();
        preDelayR_.reset();
        diff1L_.reset(); diff1R_.reset();
        diff2L_.reset(); diff2R_.reset();

        for (auto& c : combsL_) c.reset();
        for (auto& c : combsR_) c.reset();
        combDampL_.fill(0.0f);
        combDampR_.fill(0.0f);

        ap1L_.reset(); ap1R_.reset();
        ap2L_.reset(); ap2R_.reset();
    }

    void process(float inL, float inR, float param1, float param2, float& wetL, float& wetR) noexcept {
        const float scale = sampleRate_ / 44100.0f;
        const float hallMul = isHall_ ? 1.35f : 1.0f;

        // Pre-delay
        const float preDelaySamples = (isHall_ ? 0.035f : 0.010f) * sampleRate_;
        preDelayL_.write(inL);
        preDelayR_.write(inR);
        const float preL = preDelayL_.readLinear(preDelaySamples);
        const float preR = preDelayR_.readLinear(preDelaySamples);

        // Input diffusers (2 series all-pass filters per channel)
        auto processAP = [](float in, DelayBuffer<8192>& buf, float delaySamples, float g) {
            const float delayed = buf.readLinear(delaySamples);
            const float y = -g * in + delayed;
            buf.write(in + g * delayed);
            return y;
        };

        const float d1Len = 142.0f * scale * hallMul;
        const float d2Len = 107.0f * scale * hallMul;
        const float diff1OutL = processAP(preL, diff1L_, d1Len, 0.7f);
        const float diff2OutL = processAP(diff1OutL, diff2L_, d2Len, 0.6f);

        const float diff1OutR = processAP(preR, diff1R_, d1Len * 1.06f, 0.7f);
        const float diff2OutR = processAP(diff1OutR, diff2R_, d2Len * 1.08f, 0.6f);

        // param1: Decay time / RT60 (0.4s to 8.0s)
        const float rt60 = (isHall_ ? 0.8f : 0.4f) + (isHall_ ? 7.5f : 5.0f) * clamp(param1, 0.0f, 1.0f);

        // param2: High frequency damping (0.0 = bright, 1.0 = dark)
        const float dampCoeff = 0.08f + 0.65f * (1.0f - clamp(param2, 0.0f, 1.0f));

        // Mutually prime comb delays
        constexpr std::array<float, 4> baseCombsL = { 1116.0f, 1356.0f, 1422.0f, 1617.0f };
        constexpr std::array<float, 4> baseCombsR = { 1188.0f, 1277.0f, 1491.0f, 1657.0f };

        float combSumL = 0.0f;
        float combSumR = 0.0f;

        for (size_t i = 0; i < 4; ++i) {
            const float delayL = baseCombsL[i] * scale * hallMul;
            const float delayR = baseCombsR[i] * scale * hallMul;

            // Feedback decay gain: g = 10^(-3 * delay / (RT60 * fs))
            const float fbGainL = clamp(std::pow(10.0f, -3.0f * delayL / (rt60 * sampleRate_)), 0.0f, 0.97f);
            const float fbGainR = clamp(std::pow(10.0f, -3.0f * delayR / (rt60 * sampleRate_)), 0.0f, 0.97f);

            const float cOutL = combsL_[i].readLinear(delayL);
            const float cOutR = combsR_[i].readLinear(delayR);

            // 1-pole damping filter in comb loop
            combDampL_[i] = (1.0f - dampCoeff) * cOutL + dampCoeff * combDampL_[i];
            combDampR_[i] = (1.0f - dampCoeff) * cOutR + dampCoeff * combDampR_[i];

            combsL_[i].write(diff2OutL + fbGainL * combDampL_[i]);
            combsR_[i].write(diff2OutR + fbGainR * combDampR_[i]);

            combSumL += cOutL;
            combSumR += cOutR;
        }

        // Output all-pass diffusers (2 series per channel)
        const float ap1LenL = 225.0f * scale * hallMul;
        const float ap2LenL = 341.0f * scale * hallMul;
        const float ap1LenR = 251.0f * scale * hallMul;
        const float ap2LenR = 317.0f * scale * hallMul;

        const float ap1OutL = processAP(combSumL * 0.25f, ap1L_, ap1LenL, 0.5f);
        wetL = processAP(ap1OutL, ap2L_, ap2LenL, 0.5f);

        const float ap1OutR = processAP(combSumR * 0.25f, ap1R_, ap1LenR, 0.5f);
        wetR = processAP(ap1OutR, ap2R_, ap2LenR, 0.5f);
    }

private:
    bool isHall_ = false;
    float sampleRate_ = DEFAULT_SAMPLE_RATE;

    DelayBuffer<8192> preDelayL_, preDelayR_;
    DelayBuffer<8192> diff1L_, diff1R_;
    DelayBuffer<8192> diff2L_, diff2R_;

    std::array<DelayBuffer<8192>, 4> combsL_;
    std::array<DelayBuffer<8192>, 4> combsR_;
    std::array<float, 4> combDampL_{};
    std::array<float, 4> combDampR_{};

    DelayBuffer<8192> ap1L_, ap1R_;
    DelayBuffer<8192> ap2L_, ap2R_;
};

class PlateReverb : public SchroederReverb {
public:
    PlateReverb() : SchroederReverb(false) {}
};

class HallReverb : public SchroederReverb {
public:
    HallReverb() : SchroederReverb(true) {}
};

// Single Effect Slot Container
class EffectSlot {
public:
    void init(float sampleRate) noexcept {
        sampleRate_ = sampleRate;
        bbdDelay_.init(sampleRate);
        digitalDelay_.init(sampleRate);
        tapeDelay_.init(sampleRate);
        chorus_.init(sampleRate);
        flanger_.init(sampleRate);
        phaser_.init(sampleRate);
        ringMod_.init(sampleRate);
        plateReverb_.init(sampleRate);
        hallReverb_.init(sampleRate);
        reset();
    }

    void reset() noexcept {
        bbdDelay_.reset();
        digitalDelay_.reset();
        tapeDelay_.reset();
        chorus_.reset();
        flanger_.reset();
        phaser_.reset();
        ringMod_.reset();
        plateReverb_.reset();
        hallReverb_.reset();
    }

    void process(float inL, float inR, int typeInt, float param1, float param2, float mix, float& outL, float& outR) noexcept {
        if (mix <= 0.0001f || typeInt >= static_cast<int>(EffectType::Off)) {
            outL = inL;
            outR = inR;
            return;
        }

        float wetL = 0.0f;
        float wetR = 0.0f;

        const auto type = static_cast<EffectType>(typeInt);
        switch (type) {
            case EffectType::BBDDelay:
                bbdDelay_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::DigitalDelay:
                digitalDelay_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::TapeDelay:
                tapeDelay_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::Chorus:
                chorus_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::Flanger:
                flanger_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::Phaser:
                phaser_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::RingMod:
                ringMod_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::PlateReverb:
                plateReverb_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::HallReverb:
                hallReverb_.process(inL, inR, param1, param2, wetL, wetR);
                break;
            case EffectType::Off:
            default:
                outL = inL;
                outR = inR;
                return;
        }

        // Equal-power crossfade
        const float clampedMix = clamp(mix, 0.0f, 1.0f);
        const float dryGain = std::cos(clampedMix * (PI * 0.5f));
        const float wetGain = std::sin(clampedMix * (PI * 0.5f));

        outL = inL * dryGain + wetL * wetGain;
        outR = inR * dryGain + wetR * wetGain;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    BBDDelay bbdDelay_;
    DigitalDelay digitalDelay_;
    TapeDelay tapeDelay_;
    Chorus chorus_;
    Flanger flanger_;
    Phaser phaser_;
    RingModulator ringMod_;
    PlateReverb plateReverb_;
    HallReverb hallReverb_;
};

// Top-Level Dual Digital Stereo Effects Engine with True Bypass
// Two serial effect slots: Slot 1 (FX A) -> Slot 2 (FX B)
class DualEffectsProcessor {
public:
    DualEffectsProcessor() noexcept = default;

    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        slot1_.init(sampleRate);
        slot2_.init(sampleRate);
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            slot1_.init(sampleRate);
            slot2_.init(sampleRate);
        }
    }

    void reset() noexcept {
        slot1_.reset();
        slot2_.reset();
    }

    // Process a single stereo sample frame through serial dual effects
    void processSample(float& left, float& right, const SynthParameters& params) noexcept {
        if (!params.fxOn) return; // True Bypass: untouched dry signal

        float midL = left;
        float midR = right;

        // Slot 1 (FX A)
        slot1_.process(left, right, params.fx1Type, params.fx1Param1, params.fx1Param2, params.fx1Mix, midL, midR);

        // Slot 2 (FX B)
        slot2_.process(midL, midR, params.fx2Type, params.fx2Param1, params.fx2Param2, params.fx2Mix, left, right);
    }

    // Process a block of audio samples
    void process(float* left, float* right, size_t numSamples, const SynthParameters& params) noexcept {
        if (!params.fxOn || !left || !right || numSamples == 0) return; // True Bypass

        for (size_t i = 0; i < numSamples; ++i) {
            processSample(left[i], right[i], params);
        }
    }

    [[nodiscard]] EffectSlot& getSlot1() noexcept { return slot1_; }
    [[nodiscard]] const EffectSlot& getSlot1() const noexcept { return slot1_; }

    [[nodiscard]] EffectSlot& getSlot2() noexcept { return slot2_; }
    [[nodiscard]] const EffectSlot& getSlot2() const noexcept { return slot2_; }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    EffectSlot slot1_;
    EffectSlot slot2_;
};

} // namespace prophet6
