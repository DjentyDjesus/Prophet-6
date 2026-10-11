#include "PluginEditor.h"

namespace prophet6 {

Prophet6AudioProcessorEditor::Prophet6AudioProcessorEditor(Prophet6AudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef_(p)
{
    setLookAndFeel(&lookAndFeel_);

    // 1. Arpeggiator Section
    addToggleButton(bArpEnabled_, "arp_enabled", "ARP ON");
    addToggleButton(bArpHold_,    "arp_hold",    "HOLD");

    cArpMode_.addItemList({"UP", "DOWN", "UP+DOWN", "RANDOM", "ASSIGN"}, 1);
    addAndMakeVisible(cArpMode_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "arp_mode", cArpMode_));

    cArpDivision_.addItemList({"1/2", "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"}, 1);
    addAndMakeVisible(cArpDivision_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "arp_division", cArpDivision_));

    addRotaryKnob(sArpOctave_, lArpOctave_, "arp_octave", "OCTAVE");

    // 2. Step Sequencer Section
    addToggleButton(bSeqEnabled_, "seq_enabled", "SEQ ON");
    addToggleButton(bSeqRecord_,  "seq_record",  "RECORD");
    addToggleButton(bSeqPlay_,    "seq_play",    "PLAY");

    bSeqTie_.onClick  = [this] { processorRef_.recordTie(); };
    bSeqRest_.onClick = [this] { processorRef_.recordRest(); };
    addAndMakeVisible(bSeqTie_);
    addAndMakeVisible(bSeqRest_);

    addRotaryKnob(sSeqLength_, lSeqLength_, "seq_length", "LENGTH");
    addRotaryKnob(sSeqBpm_,    lSeqBpm_,    "seq_bpm",    "BPM");

    // 3. Poly Mod Section
    addRotaryKnob(sPolyModFiltEnv_, lPolyModFiltEnv_, "polymod_filter_env", "FILT ENV", true);
    addRotaryKnob(sPolyModOsc2_,    lPolyModOsc2_,    "polymod_osc2",       "OSC 2",    true);
    addToggleButton(bPolyModFreq1_,  "polymod_dest_osc1_freq",  "FREQ 1");
    addToggleButton(bPolyModShape1_, "polymod_dest_osc1_shape", "SHAPE 1");
    addToggleButton(bPolyModPW1_,    "polymod_dest_osc1_pw",    "PW 1");
    addToggleButton(bPolyModLPF_,    "polymod_dest_lpf",        "LPF");
    addToggleButton(bPolyModHPF_,    "polymod_dest_hpf",        "HPF");

    // 4. Oscillators
    addRotaryKnob(sOsc1Freq_,     lOsc1Freq_,     "osc1_freq",      "OCTAVE");
    addRotaryKnob(sOsc1Shape_,    lOsc1Shape_,    "osc1_wave",      "SHAPE");
    addRotaryKnob(sOsc1PW_,       lOsc1PW_,       "osc1_pw",        "PW");
    addRotaryKnob(sOsc1SubLevel_, lOsc1SubLevel_, "osc1_sub_level", "SUB OCT");
    addToggleButton(bOsc1Sync_, "sync", "SYNC");

    addRotaryKnob(sOsc2Freq_,  lOsc2Freq_,  "osc2_freq", "OCTAVE");
    addRotaryKnob(sOsc2Fine_,  lOsc2Fine_,  "osc2_fine", "FINE");
    addRotaryKnob(sOsc2Shape_, lOsc2Shape_, "osc2_wave", "SHAPE");
    addRotaryKnob(sOsc2PW_,    lOsc2PW_,    "osc2_pw",   "PW");
    addToggleButton(bOsc2LowFreq_,  "osc2_low_freq",  "LOW FREQ");
    addToggleButton(bOsc2KeyTrack_, "osc2_key_track", "KEY TRACK");

    addRotaryKnob(sSlop_, lSlop_, "slop", "SLOP");

    // 5. Mixer
    addRotaryKnob(sOsc1Level_,  lOsc1Level_,  "osc1_level",  "OSC 1");
    addRotaryKnob(sOsc2Level_,  lOsc2Level_,  "osc2_level",  "OSC 2");
    addRotaryKnob(sNoiseLevel_, lNoiseLevel_, "noise_level", "NOISE");

    // 6. Filters
    addRotaryKnob(sLpfCutoff_,   lLpfCutoff_,   "lpf_cutoff",     "CUTOFF");
    addRotaryKnob(sLpfRes_,      lLpfRes_,      "lpf_resonance",  "RESONANCE");
    addRotaryKnob(sLpfEnvAmt_,   lLpfEnvAmt_,   "lpf_env_amount", "ENV AMT");
    addRotaryKnob(sLpfKeyTrack_, lLpfKeyTrack_, "lpf_key_track",  "KEY AMT");

