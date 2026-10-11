# Sequential Prophet-6: Project Handover & Session Summary

> **Session Timestamp:** October 11, 2026  
> **Project Directory:** `C:\Users\sriha\Documents\Antigrav\Prophet6\`  
> **GitHub Repository:** [`https://github.com/DjentyDjesus/Prophet-6`](https://github.com/DjentyDjesus/Prophet-6)  
> **Master Architecture:** [`PROPHET_6_ARCHITECTURE_PLAN.md`](./PROPHET_6_ARCHITECTURE_PLAN.md)  
> **Current Progress:** Phases 1 through 6 Complete ✅ | CI/CD Workflow & Public GitHub Publication Live

---

## 1. Project Overview & Architecture

**Prophet-6 Virtual Analog Emulation** is a 6-voice polyphonic software synthesizer inspired by Dave Smith's Sequential Prophet-6:
* **Polyphony:** 6 discrete parallel voice instances with click-free 5ms micro-fade voice stealing and unison detune spread.
* **Dual VCOs + Sub:** Continuous wave morphing (Triangle -> Sawtooth -> Variable Pulse), hard sync, and pure **triangle sub-oscillator** on Osc 1.
* **Poly Mod Matrix:** Per-voice audio-rate cross-modulation (Filter Env & Osc 2 modulating Osc 1 Freq, Shape, PW, and both filter cutoffs).
* **Dual Series Filters:** 4-Pole (24 dB/oct) Low-Pass Filter $\rightarrow$ 2-Pole (12 dB/oct) High-Pass Filter in series.
* **Analog Slop:** Per-voice micro-tuning variance and thermal drift for massive chord width and organic life.
* **Master Stereo Distortion:** Dedicated analog saturation unit on the stereo bus.
* **Step Sequencer & Arp:** 64-step polyphonic sequencer (up to 6 notes/step, ties, rests, transposition) and multi-mode arpeggiator with DAW sync.
* **Dual Digital Effects Engine:** Studio-grade 24-bit/48kHz dual digital stereo effects with hardware-accurate True Analog Bypass.
* **Preset Manager & Hardware MIDI CC:** 100 classic Dave Smith factory patches (000–099), 900 persistent user patches (100–999), 7-segment digital patch LED display, category browser, and complete O(1) MIDI CC mapping.
* **Hardware SysEx Interoperability & Write/Compare Workflow:** Bit-exact bidirectional DSI 7-bit SysEx (`.syx`), user bank persistence (`%APPDATA%/Prophet6/UserPresets/`), hardware-accurate WRITE armed/blinking and COMPARE A/B auditioning, and standalone audio device configuration.

---

## 2. Completed Milestones

### Phase 1: Pure C++20 6-Voice DSP Core & Audio Verification ✅
* **Status:** Complete.
* **Modules Implemented (`core/include/prophet6/`):**
  * `Common.hpp`: Padé fast $\tanh$, PolyBLEP bandlimiting, per-voice Slop drift generators.
  * `Oscillator.hpp`: Dual VCOs with continuous wave morphing, triangle sub, and hard sync.
  * `LowPassFilter.hpp`: 4-pole ZDF resonant Low-Pass filter with stage saturation.
  * `HighPassFilter.hpp`: 2-pole ZDF resonant High-Pass filter in series.
  * `PolyMod.hpp`: Audio-rate cross-modulation matrix per voice.
  * `Envelope.hpp`: Snappy exponential ADSR envelopes with velocity sensitivity.
  * `LFO.hpp`: Multi-wave polyphonic LFO.
  * `Distortion.hpp`: Master stereo analog saturation curve.
  * `Voice.hpp` & `VoiceManager.hpp`: 6-voice polyphonic voice allocation and voice stealing.
  * `SynthEngine.hpp`: Main engine orchestrator.
* **Audio Test Suite (`tests/dsp_verification.cpp`):**
  * Offline `.wav` sweeps generated (poly chords, poly mod FM, dual filter sweeps, slop drift, distortion).

### Phase 2: Desktop Standalone & VST3 Plugin (JUCE) ✅
* **Status:** Complete & Built (`bin/Prophet-6.exe`, `bin/Prophet-6.vst3`).
* **Implementation (`targets/desktop/`):**
  * Full APVTS parameter tree with complete Prophet-6 parameter layout.
  * Lock-free atomic parameter caching for zero-allocation audio processing.
  * Hardware-accurate GUI front panel with walnut end cheeks, brushed black aluminum chassis, vintage red/cream knobs, and 6 real-time voice indicator LEDs (`ProphetLookAndFeel.h`, `PluginEditor.cpp`).
  * Polyphonic MIDI processing (NoteOn, NoteOff, CC 64 Sustain Pedal, Mod Wheel, Pitch Bend).

### Phase 3: Polyphonic Step Sequencer & Arpeggiator ✅
* **Status:** Complete & Built.
* **Implementation (`core/include/prophet6/`):**
  * `StepSequencer.hpp`: 64-step polyphonic sequencer (up to 6 notes per step, ties, rests, and real-time keyboard transposition).
  * `Arpeggiator.hpp`: Multi-mode arpeggiator (Up, Down, Up+Down, Assign, Random, Hold/Latch) with octave span and clock divisions.
  * DAW host tempo and transport sync via `AudioPlayHead` in `PluginProcessor.cpp`.
