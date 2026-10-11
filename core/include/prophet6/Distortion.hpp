#pragma once

#include "Common.hpp"
#include <cmath>
#include <algorithm>

namespace prophet6 {

// Master Stereo Analog Distortion Unit
// Models Dave Smith's hardware master bus analog distortion circuit:
// soft asymmetric saturation, odd + even harmonics, and op-amp analog warmth.
class Distortion {
public:
    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        invSampleRate_ = 1.0f / sampleRate;
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            invSampleRate_ = 1.0f / sampleRate;
            reset();
        }
    }

    void reset() noexcept {
        filterStateL_ = 0.0f;
        filterStateR_ = 0.0f;
    }

    // Process a stereo frame in-place
    // driveAmount: 0.0 (transparent/clean) to 1.0 (heavy analog drive)
    void process(float& left, float& right, float driveAmount) noexcept {
        if (driveAmount <= 0.001f) {
            return;
        }

        const float drive = clamp(driveAmount, 0.0f, 1.0f);

        // Pre-gain: up to +22 dB drive gain
        const float preGain = 1.0f + drive * 8.5f;

        // Post-gain compensation to prevent clipping downstream
        const float postGain = 1.0f / std::sqrt(1.0f + drive * 5.0f);

        left  = processChannel(left,  drive, preGain, postGain, filterStateL_);
        right = processChannel(right, drive, preGain, postGain, filterStateR_);
    }

private:
    [[nodiscard]] float processChannel(
        float input,
        float drive,
        float preGain,
        float postGain,
        float& filterState) noexcept
    {
        // 1. Asymmetric even-order harmonic injection
        const float asymmetric = input + 0.12f * drive * (input * input);

        // 2. Driven input
        const float driven = asymmetric * preGain;

        // 3. Smooth analog saturation curve (Padé tanh)
        const float saturated = fastTanh(driven) * postGain;

        // 4. Subtle 1-pole high-frequency rolloff (~16 kHz) modeling transformer smoothing
        constexpr float cutoffHz = 16000.0f;
        const float alpha = clamp(cutoffHz * invSampleRate_ * TWO_PI, 0.01f, 0.99f);
        filterState += alpha * (saturated - filterState);

        // 5. Dry/wet crossfade based on drive control
        return (1.0f - drive) * input + drive * filterState;
    }

    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;
    float filterStateL_ = 0.0f;
    float filterStateR_ = 0.0f;
};

} // namespace prophet6
