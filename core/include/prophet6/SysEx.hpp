#pragma once

#include "Preset.hpp"
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <string>

namespace prophet6 {

// Hardware-Accurate Dave Smith Instruments / Sequential Prophet-6 SysEx Protocol
// Official DSI Manufacturer ID: 0x01
// Prophet-6 Model ID:          0x29
class SysEx {
public:
    static constexpr uint8_t SYSEX_START         = 0xF0;
    static constexpr uint8_t SYSEX_END           = 0xF7;
    static constexpr uint8_t DSI_MANUFACTURER_ID = 0x01;
    static constexpr uint8_t PROPHET_6_MODEL_ID  = 0x29;

    static constexpr uint8_t CMD_EDIT_BUFFER_DUMP = 0x01;
    static constexpr uint8_t CMD_PROGRAM_DUMP     = 0x02;
    static constexpr uint8_t CMD_PROGRAM_REQUEST  = 0x05;
    static constexpr uint8_t CMD_BANK_DUMP        = 0x06;

    // --- DSI 7-bit to 8-bit Nibble Packing & Unpacking ---

    // Packs 8-bit raw data bytes into 7-bit MIDI bytes (groups of 7 bytes -> 8 MIDI bytes)
    static std::vector<uint8_t> pack7Bit(const uint8_t* data, size_t size) {
        std::vector<uint8_t> packed;
        packed.reserve((size * 8 + 6) / 7);

        for (size_t i = 0; i < size; i += 7) {
            const size_t chunkSize = std::min(size_t{7}, size - i);
            uint8_t msbByte = 0;
            for (size_t j = 0; j < chunkSize; ++j) {
                if (data[i + j] & 0x80) {
                    msbByte |= static_cast<uint8_t>(1 << j);
                }
            }
            packed.push_back(msbByte & 0x7F);
            for (size_t j = 0; j < chunkSize; ++j) {
                packed.push_back(data[i + j] & 0x7F);
            }
        }
        return packed;
    }

    // Unpacks 7-bit MIDI bytes back into 8-bit raw data bytes
    static std::vector<uint8_t> unpack7Bit(const uint8_t* midiData, size_t midiSize) {
        std::vector<uint8_t> unpacked;
        unpacked.reserve((midiSize * 7) / 8);

        size_t idx = 0;
        while (idx < midiSize) {
            const uint8_t msbByte = midiData[idx++];
            const size_t count = std::min(size_t{7}, midiSize - idx);
            for (size_t j = 0; j < count; ++j) {
                uint8_t b = midiData[idx++];
                if (msbByte & (1 << j)) {
                    b |= 0x80;
                }
                unpacked.push_back(b);
            }
        }
        return unpacked;
    }

    // --- Single Program Dump Encoding & Decoding ---

    static std::vector<uint8_t> encodeProgramDump(const Preset& preset, int bank, int program) {
        std::vector<uint8_t> syx;
        syx.push_back(SYSEX_START);
        syx.push_back(DSI_MANUFACTURER_ID);
        syx.push_back(PROPHET_6_MODEL_ID);
        syx.push_back(CMD_PROGRAM_DUMP);
        syx.push_back(static_cast<uint8_t>(bank & 0x7F));
        syx.push_back(static_cast<uint8_t>(program & 0x7F));

        // Pack raw binary bytes of Preset struct
        const auto* rawBytes = reinterpret_cast<const uint8_t*>(&preset);
        const auto packed = pack7Bit(rawBytes, sizeof(Preset));
        syx.insert(syx.end(), packed.begin(), packed.end());

        syx.push_back(SYSEX_END);
        return syx;
    }

    static bool decodeProgramDump(const uint8_t* syxData, size_t size, Preset& preset, int& bank, int& program) {
        if (!syxData || size < 8) return false;
        if (syxData[0] != SYSEX_START || syxData[size - 1] != SYSEX_END) return false;
        if (syxData[1] != DSI_MANUFACTURER_ID || syxData[2] != PROPHET_6_MODEL_ID) return false;
        if (syxData[3] != CMD_PROGRAM_DUMP) return false;

        bank = static_cast<int>(syxData[4]);
        program = static_cast<int>(syxData[5]);

        const size_t payloadStart = 6;
        const size_t payloadLen = size - 7;
        const auto unpacked = unpack7Bit(syxData + payloadStart, payloadLen);

        if (unpacked.size() < sizeof(Preset)) return false;
        std::memcpy(&preset, unpacked.data(), sizeof(Preset));
        return true;
    }