    addRotaryKnob(sHpfCutoff_, lHpfCutoff_, "hpf_cutoff",     "CUTOFF");
    addRotaryKnob(sHpfRes_,    lHpfRes_,    "hpf_resonance",  "RESONANCE");
    addRotaryKnob(sHpfEnvAmt_, lHpfEnvAmt_, "hpf_env_amount", "ENV AMT");

    // 7. Envelopes
    addRotaryKnob(sFiltAtt_, lFiltAtt_, "filt_attack",  "ATTACK");
    addRotaryKnob(sFiltDec_, lFiltDec_, "filt_decay",   "DECAY");
    addRotaryKnob(sFiltSus_, lFiltSus_, "filt_sustain", "SUSTAIN");
    addRotaryKnob(sFiltRel_, lFiltRel_, "filt_release", "RELEASE");

    addRotaryKnob(sAmpAtt_, lAmpAtt_, "amp_attack",  "ATTACK");
    addRotaryKnob(sAmpDec_, lAmpDec_, "amp_decay",   "DECAY");
    addRotaryKnob(sAmpSus_, lAmpSus_, "amp_sustain", "SUSTAIN");
    addRotaryKnob(sAmpRel_, lAmpRel_, "amp_release", "RELEASE");
    addToggleButton(bVelSens_, "velocity_sensitivity", "VELOCITY");

    // 8. LFO
    addRotaryKnob(sLfoFreq_, lLfoFreq_, "lfo_freq",   "FREQUENCY");
    addRotaryKnob(sLfoAmt_,  lLfoAmt_,  "lfo_amount", "INITIAL AMT");

