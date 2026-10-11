#pragma once

#include "Preset.hpp"
#include <array>

namespace prophet6 {

class FactoryPresets {
public:
    static constexpr size_t NUM_FACTORY_PRESETS = 100;

    // Retrieve the static bank of 100 classic Dave Smith Prophet-6 factory patches
    static const std::array<Preset, NUM_FACTORY_PRESETS>& getAllPresets() noexcept;

    // Retrieve single preset by index (clamped to 0..99)
    static const Preset& getPreset(size_t index) noexcept;
};

} // namespace prophet6
