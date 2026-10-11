#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include <cmath>

namespace prophet6 {

// Prophet-6 Hardware MIDI Continuous Controller (CC) Dispatch Matrix
// Constant-time O(1) execution, zero runtime dynamic allocations.
class MidiMap {
public:
    static bool applyControlChange(int ccNumber, int ccValue, SynthParameters& params, float& modWheelNorm) noexcept {
        const float norm = clamp(static_cast<float>(ccValue) * (1.0f / 127.0f), 0.0f, 1.0f);
        const bool toggle = (ccValue >= 64);

        switch (ccNumber) {
            case 1: // Modulation Wheel
                modWheelNorm = norm;
                return true;

            case 5: // Glide (Portamento Time)
                params.glide = norm * 2.0f; // 0.0s to 2.0s
                return true;

            case 7: // Master Volume
                params.masterVolume = norm;
                return true;

            // --- Oscillators ---
            case 14: // Osc 1 Frequency (-2 to +2 Octaves)
                params.osc1Octave = static_cast<int>(std::round((norm - 0.5f) * 4.0f));
                return true;

            case 15: // Osc 1 Waveshape (Tri -> Saw -> Pulse)
                params.osc1Shape = norm;
                return true;

            case 16: // Osc 1 Pulse Width
                params.osc1PulseWidth = 0.01f + 0.98f * norm;
                return true;

            case 17: // Osc 1 Sub-Octave Level
                params.osc1SubLevel = norm;
                return true;

            case 20: // Osc 2 Frequency (-2 to +2 Octaves)
                params.osc2Octave = static_cast<int>(std::round((norm - 0.5f) * 4.0f));
                return true;

            case 21: // Osc 2 Fine Tune (+/- 7 Semitones)
                params.osc2FineTune = (norm - 0.5f) * 2.0f * 7.0f;
                return true;

            case 22: // Osc 2 Waveshape
                params.osc2Shape = norm;
                return true;

            case 23: // Osc 2 Pulse Width
                params.osc2PulseWidth = 0.01f + 0.98f * norm;
                return true;

            case 24: // Osc 2 Low Freq Mode
                params.osc2LowFreq = toggle;
                return true;

            case 25: // Osc 2 Keyboard Track
                params.osc2KbdTrack = toggle;
                return true;

            case 26: // Osc 1 Hard Sync
                params.osc1Sync = toggle;
                return true;

            // --- Mixer ---
            case 30: // Osc 1 Level
                params.osc1Level = norm;
                return true;

            case 31: // Osc 2 Level
                params.osc2Level = norm;
                return true;

            case 32: // Noise Level
                params.noiseLevel = norm;
                return true;

            // --- Effects Engine ---
            case 40: // Effects On / True Bypass
                params.fxOn = toggle;
                return true;

            case 41: // FX 1 Type
                params.fx1Type = static_cast<int>(std::round(norm * 9.0f));
                return true;

            case 42: // FX 1 Param 1
                params.fx1Param1 = norm;
                return true;

            case 43: // FX 1 Param 2
                params.fx1Param2 = norm;
                return true;

            case 44: // FX 1 Mix
                params.fx1Mix = norm;
                return true;

            case 45: // FX 2 Type
                params.fx2Type = static_cast<int>(std::round(norm * 9.0f));
                return true;

            case 46: // FX 2 Param 1
                params.fx2Param1 = norm;
                return true;

            case 47: // FX 2 Param 2
                params.fx2Param2 = norm;
                return true;

            case 48: // FX 2 Mix
                params.fx2Mix = norm;
                return true;

            // --- Filters ---
            case 102: // LPF Cutoff (Exponential 20 Hz to 20000 Hz)
                params.lpfCutoff = 20.0f * std::pow(1000.0f, norm);
                return true;

            case 103: // LPF Resonance
                params.lpfResonance = norm;
                return true;

            case 104: // LPF Keyboard Tracking (0.0, 0.5, 1.0)
                params.lpfKbdTrack = norm < 0.33f ? 0.0f : (norm < 0.66f ? 0.5f : 1.0f);
                return true;

            case 105: // LPF Envelope Amount (-1.0 to +1.0)
                params.lpfEnvAmount = (norm - 0.5f) * 2.0f;
                return true;

            case 106: // HPF Cutoff (Exponential 20 Hz to 20000 Hz)
                params.hpfCutoff = 20.0f * std::pow(1000.0f, norm);
                return true;

            case 107: // HPF Resonance
                params.hpfResonance = norm;
                return true;

            case 108: // HPF Envelope Amount (-1.0 to +1.0)
                params.hpfEnvAmount = (norm - 0.5f) * 2.0f;
                return true;

            // --- Performance & Voice Dynamics ---
            case 110: // Vintage Analog Slop
                params.slop = norm;
                return true;

            case 112: // Master Analog Distortion
                params.distortion = norm;
                return true;

            case 113: // Voice Pan Spread
                params.panSpread = norm;
                return true;

            case 114: // Unison Mode
                params.unison = toggle;
                return true;

            case 115: // Unison Detune Spread
                params.unisonDetune = norm;
                return true;

            default:
                return false;
        }
    }
};

} // namespace prophet6