* **Audio Test Suite:**
  * `test_arpeggiator_sweep.wav` and `test_poly_step_sequencer.wav` generated and verified.

### Phase 4: Dual Digital Stereo Effects Engine with True Bypass ✅
* **Status:** Complete & Built (`bin/Prophet-6.exe`, `bin/Prophet-6.vst3`).
* **Implementation (`core/include/prophet6/Effects.hpp` & `SynthEngine.hpp`):**
  * **Zero Dynamic Runtime Allocation:** Pre-allocated statically sized buffers at initialization.
  * **Hardware True Bypass:** `fxOn = false` completely bypasses the effects chain with bit-exact preservation of the dry analog signal.
  * **Serial Dual Routing:** Slot 1 (FX A) $\rightarrow$ Slot 2 (FX B) with equal-power wet/dry crossfade per slot.
  * **Algorithms Implemented:** BBD Delay, Digital Delay, Tape Delay, Chorus, Flanger, Phaser, Ring Modulator, Plate Reverb, Hall Reverb.
* **Audio Verification Suite & Benchmark (`tests/dsp_verification.cpp`):**
  * `test_fx_bbd_delay.wav`, `test_fx_plate_reverb.wav`, `test_fx_chorus_flanger.wav`, `test_fx_true_bypass.wav`.

### Phase 5: 100 Classic Factory Patches, Preset Browser & MIDI CC Mapping ✅
* **Status:** Complete & Built (`bin/Prophet-6.exe`, `bin/Prophet-6.vst3`).
* **Implementation (`core/include/prophet6/` & `core/src/`):**
  * **`Preset.hpp`:** Complete zero-allocation POD `Preset` structure with metadata (name, author, category, program number), synthesizer parameter snapshot (`SynthParameters`), and Arpeggiator/Sequencer settings snapshot.
  * **`FactoryPresets.hpp` / `FactoryPresets.cpp`:** 100 curated, production-ready classic Dave Smith Prophet-6 factory patches across 5 banks (000–099).
  * **`PresetManager.hpp`:** Instantaneous click-free patch recall, category filtering, and sequential patch stepping.
  * **`PresetSerializer.hpp`:** Zero-dependency clean JSON export/import and binary snapshot serialization.
  * **`MidiMap.hpp`:** Constant-time O(1) hardware-accurate MIDI Continuous Controller (CC) dispatch matrix covering all Prophet-6 parameters.

### Phase 6: Hardware SysEx Librarian, User Bank Storage & Hardware Workflow ✅
* **Status:** Complete & Built (`bin/Prophet-6.exe`, `bin/Prophet-6.vst3`).
* **Hardware-Accurate SysEx Protocol (`core/include/prophet6/SysEx.hpp`):**
  * Official Dave Smith Instruments Manufacturer ID: `0x01`
  * Prophet-6 Model ID: `0x29`
  * Commands Implemented:
    - `0x01`: Edit Buffer Dump / Request
    - `0x02`: Single Program Dump / Request
    - `0x06`: Entire Bank Dump (100 Programs)
  * Data Packing: Strict DSI 7-bit to 8-bit MIDI nibble packing/unpacking algorithm, ensuring all transmitted bytes adhere to the 7-bit MIDI spec ($0\text{--}127$).
  * Interoperability: Direct `.syx` file import and export compatible with physical Dave Smith Prophet-6 hardware and SoundTower librarian software.
  * Live MIDI Stream Processing: Real-time zero-allocation SysEx dump parsing inside `processBlock()` in `PluginProcessor.cpp`.
* **User Preset Bank Storage & File I/O (`core/include/prophet6/UserBankManager.hpp`):**
  * 900 Persistent User Programs ($100\text{--}999$, Banks 1 through 9).
  * Automatic directory resolution: `%APPDATA%/Prophet6/UserPresets/` (Windows) with portable fallback.
  * Single program JSON persistence (`prog_NNN.json`).
  * Entire bank import and export for binary `.p6bank` and `.syx` bank streams.
* **Hardware "WRITE" and "COMPARE" Workflow (`targets/desktop/Source/`):**
  * **WRITE Button:**
    - Arming: Clicking `WRITE` puts the synthesizer into write-armed mode.
    - 7-Segment LED Display: Blinks the target destination program slot ($100\text{--}999$) at 4 Hz with "WRITE TO" indicator.
    - Slot Selection: User can step destination slot using increment/decrement buttons or selector ComboBox.
    - Confirmation: Clicking `WRITE` again confirms and commits the edited patch into persistent user disk storage.
  * **COMPARE Button:**
    - A/B Auditioning: Toggles the audio engine between the currently edited sound and the original saved patch on disk.
    - 7-Segment LED Display: Displays amber glowing "COMPARE" header with original program number.
