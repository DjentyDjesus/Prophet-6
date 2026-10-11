#pragma once

#include "Common.hpp"
#include <cmath>
#include <algorithm>

namespace prophet6 {

// 2-Pole (12 dB/oct) Zero-Delay Feedback (ZDF) High-Pass Filter
// Designed as an analog State Variable Filter (SVF) topology in series after the LPF.
// Provides simultaneous low-end control and resonant formant sculpting.
class HighPassFilter {
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
        s1_ = 0.0f;
        s2_ = 0.0f;
    }

    // Process a single audio sample
    // cutoffHz: 10 Hz to Nyquist
    // resonance: 0.0 (smooth 12dB high-pass) to 1.0 (sharp resonance peak)
    [[nodiscard]] float process(float input, float cutoffHz, float resonance) noexcept {
        const float maxCutoff = 0.485f * sampleRate_;
        const float clampedCutoff = clamp(cutoffHz, 10.0f, maxCutoff);
        const float clampedRes = clamp(resonance, 0.0f, 1.0f);

        // Bilinear pre-warping
        const float wa = clampedCutoff * invSampleRate_;
        const float g = std::tan(PI * wa);

        // Damping parameter 2R
        // Resonance ranges from Q = 0.707 (Butterworth) to Q = 25 (resonant ringing)
        const float twoR = 2.0f * (1.0f - 0.96f * clampedRes);

        // SVF denominator: 1 + 2R*g + g^2
        const float h = 1.0f / (1.0f + twoR * g + g * g);

        // Solve ZDF high-pass equation with analog saturation on resonant feedback
        const float satS1 = fastTanh(s1_);
        const float yHP = h * (input - (twoR + g) * satS1 - s2_);
        const float yBP = g * yHP + satS1;
        const float yLP = g * yBP + s2_;

        // Update trapezoidal states: s = 2 * y - s
        s1_ = clamp(2.0f * yBP - s1_, -8.0f, 8.0f);
        s2_ = clamp(2.0f * yLP - s2_, -8.0f, 8.0f);

        return yHP;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;

    float s1_ = 0.0f;
    float s2_ = 0.0f;
};

} // namespace prophet6
