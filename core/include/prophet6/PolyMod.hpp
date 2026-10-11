#pragma once

#include "Common.hpp"

namespace prophet6 {

struct PolyModModulation {
    float osc1FreqSemitones = 0.0f; // Semitone pitch shift applied to Osc 1
    float osc1ShapeDelta    = 0.0f; // Shape parameter offset
    float osc1PWDelta       = 0.0f; // Pulse-width parameter offset
    float lpfCutoffOctaves  = 0.0f; // Octave offset for LPF cutoff (multiplier = 2^oct)
    float hpfCutoffOctaves  = 0.0f; // Octave offset for HPF cutoff (multiplier = 2^oct)
};

// PolyMod Section: Dave Smith's iconic audio-rate polyphonic cross-modulation matrix.
// Sources: Filter Envelope (bipolar) and Oscillator 2 (audio-rate unipolar)
// Destinations: Osc 1 Freq, Osc 1 Shape, Osc 1 PW, LPF Cutoff, HPF Cutoff.
class PolyMod {
public:
    // Compute modulation outputs for a single audio sample
    [[nodiscard]] PolyModModulation process(
        float filterEnvNormalized, // Envelope output: 0.0 to 1.0 (or bipolar -1.0 to 1.0)
        float osc2AudioSignal,     // Audio sample from Osc 2: -1.0 to 1.0
        float filtEnvAmount,       // Bipolar -1.0 to +1.0
        float osc2Amount,          // Unipolar 0.0 to 1.0
        bool destOsc1Freq,
        bool destOsc1Shape,
        bool destOsc1PW,
        bool destLPF,
        bool destHPF) const noexcept
    {
        PolyModModulation mod{};

        // Source signal contributions:
        // Filter Env: bipolar control signal scaled up to +/- 4 octaves
        const float envSignal = filterEnvNormalized * filtEnvAmount;

        // Osc 2: Audio-rate unipolar depth (osc2AudioSignal in range [-1, 1])
        const float osc2Signal = osc2AudioSignal * osc2Amount;

        // Combined modulation signal
        const float combinedSignal = envSignal + osc2Signal;

        if (destOsc1Freq) {
            // Metallic FM / Bell textures: up to +/- 36 semitones cross-modulation
            mod.osc1FreqSemitones = combinedSignal * 36.0f;
        }

        if (destOsc1Shape) {
            // Dynamic waveshape morphing across Tri -> Saw -> Pulse
            mod.osc1ShapeDelta = combinedSignal * 0.7f;
        }

        if (destOsc1PW) {
            // Audio-rate Pulse-Width Modulation (PWM)
            mod.osc1PWDelta = combinedSignal * 0.45f;
        }

        if (destLPF) {
            // Low-pass filter FM / cutoff sweep up to +/- 5 octaves
            mod.lpfCutoffOctaves = combinedSignal * 5.0f;
        }

        if (destHPF) {
            // High-pass filter FM / formant sweeping up to +/- 5 octaves
            mod.hpfCutoffOctaves = combinedSignal * 5.0f;
        }

        return mod;
    }
};

} // namespace prophet6