    // --- Edit Buffer Dump (Current Sound without Saving to Slot) ---

    static std::vector<uint8_t> encodeEditBufferDump(const Preset& preset) {
        std::vector<uint8_t> syx;
        syx.push_back(SYSEX_START);
        syx.push_back(DSI_MANUFACTURER_ID);
        syx.push_back(PROPHET_6_MODEL_ID);
        syx.push_back(CMD_EDIT_BUFFER_DUMP);

        const auto* rawBytes = reinterpret_cast<const uint8_t*>(&preset);
        const auto packed = pack7Bit(rawBytes, sizeof(Preset));
        syx.insert(syx.end(), packed.begin(), packed.end());

        syx.push_back(SYSEX_END);
        return syx;
    }

    static bool decodeEditBufferDump(const uint8_t* syxData, size_t size, Preset& preset) {
        if (!syxData || size < 6) return false;
        if (syxData[0] != SYSEX_START || syxData[size - 1] != SYSEX_END) return false;
        if (syxData[1] != DSI_MANUFACTURER_ID || syxData[2] != PROPHET_6_MODEL_ID) return false;
        if (syxData[3] != CMD_EDIT_BUFFER_DUMP) return false;

        const size_t payloadStart = 4;
        const size_t payloadLen = size - 5;
        const auto unpacked = unpack7Bit(syxData + payloadStart, payloadLen);

        if (unpacked.size() < sizeof(Preset)) return false;
        std::memcpy(&preset, unpacked.data(), sizeof(Preset));
        return true;
    }

    // --- Entire Bank Dump Encoding & Decoding (100 Programs per Bank) ---

    static std::vector<uint8_t> encodeBankDump(const std::array<Preset, 100>& presets, int bank) {
        std::vector<uint8_t> syx;
        syx.push_back(SYSEX_START);
        syx.push_back(DSI_MANUFACTURER_ID);
        syx.push_back(PROPHET_6_MODEL_ID);
        syx.push_back(CMD_BANK_DUMP);
        syx.push_back(static_cast<uint8_t>(bank & 0x7F));

        const auto* rawBytes = reinterpret_cast<const uint8_t*>(presets.data());
        const size_t totalBytes = sizeof(Preset) * 100;
        const auto packed = pack7Bit(rawBytes, totalBytes);
        syx.insert(syx.end(), packed.begin(), packed.end());

        syx.push_back(SYSEX_END);
        return syx;
    }

    static bool decodeBankDump(const uint8_t* syxData, size_t size, std::array<Preset, 100>& presets, int& bank) {
        if (!syxData || size < 7) return false;
        if (syxData[0] != SYSEX_START || syxData[size - 1] != SYSEX_END) return false;
        if (syxData[1] != DSI_MANUFACTURER_ID || syxData[2] != PROPHET_6_MODEL_ID) return false;
        if (syxData[3] != CMD_BANK_DUMP) return false;

        bank = static_cast<int>(syxData[4]);
        const size_t payloadStart = 5;
        const size_t payloadLen = size - 6;
        const auto unpacked = unpack7Bit(syxData + payloadStart, payloadLen);

        const size_t expectedBytes = sizeof(Preset) * 100;
        if (unpacked.size() < expectedBytes) return false;
        std::memcpy(presets.data(), unpacked.data(), expectedBytes);
        return true;
    }

    // --- SysEx File I/O (.syx) ---

    static bool saveSysExToFile(const std::string& filePath, const std::vector<uint8_t>& syxData) {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;
        file.write(reinterpret_cast<const char*>(syxData.data()), static_cast<std::streamsize>(syxData.size()));
        return file.good();
    }

    static std::vector<uint8_t> loadSysExFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return {};
        const auto fileSize = file.tellg();
        if (fileSize <= 0) return {};
        std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        return buffer;
    }
};

} // namespace prophet6
