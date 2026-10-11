#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include "Arpeggiator.hpp"
#include <array>
#include <string_view>
#include <cstring>
#include <cstdint>
#include <algorithm>

namespace prophet6 {

enum class PresetCategory : uint8_t {
    Brass = 0,
    Keys,
    Pad,
    Strings,
    Bass,
    Lead,
    Arp,
    FX,
    NumCategories
};

inline const char* getCategoryName(PresetCategory cat) noexcept {
    switch (cat) {
        case PresetCategory::Brass:   return "BRASS";
        case PresetCategory::Keys:    return "KEYS";
        case PresetCategory::Pad:     return "PAD";
        case PresetCategory::Strings: return "STRINGS";
        case PresetCategory::Bass:    return "BASS";
        case PresetCategory::Lead:    return "LEAD";
        case PresetCategory::Arp:     return "ARP";
        case PresetCategory::FX:      return "FX";
        default:                      return "ALL";
    }
}

inline PresetCategory parseCategoryName(std::string_view name) noexcept {
    if (name == "BRASS" || name == "Brass") return PresetCategory::Brass;
    if (name == "KEYS" || name == "Keys") return PresetCategory::Keys;
    if (name == "PAD" || name == "Pad") return PresetCategory::Pad;
    if (name == "STRINGS" || name == "Strings") return PresetCategory::Strings;
    if (name == "BASS" || name == "Bass") return PresetCategory::Bass;
    if (name == "LEAD" || name == "Lead") return PresetCategory::Lead;
    if (name == "ARP" || name == "Arp") return PresetCategory::Arp;
    if (name == "FX" || name == "Fx") return PresetCategory::FX;
    return PresetCategory::Brass;
}

// Complete Preset Data Structure (POD / Zero-Allocation)
struct Preset {
    std::array<char, 32> name{};
    PresetCategory category = PresetCategory::Brass;
    std::array<char, 32> author{};
    int programNumber = 0; // 000 to 999

    SynthParameters params{};

    // Arpeggiator & Step Sequencer Snapshot
    bool arpEnabled = false;
    ArpMode arpMode = ArpMode::Up;
    int arpOctave = 1;
    ClockDivision arpDivision = ClockDivision::Div_1_16;
    bool arpHold = false;

    bool seqEnabled = false;
    int seqLength = 16;
    float seqBpm = 120.0f;

    void setName(std::string_view n) noexcept {
        name.fill(0);
        const size_t len = std::min(n.size(), name.size() - 1);
        std::memcpy(name.data(), n.data(), len);
        name[len] = '\0';
    }

    void setAuthor(std::string_view a) noexcept {
        author.fill(0);
        const size_t len = std::min(a.size(), author.size() - 1);
        std::memcpy(author.data(), a.data(), len);
        author[len] = '\0';
    }

    [[nodiscard]] std::string_view getName() const noexcept {
        return { name.data() };
    }

    [[nodiscard]] std::string_view getAuthor() const noexcept {
        return { author.data() };
    }
};

} // namespace prophet6