    cLfoShape_.addItemList({"TRIANGLE", "SAWTOOTH", "REV SAW", "SQUARE", "RANDOM"}, 1);
    addAndMakeVisible(cLfoShape_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "lfo_shape", cLfoShape_));

    addToggleButton(bLfoOsc1_, "lfo_dest_osc1_freq", "FREQ 1");
    addToggleButton(bLfoOsc2_, "lfo_dest_osc2_freq", "FREQ 2");
    addToggleButton(bLfoLpf_,  "lfo_dest_lpf",       "LPF");
    addToggleButton(bLfoHpf_,  "lfo_dest_hpf",       "HPF");
    addToggleButton(bLfoAmp_,  "lfo_dest_amp",       "AMP");

    // 9. Master & Performance
    addRotaryKnob(sDistortion_,   lDistortion_,   "distortion_amount", "DISTORTION");
    addRotaryKnob(sMasterVol_,    lMasterVol_,    "master_volume",     "VOLUME");
    addRotaryKnob(sPanSpread_,    lPanSpread_,    "pan_spread",        "PAN SPREAD");
    addRotaryKnob(sGlide_,        lGlide_,        "glide",             "GLIDE");
    addRotaryKnob(sUnisonDetune_, lUnisonDetune_, "unison_detune",     "DETUNE");
    addToggleButton(bUnison_, "unison", "UNISON");

    // 10. Effects Section (Phase 4: Dual Digital Stereo Effects Engine)
    addToggleButton(bFxOn_, "fx_on", "FX ON");

    const juce::StringArray fxList = {
        "BBD DELAY", "DIGITAL DELAY", "TAPE DELAY", "CHORUS", "FLANGER",
        "PHASER", "RING MOD", "PLATE REVERB", "HALL REVERB", "OFF"
    };

    cFx1Type_.addItemList(fxList, 1);
    addAndMakeVisible(cFx1Type_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "fx1_type", cFx1Type_));

    addRotaryKnob(sFx1Param1_, lFx1Param1_, "fx1_param1", "PARAM 1");
    addRotaryKnob(sFx1Param2_, lFx1Param2_, "fx1_param2", "PARAM 2");
    addRotaryKnob(sFx1Mix_,    lFx1Mix_,    "fx1_mix",    "MIX");

    cFx2Type_.addItemList(fxList, 1);
    addAndMakeVisible(cFx2Type_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "fx2_type", cFx2Type_));

    addRotaryKnob(sFx2Param1_, lFx2Param1_, "fx2_param1", "PARAM 1");
    addRotaryKnob(sFx2Param2_, lFx2Param2_, "fx2_param2", "PARAM 2");
    addRotaryKnob(sFx2Mix_,    lFx2Mix_,    "fx2_mix",    "MIX");

    // Phase 5 & 6: Preset Navigation, Hardware Write & Compare, and Standalone Settings
    bProgDec_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
    bProgDec_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8d8b8));
    bProgDec_.onClick = [this] {
        if (processorRef_.isWriteArmed()) {
            int dest = processorRef_.getWriteDestination();
            int next = (dest <= 100) ? 999 : dest - 1;
            processorRef_.setWriteDestination(next);
            repaint(220, 8, 510, 72);
        } else {
            const int maxProg = static_cast<int>(processorRef_.getPresetManager().getNumPresets()) - 1;
            int cur = processorRef_.getCurrentProgram();
            int next = (cur <= 0) ? maxProg : cur - 1;
            processorRef_.setCurrentProgram(next);
            lastProgramIndex_ = next;
            cPresetSelect_.setSelectedId(next + 1, juce::dontSendNotification);
            repaint(220, 8, 510, 72);
        }
    };
    addAndMakeVisible(bProgDec_);

    bProgInc_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
    bProgInc_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8d8b8));
    bProgInc_.onClick = [this] {
        if (processorRef_.isWriteArmed()) {
            int dest = processorRef_.getWriteDestination();
            int next = (dest >= 999) ? 100 : dest + 1;
            processorRef_.setWriteDestination(next);
            repaint(220, 8, 510, 72);
        } else {
            const int maxProg = static_cast<int>(processorRef_.getPresetManager().getNumPresets()) - 1;
            int cur = processorRef_.getCurrentProgram();
            int next = (cur >= maxProg) ? 0 : cur + 1;
            processorRef_.setCurrentProgram(next);
            lastProgramIndex_ = next;
            cPresetSelect_.setSelectedId(next + 1, juce::dontSendNotification);
            repaint(220, 8, 510, 72);
        }
    };
    addAndMakeVisible(bProgInc_);

    bWrite_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
    bWrite_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff07070));
    bWrite_.onClick = [this] {
        if (!processorRef_.isWriteArmed()) {
            processorRef_.armWrite();
            bWrite_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff881818));
        } else {
            processorRef_.confirmWrite();
            bWrite_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
            populatePresetComboBox(cCategoryFilter_.getSelectedId());
            cPresetSelect_.setSelectedId(processorRef_.getCurrentProgram() + 1, juce::dontSendNotification);
        }
        repaint(220, 8, 510, 72);
    };
    addAndMakeVisible(bWrite_);

    bCompare_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
    bCompare_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8c890));
    bCompare_.onClick = [this] {
        processorRef_.toggleCompare();
        if (processorRef_.isComparing()) {
            bCompare_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff774411));
        } else {
            bCompare_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
        }
        repaint(220, 8, 510, 72);
    };
    addAndMakeVisible(bCompare_);

    bSettings_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a1810));
    bSettings_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffecd8ba));
    bSettings_.setTooltip("Audio and MIDI Device Settings");
    bSettings_.onClick = [] {
        bool opened = false;
        if (auto* topWindow = juce::TopLevelWindow::getActiveTopLevelWindow()) {
            for (auto* child : topWindow->getChildren()) {
                if (auto* btn = dynamic_cast<juce::Button*>(child)) {
                    if (btn->getButtonText() == "Options" || (btn->getX() == 8 && btn->getY() == 6)) {
                        btn->triggerClick();
                        opened = true;
                        break;
                    }
                }
            }
        }
        if (!opened) {
            auto& desktop = juce::Desktop::getInstance();
            for (int i = 0; i < desktop.getNumComponents(); ++i) {
                if (auto* comp = desktop.getComponent(i)) {
                    for (auto* child : comp->getChildren()) {
                        if (auto* btn = dynamic_cast<juce::Button*>(child)) {
                            if (btn->getButtonText() == "Options" || (btn->getX() == 8 && btn->getY() == 6)) {
                                btn->triggerClick();
                                opened = true;
                                break;
                            }
                        }
                    }
                    if (opened) break;
                }
            }
        }
        if (!opened) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "Prophet-6 Audio Settings",
                "In DAW/Plugin mode, audio drivers (ASIO/WASAPI), buffer size, and sample rate are managed by your host application.",
                "OK");
        }
    };
    addAndMakeVisible(bSettings_);

    cCategoryFilter_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff181818));
    cCategoryFilter_.setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8c890));
    cCategoryFilter_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a3a3a));
    cCategoryFilter_.addItem("ALL", 1);
    cCategoryFilter_.addItem("BRASS", 2);
    cCategoryFilter_.addItem("KEYS", 3);
    cCategoryFilter_.addItem("PAD", 4);
    cCategoryFilter_.addItem("STRINGS", 5);
    cCategoryFilter_.addItem("BASS", 6);
    cCategoryFilter_.addItem("LEAD", 7);
    cCategoryFilter_.addItem("ARP", 8);
    cCategoryFilter_.addItem("FX", 9);
    cCategoryFilter_.setSelectedId(1, juce::dontSendNotification);
    cCategoryFilter_.onChange = [this] {
        populatePresetComboBox(cCategoryFilter_.getSelectedId());
    };
    addAndMakeVisible(cCategoryFilter_);

    cPresetSelect_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff181818));
    cPresetSelect_.setColour(juce::ComboBox::textColourId, juce::Colour(0xffffffff));
    cPresetSelect_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff444444));
    populatePresetComboBox(1);
    cPresetSelect_.onChange = [this] {
        int selId = cPresetSelect_.getSelectedId();
        if (selId > 0) {
            int progIndex = selId - 1;
            if (processorRef_.isWriteArmed()) {
                if (progIndex >= 100) {
                    processorRef_.setWriteDestination(progIndex);
                    repaint(220, 8, 510, 72);
                }
            } else {
                if (progIndex != processorRef_.getCurrentProgram()) {
                    processorRef_.setCurrentProgram(progIndex);
                    lastProgramIndex_ = progIndex;
                    repaint(220, 8, 510, 72);
                }
            }
        }
    };
    addAndMakeVisible(cPresetSelect_);

    setSize(1100, 720);
    startTimerHz(40); // 40 Hz refresh for voice activity LEDs & sequencer step counter
}

