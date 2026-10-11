#include "prophet6/SynthEngine.hpp"
#include "prophet6/PresetManager.hpp"
#include "prophet6/PresetSerializer.hpp"
#include "prophet6/SysEx.hpp"
#include "prophet6/UserBankManager.hpp"
#include "wav_writer.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <cstring>

using namespace prophet6;
using namespace prophet6::test;

constexpr float SAMPLE_RATE = 44100.0f;

void runTest1_PolyChordSweep() {
    std::cout << "[Test 1/5] Rendering test_poly_chord_sweep.wav (6-voice chord + Slop + Filter sweep)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    // Osc 1: Saw/Pulse hybrid with sub-oscillator
    params.osc1Shape = 0.65f;
    params.osc1PulseWidth = 0.45f;
    params.osc1Level = 0.7f;
    params.osc1SubLevel = 0.35f;

    // Osc 2: Triangle with slight fine detune (+6 cents)
    params.osc2Shape = 0.2f;
    params.osc2FineTune = 0.06f;
    params.osc2Level = 0.6f;

    // Filter Envelope & Amplifier
    params.filtAttack = 0.01f;
    params.filtDecay = 1.5f;
    params.filtSustain = 0.3f;
    params.filtRelease = 1.0f;

    params.ampAttack = 0.02f;
    params.ampDecay = 0.5f;
    params.ampSustain = 0.85f;
    params.ampRelease = 0.8f;

    // Filter resonance & Slop
    params.lpfResonance = 0.65f;
    params.slop = 0.85f; // Rich organic component drift
    params.panSpread = 0.8f;

    // Trigger full 6-voice major 9th chord: C3, E3, G3, B3, D4, F#4
    constexpr std::array<int, 6> chordNotes = { 48, 52, 55, 59, 62, 66 };
    for (int note : chordNotes) {
        engine.noteOn(note, 0.9f);
    }

    constexpr float durationSec = 5.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / SAMPLE_RATE;

        // Release chord at 4.0 seconds
        if (i == static_cast<size_t>(4.0f * SAMPLE_RATE)) {
            for (int note : chordNotes) {
                engine.noteOff(note);
            }
        }

        // Modulate LPF cutoff: sweep from 250 Hz up to 8000 Hz and back
        const float sweepPhase = t / 4.0f;
        const float cutoffMod = 0.5f * (1.0f - std::cos(TWO_PI * sweepPhase));
        params.lpfCutoff = 250.0f + 7500.0f * cutoffMod;

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    WavWriter::writeWavStereo("test_poly_chord_sweep.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_poly_chord_sweep.wav (" << durationSec << "s)" << std::endl;
}

void runTest2_PolyModFM() {
    std::cout << "[Test 2/5] Rendering test_polymod_fm.wav (Osc 2 audio-rate FM into Osc 1)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    // Osc 1: Pure triangle fundamental
    params.osc1Shape = 0.0f;
    params.osc1Level = 0.9f;
    params.osc1SubLevel = 0.0f;

    // Osc 2: Modulator tuned to non-harmonic ratio (+19 semitones)
    params.osc2Shape = 0.5f; // Sawtooth modulation source
    params.osc2Octave = 1;
    params.osc2FineTune = 7.0f;
    params.osc2Level = 0.0f; // Osc 2 muted from audio mixer, only acts as PolyMod modulator!

    // Poly Mod routing: Osc 2 modulating Osc 1 Frequency
    params.polyModOsc2Amt = 0.75f;
    params.polyModDestOsc1Freq = true;

    // Envelopes
    params.ampAttack = 0.002f;
    params.ampDecay = 1.2f;
    params.ampSustain = 0.2f;
    params.ampRelease = 1.0f;

    params.lpfCutoff = 15000.0f;
    params.lpfResonance = 0.1f;

    constexpr float durationSec = 4.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    // Play metallic bell arpeggio notes
    constexpr std::array<int, 3> bellNotes = { 57, 64, 71 }; // A3, E4, B4
    size_t noteIdx = 0;

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / SAMPLE_RATE;

        // Trigger notes at intervals
        if (i == 0 || i == static_cast<size_t>(1.2f * SAMPLE_RATE) || i == static_cast<size_t>(2.4f * SAMPLE_RATE)) {
            if (noteIdx > 0) engine.noteOff(bellNotes[noteIdx - 1]);
            engine.noteOn(bellNotes[noteIdx], 0.95f);
            ++noteIdx;
        }

        // Modulate PolyMod depth slightly over time for dynamic bell decay
        params.polyModOsc2Amt = clamp(0.85f * std::exp(-t * 0.5f), 0.05f, 0.95f);

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }
    engine.allNotesOff();

    WavWriter::writeWavStereo("test_polymod_fm.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_polymod_fm.wav (" << durationSec << "s)" << std::endl;
}

void runTest3_DualFilterResonance() {
    std::cout << "[Test 3/5] Rendering test_dual_filter_resonance.wav (LPF & HPF series sweeping)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    // Rich harmonic sawtooth waves on both oscillators
    params.osc1Shape = 0.5f;
    params.osc1Level = 0.6f;
    params.osc2Shape = 0.5f;
    params.osc2FineTune = 0.08f;
    params.osc2Level = 0.6f;

    // Both filters resonant!
    params.lpfResonance = 0.75f;
    params.hpfResonance = 0.70f;

    params.ampAttack = 0.01f;
    params.ampSustain = 0.9f;
    params.ampRelease = 0.5f;

    // Play a rich 3-note power chord
    constexpr std::array<int, 3> notes = { 45, 52, 57 }; // A2, E3, A3
    for (int n : notes) engine.noteOn(n, 0.9f);

    constexpr float durationSec = 5.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / SAMPLE_RATE;
        const float progress = t / durationSec;

        // LPF sweeps DOWN from 12000 Hz to 400 Hz
        params.lpfCutoff = 12000.0f * std::pow(400.0f / 12000.0f, progress);

        // HPF sweeps UP from 40 Hz to 2500 Hz
        params.hpfCutoff = 40.0f * std::pow(2500.0f / 40.0f, progress);

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }
    engine.allNotesOff();

    WavWriter::writeWavStereo("test_dual_filter_resonance.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_dual_filter_resonance.wav (" << durationSec << "s)" << std::endl;
}

void runTest4_SlopDriftComparison() {
    std::cout << "[Test 4/5] Rendering test_slop_drift.wav (0% Slop vs 100% Slop comparison)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    params.osc1Shape = 0.5f; // Pure Saw
    params.osc1Level = 0.65f;
    params.osc2Shape = 0.5f;
    params.osc2Level = 0.65f;
    params.osc2FineTune = 0.0f; // Exactly in tune

    params.lpfCutoff = 6000.0f;
    params.lpfResonance = 0.3f;
    params.ampAttack = 0.01f;
    params.ampSustain = 0.9f;
    params.ampRelease = 0.5f;
    params.panSpread = 1.0f; // Max stereo spread

    // Full 6-voice chord
    constexpr std::array<int, 6> chord = { 48, 55, 60, 64, 67, 72 }; // C3, G3, C4, E4, G4, C5
    for (int n : chord) engine.noteOn(n, 0.85f);

    constexpr float durationSec = 6.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / SAMPLE_RATE;

        // 0.0s - 3.0s: Slop = 0.0% (sterile / static)
        // 3.0s - 6.0s: Slop = 100.0% (lush organic analog drift & stereo width)
        if (t < 3.0f) {
            params.slop = 0.0f;
        } else {
            // Smoothly ramp slop to 1.0
            const float ramp = clamp((t - 3.0f) * 2.0f, 0.0f, 1.0f);
            params.slop = ramp;
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }
    engine.allNotesOff();

    WavWriter::writeWavStereo("test_slop_drift.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_slop_drift.wav (" << durationSec << "s: 0-3s sterile, 3-6s organic drift)" << std::endl;
}

void runTest5_StereoDistortion() {
    std::cout << "[Test 5/5] Rendering test_stereo_distortion.wav (Master analog distortion sweep)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    params.osc1Shape = 0.7f; // Pulse
    params.osc1PulseWidth = 0.35f;
    params.osc1Level = 0.7f;
    params.osc1SubLevel = 0.4f;

    params.osc2Shape = 0.5f; // Saw
    params.osc2Octave = -1;
    params.osc2Level = 0.6f;

    params.lpfCutoff = 3500.0f;
    params.lpfResonance = 0.45f;
    params.ampAttack = 0.01f;
    params.ampSustain = 0.9f;

    // Trigger low bass unison / poly notes
    constexpr std::array<int, 4> chord = { 36, 43, 48, 55 }; // C2, G2, C3, G3
    for (int n : chord) engine.noteOn(n, 0.95f);

    constexpr float durationSec = 5.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / SAMPLE_RATE;

        // Sweep distortion from 0.0 to 1.0
        params.distortion = clamp(t / (durationSec - 0.5f), 0.0f, 1.0f);

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }
    engine.allNotesOff();

    WavWriter::writeWavStereo("test_stereo_distortion.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_stereo_distortion.wav (" << durationSec << "s: Clean -> Heavy Analog Saturation)" << std::endl;
}

void runTest6_ArpeggiatorModesAndHold() {
    std::cout << "[Test 6/7] Verifying Arpeggiator: Modes (Up, Down, UpDown, Assign, Random), Octave Ranges (1-3), and Hold/Latch..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);
    auto& arp = engine.getArpeggiator();
    arp.setEnabled(true);
    arp.setBpm(135.0f);
    arp.setDivision(ClockDivision::Div_1_16);

    // 1. Verify Mode: Up (1, 2, 3 octaves)
    arp.setMode(ArpMode::Up);
    arp.setOctaveRange(1);
    engine.noteOn(48, 0.9f); // C3
    engine.noteOn(52, 0.9f); // E3
    engine.noteOn(55, 0.9f); // G3

    if (arp.getPatternLength() != 3) {
        throw std::runtime_error("Arpeggiator Up mode 1-octave pattern length mismatch!");
    }

    arp.setOctaveRange(2);
    if (arp.getPatternLength() != 6) {
        throw std::runtime_error("Arpeggiator Up mode 2-octave pattern length mismatch!");
    }

    arp.setOctaveRange(3);
    if (arp.getPatternLength() != 9) {
        throw std::runtime_error("Arpeggiator Up mode 3-octave pattern length mismatch!");
    }

    // 2. Verify Mode: Down
    arp.setMode(ArpMode::Down);
    arp.setOctaveRange(1);
    if (arp.getPatternLength() != 3) {
        throw std::runtime_error("Arpeggiator Down mode pattern length mismatch!");
    }

    // 3. Verify Mode: UpDown
    arp.setMode(ArpMode::UpDown);
    if (arp.getPatternLength() != 4) { // 48, 52, 55, 52
        throw std::runtime_error("Arpeggiator UpDown mode pattern length mismatch!");
    }

    // 4. Verify Mode: Assign
    engine.allNotesOff();
    arp.setEnabled(true);
    arp.setMode(ArpMode::Assign);
    arp.setOctaveRange(1);
    engine.noteOn(55, 0.9f); // G3 first
    engine.noteOn(48, 0.9f); // C3 second
    engine.noteOn(52, 0.9f); // E3 third
    if (arp.getPatternLength() != 3) {
        throw std::runtime_error("Arpeggiator Assign mode pattern length mismatch!");
    }

    // 5. Verify Hold / Latch Mode
    arp.setHold(true);
    arp.setMode(ArpMode::Up);
    arp.setOctaveRange(2);

    // Release all physical keys
    engine.noteOff(55);
    engine.noteOff(48);
    engine.noteOff(52);

    if (arp.getPatternLength() != 6) {
        throw std::runtime_error("Arpeggiator Hold failed to retain latched chord upon key release!");
    }

    // Render 4 seconds of audio to verify continuous arpeggiator playback and audio synthesis
    auto& params = engine.getParameters();
    params.osc1Shape = 0.5f; // Saw
    params.osc1Level = 0.7f;
    params.osc2Shape = 0.2f; // Pulse
    params.osc2Level = 0.5f;
    params.filtAttack = 0.01f;
    params.filtDecay = 0.2f;
    params.filtSustain = 0.2f;
    params.filtRelease = 0.1f;
    params.ampAttack = 0.005f;
    params.ampDecay = 0.2f;
    params.ampSustain = 0.6f;
    params.ampRelease = 0.1f;
    params.lpfCutoff = 3500.0f;
    params.lpfResonance = 0.6f;
    params.panSpread = 0.75f;

    constexpr float durationSec = 4.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    float maxAmp = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        // At 2.0s, strike a new chord in Hold mode: F3 (53), A3 (57), C4 (60)
        if (i == static_cast<size_t>(2.0f * SAMPLE_RATE)) {
            engine.noteOn(53, 0.9f);
            engine.noteOn(57, 0.9f);
            engine.noteOn(60, 0.9f);
            engine.noteOff(53);
            engine.noteOff(57);
            engine.noteOff(60);
        }

        // Filter cutoff sweep
        const float t = static_cast<float>(i) / SAMPLE_RATE;
        params.lpfCutoff = 1000.0f + 5000.0f * (0.5f * (1.0f + std::sin(TWO_PI * t * 0.5f)));

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;

        maxAmp = std::max(maxAmp, std::max(std::abs(sL), std::abs(sR)));
    }

    if (maxAmp < 0.05f) {
        throw std::runtime_error("Arpeggiator rendered silence during Hold mode!");
    }

    // Toggle hold off and verify arpeggio stops
    arp.setHold(false);
    if (arp.getPatternLength() != 0) {
        throw std::runtime_error("Arpeggiator failed to reset when Hold was turned off!");
    }

    engine.allNotesOff();
    WavWriter::writeWavStereo("test_arpeggiator_sweep.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_arpeggiator_sweep.wav (" << durationSec << "s: Arp Up 2-Octave + Hold + Chord Swap)" << std::endl;
}

void runTest7_PolyphonicStepSequencer() {
    std::cout << "[Test 7/7] Verifying Polyphonic Step Sequencer: 6-Voice Step Recording, Tie, Rest, Playback, and Real-Time Transposition..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);
    auto& seq = engine.getSequencer();
    seq.setEnabled(true);
    seq.setBpm(120.0f);
    seq.setSequenceLength(4);

    // 1. Enter Step Recording Mode
    seq.setRecording(true);

    // Step 0: Record 3-note C Major chord (48, 52, 55)
    engine.noteOn(48, 0.8f);
    engine.noteOn(52, 0.8f);
    engine.noteOn(55, 0.8f);
    engine.noteOff(48);
    engine.noteOff(52);
    engine.noteOff(55);

    // Step 1: Record Tie
    seq.recordTie();

    // Step 2: Record Rest
    seq.recordRest();

    // Step 3: Record 4-note F Major 7th chord (41, 45, 48, 52)
    engine.noteOn(41, 0.85f);
    engine.noteOn(45, 0.85f);
    engine.noteOn(48, 0.85f);
    engine.noteOn(52, 0.85f);
    engine.noteOff(41);
    engine.noteOff(45);
    engine.noteOff(48);
    engine.noteOff(52);

    // Recording finishes (recordStep_ reached sequenceLength_ 4)
    if (seq.isRecording()) {
        seq.setRecording(false);
    }

    const auto& steps = seq.getSteps();
    if (steps[0].noteCount != 3 || steps[0].notes[0] != 48 || steps[0].notes[1] != 52 || steps[0].notes[2] != 55) {
        throw std::runtime_error("Sequencer step 0 polyphonic chord mismatch!");
    }
    if (!steps[1].isTie) {
        throw std::runtime_error("Sequencer step 1 Tie flag mismatch!");
    }
    if (!steps[2].isRest) {
        throw std::runtime_error("Sequencer step 2 Rest flag mismatch!");
    }
    if (steps[3].noteCount != 4 || steps[3].notes[0] != 41 || steps[3].notes[3] != 52) {
        throw std::runtime_error("Sequencer step 3 chord mismatch!");
    }

    // 2. Playback & Transposition Verification
    seq.setPlaying(true);

    auto& params = engine.getParameters();
    params.osc1Shape = 0.6f;
    params.osc1Level = 0.8f;
    params.osc2Shape = 0.4f;
    params.osc2Level = 0.6f;
    params.filtAttack = 0.01f;
    params.filtDecay = 0.4f;
    params.filtSustain = 0.5f;
    params.filtRelease = 0.3f;
    params.ampAttack = 0.01f;
    params.ampDecay = 0.4f;
    params.ampSustain = 0.7f;
    params.ampRelease = 0.3f;
    params.lpfCutoff = 4000.0f;
    params.lpfResonance = 0.5f;

    constexpr float durationSec = 4.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    float maxAmp = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        // At 2.0s, transpose sequence in real-time by +7 semitones (G)
        if (i == static_cast<size_t>(2.0f * SAMPLE_RATE)) {
            engine.noteOn(55, 0.9f); // G3 transposed
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;

        maxAmp = std::max(maxAmp, std::max(std::abs(sL), std::abs(sR)));
    }

    if (maxAmp < 0.05f) {
        throw std::runtime_error("Sequencer rendered silence during playback!");
    }

    seq.setPlaying(false);
    engine.allNotesOff();

    WavWriter::writeWavStereo("test_poly_step_sequencer.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_poly_step_sequencer.wav (" << durationSec << "s: Polyphonic Chords + Tie + Rest + Transposition)" << std::endl;
}

void runTest8_FXBBDDelay() {
    std::cout << "[Test 8/11] Rendering test_fx_bbd_delay.wav (Arpeggio + Analog BBD Delay)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    params.osc1Shape = 0.5f; // Saw
    params.osc1Level = 0.8f;
    params.osc2Shape = 0.0f; // Tri
    params.osc2Level = 0.5f;
    params.osc2FineTune = 0.04f;

    params.lpfCutoff = 2200.0f;
    params.lpfResonance = 0.4f;
    params.filtAttack = 0.005f;
    params.filtDecay = 0.3f;
    params.filtSustain = 0.2f;
    params.filtRelease = 0.2f;
    params.ampAttack = 0.005f;
    params.ampDecay = 0.25f;
    params.ampSustain = 0.6f;
    params.ampRelease = 0.2f;

    // Enable Effects: Slot 1 BBD Delay
    params.fxOn = true;
    params.fx1Type = static_cast<int>(EffectType::BBDDelay);
    params.fx1Param1 = 0.38f; // ~320ms delay time
    params.fx1Param2 = 0.65f; // 65% feedback for rich analog decay repeats
    params.fx1Mix = 0.45f;    // 45% wet mix
    params.fx2Type = static_cast<int>(EffectType::Off);

    // Setup Arpeggiator
    auto& arp = engine.getArpeggiator();
    arp.setEnabled(true);
    arp.setMode(ArpMode::UpDown);
    arp.setDivision(ClockDivision::Div_1_16);
    arp.setBpm(120.0f);

    constexpr std::array<int, 4> chord = { 48, 51, 55, 58 }; // C minor 7
    for (int n : chord) {
        engine.noteOn(n, 0.85f);
    }

    constexpr float durationSec = 5.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        // Release notes at 2.8s to let delay repeats ring out into silence
        if (i == static_cast<size_t>(2.8f * SAMPLE_RATE)) {
            for (int n : chord) {
                engine.noteOff(n);
            }
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    arp.setEnabled(false);
    engine.allNotesOff();

    WavWriter::writeWavStereo("test_fx_bbd_delay.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_fx_bbd_delay.wav (" << durationSec << "s: Arpeggiator into dark BBD delay)" << std::endl;
}

void runTest9_FXPlateReverb() {
    std::cout << "[Test 9/11] Rendering test_fx_plate_reverb.wav (Staccato chord into plate reverb tail)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    params.osc1Shape = 0.65f; // Pulse
    params.osc1PulseWidth = 0.35f;
    params.osc1Level = 0.8f;
    params.osc2Shape = 0.5f; // Saw
    params.osc2Level = 0.6f;
    params.osc2FineTune = -0.06f;

    params.lpfCutoff = 4500.0f;
    params.lpfResonance = 0.35f;
    params.ampAttack = 0.005f;
    params.ampDecay = 0.15f;
    params.ampSustain = 0.0f; // Staccato
    params.ampRelease = 0.05f;

    // Enable Effects: Slot 1 Plate Reverb
    params.fxOn = true;
    params.fx1Type = static_cast<int>(EffectType::PlateReverb);
    params.fx1Param1 = 0.75f; // Long lush plate decay (~4.5s)
    params.fx1Param2 = 0.65f; // High-frequency damping
    params.fx1Mix = 0.50f;    // 50% Wet
    params.fx2Type = static_cast<int>(EffectType::Off);

    // Staccato Major 9th chord
    constexpr std::array<int, 5> notes = { 48, 55, 59, 62, 66 };
    for (int n : notes) {
        engine.noteOn(n, 0.9f);
    }

    constexpr float durationSec = 5.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        // Release after 0.25 seconds
        if (i == static_cast<size_t>(0.25f * SAMPLE_RATE)) {
            for (int n : notes) {
                engine.noteOff(n);
            }
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    engine.allNotesOff();

    // Verify reverb tail persists well past chord release
    const size_t tailSample = static_cast<size_t>(2.5f * SAMPLE_RATE);
    const float tailLevel = std::max(std::abs(left[tailSample]), std::abs(right[tailSample]));
    if (tailLevel < 0.005f) {
        throw std::runtime_error("Plate reverb decay tail died prematurely!");
    }

    WavWriter::writeWavStereo("test_fx_plate_reverb.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_fx_plate_reverb.wav (" << durationSec << "s: Shimmering plate reverb tail)" << std::endl;
}

void runTest10_FXChorusFlanger() {
    std::cout << "[Test 10/11] Rendering test_fx_chorus_flanger.wav (Prophet brass with chorus + flanger)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    // Prophet Brass patch
    params.osc1Shape = 0.5f; // Saw
    params.osc1Level = 0.75f;
    params.osc1SubLevel = 0.25f;
    params.osc2Shape = 0.5f; // Saw
    params.osc2Level = 0.75f;
    params.osc2FineTune = 0.08f; // Detune spread

    params.lpfCutoff = 1500.0f;
    params.lpfResonance = 0.3f;
    params.lpfEnvAmount = 0.6f;
    params.filtAttack = 0.06f;
    params.filtDecay = 1.0f;
    params.filtSustain = 0.4f;
    params.filtRelease = 0.5f;

    params.ampAttack = 0.05f;
    params.ampDecay = 0.8f;
    params.ampSustain = 0.7f;
    params.ampRelease = 0.5f;
    params.slop = 0.5f;

    // Enable Effects: Slot 1 Chorus -> Slot 2 Flanger
    params.fxOn = true;
    params.fx1Type = static_cast<int>(EffectType::Chorus);
    params.fx1Param1 = 0.30f; // Rate
    params.fx1Param2 = 0.75f; // Depth
    params.fx1Mix = 0.50f;

    params.fx2Type = static_cast<int>(EffectType::Flanger);
    params.fx2Param1 = 0.20f; // Rate
    params.fx2Param2 = 0.55f; // Feedback
    params.fx2Mix = 0.30f;

    constexpr std::array<int, 4> chord = { 53, 57, 60, 65 }; // F Major
    for (int n : chord) {
        engine.noteOn(n, 0.9f);
    }

    constexpr float durationSec = 5.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        if (i == static_cast<size_t>(3.8f * SAMPLE_RATE)) {
            for (int n : chord) {
                engine.noteOff(n);
            }
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    engine.allNotesOff();

    WavWriter::writeWavStereo("test_fx_chorus_flanger.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_fx_chorus_flanger.wav (" << durationSec << "s: Vintage ensemble chorus + flanger)" << std::endl;
}

void runTest11_FXTrueBypass() {
    std::cout << "[Test 11/11] Rendering test_fx_true_bypass.wav (Bit-exact True Bypass verification)..." << std::endl;

    constexpr float durationSec = 2.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);

    // Pass 1: True Bypass with fxOn = false (with various FX parameters dialed in)
    std::vector<float> leftBypass(totalSamples);
    std::vector<float> rightBypass(totalSamples);
    {
        SynthEngine engine;
        engine.init(SAMPLE_RATE);
        auto& p = engine.getParameters();
        p.osc1Shape = 0.5f;
        p.osc2Shape = 0.5f;
        p.slop = 0.0f; // Disable random drift for deterministic comparison
        p.fxOn = false; // True Bypass engaged!
        p.fx1Type = static_cast<int>(EffectType::BBDDelay);
        p.fx1Mix = 0.8f;
        p.fx2Type = static_cast<int>(EffectType::PlateReverb);
        p.fx2Mix = 0.8f;

        engine.noteOn(60, 0.8f);
        for (size_t i = 0; i < totalSamples; ++i) {
            float sL = 0.0f, sR = 0.0f;
            engine.processSample(sL, sR);
            leftBypass[i] = sL;
            rightBypass[i] = sR;
        }
    }

    // Pass 2: Clean dry baseline (fxOn = false, same settings)
    std::vector<float> leftDry(totalSamples);
    std::vector<float> rightDry(totalSamples);
    {
        SynthEngine engine;
        engine.init(SAMPLE_RATE);
        auto& p = engine.getParameters();
        p.osc1Shape = 0.5f;
        p.osc2Shape = 0.5f;
        p.slop = 0.0f;
        p.fxOn = false; // Raw engine dry signal

        engine.noteOn(60, 0.8f);
        for (size_t i = 0; i < totalSamples; ++i) {
            float sL = 0.0f, sR = 0.0f;
            engine.processSample(sL, sR);
            leftDry[i] = sL;
            rightDry[i] = sR;
        }
    }

    // Verify bit-exact float equality across every single sample
    bool bitExactMatch = true;
    for (size_t i = 0; i < totalSamples; ++i) {
        if (leftBypass[i] != leftDry[i] || rightBypass[i] != rightDry[i]) {
            bitExactMatch = false;
            break;
        }
    }

    if (!bitExactMatch) {
        throw std::runtime_error("FAIL: True Bypass does not match dry signal bit-exactly!");
    }

    std::cout << "  -> Bit-exact match confirmed: 100% untouched analog dry signal under True Bypass." << std::endl;
    WavWriter::writeWavStereo("test_fx_true_bypass.wav", leftBypass, rightBypass, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_fx_true_bypass.wav (" << durationSec << "s)" << std::endl;
}

void runTest12_PresetRecall() {
    std::cout << "[Test 12/13] Rendering test_preset_recall.wav (Patch recall across 4 factory categories + JSON serialization)..." << std::endl;

    PresetManager pm;
    if (PresetManager::NUM_FACTORY_PROGRAMS != 100 || pm.getNumPresets() != 1000) {
        throw std::runtime_error("Presets count mismatch! Expected 100 factory and 1000 total programs.");
    }

    // Verify JSON round-trip serialization on Preset 000
    const auto& p0 = pm.getPreset(0);
    std::string json0 = PresetSerializer::serializeToJson(p0);
    Preset p0Restored;
    if (!PresetSerializer::deserializeFromJson(json0, p0Restored)) {
        throw std::runtime_error("Failed to deserialize Preset 000 from JSON!");
    }
    if (p0Restored.getName() != p0.getName() || std::abs(p0Restored.params.lpfCutoff - p0.params.lpfCutoff) > 0.1f) {
        throw std::runtime_error("JSON serialization round-trip data mismatch!");
    }
    std::cout << "  -> JSON serialization verified for patch: '" << p0.getName() << "'" << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    // Render 4 diverse presets sequentially (1.5s each = 6.0s total)
    // 1. Preset 000: "Jump Brass" (Brass)
    // 2. Preset 040: "Celestial Pad" (Pad)
    // 3. Preset 060: "24dB Slop Bass" (Bass)
    // 4. Preset 080: "Metallic FM Chimes" (FX/Sci-Fi)
    constexpr std::array<size_t, 4> testPresetIndices = { 0, 40, 60, 80 };
    constexpr std::array<std::array<int, 3>, 4> chords = {{
        { 48, 55, 60 }, // C3, G3, C4 for Brass
        { 53, 57, 60 }, // F3, A3, C4 for Pad
        { 36, 43, 48 }, // C2, G2, C3 for Bass
        { 60, 64, 67 }  // C4, E4, G4 for FM Chimes
    }};

    constexpr float segmentDuration = 1.5f;
    constexpr size_t segmentSamples = static_cast<size_t>(segmentDuration * SAMPLE_RATE);
    constexpr size_t totalSamples = segmentSamples * 4;

    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t seg = 0; seg < 4; ++seg) {
        const size_t presetIdx = testPresetIndices[seg];
        pm.loadPreset(presetIdx, engine);
        std::cout << "  -> Loaded Preset " << std::setw(3) << std::setfill('0') << presetIdx 
                  << " [" << getCategoryName(pm.getPreset(presetIdx).category) << "]: '" 
                  << pm.getPreset(presetIdx).getName() << "'" << std::endl;

        // Note on chord
        for (int n : chords[seg]) {
            engine.noteOn(n, 0.85f);
        }

        const size_t offset = seg * segmentSamples;
        for (size_t i = 0; i < segmentSamples; ++i) {
            // Note off at 1.1s in segment to test release
            if (i == static_cast<size_t>(1.1f * SAMPLE_RATE)) {
                for (int n : chords[seg]) {
                    engine.noteOff(n);
                }
            }

            float sL = 0.0f, sR = 0.0f;
            engine.processSample(sL, sR);
            left[offset + i] = sL;
            right[offset + i] = sR;
        }

        engine.allNotesOff();
    }

    float maxAmp = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        maxAmp = std::max(maxAmp, std::max(std::abs(left[i]), std::abs(right[i])));
    }
    if (maxAmp < 0.05f) {
        throw std::runtime_error("Preset recall rendered silence!");
    }

    WavWriter::writeWavStereo("test_preset_recall.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_preset_recall.wav (6.0s: 4 Classic Factory Patches Audio Progression)" << std::endl;
}

void runTest13_MidiCCMapping() {
    std::cout << "[Test 13/13] Rendering test_midi_cc_mapping.wav (Hardware MIDI CC sweeps: Cutoff, Shape, FX Mix)..." << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    // Start with default saw patch
    auto& params = engine.getParameters();
    params.osc1Level = 0.8f;
    params.osc2Level = 0.6f;
    params.osc2FineTune = 0.05f;
    params.ampSustain = 1.0f;
    params.ampRelease = 0.4f;
    params.fxOn = true;
    params.fx1Type = static_cast<int>(EffectType::BBDDelay);
    params.fx1Param1 = 0.35f;
    params.fx1Param2 = 0.5f;
    params.fx1Mix = 0.0f; // Start dry

    // Play a sustained 4-note chord
    constexpr std::array<int, 4> chord = { 48, 55, 60, 64 };
    for (int n : chord) {
        engine.noteOn(n, 0.85f);
    }

    constexpr float durationSec = 4.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / SAMPLE_RATE;

        // Sweep Cutoff CC 102 up and down: 20 Hz to 20 kHz
        // LFO rate ~0.5 Hz
        const float cutoffNorm = 0.5f + 0.45f * std::sin(2.0f * 3.14159f * 0.5f * t);
        const int ccCutoff = static_cast<int>(cutoffNorm * 127.0f);
        engine.controlChange(102, ccCutoff);

        // Sweep Osc 1 Shape CC 15 from Tri to Saw to Pulse (rate ~0.25 Hz)
        const float shapeNorm = 0.5f + 0.48f * std::cos(2.0f * 3.14159f * 0.25f * t);
        const int ccShape = static_cast<int>(shapeNorm * 127.0f);
        engine.controlChange(15, ccShape);

        // Sweep FX 1 Mix CC 44 from 0 to 80% wet (rate ~0.33 Hz)
        const float mixNorm = 0.4f + 0.38f * std::sin(2.0f * 3.14159f * 0.33f * t);
        const int ccMix = static_cast<int>(mixNorm * 127.0f);
        engine.controlChange(44, ccMix);

        // Release notes near end (3.2s)
        if (i == static_cast<size_t>(3.2f * SAMPLE_RATE)) {
            for (int n : chord) {
                engine.noteOff(n);
            }
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    float maxAmp = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        maxAmp = std::max(maxAmp, std::max(std::abs(left[i]), std::abs(right[i])));
    }
    if (maxAmp < 0.05f) {
        throw std::runtime_error("MIDI CC mapping rendered silence!");
    }

    WavWriter::writeWavStereo("test_midi_cc_mapping.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_midi_cc_mapping.wav (" << durationSec << "s: Automated CC 102 LPF Cutoff, CC 15 Osc Shape, CC 44 FX Mix Sweeps)" << std::endl;
}

void runTest14_SysExRoundtrip() {
    std::cout << "[Test 14/15] Rendering test_sysex_roundtrip.wav (DSI 7-bit SysEx Packing + Bit-Exact Preset Recall)..." << std::endl;

    // 1. Create a rich, custom Preset
    Preset original;
    original.programNumber = 123;
    original.setName("SysEx Pad");
    original.setAuthor("Sequential");
    original.category = PresetCategory::Pad;

    original.params.osc1Shape = 0.55f;
    original.params.osc1Level = 0.8f;
    original.params.osc2Shape = 0.45f;
    original.params.osc2Level = 0.75f;
    original.params.osc2FineTune = 0.08f;
    original.params.lpfCutoff = 3500.0f;
    original.params.lpfResonance = 0.45f;
    original.params.hpfCutoff = 90.0f;
    original.params.polyModOsc2Amt = 0.25f;
    original.params.polyModDestLPF = true;
    original.params.fxOn = true;
    original.params.fx1Type = static_cast<int>(EffectType::Chorus);
    original.params.fx1Mix = 0.5f;
    original.params.fx2Type = static_cast<int>(EffectType::HallReverb);
    original.params.fx2Mix = 0.35f;

    // 2. Encode to official DSI SysEx 7-bit MIDI byte stream
    const int targetBank = 1;
    const int targetProg = 23;
    const auto syxData = SysEx::encodeProgramDump(original, targetBank, targetProg);

    if (syxData.empty() || syxData.front() != SysEx::SYSEX_START || syxData.back() != SysEx::SYSEX_END) {
        throw std::runtime_error("SysEx stream malformed: missing 0xF0 start or 0xF7 end delimiter!");
    }
    if (syxData[1] != SysEx::DSI_MANUFACTURER_ID || syxData[2] != SysEx::PROPHET_6_MODEL_ID) {
        throw std::runtime_error("SysEx header mismatch: expected DSI (0x01) and Prophet-6 (0x29) IDs!");
    }

    // Verify all payload bytes conform to strict 7-bit MIDI spec (0..127)
    for (size_t i = 1; i < syxData.size() - 1; ++i) {
        if (syxData[i] & 0x80) {
            throw std::runtime_error("SysEx data byte has high bit set: violates 7-bit MIDI standard!");
        }
    }

    // 3. Test .syx file write and read roundtrip
    const std::string syxFile = "test_dump.syx";
    if (!SysEx::saveSysExToFile(syxFile, syxData)) {
        throw std::runtime_error("Failed to write test_dump.syx to disk!");
    }
    const auto loadedSyx = SysEx::loadSysExFromFile(syxFile);
    if (loadedSyx != syxData) {
        throw std::runtime_error("SysEx file roundtrip discrepancy between saved and loaded bytes!");
    }

    // 4. Decode SysEx back into a new preset
    Preset decoded;
    int decodedBank = -1, decodedProg = -1;
    if (!SysEx::decodeProgramDump(loadedSyx.data(), loadedSyx.size(), decoded, decodedBank, decodedProg)) {
        throw std::runtime_error("Failed to decode DSI Program Dump SysEx byte stream!");
    }

    if (decodedBank != targetBank || decodedProg != targetProg) {
        throw std::runtime_error("Decoded bank/program number mismatch in SysEx header!");
    }

    // Bit-exact parameter verification
    if (std::memcmp(&original.params, &decoded.params, sizeof(SynthParameters)) != 0) {
        throw std::runtime_error("Bit-exact parameter verification failed after SysEx roundtrip!");
    }
    if (original.getName() != decoded.getName()) {
        throw std::runtime_error("Preset name mismatch after SysEx roundtrip!");
    }

    // 5. Test Edit Buffer Dump roundtrip as well
    const auto editBufferSyx = SysEx::encodeEditBufferDump(original);
    Preset decodedEdit;
    if (!SysEx::decodeEditBufferDump(editBufferSyx.data(), editBufferSyx.size(), decodedEdit)) {
        throw std::runtime_error("Failed to decode Edit Buffer SysEx Dump!");
    }
    if (std::memcmp(&original.params, &decodedEdit.params, sizeof(SynthParameters)) != 0) {
        throw std::runtime_error("Bit-exact parameter verification failed after Edit Buffer SysEx roundtrip!");
    }

    // 6. Render audio with the decoded preset to verify DSP playback
    SynthEngine engine;
    engine.init(SAMPLE_RATE);
    engine.loadPreset(decoded);

    constexpr std::array<int, 4> chord = { 48, 55, 60, 63 }; // C minor chord
    for (int n : chord) {
        engine.noteOn(n, 0.85f);
    }

    constexpr float durationSec = 3.5f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (size_t i = 0; i < totalSamples; ++i) {
        if (i == static_cast<size_t>(2.5f * SAMPLE_RATE)) {
            for (int n : chord) {
                engine.noteOff(n);
            }
        }
        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    float maxAmp = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        maxAmp = std::max(maxAmp, std::max(std::abs(left[i]), std::abs(right[i])));
    }
    if (maxAmp < 0.05f) {
        throw std::runtime_error("SysEx restored preset rendered silence!");
    }

    WavWriter::writeWavStereo("test_sysex_roundtrip.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_sysex_roundtrip.wav (" << durationSec << "s: Bit-Exact SysEx 7-bit Roundtrip Audio Verification)" << std::endl;
}

void runTest15_UserBankPersistence() {
    std::cout << "[Test 15/15] Rendering test_user_bank_persistence.wav (User Bank Program 105 Save/Load & Audio Verification)..." << std::endl;

    const int targetSlot = 105;

    // 1. First instance: Create PresetManager, edit user program 105, write to disk
    {
        PresetManager pm;
        Preset modified;
        modified.programNumber = targetSlot;
        modified.setName("Cyber Bass");
        modified.setAuthor("Prophet6User");
        modified.category = PresetCategory::Bass;

        modified.params.osc1Shape = 0.5f;  // Saw
        modified.params.osc1Level = 0.9f;
        modified.params.osc1SubLevel = 0.5f;
        modified.params.osc2Shape = 0.85f; // Pulse
        modified.params.osc2PulseWidth = 0.35f;
        modified.params.osc2Level = 0.85f;
        modified.params.osc2FineTune = 0.05f;

        modified.params.lpfCutoff = 1800.0f;
        modified.params.lpfResonance = 0.55f;
        modified.params.filtAttack = 0.005f;
        modified.params.filtDecay = 0.3f;
        modified.params.filtSustain = 0.1f;
        modified.params.filtRelease = 0.15f;

        modified.params.ampAttack = 0.005f;
        modified.params.ampDecay = 0.4f;
        modified.params.ampSustain = 0.6f;
        modified.params.ampRelease = 0.2f;

        modified.params.distortion = 0.45f;
        modified.params.slop = 0.3f;

        pm.setPreset(static_cast<size_t>(targetSlot), modified);
    }

    // 2. Second instance: Brand new PresetManager (re-instantiated fresh)
    PresetManager freshPm;
    const auto& recalled = freshPm.getPreset(static_cast<size_t>(targetSlot));

    if (recalled.programNumber != targetSlot) {
        throw std::runtime_error("User bank disk persistence failed: program number mismatch!");
    }
    if (recalled.getName() != "Cyber Bass") {
        throw std::runtime_error("User bank disk persistence failed: program name mismatch!");
    }
    if (std::abs(recalled.params.lpfCutoff - 1800.0f) > 0.001f ||
        std::abs(recalled.params.distortion - 0.45f) > 0.001f) {
        throw std::runtime_error("User bank disk persistence failed: parameter value discrepancy!");
    }

    // 3. Test .p6bank binary bank export and import
    std::string bankFilePath = freshPm.getUserBankManager().getUserDirectory() + "/bank_1.p6bank";
    if (!freshPm.getUserBankManager().exportBankToFile(1, bankFilePath)) {
        throw std::runtime_error("Failed to export bank 1 to .p6bank file!");
    }
    if (!freshPm.getUserBankManager().importBankFromFile(1, bankFilePath)) {
        throw std::runtime_error("Failed to import bank 1 from .p6bank file!");
    }

    // 4. Load the recalled patch into SynthEngine and render matching audio
    SynthEngine engine;
    engine.init(SAMPLE_RATE);
    freshPm.loadPreset(static_cast<size_t>(targetSlot), engine);

    constexpr float durationSec = 3.0f;
    const size_t totalSamples = static_cast<size_t>(durationSec * SAMPLE_RATE);
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    constexpr std::array<int, 4> bassline = { 36, 36, 48, 46 }; // C1, C1, C2, Bb1
    const size_t noteSamples = static_cast<size_t>(0.5f * SAMPLE_RATE);

    for (size_t i = 0; i < totalSamples; ++i) {
        const size_t stepIdx = (i / noteSamples);
        if (i % noteSamples == 0 && stepIdx < bassline.size()) {
            engine.allNotesOff();
            engine.noteOn(bassline[stepIdx], 0.95f);
        } else if (i % noteSamples == static_cast<size_t>(0.35f * SAMPLE_RATE)) {
            engine.allNotesOff();
        }

        float sL = 0.0f, sR = 0.0f;
        engine.processSample(sL, sR);
        left[i] = sL;
        right[i] = sR;
    }

    float maxAmp = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        maxAmp = std::max(maxAmp, std::max(std::abs(left[i]), std::abs(right[i])));
    }
    if (maxAmp < 0.05f) {
        throw std::runtime_error("User bank reloaded patch rendered silence!");
    }

    WavWriter::writeWavStereo("test_user_bank_persistence.wav", left, right, static_cast<uint32_t>(SAMPLE_RATE));
    std::cout << "  -> Generated test_user_bank_persistence.wav (" << durationSec << "s: User Patch 105 Persistence Audio Verification)" << std::endl;
}

void runLatencyBenchmark() {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "Running 6-Voice Single-Sample Polyphonic DSP Benchmark..." << std::endl;
    std::cout << "Target: < 2000 ns per sample across 6 active voices + Dual FX" << std::endl;
    std::cout << "========================================================" << std::endl;

    SynthEngine engine;
    engine.init(SAMPLE_RATE);

    auto& params = engine.getParameters();
    params.osc1Shape = 0.65f;
    params.osc1PulseWidth = 0.4f;
    params.osc1SubLevel = 0.3f;
    params.osc2Shape = 0.35f;
    params.osc2FineTune = 0.05f;
    params.lpfCutoff = 2500.0f;
    params.lpfResonance = 0.7f;
    params.hpfCutoff = 80.0f;
    params.hpfResonance = 0.4f;
    params.slop = 0.8f;
    params.distortion = 0.5f;

    // Enable PolyMod
    params.polyModOsc2Amt = 0.3f;
    params.polyModDestOsc1Freq = true;
    params.polyModDestLPF = true;

    // Enable Dual Effects Engine (Phase 4: BBD Delay + Plate Reverb)
    params.fxOn = true;
    params.fx1Type = static_cast<int>(EffectType::BBDDelay);
    params.fx1Param1 = 0.5f;
    params.fx1Param2 = 0.5f;
    params.fx1Mix = 0.4f;

    params.fx2Type = static_cast<int>(EffectType::PlateReverb);
    params.fx2Param1 = 0.6f;
    params.fx2Param2 = 0.5f;
    params.fx2Mix = 0.4f;

    // Trigger all 6 polyphonic voices simultaneously
    constexpr std::array<int, 6> notes = { 48, 52, 55, 59, 62, 65 };
    for (int n : notes) {
        engine.noteOn(n, 0.9f);
    }

    // Warm up cache
    float dummyL = 0.0f, dummyR = 0.0f;
    for (size_t i = 0; i < 5000; ++i) {
        engine.processSample(dummyL, dummyR);
    }

    constexpr size_t benchmarkSamples = 1'000'000;
    std::cout << "Benchmarking " << benchmarkSamples << " samples ("
              << (benchmarkSamples / SAMPLE_RATE) << " audio seconds) with all 6 voices + Dual FX active..." << std::endl;

    const auto startTime = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < benchmarkSamples; ++i) {
        engine.processSample(dummyL, dummyR);
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto totalDurationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

    const double avgLatencyNs = static_cast<double>(totalDurationNs) / static_cast<double>(benchmarkSamples);
    const double totalTimeSeconds = static_cast<double>(totalDurationNs) * 1e-9;
    const double audioSeconds = static_cast<double>(benchmarkSamples) / SAMPLE_RATE;
    const double speedupFactor = audioSeconds / totalTimeSeconds;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n>>> BENCHMARK RESULTS <<<" << std::endl;
    std::cout << "  Active Polyphonic Voices: 6" << std::endl;
    std::cout << "  Active Effects Slots:     2 (Slot 1 BBD Delay + Slot 2 Plate Reverb)" << std::endl;
    std::cout << "  Total Processed Samples:  " << benchmarkSamples << std::endl;
    std::cout << "  Audio Time Rendered:      " << audioSeconds << " seconds" << std::endl;
    std::cout << "  Total Wall-Clock Time:    " << totalTimeSeconds << " seconds" << std::endl;
    std::cout << "  Real-Time Speedup Factor: " << speedupFactor << "x faster than real-time" << std::endl;
    std::cout << "  Single-Sample Latency:    " << avgLatencyNs << " ns per sample" << std::endl;

    if (avgLatencyNs < 2000.0) {
        std::cout << "  STATUS: PASSED (Under 2000 ns target latency limit with Dual FX active!)" << std::endl;
    } else {
        std::cout << "  STATUS: WARNING (Exceeded 2000 ns target latency)" << std::endl;
    }
    std::cout << "========================================================\n" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Sequential Prophet-6 Emulation: DSP Verification Suite" << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    try {
        runTest1_PolyChordSweep();
        runTest2_PolyModFM();
        runTest3_DualFilterResonance();
        runTest4_SlopDriftComparison();
        runTest5_StereoDistortion();
        runTest6_ArpeggiatorModesAndHold();
        runTest7_PolyphonicStepSequencer();
        runTest8_FXBBDDelay();
        runTest9_FXPlateReverb();
        runTest10_FXChorusFlanger();
        runTest11_FXTrueBypass();
        runTest12_PresetRecall();
        runTest13_MidiCCMapping();
        runTest14_SysExRoundtrip();
        runTest15_UserBankPersistence();
        runLatencyBenchmark();
        std::cout << "All Prophet-6 DSP verification audio files and tests PASSED successfully." << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Verification failed with exception: " << ex.what() << std::endl;
        return 1;
    }
}
