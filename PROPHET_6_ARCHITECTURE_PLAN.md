# Sequential Prophet-6 Emulation: Master Architecture & Execution Plan

> **Living Technical Specification & Methodology Document**  
> *Target Formats:* Desktop Standalone Application (`.exe`), VST3 Plugin (`.vst3`), CLAP Plugin.  
> *Acoustic Archetype:* Sequential Prophet-6 (Designed by Dave Smith) — 6-Voice Discrete Analog Polyphonic Synthesizer.  
> *Core Mandate:* Pure C++20 zero-allocation polyphonic DSP engine, decoupled from GUI and audio drivers.

---

## 1. Architectural Philosophy & Non-Negotiable Invariants

1. **Zero Runtime Allocation on Audio Paths**:
   - All 6 voices, envelope states, filter integrators, voice allocation stacks, and modulation routing matrices are statically sized at initialization.
   - `no malloc`, `no new`, `no std::vector::push_back` anywhere on the audio thread.
2. **Deterministic Real-Time Polyphony**:
   - Fixed pool of 6 discrete polyphonic voice instances (`ProphetVoice`).
   - Click-free voice stealing with fast 5ms micro-fade smoothing when polyphony is exhausted.
3. **Single-Precision Floating Point (`float`)**:
   - Standardized on `float` throughout for optimal cache utilization, SIMD vectorization, and consistent numerical dynamics.
4. **Decoupled "Clean Core" Architecture**:
   - `core/` contains pure DSP with zero dependencies on JUCE, OS threads, file systems, or GUI code.
   - Input: MIDI messages and parameter buffers. Output: Stereo floating-point audio blocks.
5. **Per-Voice Organic Drift ("Slop")**:
   - Independent micro-pitch variance, analog component tolerance modeling, and slow thermal drift per voice to replicate genuine discrete analog polyphony.

---

## 2. Signal Path & Component Specifications

```
  [ MIDI INPUT ] ──► [ 6-VOICE POLYPHONIC VOICE MANAGER ]
                             │
       ┌───────────┬─────────┼───────────┬───────────┬───────────┐
       ▼           ▼         ▼           ▼           ▼           ▼
   [Voice 1]   [Voice 2]  [Voice 3]   [Voice 4]   [Voice 5]   [Voice 6]
 ┌─────────────────────────────────────────────────────────────────────┐
 │  Osc 1: Continuous Tri -> Saw -> Variable Pulse (PolyBLEP)          │
 │  Osc 1 Sub: Triangle Wave (-1 Octave)                               │
 │  Osc 2: Continuous Tri -> Saw -> Variable Pulse + Fine Detune + Low │
 │  Noise: White Noise Generator                                       │
 │                                                                     │
 │  ◄── POLY MOD MATRIX ──►                                            │
 │  Sources: Filter Envelope (bipolar) & Osc 2 (Audio Rate)            │
 │  Dests: Osc 1 Freq, Osc 1 Shape, Osc 1 PW, LPF Cutoff, HPF Cutoff   │
 │                                                                     │
 │  Mixer ──► [ 4-Pole 24dB ZDF LPF ] ──► [ 2-Pole 12dB ZDF HPF ] ──► VCA
 └─────────────────────────────────┬───────────────────────────────────┘
                                   │
                                   ▼ (6 Stereo Voices Summing Bus)
                    [ STEREO ANALOG DISTORTION UNIT ]
                                   │
                                   ▼
                         [ LEFT / RIGHT OUTPUT ]
```

### 2.1. Dual Voltage-Controlled Oscillators (VCOs) per Voice
* **Oscillator 1**:
  - Continuous waveshape morphing: Triangle $\rightarrow$ Sawtooth $\rightarrow$ Variable Pulse (duty cycle 1% to 50%).
  - Anti-aliasing via PolyBLEP / PolyBLAMP residual step bandlimiting.
  - **Triangle Sub-Octave Generator**: Dedicated pure triangle sub-oscillator locked 1 octave down ($f / 2$) for smooth low-end warmth without the aggressive harmonic buzz of square sub-oscillators.
  - Hard Sync slave to Osc 2.