Prophet6AudioProcessorEditor::~Prophet6AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void Prophet6AudioProcessorEditor::populatePresetComboBox(int categoryFilterIndex)
{
    cPresetSelect_.clear(juce::dontSendNotification);
    const auto& pm = processorRef_.getPresetManager();
    for (size_t i = 0; i < pm.getNumPresets(); ++i) {
        const auto& p = pm.getPreset(i);
        bool include = false;
        if (categoryFilterIndex <= 1) { // ALL
            include = true;
        } else {
            int catInt = static_cast<int>(p.category);
            // 2: BRASS(0), 3: KEYS(1), 4: PAD(2), 5: STRINGS(3), 6: BASS(4), 7: LEAD(5), 8: ARP(6), 9: FX(7)
            if (catInt == categoryFilterIndex - 2) {
                include = true;
            }
        }
        if (include) {
            juce::String label = juce::String::formatted("%03d: ", static_cast<int>(i)) + juce::String(p.getName().data());
            cPresetSelect_.addItem(label, static_cast<int>(i + 1));
        }
    }
    cPresetSelect_.setSelectedId(processorRef_.getCurrentProgram() + 1, juce::dontSendNotification);
}

void Prophet6AudioProcessorEditor::addRotaryKnob(
    juce::Slider& slider, juce::Label& label,
    const juce::String& paramId, const juce::String& text,
    bool isPolyMod)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    if (isPolyMod) {
        slider.getProperties().set("polyMod", true);
    }
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(10.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, isPolyMod ? juce::Colour(0xfff0a0a0) : juce::Colour(0xffe8d8b8));
    addAndMakeVisible(label);

    sliderAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processorRef_.getAPVTS(), paramId, slider));
}

void Prophet6AudioProcessorEditor::addToggleButton(
    juce::ToggleButton& button, const juce::String& paramId, const juce::String& text)
{
    button.setButtonText(text);
    addAndMakeVisible(button);

    buttonAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            processorRef_.getAPVTS(), paramId, button));
}

void Prophet6AudioProcessorEditor::timerCallback()
{
    bool needRepaint = false;

    blinkCounter_ = (blinkCounter_ + 1) % 20;
    if (processorRef_.isWriteArmed()) {
        needRepaint = true;
    }

    // Check voice LEDs
    for (int i = 0; i < 6; ++i) {
        const bool active = processorRef_.isVoiceActive(i);
        if (active != voiceActiveState_[static_cast<size_t>(i)]) {
            voiceActiveState_[static_cast<size_t>(i)] = active;
            needRepaint = true;
        }
    }

    // Check Sequencer step
    const size_t currentStep = processorRef_.getCurrentSequenceStep();
    const bool isRec = processorRef_.isSequencerRecording();
    if (currentStep != lastSeqStep_ || isRec != lastSeqRec_) {
        lastSeqStep_ = currentStep;
        lastSeqRec_ = isRec;
        needRepaint = true;
    }

    // Check Preset program index
    const int curProg = processorRef_.getCurrentProgram();
    if (curProg != lastProgramIndex_) {
        lastProgramIndex_ = curProg;
        cPresetSelect_.setSelectedId(curProg + 1, juce::dontSendNotification);
        needRepaint = true;
    }

    if (needRepaint) {
        // Repaint the top display area
        repaint(220, 8, 510, 72);
    }
}

