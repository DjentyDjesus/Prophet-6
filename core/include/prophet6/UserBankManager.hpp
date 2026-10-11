#pragma once

#include "Preset.hpp"
#include "PresetSerializer.hpp"
#include "SysEx.hpp"
#include <array>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace prophet6 {

// User Bank Manager for Prophet-6 Programs 100–999 (Banks 1 to 9, 900 slots total)
// Handles automatic persistent disk storage in %APPDATA%/Prophet6/UserPresets/
// Supports .p6prog, .p6bank, and .syx file formats.
class UserBankManager {
public:
    static constexpr size_t NUM_USER_PROGRAMS = 900; // Programs 100 to 999
    static constexpr int FIRST_USER_PROGRAM   = 100;
    static constexpr int LAST_USER_PROGRAM    = 999;

    UserBankManager() {
        resolveUserDirectory();
        initializeDefaults();
        loadAllUserPresetsFromDisk();
    }

    // --- Directory Resolution & Initialization ---

    void resolveUserDirectory() {
        std::string baseDir;
#if defined(_WIN32)
        const char* appData = std::getenv("APPDATA");
        if (appData) {
            baseDir = std::string(appData) + "/Prophet6/UserPresets";
        } else {
            baseDir = "./UserPresets";
        }
#else
        const char* home = std::getenv("HOME");
        if (home) {
            baseDir = std::string(home) + "/.prophet6/UserPresets";
        } else {
            baseDir = "./UserPresets";
        }
#endif
        userDirectory_ = baseDir;
        try {
            std::filesystem::create_directories(userDirectory_);
        } catch (...) {
            // Fallback gracefully to current directory if system path creation fails
            userDirectory_ = "./UserPresets";
            try {
                std::filesystem::create_directories(userDirectory_);
            } catch (...) {}
        }
    }

    [[nodiscard]] const std::string& getUserDirectory() const noexcept {
        return userDirectory_;
    }

    void initializeDefaults() {
        for (size_t i = 0; i < NUM_USER_PROGRAMS; ++i) {
            const int progNum = static_cast<int>(FIRST_USER_PROGRAM + i);
            auto& p = userPresets_[i];
            p.programNumber = progNum;
            p.setName("Init Patch");
            p.setAuthor("User");
            p.category = PresetCategory::Brass;

            // Simple warm analog template sound
            p.params.osc1Shape = 0.5f; // Saw
            p.params.osc1Level = 0.7f;
            p.params.osc2Shape = 0.5f; // Saw
            p.params.osc2Level = 0.7f;
            p.params.osc2FineTune = 0.04f;
            p.params.lpfCutoff = 8000.0f;
            p.params.lpfResonance = 0.1f;
            p.params.hpfCutoff = 20.0f;
            p.params.ampAttack = 0.01f;
            p.params.ampDecay = 0.3f;
            p.params.ampSustain = 0.8f;
            p.params.ampRelease = 0.4f;
            p.params.filtAttack = 0.01f;
            p.params.filtDecay = 0.4f;
            p.params.filtSustain = 0.2f;
            p.params.filtRelease = 0.4f;
            p.params.slop = 0.25f;
            p.params.panSpread = 0.5f;
            p.params.masterVolume = 0.85f;
        }
    }

    // --- Accessors ---

    [[nodiscard]] bool isUserProgram(int programNumber) const noexcept {
        return (programNumber >= FIRST_USER_PROGRAM && programNumber <= LAST_USER_PROGRAM);
    }

    [[nodiscard]] const Preset& getUserPreset(int programNumber) const noexcept {
        const int clamped = std::clamp(programNumber, FIRST_USER_PROGRAM, LAST_USER_PROGRAM);
        const size_t index = static_cast<size_t>(clamped - FIRST_USER_PROGRAM);
        return userPresets_[index];
    }

    void setUserPreset(int programNumber, const Preset& preset) noexcept {
        if (!isUserProgram(programNumber)) return;
        const size_t index = static_cast<size_t>(programNumber - FIRST_USER_PROGRAM);
        userPresets_[index] = preset;
        userPresets_[index].programNumber = programNumber;
    }

    // --- Single Program File I/O ---

