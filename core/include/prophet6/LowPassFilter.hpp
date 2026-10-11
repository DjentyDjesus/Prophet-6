#pragma once

#include "Common.hpp"
#include <cmath>
#include <algorithm>

namespace prophet6 {

// 4-Pole (24 dB/oct) Zero-Delay Feedback (ZDF) Low-Pass Filter
// Inspired by the discrete SSM 2040 / Curtis 4-pole transistor ladder topology.
// Features self-oscillating resonance, non-linear differential saturation, and bilinear pre-warping.
class LowPassFilter {
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
        s3_ = 0.0f;
        s4_ = 0.0f;
    }

    // Process a single audio sample through the 4-pole ZDF ladder
    // cutoffHz: 10 Hz to Nyquist
    // resonance: 0.0 (unfiltered/flat) to 1.0 (self-oscillation)
    [[nodiscard]] float process(float input, float cutoffHz, float resonance) noexcept {
        // Safe cutoff bounds with pre-warping
        const float maxCutoff = 0.485f * sampleRate_;
        const float clampedCutoff = clamp(cutoffHz, 10.0f, maxCutoff);
        const float clampedRes = clamp(resonance, 0.0f, 1.0f);

        // Bilinear pre-warping: g = tan(pi * fc / fs)
        const float wa = clampedCutoff * invSampleRate_;
        const float g = std::tan(PI * wa);
        const float G = g / (1.0f + g);
        const float inv1PlusG = 1.0f / (1.0f + g);

        // Feedback resonance factor k (SSM/Curtis self-oscillates at k ~ 4.0)
        // Scaled up to 4.15 for intense self-oscillation ringing
        const float k = clampedRes * 4.15f;

        // Predict instant response of 4 cascaded 1-pole stages
        const float S1 = s1_ * inv1PlusG;
        const float S2 = s2_ * inv1PlusG;
        const float S3 = s3_ * inv1PlusG;
        const float S4 = s4_ * inv1PlusG;

        const float G2 = G * G;
        const float G3 = G2 * G;
        const float G4 = G3 * G;

        const float S = G3 * S1 + G2 * S2 + G * S3 + S4;

        // Vintage bass compensation when resonance is boosted
        const float drivenInput = input * (1.0f + 0.35f * clampedRes);

        // Non-linear differential amplifier drive:
        // Solves zero-delay feedback loop with soft tanh saturation
        const float denominator = 1.0f + k * G4;
        const float linearFeedback = (drivenInput - k * S) / denominator;
        const float u = fastTanh(linearFeedback);

        // Cascade of four 1-pole low-pass stages
        const float y1 = G * u  + S1;
        const float y2 = G * y1 + S2;
        const float y3 = G * y2 + S3;
        const float y4 = G * y3 + S4;

        // Trapezoidal integrator state updates (2 * y - s)
        // With mild saturation to keep poles bounded during audio-rate FM
        s1_ = clamp(2.0f * y1 - s1_, -8.0f, 8.0f);
        s2_ = clamp(2.0f * y2 - s2_, -8.0f, 8.0f);
        s3_ = clamp(2.0f * y3 - s3_, -8.0f, 8.0f);
        s4_ = clamp(2.0f * y4 - s4_, -8.0f, 8.0f);

        return y4;
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;

    // Four 1-pole integrator states
    float s1_ = 0.0f;
    float s2_ = 0.0f;
    float s3_ = 0.0f;
    float s4_ = 0.0f;
};

} // namespace prophet6