void Prophet6AudioProcessorEditor::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // 1. Central Matte Black Aluminum Chassis
    g.setColour(juce::Colour(0xff141414));
    g.fillRect(0, 0, w, h);

    // Hairline brushed aluminum metallic gradient
    juce::ColourGradient chassisGrad(juce::Colour(0xff202020), 0.0f, 0.0f,
                                     juce::Colour(0xff111111), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(chassisGrad);
    g.fillRect(42, 0, w - 84, h);

    // 2. Walnut Wood Side Panels (Cheeks)
    auto drawWoodCheek = [&](int x, int width) {
        juce::ColourGradient woodGrad(juce::Colour(0xff442210), static_cast<float>(x), 0.0f,
                                      juce::Colour(0xff220e04), static_cast<float>(x + width), static_cast<float>(h), false);
        g.setGradientFill(woodGrad);
        g.fillRect(x, 0, width, h);

        // Wood grain streaks
        for (int i = 0; i < h; i += 8) {
            g.setColour(juce::Colours::black.withAlpha(0.12f));
            g.drawHorizontalLine(i, static_cast<float>(x), static_cast<float>(x + width));
        }

        // Metallic bevel join shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRect(x == 0 ? width - 2 : x, 0, 2, h);
    };
    drawWoodCheek(0, 42);
    drawWoodCheek(w - 42, 42);

    // 3. Header Branding
    g.setColour(juce::Colour(0xffecd8ba));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("SEQUENTIAL", 55, 10, 140, 20, juce::Justification::left);

    g.setColour(juce::Colour(0xffe04040));
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("PROPHET-6", 55, 28, 150, 22, juce::Justification::left);

    g.setColour(juce::Colour(0xffa09080));
    g.setFont(juce::Font(9.0f, juce::Font::plain));
    g.drawText("6-VOICE DISCRETE POLYPHONIC SYNTHESIZER", 55, 52, 170, 14, juce::Justification::left);

    // 4. Retro 7-Segment LED Patch Display (Phase 5 & 6)
    const juce::Rectangle<int> patchDisplay(230, 14, 80, 52);
    g.setColour(juce::Colour(0xff080000));
    g.fillRoundedRectangle(patchDisplay.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff401010));
    g.drawRoundedRectangle(patchDisplay.toFloat(), 4.0f, 1.5f);

    const bool isArmed = processorRef_.isWriteArmed();
    const bool isCmp = processorRef_.isComparing();

    if (isArmed) {
        // Blinking "WRITE TO" indicator & destination slot
        g.setColour(juce::Colour(0xffff4040));
        g.setFont(juce::Font(8.0f, juce::Font::bold));
        g.drawText("WRITE TO", patchDisplay.getX() + 6, patchDisplay.getY() + 3, 68, 10, juce::Justification::left);

        // Ghost background segments ("888")
        g.setColour(juce::Colour(0x18440000));
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("888", patchDisplay.withTrimmedRight(6).withTrimmedTop(6), juce::Justification::centredRight);

        const bool blinkOn = (blinkCounter_ / 5) % 2 == 0;
        if (blinkOn) {
            const int destSlot = processorRef_.getWriteDestination();
            const juce::String destStr = juce::String::formatted("%03d", destSlot);
            g.setColour(juce::Colour(0x44ff1818));
            g.drawText(destStr, patchDisplay.withTrimmedRight(6).withTrimmedTop(6).expanded(1), juce::Justification::centredRight);
            g.setColour(juce::Colour(0xffff2828));
            g.drawText(destStr, patchDisplay.withTrimmedRight(6).withTrimmedTop(6), juce::Justification::centredRight);
        }
    } else if (isCmp) {
        // Compare Mode active: amber "COMPARE" indicator
        g.setColour(juce::Colour(0xffffaa30));
        g.setFont(juce::Font(8.0f, juce::Font::bold));
        g.drawText("COMPARE", patchDisplay.getX() + 6, patchDisplay.getY() + 3, 68, 10, juce::Justification::left);

        g.setColour(juce::Colour(0x20442200));
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("888", patchDisplay.withTrimmedRight(6).withTrimmedTop(6), juce::Justification::centredRight);

        const int progNum = processorRef_.getCurrentProgram();
        const juce::String progStr = juce::String::formatted("%03d", progNum);
        g.setColour(juce::Colour(0x55ffaa20));
        g.drawText(progStr, patchDisplay.withTrimmedRight(6).withTrimmedTop(6).expanded(1), juce::Justification::centredRight);
        g.setColour(juce::Colour(0xffffb830));
        g.drawText(progStr, patchDisplay.withTrimmedRight(6).withTrimmedTop(6), juce::Justification::centredRight);
    } else {
        // Normal Mode: Red LED program digits
        g.setColour(juce::Colour(0xff882020));
        g.setFont(juce::Font(8.0f, juce::Font::bold));
        g.drawText("PROGRAM", patchDisplay.getX() + 6, patchDisplay.getY() + 3, 60, 10, juce::Justification::left);

        // Ghost background segments ("888")
        g.setColour(juce::Colour(0x18440000));
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("888", patchDisplay.withTrimmedRight(6).withTrimmedTop(6), juce::Justification::centredRight);

        // Glowing red digits
        const int progNum = processorRef_.getCurrentProgram();
        const juce::String progStr = juce::String::formatted("%03d", progNum);
        g.setColour(juce::Colour(0x44ff1818));
        g.drawText(progStr, patchDisplay.withTrimmedRight(6).withTrimmedTop(6).expanded(1), juce::Justification::centredRight);
        g.setColour(juce::Colour(0xffff2828));
        g.drawText(progStr, patchDisplay.withTrimmedRight(6).withTrimmedTop(6), juce::Justification::centredRight);
    }

    // 5. Retro 7-Segment LED Step Sequencer Display
    const juce::Rectangle<int> seqDisplay(525, 14, 80, 52);
    g.setColour(juce::Colour(0xff080000));
    g.fillRoundedRectangle(seqDisplay.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff401010));
    g.drawRoundedRectangle(seqDisplay.toFloat(), 4.0f, 1.5f);

    g.setColour(juce::Colour(0xff882020));
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.drawText("STEP SEQ", seqDisplay.getX() + 6, seqDisplay.getY() + 3, 60, 10, juce::Justification::left);

    g.setColour(juce::Colour(0x18440000));
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("88:88", seqDisplay.withTrimmedTop(6), juce::Justification::centred);

    g.setColour(juce::Colour(0xffff2020));
    const juce::String stepText = lastSeqRec_ ? "REC" : (juce::String("ST ") + juce::String::formatted("%02d", static_cast<int>(lastSeqStep_ + 1)));
    g.drawText(stepText, seqDisplay.withTrimmedTop(6), juce::Justification::centred);

    // 6. 6 Voice Activity LEDs
    const int ledBarX = 615;
    const int ledBarY = 16;
    g.setColour(juce::Colour(0xffc0b090));
    g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.drawText("VOICE ACTIVITY", ledBarX, ledBarY, 105, 14, juce::Justification::centred);

    for (int i = 0; i < 6; ++i) {
        const float lampX = static_cast<float>(ledBarX + 6 + i * 17);
        const float lampY = static_cast<float>(ledBarY + 18);
        const float lampSize = 13.0f;

        g.setColour(juce::Colour(0xff3a3a3a));
        g.fillEllipse(lampX - 1.0f, lampY - 1.0f, lampSize + 2.0f, lampSize + 2.0f);

        const bool active = voiceActiveState_[static_cast<size_t>(i)];
        if (active) {
            g.setColour(juce::Colour(0x55ff1111));
            g.fillEllipse(lampX - 3.0f, lampY - 3.0f, lampSize + 6.0f, lampSize + 6.0f);

            g.setColour(juce::Colour(0xffff2828));
            g.fillEllipse(lampX, lampY, lampSize, lampSize);

            g.setColour(juce::Colours::white.withAlpha(0.75f));
            g.fillEllipse(lampX + 3.0f, lampY + 2.5f, 3.5f, 3.5f);
        } else {
            g.setColour(juce::Colour(0xff250505));
            g.fillEllipse(lampX, lampY, lampSize, lampSize);
        }

        g.setColour(juce::Colour(0xff908070));
        g.setFont(juce::Font(8.5f, juce::Font::plain));
        g.drawText(juce::String(i + 1), static_cast<int>(lampX - 2.0f), static_cast<int>(lampY + 15.0f), 17, 11, juce::Justification::centred);
    }

    // 7. Section Boxes & Boundaries
    auto drawSection = [&](int sx, int sy, int sw, int sh, const juce::String& title, juce::Colour titleCol = juce::Colour(0xffecd8ba)) {
        const auto b = juce::Rectangle<int>(sx, sy, sw, sh).toFloat();
        g.setColour(juce::Colour(0x18ffffff));
        g.drawRoundedRectangle(b, 4.0f, 1.0f);

        g.setColour(juce::Colour(0xff181818));
        g.fillRect(sx + 8, sy - 8, sw - 16, 16);

        g.setColour(titleCol);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText(title, sx + 8, sy - 8, sw - 16, 16, juce::Justification::centred);
    };

    // Upper row sections (height 240)
    drawSection(50,  88, 140, 242, "ARPEGGIATOR", juce::Colour(0xffe8c890));
    drawSection(195, 88, 155, 242, "STEP SEQUENCER", juce::Colour(0xffe8c890));
    drawSection(355, 88, 165, 242, "POLY MOD", juce::Colour(0xfff07070));
    drawSection(525, 88, 385, 242, "OSCILLATORS");
    drawSection(915, 88, 135, 242, "MIXER");

    // Lower row sections (height 245)
    drawSection(50,  340, 225, 245, "LOW-PASS FILTER (4-POLE)");
    drawSection(280, 340, 175, 245, "HIGH-PASS FILTER (2-POLE)");
    drawSection(460, 340, 220, 245, "FILTER ENVELOPE");
    drawSection(685, 340, 220, 245, "AMPLIFIER");
    drawSection(910, 340, 140, 245, "LFO");

    // Effects row sections (height 112)
    drawSection(50,  595, 120, 112, "TRUE BYPASS", juce::Colour(0xfff07070));
    drawSection(180, 595, 430, 112, "EFFECTS 1 (FX A)", juce::Colour(0xffe8c890));
    drawSection(620, 595, 430, 112, "EFFECTS 2 (FX B)", juce::Colour(0xffe8c890));
}

