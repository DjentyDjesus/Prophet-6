#include "prophet6/FactoryPresets.hpp"
#include <array>

namespace prophet6 {

namespace {

// Helper to construct a standard preset template
Preset makeBase(int progNum, std::string_view name, PresetCategory cat) {
    Preset p;
    p.programNumber = progNum;
    p.setName(name);
    p.setAuthor("Dave Smith / Sequential");
    p.category = cat;

    // Standard baseline parameters
    p.params.osc1Octave = 0;
    p.params.osc1Shape = 0.5f; // Saw
    p.params.osc1PulseWidth = 0.5f;
    p.params.osc1Level = 0.75f;
    p.params.osc1SubLevel = 0.0f;
    p.params.osc1Sync = false;

    p.params.osc2Octave = 0;
    p.params.osc2FineTune = 0.05f;
    p.params.osc2Shape = 0.5f; // Saw
    p.params.osc2PulseWidth = 0.5f;
    p.params.osc2LowFreq = false;
    p.params.osc2KbdTrack = true;
    p.params.osc2Level = 0.75f;

    p.params.noiseLevel = 0.0f;

    p.params.lpfCutoff = 8000.0f;
    p.params.lpfResonance = 0.2f;
    p.params.lpfEnvAmount = 0.3f;
    p.params.lpfKbdTrack = 1.0f;

    p.params.hpfCutoff = 20.0f;
    p.params.hpfResonance = 0.0f;
    p.params.hpfEnvAmount = 0.0f;

    p.params.filtAttack = 0.01f;
    p.params.filtDecay = 0.5f;
    p.params.filtSustain = 0.4f;
    p.params.filtRelease = 0.4f;

    p.params.ampAttack = 0.01f;
    p.params.ampDecay = 0.5f;
    p.params.ampSustain = 0.8f;
    p.params.ampRelease = 0.4f;

    p.params.lfoFrequency = 4.0f;
    p.params.lfoInitialAmount = 0.0f;
    p.params.lfoShape = LfoWaveform::Triangle;

    p.params.slop = 0.35f;
    p.params.distortion = 0.0f;
    p.params.panSpread = 0.5f;
    p.params.masterVolume = 0.8f;
    p.params.pitchBendRange = 2.0f;
    p.params.glide = 0.0f;

    // Effects defaults (Phase 4)
    p.params.fxOn = true;
    p.params.fx1Type = 0; // BBD Delay
    p.params.fx1Param1 = 0.4f;
    p.params.fx1Param2 = 0.4f;
    p.params.fx1Mix = 0.0f;

    p.params.fx2Type = 7; // Plate Reverb
    p.params.fx2Param1 = 0.5f;
    p.params.fx2Param2 = 0.5f;
    p.params.fx2Mix = 0.25f;

    return p;
}

std::array<Preset, 100> createAllFactoryPresets() {
    std::array<Preset, 100> bank;

    // =========================================================================
    // 000–019: Prophet Brass & Horns
    // =========================================================================
    {
        auto p = makeBase(0, "000: Jump Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.osc2FineTune = 0.08f;
        p.params.lpfCutoff = 1800.0f; p.params.lpfResonance = 0.25f; p.params.lpfEnvAmount = 0.65f;
        p.params.filtAttack = 0.015f; p.params.filtDecay = 0.8f; p.params.filtSustain = 0.35f; p.params.filtRelease = 0.3f;
        p.params.ampAttack = 0.01f; p.params.ampDecay = 0.8f; p.params.ampSustain = 0.7f; p.params.ampRelease = 0.3f;
        p.params.slop = 0.45f;
        p.params.fx1Type = 7; p.params.fx1Mix = 0.35f; // Plate Reverb
        bank[0] = p;
    }
    {
        auto p = makeBase(1, "001: Blade Runner Horn", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc1SubLevel = 0.3f;
        p.params.osc2Shape = 0.5f; p.params.osc2FineTune = -0.10f;
        p.params.lpfCutoff = 1200.0f; p.params.lpfResonance = 0.45f; p.params.lpfEnvAmount = 0.55f;
        p.params.filtAttack = 0.35f; p.params.filtDecay = 2.0f; p.params.filtSustain = 0.5f; p.params.filtRelease = 1.2f;
        p.params.ampAttack = 0.25f; p.params.ampDecay = 2.0f; p.params.ampSustain = 0.8f; p.params.ampRelease = 1.2f;
        p.params.slop = 0.75f;
        p.params.fx1Type = 2; p.params.fx1Param1 = 0.45f; p.params.fx1Param2 = 0.5f; p.params.fx1Mix = 0.4f; // Tape Delay
        p.params.fx2Type = 8; p.params.fx2Param1 = 0.85f; p.params.fx2Param2 = 0.6f; p.params.fx2Mix = 0.5f; // Hall Reverb
        bank[1] = p;
    }
    {
        auto p = makeBase(2, "002: Big Poly Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.52f; p.params.osc2Shape = 0.52f;
        p.params.osc2FineTune = 0.12f;
        p.params.lpfCutoff = 2200.0f; p.params.lpfResonance = 0.3f; p.params.lpfEnvAmount = 0.5f;
        p.params.filtAttack = 0.04f; p.params.filtDecay = 1.0f; p.params.filtSustain = 0.4f;
        p.params.fx1Type = 3; p.params.fx1Param1 = 0.3f; p.params.fx1Param2 = 0.7f; p.params.fx1Mix = 0.45f; // Chorus
        bank[2] = p;
    }
    {
        auto p = makeBase(3, "003: CS-80 Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.07f;
        p.params.lpfCutoff = 2600.0f; p.params.lpfResonance = 0.4f; p.params.lpfEnvAmount = 0.6f;
        p.params.hpfCutoff = 220.0f; p.params.hpfResonance = 0.35f; // Resonant HPF formant cut
        p.params.filtAttack = 0.06f; p.params.filtDecay = 1.2f; p.params.filtSustain = 0.45f;
        p.params.fx1Type = 3; p.params.fx1Mix = 0.35f; p.params.fx2Type = 7; p.params.fx2Mix = 0.3f;
        bank[3] = p;
    }
    {
        auto p = makeBase(4, "004: Resonant Brass Stab", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.48f; p.params.osc2FineTune = 0.06f;
        p.params.lpfCutoff = 900.0f; p.params.lpfResonance = 0.75f; p.params.lpfEnvAmount = 0.85f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.35f; p.params.filtSustain = 0.1f;
        p.params.ampAttack = 0.005f; p.params.ampDecay = 0.4f; p.params.ampSustain = 0.2f;
        p.params.distortion = 0.4f;
        bank[4] = p;
    }
    {
        auto p = makeBase(5, "005: 80s Fanfare", PresetCategory::Brass);
        p.params.osc1Shape = 0.65f; p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.lpfCutoff = 3000.0f; p.params.lpfResonance = 0.35f; p.params.lpfEnvAmount = 0.5f;
        p.params.filtAttack = 0.03f; p.params.filtDecay = 0.8f;
        p.params.fx1Type = 7; p.params.fx1Mix = 0.4f;
        bank[5] = p;
    }
    {
        auto p = makeBase(6, "006: Warm French Horn", PresetCategory::Brass);
        p.params.osc1Shape = 0.3f; p.params.osc1SubLevel = 0.4f; p.params.osc2Shape = 0.3f;
        p.params.lpfCutoff = 1100.0f; p.params.lpfResonance = 0.2f; p.params.lpfEnvAmount = 0.4f;
        p.params.filtAttack = 0.1f; p.params.filtDecay = 1.0f; p.params.filtSustain = 0.6f;
        p.params.fx2Type = 8; p.params.fx2Mix = 0.4f;
        bank[6] = p;
    }
    {
        auto p = makeBase(7, "007: Aggressive Synth Horn", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.polyModOsc2Amt = 0.25f; p.params.polyModDestLPF = true;
        p.params.lpfCutoff = 1500.0f; p.params.lpfResonance = 0.6f; p.params.lpfEnvAmount = 0.7f;
        p.params.distortion = 0.55f;
        bank[7] = p;
    }
    {
        auto p = makeBase(8, "008: Dark Muted Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.4f; p.params.osc2Shape = 0.4f; p.params.lpfCutoff = 950.0f;
        p.params.lpfResonance = 0.15f; p.params.lpfEnvAmount = 0.35f;
        p.params.fx1Type = 0; p.params.fx1Param1 = 0.35f; p.params.fx1Param2 = 0.5f; p.params.fx1Mix = 0.35f; // BBD Delay
        bank[8] = p;
    }
    {
        auto p = makeBase(9, "009: Vintage Octave Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc1Octave = -1;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 0;
        p.params.lpfCutoff = 2000.0f; p.params.lpfEnvAmount = 0.6f; p.params.slop = 0.6f;
        bank[9] = p;
    }
    {
        auto p = makeBase(10, "010: Power Brass Swell", PresetCategory::Brass);
        p.params.osc1Shape = 0.55f; p.params.osc2Shape = 0.55f; p.params.osc2FineTune = 0.08f;
        p.params.filtAttack = 0.25f; p.params.filtDecay = 1.5f; p.params.filtSustain = 0.5f;
        p.params.ampAttack = 0.2f; p.params.lpfCutoff = 1400.0f; p.params.lpfEnvAmount = 0.7f;
        p.params.fx2Type = 8; p.params.fx2Mix = 0.45f;
        bank[10] = p;
    }
    {
        auto p = makeBase(11, "011: Classic Analog Horns", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.06f;
        p.params.lpfCutoff = 2400.0f; p.params.lpfEnvAmount = 0.5f; p.params.slop = 0.5f;
        p.params.panSpread = 0.8f;
        bank[11] = p;
    }
    {
        auto p = makeBase(12, "012: Punchy Brass Chomp", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.lpfCutoff = 1100.0f;
        p.params.lpfResonance = 0.55f; p.params.lpfEnvAmount = 0.75f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.25f;
        bank[12] = p;
    }
    {
        auto p = makeBase(13, "013: PolyMod FM Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.0f; // Tri
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.polyModOsc2Amt = 0.35f; p.params.polyModDestOsc1Freq = true; // FM Brass bite
        p.params.lpfCutoff = 3500.0f; p.params.lpfEnvAmount = 0.4f;
        bank[13] = p;
    }
    {
        auto p = makeBase(14, "014: Soft Trombones", PresetCategory::Brass);
        p.params.osc1Shape = 0.4f; p.params.osc1SubLevel = 0.5f; p.params.osc2Shape = 0.4f;
        p.params.lpfCutoff = 900.0f; p.params.lpfEnvAmount = 0.3f;
        p.params.filtAttack = 0.08f; p.params.filtDecay = 0.8f;
        bank[14] = p;
    }
    {
        auto p = makeBase(15, "015: Prophet Majestic Horn", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.1f;
        p.params.lpfCutoff = 2000.0f; p.params.lpfEnvAmount = 0.6f;
        p.params.fx1Type = 3; p.params.fx1Mix = 0.35f; p.params.fx2Type = 8; p.params.fx2Mix = 0.4f;
        bank[15] = p;
    }
    {
        auto p = makeBase(16, "016: Sub-Octave Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc1SubLevel = 0.6f; p.params.osc2Shape = 0.5f;
        p.params.lpfCutoff = 1600.0f; p.params.lpfEnvAmount = 0.55f;
        bank[16] = p;
    }
    {
        auto p = makeBase(17, "017: Slop Detune Horns", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.slop = 0.9f;
        p.params.lpfCutoff = 2200.0f; p.params.lpfEnvAmount = 0.5f;
        bank[17] = p;
    }
    {
        auto p = makeBase(18, "018: Bright Poly Fanfare", PresetCategory::Brass);
        p.params.osc1Shape = 0.6f; p.params.osc2Shape = 0.6f; p.params.osc2FineTune = 0.06f;
        p.params.lpfCutoff = 4500.0f; p.params.lpfResonance = 0.3f; p.params.lpfEnvAmount = 0.45f;
        bank[18] = p;
    }
    {
        auto p = makeBase(19, "019: Hall Reverb Brass", PresetCategory::Brass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lpfCutoff = 2000.0f; p.params.lpfEnvAmount = 0.55f;
        p.params.fx2Type = 8; p.params.fx2Param1 = 0.8f; p.params.fx2Param2 = 0.6f; p.params.fx2Mix = 0.5f;
        bank[19] = p;
    }

    // =========================================================================
    // 020–039: Poly Keys & Organs
    // =========================================================================
    {
        auto p = makeBase(20, "020: Warm Tines", PresetCategory::Keys);
        p.params.osc1Shape = 0.15f; p.params.osc2Shape = 0.8f; p.params.osc2PulseWidth = 0.4f;
        p.params.lpfCutoff = 3200.0f; p.params.lpfResonance = 0.2f; p.params.lpfEnvAmount = 0.45f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 1.2f; p.params.filtSustain = 0.2f;
        p.params.ampAttack = 0.005f; p.params.ampDecay = 1.5f; p.params.ampSustain = 0.3f;
        p.params.fx1Type = 3; p.params.fx1Mix = 0.45f; p.params.fx2Type = 7; p.params.fx2Mix = 0.35f;
        bank[20] = p;
    }
    {
        auto p = makeBase(21, "021: Vintage Clav", PresetCategory::Keys);
        p.params.osc1Shape = 0.85f; p.params.osc1PulseWidth = 0.15f; // Narrow pulse
        p.params.osc2Shape = 0.85f; p.params.osc2PulseWidth = 0.25f;
        p.params.hpfCutoff = 180.0f; p.params.hpfResonance = 0.3f; // Thin funk bite
        p.params.lpfCutoff = 4500.0f; p.params.lpfResonance = 0.55f; p.params.lpfEnvAmount = 0.65f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.3f; p.params.filtSustain = 0.0f;
        p.params.fx1Type = 5; p.params.fx1Param1 = 0.35f; p.params.fx1Param2 = 0.6f; p.params.fx1Mix = 0.4f; // Phaser
        bank[21] = p;
    }
    {
        auto p = makeBase(22, "022: Poly Sync Keys", PresetCategory::Keys);
        p.params.osc1Shape = 0.5f; p.params.osc1Sync = true;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.polyModFiltEnvAmt = 0.4f; p.params.polyModDestOsc1Freq = true;
        p.params.lpfCutoff = 5000.0f; p.params.lpfEnvAmount = 0.4f;
        p.params.fx1Type = 1; p.params.fx1Mix = 0.3f;
        bank[22] = p;
    }
    {
        auto p = makeBase(23, "023: 80s Soundtrack Pluck", PresetCategory::Keys);
        p.params.osc1Shape = 0.75f; p.params.osc1PulseWidth = 0.3f;
        p.params.osc2Shape = 0.0f; p.params.osc2Level = 0.5f;
        p.params.lpfCutoff = 2500.0f; p.params.lpfResonance = 0.4f; p.params.lpfEnvAmount = 0.7f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.4f; p.params.filtSustain = 0.0f;
        p.params.fx1Type = 1; p.params.fx1Param1 = 0.35f; p.params.fx1Param2 = 0.6f; p.params.fx1Mix = 0.4f; // Digital Delay
        p.params.fx2Type = 7; p.params.fx2Mix = 0.35f;
        bank[23] = p;
    }
    {
        auto p = makeBase(24, "024: Gospel B3 Organ", PresetCategory::Keys);
        p.params.osc1Shape = 0.05f; p.params.osc2Shape = 0.05f; p.params.osc2Octave = 1;
        p.params.lpfCutoff = 10000.0f; p.params.lpfEnvAmount = 0.0f;
        p.params.filtAttack = 0.001f; p.params.filtRelease = 0.01f;
        p.params.ampAttack = 0.001f; p.params.ampRelease = 0.02f;
        p.params.fx1Type = 3; p.params.fx1Param1 = 0.75f; p.params.fx1Param2 = 0.8f; p.params.fx1Mix = 0.6f; // Rotary Chorus
        p.params.distortion = 0.35f;
        bank[24] = p;
    }
    {
        auto p = makeBase(25, "025: Cheesy Farfisa", PresetCategory::Keys);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.8f; p.params.osc2PulseWidth = 0.15f;
        p.params.hpfCutoff = 350.0f; p.params.lpfCutoff = 14000.0f;
        p.params.lfoInitialAmount = 0.25f; p.params.lfoDestOsc1Freq = true; p.params.lfoDestOsc2Freq = true;
        p.params.lfoFrequency = 5.5f;
        bank[25] = p;
    }
    {
        auto p = makeBase(26, "026: Hybrid EP Tine", PresetCategory::Keys);
        p.params.osc1Shape = 0.1f; p.params.osc2Shape = 0.0f; p.params.osc2Octave = 2;
        p.params.polyModOsc2Amt = 0.2f; p.params.polyModDestOsc1Freq = true;
        p.params.lpfCutoff = 3500.0f; p.params.lpfEnvAmount = 0.5f;
        p.params.fx1Type = 3; p.params.fx1Mix = 0.4f;
        bank[26] = p;
    }
    {
        auto p = makeBase(27, "027: Analog Harpsichord", PresetCategory::Keys);
        p.params.osc1Shape = 0.9f; p.params.osc1PulseWidth = 0.12f;
        p.params.osc2Shape = 0.9f; p.params.osc2PulseWidth = 0.2f; p.params.osc2Octave = 1;
        p.params.lpfCutoff = 8000.0f; p.params.lpfEnvAmount = 0.2f;
        p.params.ampAttack = 0.002f; p.params.ampDecay = 0.6f; p.params.ampSustain = 0.0f;
        bank[27] = p;
    }
    {
        auto p = makeBase(28, "028: Mellow Bell Keys", PresetCategory::Keys);
        p.params.osc1Shape = 0.0f; p.params.osc2Shape = 0.0f; p.params.osc2Octave = 2; p.params.osc2FineTune = 0.35f;
        p.params.lpfCutoff = 2800.0f; p.params.lpfEnvAmount = 0.4f;
        p.params.fx2Type = 7; p.params.fx2Param1 = 0.7f; p.params.fx2Mix = 0.5f;
        bank[28] = p;
    }
    {
        auto p = makeBase(29, "029: Percussive Organ", PresetCategory::Keys);
        p.params.osc1Shape = 0.05f; p.params.osc2Shape = 0.05f; p.params.osc2Octave = 1;
        p.params.noiseLevel = 0.12f; p.params.lpfCutoff = 12000.0f;
        p.params.ampAttack = 0.002f; p.params.ampDecay = 0.8f;
        bank[29] = p;
    }
    {
        auto p = makeBase(30, "030: Chorus Dyno EP", PresetCategory::Keys);
        p.params.osc1Shape = 0.2f; p.params.osc1SubLevel = 0.25f;
        p.params.osc2Shape = 0.75f; p.params.osc2PulseWidth = 0.35f;
        p.params.lpfCutoff = 3000.0f; p.params.lpfEnvAmount = 0.4f;
        p.params.fx1Type = 3; p.params.fx1Param1 = 0.25f; p.params.fx1Param2 = 0.8f; p.params.fx1Mix = 0.55f;
        bank[30] = p;
    }
    {
        auto p = makeBase(31, "031: Kid A Right Place", PresetCategory::Keys);
        p.params.osc1Shape = 0.85f; // Pulse / Square
        p.params.osc1PulseWidth = 0.50f;
        p.params.osc1Level = 0.70f;
        p.params.osc1SubLevel = 0.20f; // Triangle sub-oscillator body
        p.params.osc2Shape = 0.20f; // Triangle/Saw morph
        p.params.osc2FineTune = 0.04f;
        p.params.osc2Level = 0.55f;
        p.params.lpfCutoff = 950.0f; // Warm woody EP cutoff
        p.params.lpfResonance = 0.18f;
        p.params.lpfEnvAmount = 0.35f;
        p.params.lpfKbdTrack = 1.0f; // 100% Key Tracking
        p.params.lpfVelocityAmount = 0.75f; // Dynamic touch response
        p.params.filtAttack = 0.002f;
        p.params.filtDecay = 1.35f;
        p.params.filtSustain = 0.22f;
        p.params.filtRelease = 0.45f;
        p.params.ampAttack = 0.002f;
        p.params.ampDecay = 2.10f;
        p.params.ampSustain = 0.50f;
        p.params.ampRelease = 0.45f;
        p.params.ampVelocityAmount = 0.65f;
        p.params.slop = 0.40f; // Vintage Prophet-5 drift
        p.params.distortion = 0.12f; // Analog console drive
        p.params.panSpread = 0.35f;
        p.params.fxOn = true;
        p.params.fx1Type = 7; // Plate Reverb
        p.params.fx1Param1 = 0.35f;
        p.params.fx1Param2 = 0.40f;
        p.params.fx1Mix = 0.22f;
        bank[31] = p;
    }
    {
        auto p = makeBase(32, "032: Aloboi Dreamscape", PresetCategory::Pad);
        p.params.osc1Shape = 0.50f; // Saw
        p.params.osc1Level = 0.75f;
        p.params.osc1SubLevel = 0.35f; // Deep sub-bass floor
        p.params.osc2Shape = 0.10f; // Soft triangle
        p.params.osc2FineTune = 0.06f; // +6 cents lush detune
        p.params.osc2Octave = -1; // -1 Octave
        p.params.osc2Level = 0.60f;
        p.params.lpfCutoff = 1400.0f;
        p.params.lpfResonance = 0.25f;
        p.params.lpfEnvAmount = 0.40f;
        p.params.lpfKbdTrack = 0.50f;
        p.params.filtAttack = 0.080f; // 80ms soft swell
        p.params.filtDecay = 2.50f;
        p.params.filtSustain = 0.60f;
        p.params.filtRelease = 1.80f;
        p.params.ampAttack = 0.080f;
        p.params.ampDecay = 2.50f;
        p.params.ampSustain = 0.60f;
        p.params.ampRelease = 1.80f;
        p.params.slop = 0.65f; // Rich organic thermal slop
        p.params.panSpread = 0.40f;
        p.params.fxOn = true;
        p.params.fx1Type = 1; // Tape Delay
        p.params.fx1Param1 = 0.375f; // Dotted 8th
        p.params.fx1Param2 = 0.40f;
        p.params.fx1Mix = 0.35f;
        p.params.fx2Type = 7; // Shimmer Plate Reverb
        p.params.fx2Param1 = 0.70f; // 3.5s decay
        p.params.fx2Param2 = 0.50f;
        p.params.fx2Mix = 0.45f;
        bank[32] = p;
    }
    {
        auto p = makeBase(33, "033: The Veldt Pluck", PresetCategory::Keys);
        p.params.osc1Shape = 0.65f; // Pulse/Saw blend
        p.params.osc1PulseWidth = 0.40f;
        p.params.osc1Level = 0.75f;
        p.params.osc2Shape = 0.50f; // Saw
        p.params.osc2FineTune = 0.03f; // +3 cents detune
        p.params.osc2Level = 0.70f;
        p.params.noiseLevel = 0.0f;
        p.params.lpfCutoff = 1200.0f;
        p.params.lpfResonance = 0.38f;
        p.params.lpfEnvAmount = 0.70f;
        p.params.lpfVelocityAmount = 0.50f;
        p.params.filtAttack = 0.001f; // 1ms instant punch
        p.params.filtDecay = 0.380f; // 380ms pluck decay
        p.params.filtSustain = 0.05f;
        p.params.filtRelease = 0.20f;
        p.params.ampAttack = 0.001f;
        p.params.ampDecay = 0.500f;
        p.params.ampSustain = 0.0f;
        p.params.ampRelease = 0.25f;
        p.params.slop = 0.25f;
        p.params.distortion = 0.10f;
        p.params.fxOn = true;
        p.params.fx1Type = 0; // BBD Delay
        p.params.fx1Param1 = 0.25f; // 1/8 note repeats
        p.params.fx1Param2 = 0.45f;
        p.params.fx1Mix = 0.30f;
        p.params.fx2Type = 7; // Room/Plate Reverb
        p.params.fx2Param1 = 0.30f;
        p.params.fx2Param2 = 0.35f;
        p.params.fx2Mix = 0.20f;
        bank[33] = p;
    }
    {
        auto p = makeBase(34, "034: Poly Mod Tines", PresetCategory::Keys);
        p.params.osc1Shape = 0.1f; p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.polyModOsc2Amt = 0.3f; p.params.polyModDestOsc1Freq = true;
        p.params.lpfCutoff = 3500.0f; p.params.lpfEnvAmount = 0.45f;
        bank[34] = p;
    }
    {
        auto p = makeBase(35, "035: Rotary Jazz Organ", PresetCategory::Keys);
        p.params.osc1Shape = 0.05f; p.params.osc2Shape = 0.05f;
        p.params.fx1Type = 3; p.params.fx1Param1 = 0.8f; p.params.fx1Mix = 0.65f;
        p.params.distortion = 0.2f;
        bank[35] = p;
    }
    {
        auto p = makeBase(36, "036: Bright Comping Keys", PresetCategory::Keys);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.75f;
        p.params.lpfCutoff = 4500.0f; p.params.lpfEnvAmount = 0.5f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.6f;
        bank[36] = p;
    }
    {
        auto p = makeBase(37, "037: Vintage Suitcase Piano", PresetCategory::Keys);
        p.params.osc1Shape = 0.2f; p.params.osc2Shape = 0.6f; p.params.slop = 0.6f;
        p.params.lpfCutoff = 2600.0f; p.params.fx1Type = 3; p.params.fx1Mix = 0.4f;
        bank[37] = p;
    }
    {
        auto p = makeBase(38, "038: Slop Analog Clav", PresetCategory::Keys);
        p.params.osc1Shape = 0.85f; p.params.osc1PulseWidth = 0.18f;
        p.params.osc2Shape = 0.85f; p.params.slop = 0.75f;
        p.params.lpfCutoff = 3800.0f; p.params.lpfEnvAmount = 0.6f;
        bank[38] = p;
    }
    {
        auto p = makeBase(39, "039: Ambient Rhodes", PresetCategory::Keys);
        p.params.osc1Shape = 0.15f; p.params.osc2Shape = 0.65f;
        p.params.lpfCutoff = 2400.0f; p.params.lpfEnvAmount = 0.35f;
        p.params.fx1Type = 2; p.params.fx1Mix = 0.4f; p.params.fx2Type = 7; p.params.fx2Mix = 0.45f;
        bank[39] = p;
    }

    // =========================================================================
    // 040–059: Analog Pads & Strings
    // =========================================================================
    {
        auto p = makeBase(40, "040: Celestial Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.12f;
        p.params.lpfCutoff = 1500.0f; p.params.lpfResonance = 0.35f; p.params.lpfEnvAmount = 0.4f;
        p.params.filtAttack = 1.2f; p.params.filtDecay = 2.5f; p.params.filtSustain = 0.6f; p.params.filtRelease = 2.0f;
        p.params.ampAttack = 1.0f; p.params.ampDecay = 2.5f; p.params.ampSustain = 0.8f; p.params.ampRelease = 2.0f;
        p.params.slop = 0.7f; p.params.panSpread = 0.85f;
        p.params.fx1Type = 3; p.params.fx1Mix = 0.45f; p.params.fx2Type = 7; p.params.fx2Param1 = 0.75f; p.params.fx2Mix = 0.55f;
        bank[40] = p;
    }
    {
        auto p = makeBase(41, "041: Prophet Strings", PresetCategory::Strings);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.08f;
        p.params.lpfCutoff = 3200.0f; p.params.lpfResonance = 0.25f; p.params.lpfEnvAmount = 0.35f;
        p.params.filtAttack = 0.25f; p.params.filtRelease = 1.0f;
        p.params.ampAttack = 0.2f; p.params.ampRelease = 1.0f;
        p.params.fx1Type = 3; p.params.fx1Param1 = 0.4f; p.params.fx1Param2 = 0.8f; p.params.fx1Mix = 0.5f; // Chorus
        bank[41] = p;
    }
    {
        auto p = makeBase(42, "042: Hollow Air Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.75f; p.params.osc1PulseWidth = 0.35f;
        p.params.osc2Shape = 0.0f; p.params.noiseLevel = 0.15f;
        p.params.hpfCutoff = 450.0f; p.params.hpfResonance = 0.4f;
        p.params.lpfCutoff = 2200.0f; p.params.filtAttack = 1.5f;
        p.params.fx2Type = 8; p.params.fx2Mix = 0.6f;
        bank[42] = p;
    }
    {
        auto p = makeBase(43, "043: Slop Ambient Swell", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.slop = 0.95f;
        p.params.lpfCutoff = 1200.0f; p.params.lpfEnvAmount = 0.5f;
        p.params.ampAttack = 2.0f; p.params.ampRelease = 2.5f;
        p.params.fx1Type = 2; p.params.fx1Mix = 0.4f; p.params.fx2Type = 7; p.params.fx2Mix = 0.5f;
        bank[43] = p;
    }
    {
        auto p = makeBase(44, "044: Shimmer Plate Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.lpfCutoff = 4500.0f; p.params.lpfResonance = 0.3f;
        p.params.fx2Type = 7; p.params.fx2Param1 = 0.85f; p.params.fx2Param2 = 0.75f; p.params.fx2Mix = 0.6f;
        bank[44] = p;
    }
    {
        auto p = makeBase(45, "045: Solina Ensemble", PresetCategory::Strings);
        p.params.osc1Shape = 0.8f; p.params.osc1PulseWidth = 0.4f;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.lpfCutoff = 5000.0f; p.params.fx1Type = 3; p.params.fx1Mix = 0.6f;
        bank[45] = p;
    }
    {
        auto p = makeBase(46, "046: Warm Analog Strings", PresetCategory::Strings);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.07f;
        p.params.lpfCutoff = 2800.0f; p.params.filtAttack = 0.3f; p.params.slop = 0.55f;
        bank[46] = p;
    }
    {
        auto p = makeBase(47, "047: Vangelis CS Strings", PresetCategory::Strings);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lfoInitialAmount = 0.2f; p.params.lfoFrequency = 4.2f; p.params.lfoDestOsc1Freq = true; p.params.lfoDestOsc2Freq = true;
        p.params.hpfCutoff = 150.0f; p.params.lpfCutoff = 3500.0f;
        bank[47] = p;
    }
    {
        auto p = makeBase(48, "048: Dark Ethereal Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.3f; p.params.osc1SubLevel = 0.5f; p.params.osc2Shape = 0.3f;
        p.params.lpfCutoff = 900.0f; p.params.filtAttack = 1.0f;
        p.params.fx1Type = 2; p.params.fx1Mix = 0.4f; p.params.fx2Type = 8; p.params.fx2Mix = 0.5f;
        bank[48] = p;
    }
    {
        auto p = makeBase(49, "049: Sweeping Filter Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lfoFrequency = 0.35f; p.params.lfoInitialAmount = 0.45f; p.params.lfoDestLPF = true;
        p.params.lpfCutoff = 1800.0f; p.params.lpfResonance = 0.6f;
        bank[49] = p;
    }
    {
        auto p = makeBase(50, "050: PolyMod Vocal Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.polyModFiltEnvAmt = 0.5f; p.params.polyModDestHPF = true;
        p.params.hpfCutoff = 400.0f; p.params.hpfResonance = 0.65f;
        p.params.lpfCutoff = 3000.0f; p.params.lpfResonance = 0.5f;
        bank[50] = p;
    }
    {
        auto p = makeBase(51, "051: Soft Velvet Strings", PresetCategory::Strings);
        p.params.osc1Shape = 0.45f; p.params.osc2Shape = 0.45f;
        p.params.lpfCutoff = 2200.0f; p.params.filtAttack = 0.4f;
        p.params.fx2Type = 8; p.params.fx2Mix = 0.4f;
        bank[51] = p;
    }
    {
        auto p = makeBase(52, "052: Dual Filter Ambient", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lfoFrequency = 0.25f; p.params.lfoInitialAmount = 0.4f;
        p.params.lfoDestLPF = true; p.params.lfoDestHPF = true;
        p.params.lpfCutoff = 2500.0f; p.params.hpfCutoff = 300.0f;
        bank[52] = p;
    }
    {
        auto p = makeBase(53, "053: Chorus Ensemble Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.fx1Type = 3; p.params.fx1Param1 = 0.3f; p.params.fx1Param2 = 0.85f; p.params.fx1Mix = 0.55f;
        bank[53] = p;
    }
    {
        auto p = makeBase(54, "054: Majestic Analog Choir", PresetCategory::Pad);
        p.params.osc1Shape = 0.7f; p.params.osc2Shape = 0.5f;
        p.params.hpfCutoff = 600.0f; p.params.hpfResonance = 0.7f; // Vowel resonance peak
        p.params.lpfCutoff = 2800.0f; p.params.lpfResonance = 0.4f;
        bank[54] = p;
    }
    {
        auto p = makeBase(55, "055: Golden Hour Swell", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.ampAttack = 1.8f; p.params.ampRelease = 2.2f; p.params.distortion = 0.25f;
        bank[55] = p;
    }
    {
        auto p = makeBase(56, "056: High Resonance Strings", PresetCategory::Strings);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lpfCutoff = 2600.0f; p.params.lpfResonance = 0.7f;
        bank[56] = p;
    }
    {
        auto p = makeBase(57, "057: Slow Attack Strings", PresetCategory::Strings);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.ampAttack = 2.5f; p.params.filtAttack = 2.5f;
        p.params.fx2Type = 8; p.params.fx2Mix = 0.45f;
        bank[57] = p;
    }
    {
        auto p = makeBase(58, "058: Tape Flanged Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.fx1Type = 4; p.params.fx1Param1 = 0.2f; p.params.fx1Param2 = 0.6f; p.params.fx1Mix = 0.4f; // Flanger
        bank[58] = p;
    }
    {
        auto p = makeBase(59, "059: Endless Reverb Pad", PresetCategory::Pad);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.fx2Type = 7; p.params.fx2Param1 = 0.95f; p.params.fx2Param2 = 0.7f; p.params.fx2Mix = 0.7f; // Long reverb tail
        bank[59] = p;
    }

    // =========================================================================
    // 060–079: Punchy Basses & Mono Leads
    // =========================================================================
    {
        auto p = makeBase(60, "060: 24dB Slop Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.5f; p.params.osc1Octave = -1; p.params.osc1SubLevel = 0.5f;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = -1; p.params.osc2FineTune = 0.05f;
        p.params.lpfCutoff = 450.0f; p.params.lpfResonance = 0.35f; p.params.lpfEnvAmount = 0.75f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.35f; p.params.filtSustain = 0.0f;
        p.params.ampAttack = 0.005f; p.params.ampDecay = 0.4f; p.params.ampSustain = 0.2f;
        p.params.slop = 0.65f;
        bank[60] = p;
    }
    {
        auto p = makeBase(61, "061: Unison Detune Growl", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.12f;
        p.params.unison = true; p.params.unisonDetune = 0.25f; // All 6 voices stacked
        p.params.lpfCutoff = 2500.0f; p.params.lpfResonance = 0.45f; p.params.distortion = 0.5f;
        p.params.glide = 0.05f;
        bank[61] = p;
    }
    {
        auto p = makeBase(62, "062: Reso Sub Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.0f; p.params.osc1SubLevel = 0.8f; // Pure triangle fundamental
        p.params.osc2Shape = 0.0f; p.params.osc2Level = 0.3f;
        p.params.lpfCutoff = 220.0f; p.params.lpfResonance = 0.7f; p.params.lpfEnvAmount = 0.5f;
        bank[62] = p;
    }
    {
        auto p = makeBase(63, "063: Hard Sync Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc1Sync = true;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.polyModFiltEnvAmt = 0.7f; p.params.polyModDestOsc1Freq = true;
        p.params.unison = true; p.params.unisonDetune = 0.15f;
        p.params.fx1Type = 1; p.params.fx1Mix = 0.35f; // Digital delay repeats
        bank[63] = p;
    }
    {
        auto p = makeBase(64, "064: Dual Filter Screamer", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lpfCutoff = 3000.0f; p.params.lpfResonance = 0.82f;
        p.params.hpfCutoff = 350.0f; p.params.hpfResonance = 0.75f;
        p.params.distortion = 0.7f;
        bank[64] = p;
    }
    {
        auto p = makeBase(65, "065: Funk Bass Pluck", PresetCategory::Bass);
        p.params.osc1Shape = 0.75f; p.params.osc1PulseWidth = 0.3f; p.params.osc1SubLevel = 0.35f;
        p.params.lpfCutoff = 600.0f; p.params.lpfResonance = 0.5f; p.params.lpfEnvAmount = 0.8f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.2f;
        bank[65] = p;
    }
    {
        auto p = makeBase(66, "066: 80s Synth Pop Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.75f; p.params.osc2Octave = -1;
        p.params.lpfCutoff = 800.0f; p.params.lpfEnvAmount = 0.65f;
        p.params.fx1Type = 3; p.params.fx1Mix = 0.35f; // Stereo chorus
        bank[66] = p;
    }
    {
        auto p = makeBase(67, "067: Snappy Acid Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.5f; p.params.osc2Level = 0.0f;
        p.params.lpfCutoff = 400.0f; p.params.lpfResonance = 0.82f; p.params.lpfEnvAmount = 0.9f;
        p.params.filtAttack = 0.002f; p.params.filtDecay = 0.22f; p.params.distortion = 0.45f;
        bank[67] = p;
    }
    {
        auto p = makeBase(68, "068: Mono Saw Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 0.08f;
        p.params.unison = true; p.params.glide = 0.06f;
        p.params.lpfCutoff = 4500.0f; p.params.fx1Type = 2; p.params.fx1Mix = 0.35f;
        bank[68] = p;
    }
    {
        auto p = makeBase(69, "069: Distorted Unison Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.unison = true; p.params.unisonDetune = 0.3f;
        p.params.distortion = 0.8f; p.params.lpfCutoff = 5000.0f;
        bank[69] = p;
    }
    {
        auto p = makeBase(70, "070: Vintage Minimoog Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.5f; p.params.osc1Octave = -1;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = -1; p.params.osc2FineTune = 0.04f;
        p.params.osc1SubLevel = 0.6f;
        p.params.lpfCutoff = 550.0f; p.params.lpfResonance = 0.2f; p.params.lpfEnvAmount = 0.7f;
        bank[70] = p;
    }
    {
        auto p = makeBase(71, "071: Square Wave Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.75f; p.params.osc1PulseWidth = 0.5f;
        p.params.lfoInitialAmount = 0.2f; p.params.lfoFrequency = 5.0f; p.params.lfoDestOsc1Freq = true;
        p.params.lpfCutoff = 4000.0f; p.params.fx1Type = 0; p.params.fx1Mix = 0.35f;
        bank[71] = p;
    }
    {
        auto p = makeBase(72, "072: Portamento Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.glide = 0.12f; p.params.unison = true;
        p.params.lpfCutoff = 3500.0f; p.params.lpfResonance = 0.5f;
        bank[72] = p;
    }
    {
        auto p = makeBase(73, "073: Sub Triangle Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.0f; p.params.osc1SubLevel = 0.9f; p.params.osc2Level = 0.0f;
        p.params.lpfCutoff = 250.0f; p.params.lpfEnvAmount = 0.2f;
        bank[73] = p;
    }
    {
        auto p = makeBase(74, "074: Searing Hard Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc1Sync = true;
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1;
        p.params.lpfCutoff = 6000.0f; p.params.distortion = 0.6f;
        bank[74] = p;
    }
    {
        auto p = makeBase(75, "075: Hollow Wood Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.85f; p.params.osc1PulseWidth = 0.2f;
        p.params.lpfCutoff = 500.0f; p.params.lpfResonance = 0.6f; p.params.lpfEnvAmount = 0.75f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.18f;
        bank[75] = p;
    }
    {
        auto p = makeBase(76, "076: BBD Tape Echo Lead", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.fx1Type = 0; p.params.fx1Param1 = 0.42f; p.params.fx1Param2 = 0.65f; p.params.fx1Mix = 0.45f;
        p.params.lpfCutoff = 4000.0f;
        bank[76] = p;
    }
    {
        auto p = makeBase(77, "077: Click Organ Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.1f; p.params.osc1SubLevel = 0.5f; p.params.noiseLevel = 0.15f;
        p.params.lpfCutoff = 700.0f; p.params.ampAttack = 0.002f; p.params.ampDecay = 0.35f;
        bank[77] = p;
    }
    {
        auto p = makeBase(78, "078: Screaming Resonance", PresetCategory::Lead);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lpfCutoff = 2200.0f; p.params.lpfResonance = 0.85f; // On the edge of self-oscillation
        p.params.distortion = 0.45f;
        bank[78] = p;
    }
    {
        auto p = makeBase(79, "079: Slop Detune Bass", PresetCategory::Bass);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.slop = 0.85f;
        p.params.lpfCutoff = 650.0f; p.params.lpfEnvAmount = 0.65f;
        bank[79] = p;
    }

    // =========================================================================
    // 080–099: Poly Mod Sci-Fi, Arps & Sound FX
    // =========================================================================
    {
        auto p = makeBase(80, "080: Metallic FM Chimes", PresetCategory::FX);
        p.params.osc1Shape = 0.0f; // Tri
        p.params.osc2Shape = 0.5f; p.params.osc2Octave = 1; p.params.osc2FineTune = 3.5f; // Non-harmonic ratio
        p.params.polyModOsc2Amt = 0.55f; p.params.polyModDestOsc1Freq = true;
        p.params.lpfCutoff = 7000.0f; p.params.filtAttack = 0.005f; p.params.filtDecay = 1.5f;
        p.params.fx2Type = 7; p.params.fx2Mix = 0.45f;
        bank[80] = p;
    }
    {
        auto p = makeBase(81, "081: Alien Bell", PresetCategory::FX);
        p.params.osc1Shape = 0.0f; p.params.osc2Shape = 0.5f; p.params.osc2Octave = 2; p.params.osc2FineTune = 5.2f;
        p.params.polyModOsc2Amt = 0.6f; p.params.polyModDestOsc1Freq = true;
        p.params.fx1Type = 6; p.params.fx1Param1 = 0.6f; p.params.fx1Mix = 0.4f; // Ring Mod
        p.params.fx2Type = 7; p.params.fx2Mix = 0.5f;
        bank[81] = p;
    }
    {
        auto p = makeBase(82, "082: Retro Tape Arp", PresetCategory::Arp);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.75f;
        p.params.lpfCutoff = 2500.0f; p.params.lpfResonance = 0.4f; p.params.lpfEnvAmount = 0.7f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.25f;
        p.arpEnabled = true; p.arpMode = ArpMode::UpDown; p.arpDivision = ClockDivision::Div_1_16;
        p.params.fx1Type = 2; p.params.fx1Param1 = 0.35f; p.params.fx1Param2 = 0.6f; p.params.fx1Mix = 0.45f; // Tape delay
        bank[82] = p;
    }
    {
        auto p = makeBase(83, "083: Sci-Fi Drone", PresetCategory::FX);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.polyModOsc2Amt = 0.75f; p.params.polyModDestLPF = true;
        p.params.lfoFrequency = 0.15f; p.params.lfoInitialAmount = 0.5f; p.params.lfoDestOsc1PW = true;
        p.params.lpfCutoff = 1800.0f; p.params.lpfResonance = 0.7f;
        bank[83] = p;
    }
    {
        auto p = makeBase(84, "084: Ring Mod Clang", PresetCategory::FX);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.fx1Type = 6; p.params.fx1Param1 = 0.45f; p.params.fx1Param2 = 0.7f; p.params.fx1Mix = 0.7f; // Ring Mod
        bank[84] = p;
    }
    {
        auto p = makeBase(85, "085: 16th Note Clock Arp", PresetCategory::Arp);
        p.params.osc1Shape = 0.75f; p.params.osc1PulseWidth = 0.3f;
        p.params.lpfCutoff = 3500.0f; p.params.lpfEnvAmount = 0.65f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.2f;
        p.arpEnabled = true; p.arpMode = ArpMode::Up; p.arpDivision = ClockDivision::Div_1_16;
        p.params.fx1Type = 1; p.params.fx1Mix = 0.35f;
        bank[85] = p;
    }
    {
        auto p = makeBase(86, "086: Thunder Explosion FX", PresetCategory::FX);
        p.params.osc1Level = 0.0f; p.params.osc2Level = 0.0f;
        p.params.noiseLevel = 1.0f; // Pure noise source
        p.params.lpfCutoff = 600.0f; p.params.lpfResonance = 0.5f; p.params.lpfEnvAmount = 0.8f;
        p.params.filtAttack = 0.01f; p.params.filtDecay = 3.0f; p.params.filtSustain = 0.0f;
        p.params.ampAttack = 0.01f; p.params.ampDecay = 3.5f; p.params.ampSustain = 0.0f;
        p.params.distortion = 0.5f; p.params.fx2Type = 8; p.params.fx2Mix = 0.7f;
        bank[86] = p;
    }
    {
        auto p = makeBase(87, "087: Laser Gun Sweeper", PresetCategory::FX);
        p.params.osc1Shape = 0.5f;
        p.params.polyModFiltEnvAmt = -0.8f; p.params.polyModDestOsc1Freq = true; // Rapid pitch dive
        p.params.filtAttack = 0.002f; p.params.filtDecay = 0.15f;
        p.params.lpfCutoff = 8000.0f;
        bank[87] = p;
    }
    {
        auto p = makeBase(88, "088: Space Wind Noise", PresetCategory::FX);
        p.params.osc1Level = 0.0f; p.params.osc2Level = 0.0f; p.params.noiseLevel = 0.9f;
        p.params.hpfCutoff = 350.0f; p.params.hpfResonance = 0.6f;
        p.params.lpfCutoff = 1800.0f; p.params.lpfResonance = 0.6f;
        p.params.lfoFrequency = 0.2f; p.params.lfoInitialAmount = 0.4f; p.params.lfoDestLPF = true;
        p.params.fx2Type = 8; p.params.fx2Mix = 0.6f;
        bank[88] = p;
    }
    {
        auto p = makeBase(89, "089: Random S&H Modular", PresetCategory::FX);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lfoShape = LfoWaveform::Random; p.params.lfoFrequency = 6.0f;
        p.params.lfoInitialAmount = 0.75f; p.params.lfoDestLPF = true;
        p.params.lpfCutoff = 2200.0f; p.params.lpfResonance = 0.7f;
        bank[89] = p;
    }
    {
        auto p = makeBase(90, "090: BBD Space Repeats", PresetCategory::FX);
        p.params.osc1Shape = 0.5f;
        p.params.lpfCutoff = 2500.0f; p.params.filtAttack = 0.005f; p.params.filtDecay = 0.2f;
        p.params.fx1Type = 0; p.params.fx1Param1 = 0.45f; p.params.fx1Param2 = 0.85f; p.params.fx1Mix = 0.7f; // High feedback
        bank[90] = p;
    }
    {
        auto p = makeBase(91, "091: Robot Vocalizer", PresetCategory::FX);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2Octave = -1;
        p.params.polyModOsc2Amt = 0.65f; p.params.polyModDestLPF = true; p.params.polyModDestHPF = true;
        p.params.lpfCutoff = 2200.0f; p.params.hpfCutoff = 400.0f;
        bank[91] = p;
    }
    {
        auto p = makeBase(92, "092: Dark Nebula Drone", PresetCategory::FX);
        p.params.osc1Shape = 0.1f; p.params.osc1SubLevel = 0.8f;
        p.params.osc2Shape = 0.5f; p.params.osc2LowFreq = true; p.params.osc2FineTune = 1.0f;
        p.params.polyModOsc2Amt = 0.4f; p.params.polyModDestLPF = true;
        p.params.lpfCutoff = 600.0f; p.params.lpfResonance = 0.75f;
        bank[92] = p;
    }
    {
        auto p = makeBase(93, "093: Crystal Glass FM", PresetCategory::FX);
        p.params.osc1Shape = 0.0f; p.params.osc2Shape = 0.0f; p.params.osc2Octave = 2; p.params.osc2FineTune = 7.0f;
        p.params.polyModOsc2Amt = 0.45f; p.params.polyModDestOsc1Freq = true;
        p.params.lpfCutoff = 8000.0f; p.params.fx2Type = 7; p.params.fx2Mix = 0.5f;
        bank[93] = p;
    }
    {
        auto p = makeBase(94, "094: Staccato Ping Arp", PresetCategory::Arp);
        p.params.osc1Shape = 0.8f; p.params.osc1PulseWidth = 0.2f;
        p.params.lpfCutoff = 4000.0f; p.params.lpfEnvAmount = 0.7f;
        p.params.filtAttack = 0.002f; p.params.filtDecay = 0.15f;
        p.arpEnabled = true; p.arpMode = ArpMode::Random; p.arpDivision = ClockDivision::Div_1_16;
        p.params.fx1Type = 1; p.params.fx1Param1 = 0.375f; p.params.fx1Param2 = 0.55f; p.params.fx1Mix = 0.45f;
        bank[94] = p;
    }
    {
        auto p = makeBase(95, "095: Industrial Metallic FX", PresetCategory::FX);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f; p.params.osc2FineTune = 4.2f;
        p.params.fx1Type = 6; p.params.fx1Param1 = 0.55f; p.params.fx1Mix = 0.6f; // Ring Mod
        p.params.distortion = 0.7f;
        bank[95] = p;
    }
    {
        auto p = makeBase(96, "096: Cosmic Beam Pluck", PresetCategory::Arp);
        p.params.osc1Shape = 0.5f; p.params.osc1SubLevel = 0.4f;
        p.params.lpfCutoff = 3200.0f; p.params.lpfEnvAmount = 0.6f;
        p.arpEnabled = true; p.arpMode = ArpMode::UpDown; p.arpDivision = ClockDivision::Div_1_16;
        p.params.fx1Type = 5; p.params.fx1Mix = 0.4f; // Phaser
        bank[96] = p;
    }
    {
        auto p = makeBase(97, "097: Tape Flutter Bells", PresetCategory::FX);
        p.params.osc1Shape = 0.0f; p.params.osc2Shape = 0.0f; p.params.osc2Octave = 1;
        p.params.polyModOsc2Amt = 0.35f; p.params.polyModDestOsc1Freq = true;
        p.params.fx1Type = 2; p.params.fx1Param1 = 0.4f; p.params.fx1Param2 = 0.6f; p.params.fx1Mix = 0.5f; // Tape delay
        bank[97] = p;
    }
    {
        auto p = makeBase(98, "098: Phaser Jet Swirl", PresetCategory::FX);
        p.params.osc1Shape = 0.5f; p.params.osc2Shape = 0.5f;
        p.params.lpfCutoff = 6000.0f;
        p.params.fx1Type = 5; p.params.fx1Param1 = 0.25f; p.params.fx1Param2 = 0.8f; p.params.fx1Mix = 0.65f; // Deep Phaser
        p.params.distortion = 0.35f;
        bank[98] = p;
    }
    {
        auto p = makeBase(99, "099: Prophet Initialization", PresetCategory::Brass);
        // Pure raw initialization sound
        p.params.osc1Shape = 0.5f; p.params.osc1Level = 0.8f;
        p.params.osc2Shape = 0.5f; p.params.osc2Level = 0.8f; p.params.osc2FineTune = 0.0f;
        p.params.lpfCutoff = 20000.0f; p.params.lpfResonance = 0.0f; p.params.lpfEnvAmount = 0.0f;
        p.params.hpfCutoff = 20.0f; p.params.hpfResonance = 0.0f;
        p.params.filtAttack = 0.005f; p.params.filtDecay = 0.5f; p.params.filtSustain = 1.0f; p.params.filtRelease = 0.2f;
        p.params.ampAttack = 0.005f; p.params.ampDecay = 0.5f; p.params.ampSustain = 1.0f; p.params.ampRelease = 0.2f;
        p.params.fxOn = false; // True Bypass for raw dry initialization
        p.params.slop = 0.0f;
        p.params.distortion = 0.0f;
        bank[99] = p;
    }

    return bank;
}

const std::array<Preset, 100>& getStaticPresets() noexcept {
    static const std::array<Preset, 100> sPresets = createAllFactoryPresets();
    return sPresets;
}

} // namespace

const std::array<Preset, FactoryPresets::NUM_FACTORY_PRESETS>& FactoryPresets::getAllPresets() noexcept {
    return getStaticPresets();
}

const Preset& FactoryPresets::getPreset(size_t index) noexcept {
    const auto& all = getStaticPresets();
    return all[index % NUM_FACTORY_PRESETS];
}

} // namespace prophet6