* **Oscillator 2**:
  - Continuous waveshape morphing: Triangle $\rightarrow$ Sawtooth $\rightarrow$ Variable Pulse.
  - Fine tune (+/- 7 semitones, +/- 100 cents).
  - **Low Frequency Mode**: Drops Osc 2 down into LFO frequencies (0.05 Hz to 20 Hz) while retaining keyboard tracking options.
  - Keyboard Tracking switch (On/Off).

### 2.2. The Poly Mod Section (Dave Smith's Signature Feature)
* Dedicated audio-rate cross-modulation matrix computed inside every voice independently:
* **Modulation Sources**:
  1. **Filter Envelope** (bipolar amount: $-100\%$ to $+100\%$)
  2. **Oscillator 2** (audio rate or low frequency: $0\%$ to $100\%$)
* **Modulation Destinations**:
  1. **Osc 1 Frequency** (metallic FM, bells, cross-modulation grind)
  2. **Osc 1 Waveshape** (dynamic shape morphing)
  3. **Osc 1 Pulse Width** (audio-rate pulse-width modulation)
  4. **Low-Pass Filter Cutoff** (growling audio-rate filter FM)
  5. **High-Pass Filter Cutoff** (formant and vocal sweeps)

### 2.3. Dual Series Resonant Filters
* **4-Pole (24 dB/oct) Low-Pass Filter**:
  - Discrete Curtis/SSM 2040-inspired Zero-Delay Feedback (ZDF) transistor ladder topology.
  - Self-oscillating resonance with bilinear pre-warping and stage saturation.
  - Keyboard tracking: 0%, 50%, 100%.
* **2-Pole (12 dB/oct) High-Pass Filter**:
  - Resonant 12 dB/oct ZDF filter placed directly in series after the Low-Pass filter.
  - Allows simultaneous low-end trimming and resonant high-pass formant shaping.

### 2.4. Envelopes & Modulation
* **Filter Envelope**: 4-stage ADSR with velocity sensitivity and bipolar modulation.
* **Amplifier Envelope**: 4-stage ADSR with velocity sensitivity and stereo spread.
* **Global Low Frequency Oscillator (LFO)**:
  - Waveforms: Triangle, Sawtooth, Reverse Saw, Square, Random (Sample & Hold).
  - Frequency range: 0.05 Hz to 500 Hz.
  - Initial routing to Pitch, Pulse Width, Cutoff, and Volume via mod wheel / aftertouch.

### 2.5. Analog Slop (Discrete Component Variance)
* In vintage polyphonic synthesizers, each voice card behaves slightly differently.
* The **Slop** control introduces controlled, calibrated micro-offsets per voice:
  - Per-voice initial tuning deviations ($\pm 5\text{ cents}$).
  - Slow thermal drift using low-frequency Perlin/Pink noise generators.
  - Minor filter cutoff and resonance tracking variations between voices.
* Produces the iconic Prophet lushness, stereo width, and organic warmth when playing chords.

### 2.6. Stereo Output & Analog Distortion
* Per-voice stereo panning (Pan Spread mode).
* Dedicated output analog distortion circuit modeling harmonic saturation and soft-clipping on the combined stereo bus.

---

## 3. Polyphonic Voice Manager (`VoiceManager.hpp`)

* **Fixed Voice Pool**: `std::array<ProphetVoice, 6> voices_`.
* **Voice Stealing Algorithm**:
  1. Find inactive/idle voice.
  2. If all 6 voices are active, pick voice in `Release` state with lowest current amplitude.
  3. If all voices are in `Sustain`, pick the oldest triggered voice (FIFO).
  4. Perform a 5ms micro-fade to zero before re-pitching to avoid audible transients or clicks.
* **Unison Mode**:
  - Configurable 1 to 6 voice unison stacking.
  - Stereo detune spread across stacked voices for massive monophonic leads.
  - Chord memory mode.

---

## 4. Multi-Target Directory Structure

