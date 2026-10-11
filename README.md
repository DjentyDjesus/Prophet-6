# Sequential Prophet-6 Virtual Analog Polyphonic Synthesizer

[![CI / Build & Test](https://github.com/DjentyDjesus/Prophet-6/actions/workflows/ci.yml/badge.svg)](https://github.com/DjentyDjesus/Prophet-6/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/Standard-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![JUCE 8](https://img.shields.io/badge/Framework-JUCE%208.0.6-orange.svg)](https://juce.com/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20x64%20%7C%20macOS%20Universal-lightgrey.svg)]()
[![Artifacts](https://img.shields.io/badge/Release%20Artifacts-EXE%20%7C%20DMG%20%7C%20VST3-green.svg)]()

A modern, high-fidelity virtual analog polyphonic software synthesizer inspired by the iconic **Sequential Prophet-6** designed by Dave Smith. Built with pure modern C++20 DSP algorithms, zero runtime allocations on the audio thread, and a photorealistic JUCE 8 desktop front-panel GUI.

---

## Architecture & Signal Flow

```
 +-----------------------------------------------------------------------------------------------------+
 |                                       PER-VOICE SIGNAL PATH (x6)                                    |
 |                                                                                                     |
 |  +---------------------+                                                                            |
 |  |     Oscillator 1    |---+                                                                        |
 |  | (Tri -> Saw -> Pulse|   |                                                                        |
 |  |    + Sub-Triangle)  |   |                                                                        |
 |  +---------------------+   |      +---------+      +------------------+      +------------------+   |
 |             ^              +----->|  Mixer  |----->| 4-Pole (24dB)    |----->| 2-Pole (12dB)    |   |
 |             | Hard Sync    |      |         |      | Resonant LPF     |      | Resonant HPF     |   |
 |             |              +----->| (VCO 1, |      | (ZDF Non-Linear) |      | (ZDF Non-Linear) |   |
 |  +---------------------+   |      |  VCO 2, |      +------------------+      +------------------+   |
 |  |     Oscillator 2    |---+      |  Noise) |               ^                         ^             |
 |  | (Tri -> Saw -> Pulse|          +---------+               |                         |             |
 |  |   Low Freq / Kbd)   |                                    |                         |             |
 |  +---------------------+                                    |                         |             |
 |             |                                               |                         |             |
 |             +-----------------[ Poly Mod Matrix ]-----------+-------------------------+             |
 |                               (Filter Env & Osc 2)                                                  |
 |                                                                                                     |
 |  +--------------------+   +--------------------+                                                    |
 |  |  Filter ADSR Env   |   |  Amplifier ADSR    |--------> [ Voice VCA ]                             |
 |  +--------------------+   +--------------------+                 |                                  |
 |  +--------------------+   +--------------------+                 v                                  |
 |  |   Polyphonic LFO   |   | Analog Slop Drift  |--------> [ Voice Stereo Pan ]                      |
 |  +--------------------+   +--------------------+                 |                                  |
 +------------------------------------------------------------------|----------------------------------+
                                                                    v
                                                     [ 6-Voice Stereo Summing Bus ]
                                                                    |
                                                                    v
                                                    +--------------------------------+
                                                    |    Master Stereo Distortion    |
                                                    +--------------------------------+
                                                                    |
                                           +------------------------+------------------------+
                                           |                                                 |
                                (True Bypass Dry)                                     (FX On: Wet Path)
                                           |                                                 |
                                           |                                                 v
                                           |                                   +---------------------------+
                                           |                                   |  Effect Slot 1 (FX A)     |
                                           |                                   | (BBD/Tape/Dig Delay, Flg) |
                                           |                                   +---------------------------+
                                           |                                                 |
                                           |                                                 v
                                           |                                   +---------------------------+
                                           |                                   |  Effect Slot 2 (FX B)     |
                                           |                                   | (Plate/Hall Reverb, Cho)  |
                                           |                                   +---------------------------+
                                           |                                                 |
                                           +------------------------+------------------------+
                                                                    |
                                                                    v
                                                    +--------------------------------+
                                                    |     Master Volume & Output     |
                                                    +--------------------------------+
```

---

## Key Features

### 1. Pure C++20 Polyphonic DSP Engine
* **6 Discrete Polyphonic Voices:** Authentic voice allocation with smooth 5ms micro-fade voice stealing and unison mode with adjustable stereo detune spread.
* **Dual Continuous Wave-Morphing VCOs:** Smooth variable shape morphing from Triangle through Sawtooth to variable Pulse Width, with PolyBLEP anti-aliased transitions.
* **Triangle Sub-Oscillator:** Sub-octave triangle wave locked one octave below Oscillator 1 for tight analog low-end weight.
* **Hard Sync:** Oscillator 1 slaved to Oscillator 2 for aggressive sync harmonic sweeps.
* **Poly Mod Cross-Modulation Matrix:** Hardware-accurate per-voice modulation where Filter Envelope and Oscillator 2 modulate Osc 1 Frequency, Shape, Pulse Width, Low-Pass Cutoff, and High-Pass Cutoff.
* **Dual Resonant Filters in Series:**
  * 4-Pole (24 dB/octave) Zero-Delay Feedback (ZDF) Low-Pass Filter with internal stage saturation.
  * 2-Pole (12 dB/octave) Zero-Delay Feedback (ZDF) High-Pass Filter in series with independent resonance.
* **Organic Analog Slop:** Per-voice thermal drift generator with low-frequency random walk, creating natural micro-detuning variance between physical voice circuits.
* **Master Stereo Analog Distortion:** Dedicated non-linear tanh saturation on the final stereo mix bus.

### 2. Dual Digital Stereo Effects Engine (24-bit / 48 kHz)
* **True Analog Bypass:** Hardware-accurate relay emulation — when `FX ON` is disabled, the DSP engine completely bypasses processing with bit-exact preservation of the pure analog dry signal.
* **Serial Dual Routing:** Slot 1 (FX A) into Slot 2 (FX B) with independent parameter controls and equal-power mix crossfading.
* **Included Algorithms:**
  * **Delays:** Analog BBD Delay, Vintage Tape Delay, Clean Digital Delay.
  * **Modulation:** Dual Chorus, Resonant Flanger, Phase Shifter, Ring Modulator.
  * **Reverbs:** Dense Vintage Plate Reverb, Expansive Concert Hall Reverb.

### 3. Polyphonic Step Sequencer & Arpeggiator
* **64-Step Polyphonic Sequencer:** Up to 6 notes per step, support for ties, rests, step-length configuration, and real-time keyboard transposition.
* **Multi-Mode Arpeggiator:** Up, Down, Up+Down, Assign, and Random modes with 1–3 octave span and multiple clock divisions (1/2 to 1/32T).
* **Host DAW Synchronization:** Full synchronization to host BPM and playhead position via `juce::AudioPlayHead`.

### 4. Preset Management, Hardware SysEx & "Write/Compare" Workflow
* **100 Curated Factory Patches (000–099):** Classic Dave Smith brass, pads, basses, keys, leads, and sci-fi FX.
* **900 Persistent User Slots (100–999):** Auto-persisted to `%APPDATA%/Prophet6/UserPresets/` in clean JSON.
* **Hardware "WRITE" Workflow:** Arms saving mode, flashes the target slot on the 7-segment display at 4 Hz, and commits edited patches on confirmation.
* **Hardware "COMPARE" Workflow:** A/B toggle between edited parameters and original saved disk presets.
* **Bidirectional MIDI SysEx Interoperability:** Official Dave Smith Instruments 7-bit to 8-bit packed SysEx implementation (`0x01` DSI ID, `0x29` Prophet-6 Model ID) for importing and exporting `.syx` patches compatible with the physical hardware and SoundTower librarian.
* **1:1 Hardware MIDI CC Mapping:** Instant O(1) continuous controller mapping across all front-panel parameters.

### 5. Authentic Desktop UI & Standalone Configuration
* **Photorealistic Front Panel:** Hand-crafted walnut wood end cheeks, brushed matte black aluminum chassis, vintage cream/red chicken-head knobs, and real-time 6-voice activity LEDs.
* **Dual 7-Segment LED Displays:** Authentic red/amber glowing segment displays for Program selection, Sequencer step count, and Write/Compare states.
* **Standalone Audio & MIDI Settings:** Integrated audio driver (ASIO/WASAPI), buffer size, sample rate, and MIDI input port configuration via the top-right wood cheek settings panel.

---

## Pre-built Releases & CI Artifacts

Pre-compiled production binaries are automatically generated and tested on every push via our GitHub Actions CI/CD matrix:

* **Windows (x64):**
  * `Prophet-6-Windows-EXE`: Standalone executable (`Prophet-6.exe`)
  * `Prophet-6-Windows-VST3`: VST3 plugin bundle (`Prophet-6.vst3`)
  * `Prophet-6-Windows-x64-Package`: Combined EXE and VST3 distribution package
* **macOS (Universal / Apple Silicon & Intel):**
  * `Prophet-6-macOS-DMG`: Mountable disk image (`Prophet-6-macOS.dmg`) containing `Prophet-6.app`, `Prophet-6.vst3`, and a drag-and-drop link to `/Applications`.

Download the latest automated builds directly from the [GitHub Actions Artifacts tab](https://github.com/DjentyDjesus/Prophet-6/actions).

---

## Building from Source

### Prerequisites
* **CMake:** Version 3.20 or newer
* **C++ Compiler:** Supporting C++20 (`MSVC 2019/2022`, `Apple Clang 15+`, or `GCC 12+`)
* **Build System:** `Ninja` (recommended) or Visual Studio / Xcode

JUCE 8.0.6 is automatically fetched via CMake `FetchContent` during the configuration step.

### Windows (Visual Studio / Ninja)

```powershell
# Clone the repository
git clone https://github.com/DjentyDjesus/Prophet-6.git
cd Prophet-6

# Configure the project with C++20 Release mode
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build and run the DSP verification test suite
cmake --build build --config Release --target dsp_verification
.\build\tests\dsp_verification.exe

# Build the Standalone executable and VST3 plugin
cmake --build build --config Release --target Prophet6_Standalone Prophet6_VST3
```

Compiled Windows binaries will be available in:
* **Standalone:** `build/targets/desktop/Prophet6_artefacts/Release/Standalone/Prophet-6.exe`
* **VST3 Plugin:** `build/targets/desktop/Prophet6_artefacts/Release/VST3/Prophet-6.vst3`

### macOS (Clang / Ninja)

```bash
# Clone the repository
git clone https://github.com/DjentyDjesus/Prophet-6.git
cd Prophet-6

# Configure the project with C++20 Release mode
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build and run the DSP verification test suite
cmake --build build --config Release --target dsp_verification
./build/tests/dsp_verification

# Build the Standalone App and VST3 plugin
cmake --build build --config Release --target Prophet6_Standalone Prophet6_VST3
```

Compiled macOS bundles will be located in:
* **Standalone:** `build/targets/desktop/Prophet6_artefacts/Release/Standalone/Prophet-6.app`
* **VST3 Plugin:** `build/targets/desktop/Prophet6_artefacts/Release/VST3/Prophet-6.vst3`

To package a `.dmg` installer locally on macOS:
```bash
mkdir -p dmg_root
cp -R build/targets/desktop/Prophet6_artefacts/Release/Standalone/Prophet-6.app dmg_root/
cp -R build/targets/desktop/Prophet6_artefacts/Release/VST3/Prophet-6.vst3 dmg_root/
ln -s /Applications dmg_root/Applications
hdiutil create -volname "Prophet-6" -srcfolder dmg_root -ov -format UDZO Prophet-6-macOS.dmg
```

---

## DSP Verification & Latency Benchmark

The repository includes a comprehensive verification suite (`tests/dsp_verification.cpp`) that renders offline `.wav` test sweeps verifying all stages of synthesis:
* Polyphonic chord sweeps with organic thermal drift
* Audio-rate FM Poly Mod sweeps
* Dual ZDF filter resonance and self-oscillation
* Analog BBD, Plate, Chorus, and True Bypass bit-exact verification
* DSI 7-bit SysEx encoding and decoding roundtrip
* Single-sample polyphonic latency benchmark (< 2000 ns/sample target, measured at **~809 ns/sample** — over **28x faster than real-time**).

---

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

*Disclaimer: Sequential and Prophet are registered trademarks of Dave Smith Instruments / Focusrite. This project is an independent educational emulation.*