void Prophet6AudioProcessorEditor::resized()
{
    // Preset Browser & Navigation Controls (Phase 5 & 6)
    bProgDec_.setBounds(313, 16, 24, 23);
    bProgInc_.setBounds(313, 42, 24, 23);
    bWrite_.setBounds(340, 16, 58, 23);
    bCompare_.setBounds(340, 42, 58, 23);
    cCategoryFilter_.setBounds(402, 16, 118, 23);
    cPresetSelect_.setBounds(402, 42, 118, 23);

    // Standalone Settings Button on top-right wood cheek
    bSettings_.setBounds(1060, 14, 36, 22);

    // Top Bar Master Controls
    sSlop_.setBounds(730, 20, 48, 48);
    lSlop_.setBounds(730, 68, 48, 14);

    sDistortion_.setBounds(785, 20, 48, 48);
    lDistortion_.setBounds(785, 68, 48, 14);

    sPanSpread_.setBounds(840, 20, 48, 48);
    lPanSpread_.setBounds(840, 68, 48, 14);

    sGlide_.setBounds(895, 20, 48, 48);
    lGlide_.setBounds(895, 68, 48, 14);

    bUnison_.setBounds(950, 18, 85, 24);
    sUnisonDetune_.setBounds(955, 42, 40, 36);
    lUnisonDetune_.setBounds(945, 78, 60, 12);

    sMasterVol_.setBounds(1005, 36, 44, 44);
    lMasterVol_.setBounds(1005, 80, 44, 12);

    // 1. Arpeggiator (x: 50, y: 88, w: 140, h: 242)
    bArpEnabled_.setBounds(58, 110, 62, 22);
    bArpHold_.setBounds(122, 110, 62, 22);

    cArpMode_.setBounds(58, 140, 124, 22);
    cArpDivision_.setBounds(58, 170, 124, 22);

    sArpOctave_.setBounds(95, 205, 50, 50);
    lArpOctave_.setBounds(90, 258, 60, 14);

    // 2. Step Sequencer (x: 195, y: 88, w: 155, h: 242)
    bSeqEnabled_.setBounds(202, 110, 66, 22);
    bSeqPlay_.setBounds(272, 110, 66, 22);

    bSeqRecord_.setBounds(202, 138, 72, 22);
    bSeqTie_.setBounds(202, 168, 64, 22);
    bSeqRest_.setBounds(274, 168, 64, 22);

    sSeqLength_.setBounds(205, 205, 50, 50);
    lSeqLength_.setBounds(200, 258, 60, 14);

    sSeqBpm_.setBounds(280, 205, 50, 50);
    lSeqBpm_.setBounds(275, 258, 60, 14);

    // 3. Poly Mod (x: 355, y: 88, w: 165, h: 242)
    sPolyModFiltEnv_.setBounds(365, 110, 65, 65);
    lPolyModFiltEnv_.setBounds(365, 176, 65, 14);
    sPolyModOsc2_.setBounds(445, 110, 65, 65);
    lPolyModOsc2_.setBounds(445, 176, 65, 14);

    bPolyModFreq1_.setBounds(362, 200, 75, 20);
    bPolyModShape1_.setBounds(362, 222, 75, 20);
    bPolyModPW1_.setBounds(362, 244, 75, 20);
    bPolyModLPF_.setBounds(445, 200, 70, 20);
    bPolyModHPF_.setBounds(445, 222, 70, 20);

    // 4. Oscillators (x: 525, y: 88, w: 385, h: 242)
    // Osc 1
    sOsc1Freq_.setBounds(535, 110, 48, 48);
    lOsc1Freq_.setBounds(535, 160, 48, 14);
    sOsc1Shape_.setBounds(585, 110, 48, 48);
    lOsc1Shape_.setBounds(585, 160, 48, 14);
    sOsc1PW_.setBounds(635, 110, 48, 48);
    lOsc1PW_.setBounds(635, 160, 48, 14);
    sOsc1SubLevel_.setBounds(685, 110, 48, 48);
    lOsc1SubLevel_.setBounds(685, 160, 48, 14);
    bOsc1Sync_.setBounds(650, 185, 65, 20);

    // Osc 2
    sOsc2Freq_.setBounds(745, 110, 48, 48);
    lOsc2Freq_.setBounds(745, 160, 48, 14);
    sOsc2Fine_.setBounds(795, 110, 48, 48);
    lOsc2Fine_.setBounds(795, 160, 48, 14);
    sOsc2Shape_.setBounds(845, 110, 48, 48);
    lOsc2Shape_.setBounds(845, 160, 48, 14);
    sOsc2PW_.setBounds(745, 215, 48, 48);
    lOsc2PW_.setBounds(745, 265, 48, 14);
    bOsc2LowFreq_.setBounds(800, 220, 95, 20);
    bOsc2KeyTrack_.setBounds(800, 245, 95, 20);

    // 5. Mixer (x: 915, y: 88, w: 135, h: 242)
    sOsc1Level_.setBounds(955, 105, 52, 52);
    lOsc1Level_.setBounds(955, 158, 52, 14);
    sOsc2Level_.setBounds(955, 175, 52, 52);
    lOsc2Level_.setBounds(955, 228, 52, 14);
    sNoiseLevel_.setBounds(955, 245, 52, 52);
    lNoiseLevel_.setBounds(955, 298, 52, 14);

    // 6. Low-Pass Filter (x: 50, y: 340, w: 225, h: 245)
    sLpfCutoff_.setBounds(65, 365, 72, 72);
    lLpfCutoff_.setBounds(65, 438, 72, 16);
    sLpfRes_.setBounds(145, 365, 62, 62);
    lLpfRes_.setBounds(145, 428, 62, 14);
    sLpfEnvAmt_.setBounds(65, 475, 56, 56);
    lLpfEnvAmt_.setBounds(65, 532, 56, 14);
    sLpfKeyTrack_.setBounds(145, 475, 56, 56);
    lLpfKeyTrack_.setBounds(145, 532, 56, 14);

    // 7. High-Pass Filter (x: 280, y: 340, w: 175, h: 245)
    sHpfCutoff_.setBounds(295, 375, 60, 60);
    lHpfCutoff_.setBounds(295, 436, 60, 14);
    sHpfRes_.setBounds(375, 375, 60, 60);
    lHpfRes_.setBounds(375, 436, 60, 14);
    sHpfEnvAmt_.setBounds(335, 475, 56, 56);
    lHpfEnvAmt_.setBounds(335, 532, 56, 14);

    // 8. Filter Envelope (x: 460, y: 340, w: 220, h: 245)
    sFiltAtt_.setBounds(470, 385, 46, 46);
    lFiltAtt_.setBounds(470, 434, 46, 14);
    sFiltDec_.setBounds(522, 385, 46, 46);
    lFiltDec_.setBounds(522, 434, 46, 14);
    sFiltSus_.setBounds(574, 385, 46, 46);
    lFiltSus_.setBounds(574, 434, 46, 14);
    sFiltRel_.setBounds(626, 385, 46, 46);
    lFiltRel_.setBounds(626, 434, 46, 14);

    // 9. Amplifier (x: 685, y: 340, w: 220, h: 245)
    sAmpAtt_.setBounds(695, 385, 46, 46);
    lAmpAtt_.setBounds(695, 434, 46, 14);
    sAmpDec_.setBounds(747, 385, 46, 46);
    lAmpDec_.setBounds(747, 434, 46, 14);
    sAmpSus_.setBounds(799, 385, 46, 46);
    lAmpSus_.setBounds(799, 434, 46, 14);
    sAmpRel_.setBounds(851, 385, 46, 46);
    lAmpRel_.setBounds(851, 434, 46, 14);
    bVelSens_.setBounds(745, 480, 100, 24);

    // 10. LFO (x: 910, y: 340, w: 140, h: 245)
    sLfoFreq_.setBounds(920, 365, 50, 50);
    lLfoFreq_.setBounds(920, 416, 50, 14);
    sLfoAmt_.setBounds(980, 365, 50, 50);
    lLfoAmt_.setBounds(980, 416, 50, 14);
    cLfoShape_.setBounds(925, 440, 110, 22);

    bLfoOsc1_.setBounds(925, 475, 60, 18);
    bLfoOsc2_.setBounds(985, 475, 60, 18);
    bLfoLpf_.setBounds(925, 498, 55, 18);
    bLfoHpf_.setBounds(985, 498, 55, 18);
    bLfoAmp_.setBounds(955, 521, 55, 18);

    // 11. Effects Section (Phase 4)
    bFxOn_.setBounds(65, 638, 90, 26);

    // Effects 1 (FX A)
    cFx1Type_.setBounds(195, 638, 120, 24);
    sFx1Param1_.setBounds(330, 615, 50, 50);
    lFx1Param1_.setBounds(330, 668, 50, 14);
    sFx1Param2_.setBounds(420, 615, 50, 50);
    lFx1Param2_.setBounds(420, 668, 50, 14);
    sFx1Mix_.setBounds(510, 615, 50, 50);
    lFx1Mix_.setBounds(510, 668, 50, 14);

    // Effects 2 (FX B)
    cFx2Type_.setBounds(635, 638, 120, 24);
    sFx2Param1_.setBounds(770, 615, 50, 50);
    lFx2Param1_.setBounds(770, 668, 50, 14);
    sFx2Param2_.setBounds(860, 615, 50, 50);
    lFx2Param2_.setBounds(860, 668, 50, 14);
    sFx2Mix_.setBounds(950, 615, 50, 50);
    lFx2Mix_.setBounds(950, 668, 50, 14);
}

} // namespace prophet6
