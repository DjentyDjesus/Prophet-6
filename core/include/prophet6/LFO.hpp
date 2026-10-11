#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include <cmath>

namespace prophet6 {

// Multi-waveform polyphonic/global LFO with Sample & Hold
// Covers 0.05 Hz to 500 Hz (audio-rate modulation capable)
class LFO {
public:
    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        invSampleRate_ = 1.0f / sampleRate;
        phase_ = 0.0f;
        shValue_ = 0.0f;
        noisePrng_ = 543216789;
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            invSampleRate_ = 1.0f / sampleRate;
        }
    }

    void reset(float phaseOffset = 0.0f) noexcept {
        phase_ = phaseOffset;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        sampleAndHold();
    }

    // Step LFO phase and return current modulation value [-1.0, 1.0]
    [[nodiscard]] float process(float frequencyHz, LfoWaveform waveform) noexcept {
        const float clampedFreq = clamp(frequencyHz, 0.05f, 500.0f);
        const float dt = clampedFreq * invSampleRate_;

        phase_ += dt;
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
            sampleAndHold();
        }

        switch (waveform) {
            case LfoWaveform::Triangle:
                // Triangle wave: -1 to +1
                return 2.0f * std::abs(2.0f * phase_ - 1.0f) - 1.0f;

            case LfoWaveform::Sawtooth:
                // Falling sawtooth
                return 1.0f - 2.0f * phase_;

            case LfoWaveform::ReverseSawtooth:
                // Rising ramp
                return 2.0f * phase_ - 1.0f;

            case LfoWaveform::Square:
                // 50% square wave
                return (phase_ < 0.5f) ? 1.0f : -1.0f;

            case LfoWaveform::Random:
                // Sample & Hold random step
                return shValue_;

            default:
                return 0.0f;
        }
    }

private:
    void sampleAndHold() noexcept {
        // xorshift32 PRNG
        noisePrng_ ^= noisePrng_ << 13;
        noisePrng_ ^= noisePrng_ >> 17;
        noisePrng_ ^= noisePrng_ << 5;
        constexpr float scale = 2.0f / 4294967295.0f;
        shValue_ = static_cast<float>(noisePrng_) * scale - 1.0f;
    }

    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;
    float phase_ = 0.0f;
    float shValue_ = 0.0f;
    uint32_t noisePrng_ = 543216789;
};

} // namespace prophet6