```
Prophet6/
├── CMakeLists.txt                  <-- Root build orchestration
├── PROPHET_6_ARCHITECTURE_PLAN.md  <-- This master plan
│
├── core/                           <-- PURE C++20 ZERO-ALLOCATION DSP
│   ├── CMakeLists.txt
│   ├── include/prophet6/
│   │   ├── Common.hpp              <-- Fast math, Slop generator, PolyBLEP
│   │   ├── Oscillator.hpp          <-- VCO 1 & 2 continuous morph + Sub
│   │   ├── LowPassFilter.hpp       <-- 4-pole ZDF resonant LPF
│   │   ├── HighPassFilter.hpp      <-- 2-pole ZDF resonant HPF
│   │   ├── PolyMod.hpp             <-- Audio-rate modulation routing
│   │   ├── Distortion.hpp          <-- Stereo analog distortion
│   │   ├── Envelope.hpp            <-- 4-stage ADSR exponential curves
│   │   ├── LFO.hpp                 <-- Multi-waveform polyphonic LFO
│   │   ├── Voice.hpp               <-- Single Prophet voice container
│   │   ├── VoiceManager.hpp        <-- 6-voice polyphony & stealing logic
│   │   ├── Parameters.hpp          <-- Parameter IDs & MIDI CC bindings
│   │   └── SynthEngine.hpp         <-- Main 6-voice polyphonic orchestrator
│   └── src/
│       ├── Voice.cpp
│       ├── VoiceManager.cpp
│       └── SynthEngine.cpp
│
├── targets/
│   └── desktop/                    <-- JUCE VST3 & STANDALONE APP
│       ├── CMakeLists.txt
│       └── Source/
│           ├── PluginProcessor.cpp <-- APVTS parameter tree & MIDI dispatch
│           ├── PluginProcessor.h
│           ├── PluginEditor.cpp    <-- Hardware-style front panel GUI
│           └── PluginEditor.h
│
└── tests/
    ├── CMakeLists.txt
    ├── dsp_verification.cpp        <-- Offline 6-voice polyphonic test harness
    └── wav_writer.hpp              <-- 16-bit PCM WAV exporter
```

---

## 5. Multi-Phase Implementation Roadmap

### Phase 1: Pure C++20 6-Voice DSP Core & Audio Verification
* **Goal**: Implement all mathematical DSP components, Poly Mod matrix, and 6-voice VoiceManager.
* **Test Deliverables (`tests/dsp_verification.cpp`)**:
  1. `test_poly_chord_sweep.wav`: 6-note major 9th chord with analog Slop and filter sweep.
  2. `test_polymod_fm.wav`: Osc 2 modulating Osc 1 frequency (metallic FM bell timbre).
  3. `test_dual_filter_resonance.wav`: Low-Pass and High-Pass filters sweeping in opposite directions.
  4. `test_slop_drift.wav`: Demonstration of chord width with Slop set from 0% (sterile) to 100% (lush analog).
  5. `test_stereo_distortion.wav`: Master stereo distortion saturation sweep.
  6. **Benchmark**: Measure single-sample latency across all 6 active polyphonic voices ($< 1500\text{ ns}$ target).

### Phase 2: Desktop Standalone & VST3 Plugin (JUCE)
* **Goal**: Wrap `SynthEngine` into JUCE standalone `.exe` and `.vst3` plugin with complete parameter automation.
* **GUI Design**:
  - Dimensions: ~1100 x 600 px.
  - Sequential vintage aesthetic: Walnut side panels, textured black aluminum chassis, red 7-segment digital patch display, vintage red and cream control knobs.
  - Dedicated sections: `POLY MOD`, `OSCILLATORS`, `LOW-PASS FILTER`, `HIGH-PASS FILTER`, `AMPLIFIER`, `LFO`, `DISTORTION`.

### Phase 3: Polyphonic Step Sequencer & Arpeggiator
* Up to 64-step polyphonic sequencer (up to 6 notes per step).
* Variable rate arpeggiator (Up, Down, Up+Down, Random, Assign).

### Phase 4: Preset Management & Factory Patches
* 100 classic Prophet factory patches (Brass, Strings, Poly Keys, Pads, Bass, Sound FX).
* JSON/XML preset serialization.
