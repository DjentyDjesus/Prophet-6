#pragma once

#include "Preset.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <string_view>
#include <vector>
#include <charconv>
#include <cstdint>
#include <cstring>

namespace prophet6 {

class PresetSerializer {
public:
    static constexpr uint32_t BINARY_MAGIC = 0x50365052; // 'P6PR' (Prophet-6 Preset)
    static constexpr uint32_t BINARY_VERSION = 1;

    // --- JSON Serialization ---

    static std::string serializeToJson(const Preset& p) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << "{\n";
        ss << "  \"schema\": \"Prophet6Preset_v1\",\n";
        ss << "  \"programNumber\": " << p.programNumber << ",\n";
        ss << "  \"name\": \"" << p.getName() << "\",\n";
        ss << "  \"category\": \"" << getCategoryName(p.category) << "\",\n";
        ss << "  \"author\": \"" << p.getAuthor() << "\",\n";

        // Arp & Sequencer
        ss << "  \"arp\": {\n";
        ss << "    \"enabled\": " << (p.arpEnabled ? "true" : "false") << ",\n";
        ss << "    \"mode\": " << static_cast<int>(p.arpMode) << ",\n";
        ss << "    \"octave\": " << p.arpOctave << ",\n";
        ss << "    \"division\": " << static_cast<int>(p.arpDivision) << ",\n";
        ss << "    \"hold\": " << (p.arpHold ? "true" : "false") << "\n";
        ss << "  },\n";

        ss << "  \"seq\": {\n";
        ss << "    \"enabled\": " << (p.seqEnabled ? "true" : "false") << ",\n";
        ss << "    \"length\": " << p.seqLength << ",\n";
        ss << "    \"bpm\": " << p.seqBpm << "\n";
        ss << "  },\n";

        // Parameters
        const auto& pm = p.params;
        ss << "  \"params\": {\n";
        // Osc 1
        ss << "    \"osc1Octave\": " << pm.osc1Octave << ",\n";
        ss << "    \"osc1Shape\": " << pm.osc1Shape << ",\n";
        ss << "    \"osc1PulseWidth\": " << pm.osc1PulseWidth << ",\n";
        ss << "    \"osc1Sync\": " << (pm.osc1Sync ? "true" : "false") << ",\n";
        ss << "    \"osc1Level\": " << pm.osc1Level << ",\n";
        ss << "    \"osc1SubLevel\": " << pm.osc1SubLevel << ",\n";

        // Osc 2
        ss << "    \"osc2Octave\": " << pm.osc2Octave << ",\n";
        ss << "    \"osc2FineTune\": " << pm.osc2FineTune << ",\n";
        ss << "    \"osc2Shape\": " << pm.osc2Shape << ",\n";
        ss << "    \"osc2PulseWidth\": " << pm.osc2PulseWidth << ",\n";
        ss << "    \"osc2LowFreq\": " << (pm.osc2LowFreq ? "true" : "false") << ",\n";
        ss << "    \"osc2KbdTrack\": " << (pm.osc2KbdTrack ? "true" : "false") << ",\n";
        ss << "    \"osc2Level\": " << pm.osc2Level << ",\n";

        // Noise
        ss << "    \"noiseLevel\": " << pm.noiseLevel << ",\n";

        // LPF
        ss << "    \"lpfCutoff\": " << pm.lpfCutoff << ",\n";
        ss << "    \"lpfResonance\": " << pm.lpfResonance << ",\n";
        ss << "    \"lpfEnvAmount\": " << pm.lpfEnvAmount << ",\n";
        ss << "    \"lpfKbdTrack\": " << pm.lpfKbdTrack << ",\n";
        ss << "    \"lpfVelocityAmount\": " << pm.lpfVelocityAmount << ",\n";

        // HPF
        ss << "    \"hpfCutoff\": " << pm.hpfCutoff << ",\n";
        ss << "    \"hpfResonance\": " << pm.hpfResonance << ",\n";
        ss << "    \"hpfEnvAmount\": " << pm.hpfEnvAmount << ",\n";
        ss << "    \"hpfKbdTrack\": " << pm.hpfKbdTrack << ",\n";