* **Standalone Audio & MIDI Settings Dialog:**
  * Discrete "SETTINGS" (`CFG`) button on the top-right walnut wood cheek.
  * Opens the native JUCE Standalone audio/MIDI options dialog for managing audio drivers (ASIO/WASAPI), buffer size, sample rate, and active MIDI input ports.
  * Context-aware: When loaded as a VST3 plugin inside a DAW host, displays a modal notification explaining that drivers and buffer sizes are managed by the host.
* **Audio Verification Suite & Benchmark (`tests/dsp_verification.cpp`):**
  * `test_sysex_roundtrip.wav`: Encodes patch to official DSI SysEx 7-bit stream, saves to `.syx`, reads back, decodes, confirms bit-exact parameter preservation, and renders audio verification.
  * `test_user_bank_persistence.wav`: Modifies user patch 105, saves to disk, re-instantiates new PresetManager, verifies parameter persistence, and renders audio playback.
  * **Benchmark Results (15/15 Tests Passing):**
    * Single-sample latency: **809.24 ns / sample** (**28.02x faster than real-time**) with 6 active voices + Dual FX active.
    * Target: < 2000 ns / sample (**PASSED**).

### Phase 8: SoundTower UI Overhaul, Real-Time Visualizers, Virtual Keyboard & v1.1 Release ✅
* **Status:** Complete, Verified & Tagged `v1.1`.
* **SoundTower PlugSE Front Panel ($1200 \times 820\text{ px}$):**
  * Hardware-accurate replica of the SoundTower Prophet-6 controller editor (`prop6 ref.png`).
  * Inset LCD preset display, Clock/BPM subdivision dots, Arp/Seq controls, and brushed metallic Sequential badge.
  * Reorganized grid: Dual VCOs, vintage Slop, Mixer, series 4-pole LPF and 2-pole HPF.
  * Poly Mod matrix, Aftertouch routing, Dual Digital FX, Misc parameters, LFO, Distortion, and Unison/Glide.
* **Real-Time Visualizers:**
  * **Mini Oscillator Waveshapes ($40 \times 24\text{ px}$):** Continuous vector morphing from triangle to sawtooth to variable pulse width directly beside Osc 1 and Osc 2 shape knobs.
  * **1-Wavelength Pitch-Synchronized CRT Oscilloscope:** Lock-free pre-effects circular audio tap (`preFxBuffer_` in `SynthEngine.hpp`) rendering a vintage phosphor green CRT trace windowed to exactly 1 fundamental wavelength.
  * **Vector Graphical ADSR Envelopes:** Illuminated dynamic ADSR attack-decay-sustain-release curves for both Filter and Amplifier stages.
* **Interactive Virtual Piano Keyboard & Performance Strip:**
  * Full 6-octave virtual keyboard (`C1` through `C7`) hooked directly to `juce::MidiKeyboardState`.
  * Vertical note velocity slider, spring-loaded pitch bend wheel (auto-returns to center), and friction modulation wheel.
* **Signature Factory Presets (v1.1):**
  * `031: Kid A Right Place` (Classic electric piano keys with woody sub-oscillator body)
  * `032: Aloboi Dreamscape` (Lush warm ambient pad with shimmer plate reverb & tape delay)
  * `033: The Veldt Pluck` (Crisp progressive house plucks inspired by deadmau5)
* **Multi-Platform GitHub Release Workflow (`.github/workflows/release.yml`):**
  * Automated builds on push to `main` and tags matching `v*`.
  * Windows matrix: Packages Standalone `.exe` + `.vst3` into `Prophet-6-v1.1-Windows.zip`.
  * macOS matrix: Universal Binary (`arm64` + `x86_64`) packaged into `Prophet-6-v1.1-macOS.dmg`.
  * Tag-triggered release publishing with release notes and binaries attached.

---

## 3. Verified Artifacts & Delivery

| Binary / Artifact | Location | Status |
|---|---|---|
| Desktop Standalone Executable | `build/targets/desktop/Prophet6_artefacts/Release/Standalone/Prophet-6.exe` | Built & Verified |
| VST3 Plugin Bundle | `build/targets/desktop/Prophet6_artefacts/Release/VST3/Prophet-6.vst3` | Built & Verified |
| Offline DSP Verification Suite | `build/tests/dsp_verification.exe` | 15/15 Tests Passed (817 ns/sample) |
| Release Workflow | `.github/workflows/release.yml` | Multi-Platform v1.1 Release |
| Windows Release Zip | `Prophet-6-v1.1-Windows.zip` | Standalone .exe + .vst3 |
| macOS Release DMG | `Prophet-6-v1.1-macOS.dmg` | Universal Binary Standalone .app + .vst3 |
| GitHub Release v1.1 | [`v1.1 Release`](https://github.com/DjentyDjesus/Prophet-6/releases/tag/v1.1) | Published Live with Assets |

---

## 4. Parallel Project Status (SubGyatty)

* **SubGyatty (Moog Sub Phatty Emulation):**
  * Phase 1 (DSP Core) & Phase 2 (Desktop Standalone & VST3) are **Complete & Working**.
  * Repository: `https://github.com/DjentyDjesus/SubGyatty.git` (`main` branch clean).
  * Ready for Phase 3 (Raspberry Pi Zero) and Phase 4 (Teensy 4.x MCU).

