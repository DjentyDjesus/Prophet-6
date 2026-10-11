#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "ProphetLookAndFeel.h"
#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include <cmath>

namespace prophet6 {

// ==============================================================================
// 1. Mini Real-Time Waveshape Display (40 x 24 px)
// ==============================================================================
class MiniWaveDisplayComponent : public juce::Component {
public:
    MiniWaveDisplayComponent() = default;

    void setParams(float shape, float pulseWidth) {
        if (std::abs(shape_ - shape) > 0.005f || std::abs(pw_ - pulseWidth) > 0.005f) {
            shape_ = shape;
            pw_ = pulseWidth;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat();
        // Inset LCD screen background
        g.setColour(juce::Colour(0xff0d0e12));
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(juce::Colour(0xff2a2c32));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);

        // Compute morphing wave path
        juce::Path p;
        const float w = b.getWidth() - 6.0f;
        const float h = b.getHeight() - 6.0f;
        const float left = b.getX() + 3.0f;
        const float midY = b.getY() + 3.0f + h * 0.5f;
        const float amp = h * 0.42f;

        constexpr int numPoints = 36;
        for (int i = 0; i <= numPoints; ++i) {
            const float phase = static_cast<float>(i) / static_cast<float>(numPoints);
            float val = 0.0f;

            if (shape_ <= 0.5f) {
                // Morph Triangle -> Sawtooth
                const float tri = (phase < 0.25f) ? (phase * 4.0f) :
                                  (phase < 0.75f) ? (1.0f - (phase - 0.25f) * 4.0f) :
                                  (-1.0f + (phase - 0.75f) * 4.0f);
                const float saw = 1.0f - 2.0f * phase;
                const float morph = shape_ * 2.0f;
                val = (1.0f - morph) * tri + morph * saw;
            } else {
                // Morph Sawtooth -> Pulse (width controlled by pw_)
                const float saw = 1.0f - 2.0f * phase;
                const float duty = juce::jlimit(0.05f, 0.95f, pw_);
                const float pulse = (phase < duty) ? 1.0f : -1.0f;
                const float morph = (shape_ - 0.5f) * 2.0f;
                val = (1.0f - morph) * saw + morph * pulse;
            }

            const float px = left + phase * w;
            const float py = midY - val * amp;

            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }

        // Vector phosphor trace
        g.setColour(juce::Colour(0x33ffaa33));
        g.strokePath(p, juce::PathStrokeType(2.5f));
        g.setColour(juce::Colour(0xffffbf40));
        g.strokePath(p, juce::PathStrokeType(1.4f));
    }

private:
    float shape_ = 0.5f;
    float pw_ = 0.5f;
};

// ==============================================================================
// 2. Vector Graphical ADSR Envelope Display
// ==============================================================================
class VectorAdsrDisplayComponent : public juce::Component {
public:
    explicit VectorAdsrDisplayComponent(juce::Colour traceColour = juce::Colour(0xff44d0ff))
        : traceColour_(traceColour) {}

