#pragma once

#include <cstdint>
#include <string_view>

namespace prophet6 {

enum class LfoWaveform : uint8_t {
    Triangle = 0,
    Sawtooth = 1,
    ReverseSawtooth = 2,
    Square = 3,
    Random = 4 // Sample & Hold
};

struct SynthParameters {
    // Oscillator 1
    int   osc1Octave         = 0;      // -2 to +2
    float osc1Shape          = 0.5f;   // 0.0 (Tri) -> 0.5 (Saw) -> 1.0 (Pulse)
    float osc1PulseWidth     = 0.5f;   // 0.01 to 0.99
    bool  osc1Sync           = false;  // Hard sync slave to Osc 2
    float osc1Level          = 0.7f;   // Mixer level 0.0 to 1.0
    float osc1SubLevel       = 0.0f;   // Triangle Sub-Osc level (-1 Octave)

    // Oscillator 2
    int   osc2Octave         = 0;      // -2 to +2
    float osc2FineTune       = 0.0f;   // Semitones +/- 7.0 (or cents)
    float osc2Shape          = 0.5f;   // 0.0 (Tri) -> 0.5 (Saw) -> 1.0 (Pulse)
    float osc2PulseWidth     = 0.5f;   // 0.01 to 0.99
    bool  osc2LowFreq        = false;  // Low frequency LFO mode (0.05 Hz - 20 Hz)
    bool  osc2KbdTrack       = true;   // Keyboard pitch tracking switch
    float osc2Level          = 0.7f;   // Mixer level 0.0 to 1.0

    // Noise Generator
    float noiseLevel         = 0.0f;   // White noise level 0.0 to 1.0

    // Low-Pass Filter (4-Pole ZDF)
    float lpfCutoff          = 20000.0f; // Hz (10 to 20000)
    float lpfResonance       = 0.0f;     // 0.0 to 1.0 (self-oscillates above ~0.85)
    float lpfEnvAmount       = 0.0f;     // Bipolar -1.0 to +1.0
    float lpfKbdTrack        = 1.0f;     // 0.0, 0.5, 1.0
    float lpfVelocityAmount  = 0.0f;     // 0.0 to 1.0

    // High-Pass Filter (2-Pole ZDF in series)
    float hpfCutoff          = 20.0f;    // Hz (10 to 20000)
    float hpfResonance       = 0.0f;     // 0.0 to 1.0
    float hpfEnvAmount       = 0.0f;     // Bipolar -1.0 to +1.0
    float hpfKbdTrack        = 0.0f;     // 0.0, 0.5, 1.0

    // Poly Mod Matrix
    float polyModFiltEnvAmt   = 0.0f;    // Bipolar source amount -1.0 to +1.0
    float polyModOsc2Amt      = 0.0f;    // Unipolar source amount 0.0 to 1.0
    bool  polyModDestOsc1Freq = false;   // Dest 1: Osc 1 Frequency (cross-mod FM)
    bool  polyModDestOsc1Shape= false;   // Dest 2: Osc 1 Waveshape
    bool  polyModDestOsc1PW   = false;   // Dest 3: Osc 1 Pulse Width
    bool  polyModDestLPF      = false;   // Dest 4: LPF Cutoff
    bool  polyModDestHPF      = false;   // Dest 5: HPF Cutoff

    // Filter Envelope (ADSR)
    float filtAttack         = 0.005f;  // Seconds (0.001 to 10.0)
    float filtDecay          = 0.4f;    // Seconds
    float filtSustain        = 0.0f;    // 0.0 to 1.0
    float filtRelease        = 0.4f;    // Seconds

    // Amplifier Envelope (ADSR)
    float ampAttack          = 0.005f;  // Seconds
    float ampDecay           = 0.3f;    // Seconds
    float ampSustain         = 1.0f;    // 0.0 to 1.0
    float ampRelease         = 0.4f;    // Seconds
    float ampVelocityAmount  = 0.0f;    // Velocity sensitivity 0.0 to 1.0

    // LFO
    LfoWaveform lfoShape     = LfoWaveform::Triangle;
    float lfoFrequency       = 4.0f;    // 0.05 to 500.0 Hz
    float lfoInitialAmount   = 0.0f;    // 0.0 to 1.0
    bool  lfoSync            = false;   // Retrigger on Note On
    bool  lfoDestOsc1Freq    = false;
    bool  lfoDestOsc2Freq    = false;
    bool  lfoDestOsc1PW      = false;
    bool  lfoDestOsc2PW      = false;
    bool  lfoDestLPF         = false;
    bool  lfoDestHPF         = false;
    bool  lfoDestAmp         = false;

    // Master / Voice Performance
    float slop               = 0.0f;    // Organic analog component drift 0.0 to 1.0
    float distortion         = 0.0f;    // Stereo analog master saturation 0.0 to 1.0
    float panSpread          = 0.5f;    // Stereo voice panning spread 0.0 to 1.0
    float masterVolume       = 0.8f;    // Master output level 0.0 to 1.0
    bool  unison             = false;   // All 6 voices stacked monophonically
    float unisonDetune       = 0.1f;    // Spread for unison mode
    float pitchBendRange     = 2.0f;    // Semitones (+/-)
    float glide              = 0.0f;    // Portamento time in seconds

    // Effects Engine (Phase 4: Dual Digital Stereo Effects with True Bypass)
    bool  fxOn               = false;   // True Bypass switch
    int   fx1Type            = 0;       // Slot 1 (FX A) algorithm (0: BBD Delay)
    float fx1Param1          = 0.5f;    // Slot 1 Parameter 1
    float fx1Param2          = 0.5f;    // Slot 1 Parameter 2
    float fx1Mix             = 0.3f;    // Slot 1 Equal-power Wet/Dry mix
    int   fx2Type            = 0;       // Slot 2 (FX B) algorithm (0: BBD Delay)
    float fx2Param1          = 0.5f;    // Slot 2 Parameter 1
    float fx2Param2          = 0.5f;    // Slot 2 Parameter 2
    float fx2Mix             = 0.3f;    // Slot 2 Equal-power Wet/Dry mix
};

} // namespace prophet6
