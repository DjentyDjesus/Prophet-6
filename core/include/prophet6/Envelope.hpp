#pragma once

#include "Common.hpp"
#include <cmath>
#include <algorithm>

namespace prophet6 {

enum class EnvelopeStage : uint8_t {
    Idle = 0,
    Attack,
    Decay,
    Sustain,
    Release
};

// Snappy 4-stage exponential ADSR envelope generator.
// Accurately replicates analog capacitor charge/discharge curves with overshoot punch.
class Envelope {
public:
    void init(float sampleRate = DEFAULT_SAMPLE_RATE) noexcept {
        sampleRate_ = sampleRate;
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            sampleRate_ = sampleRate;
        }
    }

    void reset() noexcept {
        stage_ = EnvelopeStage::Idle;
        level_ = 0.0f;
        velocityScale_ = 1.0f;
    }

    void noteOn(float velocity, float velocitySensitivity = 0.0f) noexcept {
        const float clampedVel = clamp(velocity, 0.0f, 1.0f);
        const float clampedSens = clamp(velocitySensitivity, 0.0f, 1.0f);

        // Peak scaling based on velocity sensitivity
        velocityScale_ = (1.0f - clampedSens) + clampedSens * clampedVel;

        // Retrigger seamlessly from current level (click-free)
        stage_ = EnvelopeStage::Attack;
    }

    void noteOff() noexcept {
        if (stage_ != EnvelopeStage::Idle) {
            stage_ = EnvelopeStage::Release;
        }
    }

    // Process a single sample and return current envelope level [0.0, 1.0]
    [[nodiscard]] float process(
        float attackTimeSec,
        float decayTimeSec,
        float sustainLevel,
        float releaseTimeSec) noexcept
    {
        if (stage_ == EnvelopeStage::Idle) {
            return 0.0f;
        }

        const float attClamped = clamp(attackTimeSec, 0.001f, 10.0f);
        const float decClamped = clamp(decayTimeSec, 0.001f, 10.0f);
        const float susClamped = clamp(sustainLevel, 0.0f, 1.0f);
        const float relClamped = clamp(releaseTimeSec, 0.001f, 10.0f);

        switch (stage_) {
            case EnvelopeStage::Attack: {
                // Target an overshoot of 1.5 to provide punchy, convex rise
                constexpr float attackTarget = 1.5f;
                const float rate = 1.0f / (attClamped * sampleRate_);
                level_ += rate * (attackTarget - level_);

                if (level_ >= 1.0f) {
                    level_ = 1.0f;
                    stage_ = EnvelopeStage::Decay;
                }
                break;
            }

            case EnvelopeStage::Decay: {
                // Exponential decay towards sustain level
                const float decayRate = 1.0f - std::exp(-3.0f / (decClamped * sampleRate_));
                level_ += decayRate * (susClamped - level_);

                if (std::abs(level_ - susClamped) <= 1e-4f) {
                    level_ = susClamped;
                    stage_ = EnvelopeStage::Sustain;
                }
                break;
            }

            case EnvelopeStage::Sustain: {
                level_ = susClamped;
                break;
            }

            case EnvelopeStage::Release: {
                // Exponential discharge targeting slightly below 0 to finish cleanly
                constexpr float releaseTarget = -0.005f;
                const float releaseRate = 1.0f - std::exp(-3.5f / (relClamped * sampleRate_));
                level_ += releaseRate * (releaseTarget - level_);

                if (level_ <= 0.0f) {
                    level_ = 0.0f;
                    stage_ = EnvelopeStage::Idle;
                }
                break;
            }

            case EnvelopeStage::Idle:
            default:
                level_ = 0.0f;
                break;
        }

        return level_ * velocityScale_;
    }

    [[nodiscard]] bool isIdle() const noexcept { return stage_ == EnvelopeStage::Idle; }
    [[nodiscard]] bool isRelease() const noexcept { return stage_ == EnvelopeStage::Release; }
    [[nodiscard]] EnvelopeStage getStage() const noexcept { return stage_; }
    [[nodiscard]] float getLevel() const noexcept { return level_ * velocityScale_; }

private:
    float sampleRate_ = DEFAULT_SAMPLE_RATE;
    EnvelopeStage stage_ = EnvelopeStage::Idle;
    float level_ = 0.0f;
    float velocityScale_ = 1.0f;
};

} // namespace prophet6