    void setAdsr(float a, float d, float s, float r) {
        if (std::abs(a_ - a) > 0.002f || std::abs(d_ - d) > 0.005f ||
            std::abs(s_ - s) > 0.005f || std::abs(r_ - r) > 0.005f)
        {
            a_ = a; d_ = d; s_ = s; r_ = r;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat();
        // LCD glass container
        g.setColour(juce::Colour(0xff0c1014));
        g.fillRoundedRectangle(b, 4.0f);
        g.setColour(juce::Colour(0xff202830));
        g.drawRoundedRectangle(b, 4.0f, 1.0f);

        // Subtle background grid
        g.setColour(juce::Colour(0xff141c22));
        const float w = b.getWidth() - 8.0f;
        const float h = b.getHeight() - 8.0f;
        const float left = b.getX() + 4.0f;
        const float top = b.getY() + 4.0f;

        for (int i = 1; i < 4; ++i) {
            g.drawHorizontalLine(static_cast<int>(top + h * 0.25f * static_cast<float>(i)), left, left + w);
            g.drawVerticalLine(static_cast<int>(left + w * 0.25f * static_cast<float>(i)), top, top + h);
        }

        // Relative ADSR timings
        float aW = std::clamp(a_ / 3.0f, 0.04f, 0.40f);
        float dW = std::clamp(d_ / 3.0f, 0.04f, 0.40f);
        float rW = std::clamp(r_ / 3.0f, 0.04f, 0.40f);
        const float sW_norm = 0.22f;
        const float totalNorm = aW + dW + sW_norm + rW;

        aW = (aW / totalNorm) * w;
        dW = (dW / totalNorm) * w;
        const float sW = (sW_norm / totalNorm) * w;
        rW = (rW / totalNorm) * w;

        const float bottomY = top + h;
        const float peakY   = top + 3.0f;
        const float susY    = bottomY - std::clamp(s_, 0.0f, 1.0f) * (h - 6.0f);

        juce::Path p, fillPath;
        p.startNewSubPath(left, bottomY);
        p.lineTo(left + aW, peakY);
        p.lineTo(left + aW + dW, susY);
        p.lineTo(left + aW + dW + sW, susY);
        p.lineTo(left + aW + dW + sW + rW, bottomY);

        fillPath = p;
        fillPath.lineTo(left, bottomY);
        fillPath.closeSubPath();

        // Shaded fill
        juce::ColourGradient grad(traceColour_.withAlpha(0.30f), left, top,
                                  traceColour_.withAlpha(0.02f), left, bottomY, false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);

        // Vector glow and outline
        g.setColour(traceColour_.withAlpha(0.40f));
        g.strokePath(p, juce::PathStrokeType(2.5f));
        g.setColour(traceColour_);
        g.strokePath(p, juce::PathStrokeType(1.4f));
    }

private:
    juce::Colour traceColour_;
    float a_ = 0.01f, d_ = 0.5f, s_ = 0.7f, r_ = 0.4f;
};

// ==============================================================================
// 3. 1-Wavelength Pitch-Synchronized Pre-Effects Audio Oscilloscope
// ==============================================================================
class AudioOscilloscopeComponent : public juce::Component {
public:
    AudioOscilloscopeComponent() = default;

    void updateWaveform(const float* buffer, size_t bufferSize, float fundamentalFreq, float sampleRate) {
        if (!buffer || bufferSize < 64) return;

        // Window length for 1 wavelength: T = sampleRate / f0
        float periodSamples = (fundamentalFreq > 20.0f && fundamentalFreq < 10000.0f)
            ? (sampleRate / fundamentalFreq)
            : 400.0f;
        periodSamples = std::clamp(periodSamples, 16.0f, 1024.0f);

        // Zero crossing detection with positive slope in recent samples
        size_t startIdx = 0;
        const size_t searchMax = std::min<size_t>(bufferSize - static_cast<size_t>(periodSamples) - 1, 512);
        for (size_t i = 1; i < searchMax; ++i) {
            if (buffer[i - 1] <= 0.0f && buffer[i] > 0.0f) {
                startIdx = i;
                break;
            }
        }

        constexpr size_t numPoints = 120;
        wavePoints_.resize(numPoints);
        for (size_t i = 0; i < numPoints; ++i) {
            const float frac = static_cast<float>(i) / static_cast<float>(numPoints - 1);
            const size_t sampleIdx = startIdx + static_cast<size_t>(frac * periodSamples);
            wavePoints_[i] = (sampleIdx < bufferSize) ? buffer[sampleIdx] : 0.0f;
        }
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat();
        // Oscilloscope bezel
        g.setColour(juce::Colour(0xff090d0a));
        g.fillRoundedRectangle(b, 4.0f);
        g.setColour(juce::Colour(0xff182418));
        g.drawRoundedRectangle(b, 4.0f, 1.2f);

        const float w = b.getWidth() - 8.0f;
        const float h = b.getHeight() - 8.0f;
        const float left = b.getX() + 4.0f;
        const float top = b.getY() + 4.0f;
        const float midY = top + h * 0.5f;

        // Vintage phosphor graticule grid
        g.setColour(juce::Colour(0x1822ff44));
        for (int i = 1; i < 4; ++i) {
            g.drawHorizontalLine(static_cast<int>(top + h * 0.25f * static_cast<float>(i)), left, left + w);
        }
        for (int i = 1; i < 8; ++i) {
            g.drawVerticalLine(static_cast<int>(left + w * 0.125f * static_cast<float>(i)), top, top + h);
        }

        // Center line
        g.setColour(juce::Colour(0x2822ff44));
        g.drawHorizontalLine(static_cast<int>(midY), left, left + w);

        // Header text
        g.setColour(juce::Colour(0x8844ff66));
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText("PRE-FX OSCILLOSCOPE (1-PERIOD)", left + 4, top + 2, 180, 10, juce::Justification::left);

        if (wavePoints_.size() < 2) return;

        // Auto-scale peak
        float maxVal = 0.01f;
        for (float v : wavePoints_) {
            maxVal = std::max(maxVal, std::abs(v));
        }
        const float scale = (h * 0.40f) / std::max(maxVal, 0.05f);

        juce::Path p;
        for (size_t i = 0; i < wavePoints_.size(); ++i) {
            const float px = left + (static_cast<float>(i) / static_cast<float>(wavePoints_.size() - 1)) * w;
            float py = midY - wavePoints_[i] * scale;
            py = std::clamp(py, top + 2.0f, top + h - 2.0f);
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }

        // Outer CRT phosphor glow
        g.setColour(juce::Colour(0x4433ff55));
        g.strokePath(p, juce::PathStrokeType(3.0f));
        // Bright core phosphor line
        g.setColour(juce::Colour(0xff66ff88));
        g.strokePath(p, juce::PathStrokeType(1.4f));
    }

private:
    std::vector<float> wavePoints_;
};

// ==============================================================================
// 4. Performance Controls: Velocity Slider, Pitch Bend & Mod Wheels
// ==============================================================================
class PitchModWheelsComponent : public juce::Component {
public:
    explicit PitchModWheelsComponent(Prophet6AudioProcessor& p) : processorRef_(p) {
        sVelocity_.setSliderStyle(juce::Slider::LinearVertical);
        sVelocity_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        sVelocity_.setRange(1.0, 127.0, 1.0);
        sVelocity_.setValue(100.0);
        sVelocity_.onValueChange = [this]() {
            lVelocityVal_.setText(juce::String(static_cast<int>(sVelocity_.getValue())), juce::dontSendNotification);
        };
        addAndMakeVisible(sVelocity_);

        lVelocityLabel_.setText("VEL", juce::dontSendNotification);
        lVelocityLabel_.setFont(juce::Font(9.0f, juce::Font::bold));
        lVelocityLabel_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lVelocityLabel_);

