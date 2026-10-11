#pragma once

#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>
#include <numbers>

namespace prophet6 {

// Mathematical constants
inline constexpr float PI = std::numbers::pi_v<float>;
inline constexpr float TWO_PI = 2.0f * PI;
inline constexpr float DEFAULT_SAMPLE_RATE = 44100.0f;

// Utility functions
template <typename T>
[[nodiscard]] constexpr T clamp(T val, T minVal, T maxVal) noexcept {
    return (val < minVal) ? minVal : ((val > maxVal) ? maxVal : val);
}

// Convert MIDI note number (can be fractional for detune) to Frequency in Hz
[[nodiscard]] inline float midiNoteToFrequency(float note) noexcept {
    return 440.0f * std::pow(2.0f, (note - 69.0f) * (1.0f / 12.0f));
}

// Convert semitones to frequency multiplier ratio
[[nodiscard]] inline float semitonesToRatio(float semitones) noexcept {
    return std::pow(2.0f, semitones * (1.0f / 12.0f));
}

// Decibel conversions
[[nodiscard]] inline float dbToGain(float db) noexcept {
    return std::pow(10.0f, db * 0.05f);
}

[[nodiscard]] inline float gainToDb(float gain) noexcept {
    return (gain > 1e-5f) ? (20.0f * std::log10(gain)) : -100.0f;
}

// High-speed, C1-continuous Padé [3/2] hyperbolic tangent approximation
// Exactly reaches +/- 1.0f at +/- 3.0f with zero derivative discontinuity.
[[nodiscard]] inline float fastTanh(float x) noexcept {
    if (x <= -3.0f) return -1.0f;
    if (x >= 3.0f) return 1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// PolyBLEP (Polynomial Band-Limited Step) residual
// Suppresses high-frequency aliasing reflections around discontinuities.
[[nodiscard]] inline float polyBlep(float t, float dt) noexcept {
    if (dt <= 0.0f) return 0.0f;
    if (t < dt) {
        const float v = t / dt;
        return 2.0f * v - v * v - 1.0f;
    } else if (t > 1.0f - dt) {
        const float v = (t - 1.0f) / dt;
        return v * v + 2.0f * v + 1.0f;
    }
    return 0.0f;
}

// PolyBLAMP (Polynomial Band-Limited Ramp) residual
// Suppresses slope-derivative aliasing (for triangle corners)
[[nodiscard]] inline float polyBlamp(float t, float dt) noexcept {
    if (dt <= 0.0f) return 0.0f;
    if (t < dt) {
        const float v = t / dt - 1.0f;
        return -dt * (1.0f / 3.0f) * v * v * v;
    } else if (t > 1.0f - dt) {
        const float v = (t - 1.0f) / dt + 1.0f;
        return dt * (1.0f / 3.0f) * v * v * v;
    }
    return 0.0f;
}

// Per-Voice Slop Generator
// Models vintage analog component tolerances and low-frequency thermal drift.
class SlopGenerator {
public:
    void init(uint32_t voiceIndex, float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        voiceIndex_ = voiceIndex % 6;
        sampleRate_ = sampleRate;

        // Discrete static tolerance deviations per voice card
        // Pitch: +/- 4.5 cents spread
        constexpr std::array<float, 6> staticPitchCents = {
            -3.8f, 2.7f, -1.9f, 4.2f, -2.5f, 3.3f
        };
        // Filter Cutoff: +/- 3.5% deviation
        constexpr std::array<float, 6> staticCutoffTol = {
            -0.028f, 0.021f, -0.015f, 0.034f, 0.018f, -0.031f
        };
        // Resonance tolerance: +/- 2.5%
        constexpr std::array<float, 6> staticResTol = {
            0.015f, -0.022f, 0.019f, -0.012f, 0.025f, -0.018f
        };

        basePitchCents_ = staticPitchCents[voiceIndex_];
        baseCutoffTol_  = staticCutoffTol[voiceIndex_];
        baseResTol_     = staticResTol[voiceIndex_];

        // Unique prime-like sub-audio drift phase offsets
        phase1_ = static_cast<float>(voiceIndex_ * 1.047f);
        phase2_ = static_cast<float>(voiceIndex_ * 2.391f);
        phase3_ = static_cast<float>(voiceIndex_ * 0.732f);

        // Frequencies in Hz: ~0.08 Hz, ~0.17 Hz, ~0.29 Hz (ultra slow thermal drift)
        inc1_ = TWO_PI * (0.073f + 0.011f * voiceIndex_) / sampleRate_;
        inc2_ = TWO_PI * (0.161f + 0.017f * voiceIndex_) / sampleRate_;
        inc3_ = TWO_PI * (0.283f + 0.023f * voiceIndex_) / sampleRate_;
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            inc1_ = TWO_PI * (0.073f + 0.011f * voiceIndex_) / sampleRate_;
            inc2_ = TWO_PI * (0.161f + 0.017f * voiceIndex_) / sampleRate_;
            inc3_ = TWO_PI * (0.283f + 0.023f * voiceIndex_) / sampleRate_;
        }
    }

    void advance() noexcept {
        phase1_ += inc1_;
        if (phase1_ >= TWO_PI) phase1_ -= TWO_PI;

        phase2_ += inc2_;
        if (phase2_ >= TWO_PI) phase2_ -= TWO_PI;

        phase3_ += inc3_;
        if (phase3_ >= TWO_PI) phase3_ -= TWO_PI;

        // Smooth continuous drift value in range [-1.0, 1.0]
        thermalDrift_ = 0.50f * std::sin(phase1_) +
                        0.35f * std::sin(phase2_) +
                        0.15f * std::sin(phase3_);
    }

    // Pitch drift in semitones (1 cent = 0.01 semitone)
    [[nodiscard]] float getPitchDriftSemitones(float slopAmount) const noexcept {
        if (slopAmount <= 0.0f) return 0.0f;
        // Total drift up to +/- 8 cents at 100% slop
        const float totalCents = (basePitchCents_ * 0.7f + thermalDrift_ * 4.5f) * slopAmount;
        return totalCents * 0.01f;
    }

    // Filter cutoff drift multiplier (e.g. 0.96 to 1.04)
    [[nodiscard]] float getCutoffDriftMultiplier(float slopAmount) const noexcept {
        if (slopAmount <= 0.0f) return 1.0f;
        const float delta = (baseCutoffTol_ * 0.6f + thermalDrift_ * 0.035f) * slopAmount;
        return 1.0f + delta;
    }

    // Resonance tolerance drift offset
    [[nodiscard]] float getResonanceOffset(float slopAmount) const noexcept {
        if (slopAmount <= 0.0f) return 0.0f;
        return (baseResTol_ * 0.6f + thermalDrift_ * 0.02f) * slopAmount;
    }

private:
    uint32_t voiceIndex_ = 0;
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float basePitchCents_ = 0.0f;
    float baseCutoffTol_  = 0.0f;
    float baseResTol_     = 0.0f;

    float phase1_ = 0.0f;
    float phase2_ = 0.0f;
    float phase3_ = 0.0f;
    float inc1_ = 0.0f;
    float inc2_ = 0.0f;
    float inc3_ = 0.0f;
    float thermalDrift_ = 0.0f;
};

} // namespace prophet6
