#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ProphetLookAndFeel.h"
#include <vector>
#include <memory>

namespace prophet6 {

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

    // Preset Browser & Navigation (Phase 5 & 6)
    juce::TextButton bProgDec_{"-"}, bProgInc_{"+"};
    juce::TextButton bWrite_{"WRITE"}, bCompare_{"COMPARE"};
    juce::TextButton bSettings_{"CFG"};
    juce::ComboBox cCategoryFilter_, cPresetSelect_;
    int blinkCounter_ = 0;
    void populatePresetComboBox(int categoryFilterIndex);

    // Arpeggiator
    juce::ToggleButton bArpEnabled_, bArpHold_;
    juce::ComboBox cArpMode_, cArpDivision_;
    juce::Slider sArpOctave_;
    juce::Label  lArpOctave_;

    // Sequencer
    juce::ToggleButton bSeqEnabled_, bSeqRecord_, bSeqPlay_;
    juce::TextButton bSeqTie_{"TIE"}, bSeqRest_{"REST"};
    juce::Slider sSeqLength_, sSeqBpm_;
    juce::Label  lSeqLength_, lSeqBpm_;

    // Poly Mod
    juce::Slider sPolyModFiltEnv_, sPolyModOsc2_;
    juce::Label  lPolyModFiltEnv_, lPolyModOsc2_;
    juce::ToggleButton bPolyModFreq1_, bPolyModShape1_, bPolyModPW1_, bPolyModLPF_, bPolyModHPF_;

    // Oscillators
    juce::Slider sOsc1Freq_, sOsc1Shape_, sOsc1PW_, sOsc1SubLevel_;
    juce::Label  lOsc1Freq_, lOsc1Shape_, lOsc1PW_, lOsc1SubLevel_;
    juce::ToggleButton bOsc1Sync_;

    juce::Slider sOsc2Freq_, sOsc2Fine_, sOsc2Shape_, sOsc2PW_;
    juce::Label  lOsc2Freq_, lOsc2Fine_, lOsc2Shape_, lOsc2PW_;
    juce::ToggleButton bOsc2LowFreq_, bOsc2KeyTrack_;

    juce::Slider sSlop_;
    juce::Label  lSlop_;

    // Mixer
    juce::Slider sOsc1Level_, sOsc2Level_, sNoiseLevel_;
    juce::Label  lOsc1Level_, lOsc2Level_, lNoiseLevel_;

    // Filters
    juce::Slider sLpfCutoff_, sLpfRes_, sLpfEnvAmt_, sLpfKeyTrack_;
    juce::Label  lLpfCutoff_, lLpfRes_, lLpfEnvAmt_, lLpfKeyTrack_;

    juce::Slider sHpfCutoff_, sHpfRes_, sHpfEnvAmt_;
    juce::Label  lHpfCutoff_, lHpfRes_, lHpfEnvAmt_;

    // Envelopes
    juce::Slider sFiltAtt_, sFiltDec_, sFiltSus_, sFiltRel_;
    juce::Label  lFiltAtt_, lFiltDec_, lFiltSus_, lFiltRel_;

    juce::Slider sAmpAtt_, sAmpDec_, sAmpSus_, sAmpRel_;
    juce::Label  lAmpAtt_, lAmpDec_, lAmpSus_, lAmpRel_;
    juce::ToggleButton bVelSens_;

    // LFO
    juce::Slider sLfoFreq_, sLfoAmt_;
    juce::Label  lLfoFreq_, lLfoAmt_;
    juce::ComboBox cLfoShape_;
    juce::ToggleButton bLfoOsc1_, bLfoOsc2_, bLfoLpf_, bLfoHpf_, bLfoAmp_;

    // Master / Performance
    juce::Slider sDistortion_, sMasterVol_, sPanSpread_, sGlide_, sUnisonDetune_;
    juce::Label  lDistortion_, lMasterVol_, lPanSpread_, lGlide_, lUnisonDetune_;
    juce::ToggleButton bUnison_;

    // Effects Section (Phase 4: Dual Digital Stereo Effects)
    juce::ToggleButton bFxOn_;
    juce::ComboBox cFx1Type_, cFx2Type_;
    juce::Slider sFx1Param1_, sFx1Param2_, sFx1Mix_;
    juce::Label  lFx1Param1_, lFx1Param2_, lFx1Mix_;
    juce::Slider sFx2Param1_, sFx2Param2_, sFx2Mix_;
    juce::Label  lFx2Param1_, lFx2Param2_, lFx2Mix_;

    // Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Prophet6AudioProcessorEditor)
};

} // namespace prophet6