        lVelocityVal_.setText("100", juce::dontSendNotification);
        lVelocityVal_.setFont(juce::Font(10.0f, juce::Font::bold));
        lVelocityVal_.setColour(juce::Label::textColourId, juce::Colour(0xffff9933));
        lVelocityVal_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lVelocityVal_);
    }

    void resized() override {
        sVelocity_.setBounds(4, 18, 22, 100);
        lVelocityLabel_.setBounds(2, 2, 26, 14);
        lVelocityVal_.setBounds(2, 120, 26, 16);
    }

    void paint(juce::Graphics& g) override {
        // Draw Pitch Wheel & Mod Wheel wells
        // 1. Pitch Bend Wheel Box ($x = 34, w = 38, y = 14, h = 104$)
        const juce::Rectangle<float> pitchWell(34.0f, 14.0f, 38.0f, 104.0f);
        drawWheelWell(g, pitchWell, "PITCH");
        drawWheel(g, pitchWell, pitchNorm_, true);

        // 2. Mod Wheel Box ($x = 80, w = 38, y = 14, h = 104$)
        const juce::Rectangle<float> modWell(80.0f, 14.0f, 38.0f, 104.0f);
        drawWheelWell(g, modWell, "MOD");
        drawWheel(g, modWell, modNorm_, false);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        handleWheelDrag(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        handleWheelDrag(e);
    }

    void mouseUp(const juce::MouseEvent& e) override {
        if (e.x >= 34 && e.x <= 72) {
            // Spring-loaded Pitch Bend wheel snaps back to 0.0
            pitchNorm_ = 0.0f;
            processorRef_.getSynthEngine().pitchBend(0.0f);
            repaint();
        }
    }

    [[nodiscard]] float getVelocity() const noexcept {
        return static_cast<float>(sVelocity_.getValue()) * (1.0f / 127.0f);
    }

private:
    void handleWheelDrag(const juce::MouseEvent& e) {
        if (e.x >= 34 && e.x <= 72) {
            // Pitch Bend (-1.0 to +1.0, center at y = 66)
            const float midY = 66.0f;
            const float delta = (midY - static_cast<float>(e.y)) / 44.0f;
            pitchNorm_ = std::clamp(delta, -1.0f, 1.0f);
            processorRef_.getSynthEngine().pitchBend(pitchNorm_);
            repaint();
        } else if (e.x >= 80 && e.x <= 118) {
            // Mod Wheel (0.0 at bottom y = 110, 1.0 at top y = 22)
            const float val = 1.0f - (static_cast<float>(e.y) - 20.0f) / 88.0f;
            modNorm_ = std::clamp(val, 0.0f, 1.0f);
            processorRef_.getSynthEngine().controlChange(1, static_cast<int>(modNorm_ * 127.0f));
            repaint();
        }
    }

    void drawWheelWell(juce::Graphics& g, const juce::Rectangle<float>& r, const juce::String& label) {
        // Deep recess
        g.setColour(juce::Colour(0xff0e0e12));
        g.fillRoundedRectangle(r, 4.0f);
        g.setColour(juce::Colour(0xff2a2c32));
        g.drawRoundedRectangle(r, 4.0f, 1.0f);

        // Label above well
        g.setColour(juce::Colour(0xffb0b0b0));
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText(label, r.getX() - 4, r.getY() - 13, r.getWidth() + 8, 12, juce::Justification::centred);
    }

    void drawWheel(juce::Graphics& g, const juce::Rectangle<float>& r, float normValue, bool isPitch) {
        // Calculate vertical center of the wheel groove
        const float midY = isPitch ? (r.getCentreY() - normValue * (r.getHeight() * 0.38f))
                                   : (r.getBottom() - 10.0f - normValue * (r.getHeight() * 0.76f));

        const auto wheelRect = r.reduced(4.0f, 2.0f);

        // Cylinder body with vertical shading
        juce::ColourGradient grad(juce::Colour(0xff18181c), wheelRect.getX(), 0.0f,
                                  juce::Colour(0xff3a3a42), wheelRect.getRight(), 0.0f, false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(wheelRect, 3.0f);

        // Ribbed grooves
        constexpr int numRibs = 9;
        for (int i = -numRibs; i <= numRibs; ++i) {
            const float ribY = midY + static_cast<float>(i) * 7.5f;
            if (ribY >= wheelRect.getY() + 4.0f && ribY <= wheelRect.getBottom() - 4.0f) {
                g.setColour(juce::Colour(0xff111114));
                g.drawHorizontalLine(static_cast<int>(ribY), wheelRect.getX() + 2.0f, wheelRect.getRight() - 2.0f);
                g.setColour(juce::Colour(0xff555562));
                g.drawHorizontalLine(static_cast<int>(ribY + 1.0f), wheelRect.getX() + 2.0f, wheelRect.getRight() - 2.0f);
            }
        }

        // Center line on pitch wheel
        if (isPitch) {
            g.setColour(juce::Colour(0xffff4444));
            g.fillRect(wheelRect.getX() + 2.0f, midY - 1.0f, wheelRect.getWidth() - 4.0f, 2.0f);
        }
    }

    Prophet6AudioProcessor& processorRef_;
    juce::Slider sVelocity_;
    juce::Label  lVelocityLabel_, lVelocityVal_;
    float pitchNorm_ = 0.0f;
    float modNorm_   = 0.0f;
};

// ==============================================================================
// 5. Main SoundTower Layout AudioProcessorEditor
// ==============================================================================
class Prophet6AudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit Prophet6AudioProcessorEditor(Prophet6AudioProcessor&);
    ~Prophet6AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void addRotaryKnob(juce::Slider& slider, juce::Label& label,
                       const juce::String& paramId, const juce::String& labelText,
                       bool isPolyMod = false);

    void addToggleButton(juce::ToggleButton& button, const juce::String& paramId,
                         const juce::String& text);

    Prophet6AudioProcessor& processorRef_;
    ProphetLookAndFeel lookAndFeel_;

    // Real-time cached GUI state
    std::array<bool, 6> voiceActiveState_{};
    size_t lastSeqStep_ = 0;
    bool lastSeqRec_ = false;
    int lastProgramIndex_ = -1;
    int blinkCounter_ = 0;

    // --- Header & Presets ---
    juce::TextButton bProgDec_{"-"}, bProgInc_{"+"};
    juce::TextButton bWrite_{"WRITE"}, bCompare_{"COMPARE"};
    juce::TextButton bSettings_{"CFG"};
    juce::ComboBox cCategoryFilter_, cPresetSelect_;
    void populatePresetComboBox(int categoryFilterIndex);

    // --- Clock, Arp & Seq Header ---
    juce::Slider sSeqBpm_;
    juce::Label  lSeqBpm_;
    juce::ToggleButton bArpEnabled_, bArpHold_;
    juce::ComboBox cArpMode_, cArpDivision_;
    juce::Slider sArpOctave_;
    juce::Label  lArpOctave_;
    juce::ToggleButton bSeqEnabled_, bSeqRecord_, bSeqPlay_;
    juce::TextButton bSeqTie_{"TIE"}, bSeqRest_{"REST"};
    juce::Slider sSeqLength_;
    juce::Label  lSeqLength_;

    // --- Oscillators, Slop & Mixer ---
    juce::Slider sOsc1Freq_, sOsc1Shape_, sOsc1PW_, sOsc1SubLevel_;
    juce::Label  lOsc1Freq_, lOsc1Shape_, lOsc1PW_, lOsc1SubLevel_;
    juce::ToggleButton bOsc1Sync_;
    MiniWaveDisplayComponent miniWaveOsc1_;

    juce::Slider sOsc2Freq_, sOsc2Fine_, sOsc2Shape_, sOsc2PW_;
    juce::Label  lOsc2Freq_, lOsc2Fine_, lOsc2Shape_, lOsc2PW_;
    juce::ToggleButton bOsc2LowFreq_, bOsc2KeyTrack_;
    MiniWaveDisplayComponent miniWaveOsc2_;

    juce::Slider sSlop_;
    juce::Label  lSlop_;

    juce::Slider sOsc1Level_, sOsc2Level_, sNoiseLevel_;
    juce::Label  lOsc1Level_, lOsc2Level_, lNoiseLevel_;

    // --- Filters ---
    juce::Slider sHpfCutoff_, sHpfRes_, sHpfEnvAmt_;
    juce::Label  lHpfCutoff_, lHpfRes_, lHpfEnvAmt_;

    juce::Slider sLpfCutoff_, sLpfRes_, sLpfEnvAmt_, sLpfKeyTrack_;
    juce::Label  lLpfCutoff_, lLpfRes_, lLpfEnvAmt_, lLpfKeyTrack_;

    // --- Envelopes ---
    juce::Slider sFiltAtt_, sFiltDec_, sFiltSus_, sFiltRel_;
    juce::Label  lFiltAtt_, lFiltDec_, lFiltSus_, lFiltRel_;
    VectorAdsrDisplayComponent adsrFilter_{juce::Colour(0xff44d0ff)};

    juce::Slider sAmpAtt_, sAmpDec_, sAmpSus_, sAmpRel_;
    juce::Label  lAmpAtt_, lAmpDec_, lAmpSus_, lAmpRel_;
    juce::ToggleButton bVelSens_;
    VectorAdsrDisplayComponent adsrAmp_{juce::Colour(0xffff9933)};

    // --- Poly Mod & Aftertouch ---
    juce::Slider sPolyModFiltEnv_, sPolyModOsc2_;
    juce::Label  lPolyModFiltEnv_, lPolyModOsc2_;
    juce::ToggleButton bPolyModFreq1_, bPolyModShape1_, bPolyModPW1_, bPolyModLPF_, bPolyModHPF_;

    juce::Slider sAftertouchAmt_;
    juce::Label  lAftertouchAmt_;
    juce::ToggleButton bAtFreq1_, bAtFreq2_, bAtLfoAmt_, bAtAmp_, bAtLPF_, bAtHPF_;

    // --- Effects & Oscilloscope ---
    juce::ToggleButton bFxOn_;
    juce::ComboBox cFx1Type_, cFx2Type_;
    juce::Slider sFx1Param1_, sFx1Param2_, sFx1Mix_;
    juce::Label  lFx1Param1_, lFx1Param2_, lFx1Mix_;
    juce::Slider sFx2Param1_, sFx2Param2_, sFx2Mix_;
    juce::Label  lFx2Param1_, lFx2Param2_, lFx2Mix_;
    AudioOscilloscopeComponent oscilloscope_;

    // --- Misc & Master Parameters ---
    juce::Slider sPanSpread_, sMasterVol_;
    juce::Label  lPanSpread_, lMasterVol_;

    // --- LFO, Distortion, Glide, Unison ---
    juce::Slider sLfoFreq_, sLfoAmt_;
    juce::Label  lLfoFreq_, lLfoAmt_;
    juce::ComboBox cLfoShape_;
    juce::ToggleButton bLfoOsc1_, bLfoOsc2_, bLfoLpf_, bLfoHpf_, bLfoAmp_;

    juce::Slider sDistortion_, sGlide_, sUnisonDetune_;
    juce::Label  lDistortion_, lGlide_, lUnisonDetune_;
    juce::ToggleButton bUnison_;

    // --- Performance Controls & Virtual Keyboard Strip ---
    PitchModWheelsComponent performanceWheels_;
    juce::MidiKeyboardComponent keyboardComponent_;

    // APVTS Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Prophet6AudioProcessorEditor)
};

} // namespace prophet6
