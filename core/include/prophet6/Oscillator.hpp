#pragma once

#include "Common.hpp"
#include <cmath>
#include <algorithm>

namespace prophet6 {

class Oscillator {
public:
    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        invSampleRate_ = 1.0f / sampleRate;
        phase_ = 0.0f;
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            invSampleRate_ = 1.0f / sampleRate;
        }
    }

    void resetPhase(float phaseOffset = 0.0f) noexcept {
        phase_ = phaseOffset;
        while (phase_ >= 1.0f) phase_ -= 1.0f;
        while (phase_ < 0.0f)  phase_ += 1.0f;
    }

    // Step oscillator phase by current frequency.
    // Returns true if a phase wrap (cycle reset) occurred, which can hard-sync a slave.
    bool advance(float frequencyHz) noexcept {
        const float nyquist = 0.495f * sampleRate_;
        const float clampedFreq = clamp(frequencyHz, 0.001f, nyquist);
        dt_ = clampedFreq * invSampleRate_;

        phase_ += dt_;
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
            return true;
        }
        return false;
    }

    // Hard-sync reset triggered by a master oscillator
    void hardSync(float fractionalOffset = 0.0f) noexcept {
        phase_ = fractionalOffset * dt_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
    }

    // Render current sample with continuous Tri -> Saw -> Variable Pulse morphing
    // shape: 0.0 = Triangle, 0.5 = Sawtooth, 1.0 = Pulse
    // pulseWidth: 0.01 to 0.99
    [[nodiscard]] float render(float shape, float pulseWidth) const noexcept {
        const float clampedShape = clamp(shape, 0.0f, 1.0f);
        const float clampedPW = clamp(pulseWidth, 0.02f, 0.98f);

        // 1. Bandlimited Triangle
        const float naiveTri = 2.0f * std::abs(2.0f * phase_ - 1.0f) - 1.0f;
        float triPhase2 = phase_ + 0.5f;
        if (triPhase2 >= 1.0f) triPhase2 -= 1.0f;
        const float tri = naiveTri + 4.0f * polyBlamp(phase_, dt_) - 4.0f * polyBlamp(triPhase2, dt_);

        // 2. Bandlimited Sawtooth (falling from +1 to -1)
        const float naiveSaw = 1.0f - 2.0f * phase_;
        const float saw = naiveSaw + 2.0f * polyBlep(phase_, dt_);

        // 3. Bandlimited Variable Pulse
        const float naivePulse = (phase_ < clampedPW) ? 1.0f : -1.0f;
        float pwPhase = phase_ - clampedPW;
        if (pwPhase < 0.0f) pwPhase += 1.0f;
        const float pulse = naivePulse + 2.0f * polyBlep(phase_, dt_) - 2.0f * polyBlep(pwPhase, dt_);

        // Continuous morphing: Tri (0.0) -> Saw (0.5) -> Pulse (1.0)
        if (clampedShape <= 0.5f) {
            const float t = clampedShape * 2.0f;
            return (1.0f - t) * tri + t * saw;
        } else {
            const float t = (clampedShape - 0.5f) * 2.0f;
            return (1.0f - t) * saw + t * pulse;
        }
    }

    [[nodiscard]] float getPhase() const noexcept { return phase_; }
    [[nodiscard]] float getDt() const noexcept { return dt_; }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;
    float phase_ = 0.0f;
    float dt_ = 0.0f;
};

// Dedicated Triangle Sub-Oscillator (locked 1 octave below Osc 1)
class SubOscillator {
public:
    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        invSampleRate_ = 1.0f / sampleRate;
        phase_ = 0.0f;
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
            invSampleRate_ = 1.0f / sampleRate;
        }
    }

    void resetPhase(float phaseOffset = 0.0f) noexcept {
        phase_ = phaseOffset;
        while (phase_ >= 1.0f) phase_ -= 1.0f;
        while (phase_ < 0.0f)  phase_ += 1.0f;
    }

    void advance(float masterFrequencyHz) noexcept {
        // Runs at half frequency (1 octave down)
        const float subFreq = clamp(masterFrequencyHz * 0.5f, 0.001f, 0.495f * sampleRate_);
        dt_ = subFreq * invSampleRate_;
        phase_ += dt_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
    }

    [[nodiscard]] float render() const noexcept {
        // Smooth bandlimited triangle wave
        const float naiveTri = 2.0f * std::abs(2.0f * phase_ - 1.0f) - 1.0f;
        float triPhase2 = phase_ + 0.5f;
        if (triPhase2 >= 1.0f) triPhase2 -= 1.0f;
        return naiveTri + 4.0f * polyBlamp(phase_, dt_) - 4.0f * polyBlamp(triPhase2, dt_);
    }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    float invSampleRate_ = 1.0f / DEFAULT_SAMPLE_RATE;
    float phase_ = 0.0f;
    float dt_ = 0.0f;
};

// Pure White Noise Generator
class NoiseGenerator {
public:
    void seed(uint32_t s) noexcept {
        state_ = s != 0 ? s : 123456789;
    }

    // High performance xorshift32 PRNG normalized to [-1.0f, 1.0f]
    [[nodiscard]] float process() noexcept {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        // Map 32-bit uint to float [-1.0f, 1.0f]
        constexpr float scale = 2.0f / 4294967295.0f;
        return static_cast<float>(state_) * scale - 1.0f;
    }

private:
    uint32_t state_ = 987654321;
};

} // namespace prophet6
