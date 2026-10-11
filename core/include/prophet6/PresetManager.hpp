#pragma once

#include "Preset.hpp"
#include "FactoryPresets.hpp"
#include "PresetSerializer.hpp"
#include "UserBankManager.hpp"
#include "SynthEngine.hpp"
#include <array>
#include <vector>
#include <string_view>
#include <algorithm>
#include <cstddef>
#include <memory>

namespace prophet6 {

class PresetManager {
public:
    static constexpr size_t NUM_FACTORY_PROGRAMS = FactoryPresets::NUM_FACTORY_PRESETS; // 100 (000-099)
    static constexpr size_t NUM_USER_PROGRAMS    = UserBankManager::NUM_USER_PROGRAMS;   // 900 (100-999)
    static constexpr size_t NUM_TOTAL_PROGRAMS   = 1000;                                 // 000-999

    PresetManager() {
        init();
    }

    void init() {
        const auto& factory = FactoryPresets::getAllPresets();
        for (size_t i = 0; i < NUM_FACTORY_PROGRAMS; ++i) {
            factoryPresets_[i] = factory[i];
        }
        currentIndex_ = 0;
        originalPreset_ = factoryPresets_[0];
        editBuffer_ = factoryPresets_[0];
        isComparing_ = false;
        isWriteArmed_ = false;
        writeDestinationSlot_ = 100;
    }

    [[nodiscard]] size_t getNumPresets() const noexcept {
        return NUM_TOTAL_PROGRAMS;
    }

    [[nodiscard]] size_t getCurrentProgramIndex() const noexcept {
        return currentIndex_;
    }

    void setCurrentProgramIndex(size_t index) noexcept {
        currentIndex_ = std::min(index, NUM_TOTAL_PROGRAMS - 1);
    }

    [[nodiscard]] const Preset& getPreset(size_t index) const noexcept {
        const size_t clamped = std::min(index, NUM_TOTAL_PROGRAMS - 1);
        if (clamped < NUM_FACTORY_PROGRAMS) {
            return factoryPresets_[clamped];
        }
        return userBank_.getUserPreset(static_cast<int>(clamped));
    }

    [[nodiscard]] const Preset& getCurrentPreset() const noexcept {
        return getPreset(currentIndex_);
    }

    void setPreset(size_t index, const Preset& preset) {
        if (index >= NUM_TOTAL_PROGRAMS) return;
        if (index < NUM_FACTORY_PROGRAMS) {
            factoryPresets_[index] = preset;
        } else {
            userBank_.setUserPreset(static_cast<int>(index), preset);
            userBank_.saveProgramToDisk(static_cast<int>(index));
        }
    }

    // Instantaneous, click-free patch recall directly into the SynthEngine
    void loadPreset(size_t index, SynthEngine& engine) noexcept {
        currentIndex_ = std::min(index, NUM_TOTAL_PROGRAMS - 1);
        const auto& p = getPreset(currentIndex_);

        // 1. Copy synth parameters
        engine.setParameters(p.params);

        // 2. Configure Arpeggiator
        auto& arp = engine.getArpeggiator();
        arp.setEnabled(p.arpEnabled);
        arp.setMode(p.arpMode);
        arp.setOctave(p.arpOctave);
        arp.setDivision(p.arpDivision);
        arp.setHold(p.arpHold);

        // 3. Configure Sequencer
        auto& seq = engine.getSequencer();
        seq.setEnabled(p.seqEnabled);
        seq.setLength(p.seqLength);
        seq.setBpm(p.seqBpm);

        // Update baseline for Compare
        originalPreset_ = p;
        editBuffer_ = p;
        isComparing_ = false;
        isWriteArmed_ = false;
    }

    void loadCurrentPreset(SynthEngine& engine) noexcept {
        loadPreset(currentIndex_, engine);
    }

    void nextPreset(SynthEngine& engine) noexcept {
        size_t next = (currentIndex_ + 1) % NUM_TOTAL_PROGRAMS;
        loadPreset(next, engine);
    }

    void prevPreset(SynthEngine& engine) noexcept {
        size_t prev = (currentIndex_ == 0) ? (NUM_TOTAL_PROGRAMS - 1) : (currentIndex_ - 1);
        loadPreset(prev, engine);
    }

    // --- Hardware COMPARE Workflow ---

    [[nodiscard]] bool isComparing() const noexcept {
        return isComparing_;
    }

    // Toggles between current edits and the original saved patch
    void toggleCompare(SynthEngine& engine) noexcept {
        if (!isComparing_) {
            // Save current live state into edit buffer
            captureEngineState(engine, editBuffer_);
            // Load original saved preset into engine
            applyPresetToEngine(originalPreset_, engine);
            isComparing_ = true;
        } else {
            // Restore edited sound
            applyPresetToEngine(editBuffer_, engine);
            isComparing_ = false;
        }
    }