        // Poly Mod
        ss << "    \"polyModFiltEnvAmt\": " << pm.polyModFiltEnvAmt << ",\n";
        ss << "    \"polyModOsc2Amt\": " << pm.polyModOsc2Amt << ",\n";
        ss << "    \"polyModDestOsc1Freq\": " << (pm.polyModDestOsc1Freq ? "true" : "false") << ",\n";
        ss << "    \"polyModDestOsc1Shape\": " << (pm.polyModDestOsc1Shape ? "true" : "false") << ",\n";
        ss << "    \"polyModDestOsc1PW\": " << (pm.polyModDestOsc1PW ? "true" : "false") << ",\n";
        ss << "    \"polyModDestLPF\": " << (pm.polyModDestLPF ? "true" : "false") << ",\n";
        ss << "    \"polyModDestHPF\": " << (pm.polyModDestHPF ? "true" : "false") << ",\n";

        // Envelopes
        ss << "    \"filtAttack\": " << pm.filtAttack << ",\n";
        ss << "    \"filtDecay\": " << pm.filtDecay << ",\n";
        ss << "    \"filtSustain\": " << pm.filtSustain << ",\n";
        ss << "    \"filtRelease\": " << pm.filtRelease << ",\n";

        ss << "    \"ampAttack\": " << pm.ampAttack << ",\n";
        ss << "    \"ampDecay\": " << pm.ampDecay << ",\n";
        ss << "    \"ampSustain\": " << pm.ampSustain << ",\n";
        ss << "    \"ampRelease\": " << pm.ampRelease << ",\n";
        ss << "    \"ampVelocityAmount\": " << pm.ampVelocityAmount << ",\n";

        // LFO
        ss << "    \"lfoShape\": " << static_cast<int>(pm.lfoShape) << ",\n";
        ss << "    \"lfoFrequency\": " << pm.lfoFrequency << ",\n";
        ss << "    \"lfoInitialAmount\": " << pm.lfoInitialAmount << ",\n";
        ss << "    \"lfoSync\": " << (pm.lfoSync ? "true" : "false") << ",\n";
        ss << "    \"lfoDestOsc1Freq\": " << (pm.lfoDestOsc1Freq ? "true" : "false") << ",\n";
        ss << "    \"lfoDestOsc2Freq\": " << (pm.lfoDestOsc2Freq ? "true" : "false") << ",\n";
        ss << "    \"lfoDestOsc1PW\": " << (pm.lfoDestOsc1PW ? "true" : "false") << ",\n";
        ss << "    \"lfoDestOsc2PW\": " << (pm.lfoDestOsc2PW ? "true" : "false") << ",\n";
        ss << "    \"lfoDestLPF\": " << (pm.lfoDestLPF ? "true" : "false") << ",\n";
        ss << "    \"lfoDestHPF\": " << (pm.lfoDestHPF ? "true" : "false") << ",\n";
        ss << "    \"lfoDestAmp\": " << (pm.lfoDestAmp ? "true" : "false") << ",\n";

        // Master
        ss << "    \"slop\": " << pm.slop << ",\n";
        ss << "    \"distortion\": " << pm.distortion << ",\n";
        ss << "    \"panSpread\": " << pm.panSpread << ",\n";
        ss << "    \"masterVolume\": " << pm.masterVolume << ",\n";
        ss << "    \"unison\": " << (pm.unison ? "true" : "false") << ",\n";
        ss << "    \"unisonDetune\": " << pm.unisonDetune << ",\n";
        ss << "    \"glide\": " << pm.glide << ",\n";

