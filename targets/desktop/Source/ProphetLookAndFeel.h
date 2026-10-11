#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <cmath>

namespace prophet6 {

class ProphetLookAndFeel : public juce::LookAndFeel_V4 {
public:
    ProphetLookAndFeel() {
        setColour(juce::Label::textColourId, juce::Colour(0xffe8d8b8));
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc0c0c0));
    }

    void drawRotarySlider(
        juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        const bool isPolyMod = slider.getProperties().contains("polyMod");

        // 1. Knob drop shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillEllipse(centre.x - radius + 1.0f, centre.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

        // 2. Fluted Outer Skirt
        const float skirtRadius = radius;
        juce::Colour skirtColorDark  = isPolyMod ? juce::Colour(0xff55111b) : juce::Colour(0xff181818);
        juce::Colour skirtColorLight = isPolyMod ? juce::Colour(0xff8c1e2d) : juce::Colour(0xff323232);

        juce::ColourGradient skirtGrad(skirtColorLight, centre.x - skirtRadius, centre.y - skirtRadius,
                                       skirtColorDark, centre.x + skirtRadius, centre.y + skirtRadius, false);
        g.setGradientFill(skirtGrad);
        g.fillEllipse(centre.x - skirtRadius, centre.y - skirtRadius, skirtRadius * 2.0f, skirtRadius * 2.0f);

        // Fluted ridges around perimeter
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(centre.x - skirtRadius, centre.y - skirtRadius, skirtRadius * 2.0f, skirtRadius * 2.0f, 1.5f);

        constexpr int numFlutes = 20;
        for (int i = 0; i < numFlutes; ++i) {
            const float fluteAngle = (static_cast<float>(i) / static_cast<float>(numFlutes)) * juce::MathConstants<float>::twoPi;
            const float innerR = skirtRadius * 0.82f;
            const float outerR = skirtRadius * 0.98f;
            const float fx1 = centre.x + innerR * std::sin(fluteAngle);
            const float fy1 = centre.y - innerR * std::cos(fluteAngle);
            const float fx2 = centre.x + outerR * std::sin(fluteAngle);
            const float fy2 = centre.y - outerR * std::cos(fluteAngle);

            g.setColour(juce::Colours::black.withAlpha(0.45f));
            g.drawLine(fx1, fy1, fx2, fy2, 1.2f);
        }

        // 3. Central Brushed Aluminum Cap
        const float capRadius = radius * 0.68f;
        juce::ColourGradient capGrad(juce::Colour(0xffe6e6e6), centre.x - capRadius * 0.5f, centre.y - capRadius * 0.5f,
                                     juce::Colour(0xff8a8a8a), centre.x + capRadius * 0.8f, centre.y + capRadius * 0.8f, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

        // Cap metallic bevel border
        g.setColour(juce::Colour(0xff505050));
        g.drawEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

        // 4. White Indicator Line Pointer
        const float lineInner = capRadius * 0.15f;
        const float lineOuter = skirtRadius * 0.92f;
        const float px1 = centre.x + lineInner * std::sin(angle);
        const float py1 = centre.y - lineInner * std::cos(angle);
        const float px2 = centre.x + lineOuter * std::sin(angle);
        const float py2 = centre.y - lineOuter * std::cos(angle);

        // Indicator shadow
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.drawLine(px1 + 1.0f, py1 + 1.0f, px2 + 1.0f, py2 + 1.0f, 2.4f);

        // Bright white indicator line
        g.setColour(juce::Colours::white);
        g.drawLine(px1, py1, px2, py2, 2.2f);
    }

    void drawButtonBackground(
        juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& /*backgroundColour*/,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        const auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const bool isToggled = button.getToggleState();

        // Button casing
        juce::Colour baseColor = shouldDrawButtonAsDown ? juce::Colour(0xff121212) :
                                (shouldDrawButtonAsHighlighted ? juce::Colour(0xff282828) : juce::Colour(0xff1e1e1e));

        g.setColour(baseColor);
        g.fillRoundedRectangle(bounds, 3.0f);

        // Bevel highlight
        g.setColour(juce::Colour(0xff444444));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // Vintage red LED indicator pip on toggled/active state
        const float pipSize = 6.0f;
        const float pipX = bounds.getX() + 5.0f;
        const float pipY = bounds.getCentreY() - pipSize * 0.5f;

        if (isToggled) {
            // Glowing bright red LED
            g.setColour(juce::Colour(0x66ff1111));
            g.fillEllipse(pipX - 2.0f, pipY - 2.0f, pipSize + 4.0f, pipSize + 4.0f);
            g.setColour(juce::Colour(0xffff2222));
            g.fillEllipse(pipX, pipY, pipSize, pipSize);
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.fillEllipse(pipX + 1.5f, pipY + 1.0f, 2.0f, 2.0f);
        } else {
            // Dark inactive LED
            g.setColour(juce::Colour(0xff2a0808));
            g.fillEllipse(pipX, pipY, pipSize, pipSize);
            g.setColour(juce::Colour(0xff1a0404));
            g.drawEllipse(pipX, pipY, pipSize, pipSize, 0.8f);
        }
    }
};

} // namespace prophet6