    // --- Hardware WRITE Workflow ---

    [[nodiscard]] bool isWriteArmed() const noexcept {
        return isWriteArmed_;
    }

    [[nodiscard]] int getWriteDestination() const noexcept {
        return writeDestinationSlot_;
    }

    void setWriteDestination(int slot) noexcept {
        writeDestinationSlot_ = std::clamp(slot, static_cast<int>(UserBankManager::FIRST_USER_PROGRAM),
                                                 static_cast<int>(UserBankManager::LAST_USER_PROGRAM));
    }

    void armWrite() noexcept {
        isWriteArmed_ = true;
        if (currentIndex_ >= NUM_FACTORY_PROGRAMS) {
            writeDestinationSlot_ = static_cast<int>(currentIndex_);
        } else {
            writeDestinationSlot_ = 100; // First user program slot
        }
    }

    void cancelWrite() noexcept {
        isWriteArmed_ = false;
    }

    // Confirms write operation: saves current sound to writeDestinationSlot_ and persists to disk
    bool confirmWrite(const SynthEngine& engine, std::string_view patchName = {}) {
        if (!isWriteArmed_) return false;

        Preset toSave;
        toSave.programNumber = writeDestinationSlot_;
        captureEngineState(engine, toSave);

        if (!patchName.empty()) {
            toSave.setName(patchName);
        } else {
            toSave.setName(getCurrentPreset().getName());
        }
        toSave.setAuthor("User");
        toSave.category = getCurrentPreset().category;

        // Persist to user bank and disk
        userBank_.setUserPreset(writeDestinationSlot_, toSave);
        userBank_.saveProgramToDisk(writeDestinationSlot_);

        // Set as current program and update baseline
        currentIndex_ = static_cast<size_t>(writeDestinationSlot_);
        originalPreset_ = toSave;
        editBuffer_ = toSave;
        isWriteArmed_ = false;
        isComparing_ = false;
        return true;
    }

    // --- Filter & Export ---

    [[nodiscard]] std::vector<size_t> getIndicesForCategory(PresetCategory cat) const {
        std::vector<size_t> indices;
        indices.reserve(NUM_TOTAL_PROGRAMS);
        for (size_t i = 0; i < NUM_TOTAL_PROGRAMS; ++i) {
            if (getPreset(i).category == cat) {
                indices.push_back(i);
            }
        }
        return indices;
    }

    [[nodiscard]] std::string exportPresetToJson(size_t index) const {
        return PresetSerializer::serializeToJson(getPreset(index));
    }

    bool importPresetFromJson(size_t index, std::string_view json) {
        if (index >= NUM_TOTAL_PROGRAMS) return false;
        Preset p = getPreset(index);
        if (PresetSerializer::deserializeFromJson(json, p)) {
            setPreset(index, p);
            return true;
        }
        return false;
    }

    [[nodiscard]] UserBankManager& getUserBankManager() noexcept {
        return userBank_;
    }

    [[nodiscard]] const UserBankManager& getUserBankManager() const noexcept {
        return userBank_;
    }

private:
    static void captureEngineState(const SynthEngine& engine, Preset& p) noexcept {
        p.params = engine.getParameters();
        p.arpEnabled = engine.getArpeggiator().isEnabled();
        p.arpMode = engine.getArpeggiator().getMode();
        p.arpOctave = engine.getArpeggiator().getOctave();
        p.arpDivision = engine.getArpeggiator().getDivision();
        p.arpHold = engine.getArpeggiator().getHold();
        p.seqEnabled = engine.getSequencer().isEnabled();
        p.seqLength = static_cast<int>(engine.getSequencer().getLength());
        p.seqBpm = engine.getSequencer().getBpm();
    }

    static void applyPresetToEngine(const Preset& p, SynthEngine& engine) noexcept {
        engine.setParameters(p.params);
        auto& arp = engine.getArpeggiator();
        arp.setEnabled(p.arpEnabled);
        arp.setMode(p.arpMode);
        arp.setOctave(p.arpOctave);
        arp.setDivision(p.arpDivision);
        arp.setHold(p.arpHold);
        auto& seq = engine.getSequencer();
        seq.setEnabled(p.seqEnabled);
        seq.setLength(p.seqLength);
        seq.setBpm(p.seqBpm);
    }

    std::array<Preset, NUM_FACTORY_PROGRAMS> factoryPresets_{};
    UserBankManager userBank_;
    size_t currentIndex_ = 0;

    // Compare & Write states
    Preset originalPreset_{};
    Preset editBuffer_{};
    bool isComparing_ = false;
    bool isWriteArmed_ = false;
    int writeDestinationSlot_ = 100;
};

} // namespace prophet6