        // Effects Engine
        ss << "    \"fxOn\": " << (pm.fxOn ? "true" : "false") << ",\n";
        ss << "    \"fx1Type\": " << pm.fx1Type << ",\n";
        ss << "    \"fx1Param1\": " << pm.fx1Param1 << ",\n";
        ss << "    \"fx1Param2\": " << pm.fx1Param2 << ",\n";
        ss << "    \"fx1Mix\": " << pm.fx1Mix << ",\n";
        ss << "    \"fx2Type\": " << pm.fx2Type << ",\n";
        ss << "    \"fx2Param1\": " << pm.fx2Param1 << ",\n";
        ss << "    \"fx2Param2\": " << pm.fx2Param2 << ",\n";
        ss << "    \"fx2Mix\": " << pm.fx2Mix << "\n";
        ss << "  }\n";
        ss << "}\n";

        return ss.str();
    }

    // Lightweight robust JSON Deserialization
    static bool deserializeFromJson(std::string_view json, Preset& p) {
        if (json.empty()) return false;

        auto extractString = [&](std::string_view key) -> std::string {
            const std::string needle = "\"" + std::string(key) + "\"";
            size_t pos = json.find(needle);
            if (pos == std::string_view::npos) return "";
            pos = json.find(':', pos);
            if (pos == std::string_view::npos) return "";
            pos = json.find('"', pos);
            if (pos == std::string_view::npos) return "";
            size_t end = json.find('"', pos + 1);
            if (end == std::string_view::npos) return "";
            return std::string(json.substr(pos + 1, end - pos - 1));
        };

        auto extractFloat = [&](std::string_view key, float defaultVal) -> float {
            const std::string needle = "\"" + std::string(key) + "\"";
            size_t pos = json.find(needle);
            if (pos == std::string_view::npos) return defaultVal;
            pos = json.find(':', pos);
            if (pos == std::string_view::npos) return defaultVal;
            pos = json.find_first_not_of(" \t\r\n", pos + 1);
            if (pos == std::string_view::npos) return defaultVal;
            size_t end = json.find_first_of(",}\r\n \t", pos);
            if (end == std::string_view::npos) end = json.size();
            std::string token(json.substr(pos, end - pos));
            try {
                return std::stof(token);
            } catch (...) {
                return defaultVal;
            }
        };

        auto extractInt = [&](std::string_view key, int defaultVal) -> int {
            const std::string needle = "\"" + std::string(key) + "\"";
            size_t pos = json.find(needle);
            if (pos == std::string_view::npos) return defaultVal;
            pos = json.find(':', pos);
            if (pos == std::string_view::npos) return defaultVal;
            pos = json.find_first_not_of(" \t\r\n", pos + 1);
            if (pos == std::string_view::npos) return defaultVal;
            size_t end = json.find_first_of(",}\r\n \t", pos);
            if (end == std::string_view::npos) end = json.size();
            std::string token(json.substr(pos, end - pos));
            try {
                return std::stoi(token);
            } catch (...) {
                return defaultVal;
            }
        };

        auto extractBool = [&](std::string_view key, bool defaultVal) -> bool {
            const std::string needle = "\"" + std::string(key) + "\"";
            size_t pos = json.find(needle);
            if (pos == std::string_view::npos) return defaultVal;
            pos = json.find(':', pos);
            if (pos == std::string_view::npos) return defaultVal;
            pos = json.find_first_not_of(" \t\r\n", pos + 1);
            if (pos == std::string_view::npos) return defaultVal;
            return (json.substr(pos, 4) == "true");
        };

        p.programNumber = extractInt("programNumber", p.programNumber);
        std::string nameStr = extractString("name");
        if (!nameStr.empty()) p.setName(nameStr);
        std::string authorStr = extractString("author");
        if (!authorStr.empty()) p.setAuthor(authorStr);
        std::string catStr = extractString("category");
        if (!catStr.empty()) p.category = parseCategoryName(catStr);

        p.arpEnabled = extractBool("enabled", p.arpEnabled);
        p.arpMode = static_cast<ArpMode>(extractInt("mode", static_cast<int>(p.arpMode)));
        p.arpOctave = extractInt("octave", p.arpOctave);
        p.arpDivision = static_cast<ClockDivision>(extractInt("division", static_cast<int>(p.arpDivision)));
        p.arpHold = extractBool("hold", p.arpHold);

        p.seqEnabled = extractBool("seqEnabled", p.seqEnabled);
        p.seqLength = extractInt("length", p.seqLength);
        p.seqBpm = extractFloat("bpm", p.seqBpm);

        auto& pm = p.params;
        pm.osc1Octave = extractInt("osc1Octave", pm.osc1Octave);
        pm.osc1Shape = extractFloat("osc1Shape", pm.osc1Shape);
        pm.osc1PulseWidth = extractFloat("osc1PulseWidth", pm.osc1PulseWidth);
        pm.osc1Sync = extractBool("osc1Sync", pm.osc1Sync);
        pm.osc1Level = extractFloat("osc1Level", pm.osc1Level);
        pm.osc1SubLevel = extractFloat("osc1SubLevel", pm.osc1SubLevel);

        pm.osc2Octave = extractInt("osc2Octave", pm.osc2Octave);
        pm.osc2FineTune = extractFloat("osc2FineTune", pm.osc2FineTune);
        pm.osc2Shape = extractFloat("osc2Shape", pm.osc2Shape);
        pm.osc2PulseWidth = extractFloat("osc2PulseWidth", pm.osc2PulseWidth);
        pm.osc2LowFreq = extractBool("osc2LowFreq", pm.osc2LowFreq);
        pm.osc2KbdTrack = extractBool("osc2KbdTrack", pm.osc2KbdTrack);
        pm.osc2Level = extractFloat("osc2Level", pm.osc2Level);

        pm.noiseLevel = extractFloat("noiseLevel", pm.noiseLevel);

        pm.lpfCutoff = extractFloat("lpfCutoff", pm.lpfCutoff);
        pm.lpfResonance = extractFloat("lpfResonance", pm.lpfResonance);
        pm.lpfEnvAmount = extractFloat("lpfEnvAmount", pm.lpfEnvAmount);
        pm.lpfKbdTrack = extractFloat("lpfKbdTrack", pm.lpfKbdTrack);
        pm.lpfVelocityAmount = extractFloat("lpfVelocityAmount", pm.lpfVelocityAmount);

        pm.hpfCutoff = extractFloat("hpfCutoff", pm.hpfCutoff);
        pm.hpfResonance = extractFloat("hpfResonance", pm.hpfResonance);
        pm.hpfEnvAmount = extractFloat("hpfEnvAmount", pm.hpfEnvAmount);
        pm.hpfKbdTrack = extractFloat("hpfKbdTrack", pm.hpfKbdTrack);

        pm.polyModFiltEnvAmt = extractFloat("polyModFiltEnvAmt", pm.polyModFiltEnvAmt);
        pm.polyModOsc2Amt = extractFloat("polyModOsc2Amt", pm.polyModOsc2Amt);
        pm.polyModDestOsc1Freq = extractBool("polyModDestOsc1Freq", pm.polyModDestOsc1Freq);
        pm.polyModDestOsc1Shape = extractBool("polyModDestOsc1Shape", pm.polyModDestOsc1Shape);
        pm.polyModDestOsc1PW = extractBool("polyModDestOsc1PW", pm.polyModDestOsc1PW);
        pm.polyModDestLPF = extractBool("polyModDestLPF", pm.polyModDestLPF);
        pm.polyModDestHPF = extractBool("polyModDestHPF", pm.polyModDestHPF);

        pm.filtAttack = extractFloat("filtAttack", pm.filtAttack);
        pm.filtDecay = extractFloat("filtDecay", pm.filtDecay);
        pm.filtSustain = extractFloat("filtSustain", pm.filtSustain);
        pm.filtRelease = extractFloat("filtRelease", pm.filtRelease);

        pm.ampAttack = extractFloat("ampAttack", pm.ampAttack);
        pm.ampDecay = extractFloat("ampDecay", pm.ampDecay);
        pm.ampSustain = extractFloat("ampSustain", pm.ampSustain);
        pm.ampRelease = extractFloat("ampRelease", pm.ampRelease);
        pm.ampVelocityAmount = extractFloat("ampVelocityAmount", pm.ampVelocityAmount);

        pm.lfoShape = static_cast<LfoWaveform>(extractInt("lfoShape", static_cast<int>(pm.lfoShape)));
        pm.lfoFrequency = extractFloat("lfoFrequency", pm.lfoFrequency);
        pm.lfoInitialAmount = extractFloat("lfoInitialAmount", pm.lfoInitialAmount);
        pm.lfoSync = extractBool("lfoSync", pm.lfoSync);
        pm.lfoDestOsc1Freq = extractBool("lfoDestOsc1Freq", pm.lfoDestOsc1Freq);
        pm.lfoDestOsc2Freq = extractBool("lfoDestOsc2Freq", pm.lfoDestOsc2Freq);
        pm.lfoDestOsc1PW = extractBool("lfoDestOsc1PW", pm.lfoDestOsc1PW);
        pm.lfoDestOsc2PW = extractBool("lfoDestOsc2PW", pm.lfoDestOsc2PW);
        pm.lfoDestLPF = extractBool("lfoDestLPF", pm.lfoDestLPF);
        pm.lfoDestHPF = extractBool("lfoDestHPF", pm.lfoDestHPF);
        pm.lfoDestAmp = extractBool("lfoDestAmp", pm.lfoDestAmp);

        pm.slop = extractFloat("slop", pm.slop);
        pm.distortion = extractFloat("distortion", pm.distortion);
        pm.panSpread = extractFloat("panSpread", pm.panSpread);
        pm.masterVolume = extractFloat("masterVolume", pm.masterVolume);
        pm.unison = extractBool("unison", pm.unison);
        pm.unisonDetune = extractFloat("unisonDetune", pm.unisonDetune);
        pm.glide = extractFloat("glide", pm.glide);

        pm.fxOn = extractBool("fxOn", pm.fxOn);
        pm.fx1Type = extractInt("fx1Type", pm.fx1Type);
        pm.fx1Param1 = extractFloat("fx1Param1", pm.fx1Param1);
        pm.fx1Param2 = extractFloat("fx1Param2", pm.fx1Param2);
        pm.fx1Mix = extractFloat("fx1Mix", pm.fx1Mix);

        pm.fx2Type = extractInt("fx2Type", pm.fx2Type);
        pm.fx2Param1 = extractFloat("fx2Param1", pm.fx2Param1);
        pm.fx2Param2 = extractFloat("fx2Param2", pm.fx2Param2);
        pm.fx2Mix = extractFloat("fx2Mix", pm.fx2Mix);

        return true;
    }

    // --- Compact Binary Serialization ---

    struct BinaryHeader {
        uint32_t magic = BINARY_MAGIC;
        uint32_t version = BINARY_VERSION;
        uint32_t payloadSize = sizeof(Preset);
        uint32_t checksum = 0;
    };

    static std::vector<uint8_t> serializeToBinary(const Preset& p) {
        BinaryHeader header;
        std::vector<uint8_t> buffer(sizeof(BinaryHeader) + sizeof(Preset));
        std::memcpy(buffer.data(), &header, sizeof(BinaryHeader));
        std::memcpy(buffer.data() + sizeof(BinaryHeader), &p, sizeof(Preset));
        return buffer;
    }

    static bool deserializeFromBinary(const uint8_t* data, size_t size, Preset& p) {
        if (!data || size < sizeof(BinaryHeader) + sizeof(Preset)) return false;
        BinaryHeader header;
        std::memcpy(&header, data, sizeof(BinaryHeader));
        if (header.magic != BINARY_MAGIC || header.version != BINARY_VERSION) return false;
        std::memcpy(&p, data + sizeof(BinaryHeader), sizeof(Preset));
        return true;
    }
};

} // namespace prophet6