    std::string getProgramFilePath(int programNumber, const std::string& ext = ".json") const {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "/prog_%03d%s", programNumber, ext.c_str());
        return userDirectory_ + buf;
    }

    bool saveProgramToDisk(int programNumber) {
        if (!isUserProgram(programNumber)) return false;
        const auto& p = getUserPreset(programNumber);
        const std::string json = PresetSerializer::serializeToJson(p);
        const std::string path = getProgramFilePath(programNumber, ".json");

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << json;
        return file.good();
    }

    bool loadProgramFromDisk(int programNumber) {
        if (!isUserProgram(programNumber)) return false;
        const std::string path = getProgramFilePath(programNumber, ".json");
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        Preset p = getUserPreset(programNumber);
        if (PresetSerializer::deserializeFromJson(json, p)) {
            p.programNumber = programNumber;
            setUserPreset(programNumber, p);
            return true;
        }
        return false;
    }

    void loadAllUserPresetsFromDisk() {
        for (int p = FIRST_USER_PROGRAM; p <= LAST_USER_PROGRAM; ++p) {
            loadProgramFromDisk(p);
        }
    }

    // --- Bank File I/O (.p6bank / .syx) ---

    // Export a bank of 100 presets (bank 1..9) to .p6bank file
    bool exportBankToFile(int bankIndex, const std::string& filePath) const {
        if (bankIndex < 1 || bankIndex > 9) return false;
        const int startProg = bankIndex * 100;

        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;

        for (int i = 0; i < 100; ++i) {
            const auto& p = getUserPreset(startProg + i);
            const auto bin = PresetSerializer::serializeToBinary(p);
            const uint32_t len = static_cast<uint32_t>(bin.size());
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(bin.data()), static_cast<std::streamsize>(bin.size()));
        }
        return file.good();
    }

    // Import a bank of 100 presets (bank 1..9) from .p6bank file
    bool importBankFromFile(int bankIndex, const std::string& filePath) {
        if (bankIndex < 1 || bankIndex > 9) return false;
        const int startProg = bankIndex * 100;

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;

        for (int i = 0; i < 100; ++i) {
            uint32_t len = 0;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            if (!file || len == 0 || len > 10000) break;

            std::vector<uint8_t> bin(len);
            file.read(reinterpret_cast<char*>(bin.data()), static_cast<std::streamsize>(len));
            if (!file) break;

            Preset p;
            if (PresetSerializer::deserializeFromBinary(bin.data(), bin.size(), p)) {
                p.programNumber = startProg + i;
                setUserPreset(p.programNumber, p);
                saveProgramToDisk(p.programNumber);
            }
        }
        return true;
    }

    // --- Hardware SysEx File I/O (.syx) ---

    bool exportProgramToSysEx(int programNumber, const std::string& filePath) const {
        if (!isUserProgram(programNumber)) return false;
        const auto& p = getUserPreset(programNumber);
        const int bank = programNumber / 100;
        const int prog = programNumber % 100;
        const auto syx = SysEx::encodeProgramDump(p, bank, prog);
        return SysEx::saveSysExToFile(filePath, syx);
    }

    bool importProgramFromSysEx(const std::string& filePath, int targetProgramNumber) {
        if (!isUserProgram(targetProgramNumber)) return false;
        const auto syx = SysEx::loadSysExFromFile(filePath);
        if (syx.empty()) return false;

        Preset p;
        int bank = 0, prog = 0;
        if (SysEx::decodeProgramDump(syx.data(), syx.size(), p, bank, prog)) {
            p.programNumber = targetProgramNumber;
            setUserPreset(targetProgramNumber, p);
            saveProgramToDisk(targetProgramNumber);
            return true;
        } else if (SysEx::decodeEditBufferDump(syx.data(), syx.size(), p)) {
            p.programNumber = targetProgramNumber;
            setUserPreset(targetProgramNumber, p);
            saveProgramToDisk(targetProgramNumber);
            return true;
        }
        return false;
    }

    bool exportBankToSysEx(int bankIndex, const std::string& filePath) const {
        if (bankIndex < 1 || bankIndex > 9) return false;
        const int startProg = bankIndex * 100;

        std::array<Preset, 100> bankPresets{};
        for (size_t i = 0; i < 100; ++i) {
            bankPresets[i] = getUserPreset(startProg + static_cast<int>(i));
        }

        const auto syx = SysEx::encodeBankDump(bankPresets, bankIndex);
        return SysEx::saveSysExToFile(filePath, syx);
    }

    bool importBankFromSysEx(int targetBankIndex, const std::string& filePath) {
        if (targetBankIndex < 1 || targetBankIndex > 9) return false;
        const auto syx = SysEx::loadSysExFromFile(filePath);
        if (syx.empty()) return false;

        std::array<Preset, 100> bankPresets{};
        int fileBank = 0;
        if (SysEx::decodeBankDump(syx.data(), syx.size(), bankPresets, fileBank)) {
            const int startProg = targetBankIndex * 100;
            for (int i = 0; i < 100; ++i) {
                auto& p = bankPresets[static_cast<size_t>(i)];
                p.programNumber = startProg + i;
                setUserPreset(p.programNumber, p);
                saveProgramToDisk(p.programNumber);
            }
            return true;
        }
        return false;
    }

private:
    std::string userDirectory_;
    std::array<Preset, NUM_USER_PROGRAMS> userPresets_{};
};

} // namespace prophet6
