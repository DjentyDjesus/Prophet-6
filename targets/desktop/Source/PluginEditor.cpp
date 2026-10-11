#include "PluginEditor.h"

namespace prophet6 {

Prophet6AudioProcessorEditor::Prophet6AudioProcessorEditor(Prophet6AudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef_(p),
      performanceWheels_(p),
      keyboardComponent_(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
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
    addToggleButton(bSeqRecord_,  "seq_record",  "REC");
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

    // 4. Oscillators, Slop & Mini Waves
    addRotaryKnob(sOsc1Freq_,     lOsc1Freq_,     "osc1_freq",      "FREQ");
    addRotaryKnob(sOsc1Shape_,    lOsc1Shape_,    "osc1_wave",      "SHAPE");
    addRotaryKnob(sOsc1PW_,       lOsc1PW_,       "osc1_pw",        "PW");
    addRotaryKnob(sOsc1SubLevel_, lOsc1SubLevel_, "osc1_sub_level", "SUB OCT");
    addToggleButton(bOsc1Sync_, "sync", "SYNC");
    addAndMakeVisible(miniWaveOsc1_);

    addRotaryKnob(sOsc2Freq_,  lOsc2Freq_,  "osc2_freq", "FREQ");
    addRotaryKnob(sOsc2Fine_,  lOsc2Fine_,  "osc2_fine", "FINE");
    addRotaryKnob(sOsc2Shape_, lOsc2Shape_, "osc2_wave", "SHAPE");
    addRotaryKnob(sOsc2PW_,    lOsc2PW_,    "osc2_pw",   "PW");
    addToggleButton(bOsc2LowFreq_,  "osc2_low_freq",  "LOW FREQ");
    addToggleButton(bOsc2KeyTrack_, "osc2_key_track", "KEYBOARD");
    addAndMakeVisible(miniWaveOsc2_);

    addRotaryKnob(sSlop_, lSlop_, "slop", "AMOUNT");

    // 5. Mixer
    addRotaryKnob(sOsc1Level_,  lOsc1Level_,  "osc1_level",  "OSC 1");
    addRotaryKnob(sOsc2Level_,  lOsc2Level_,  "osc2_level",  "OSC 2");
    addRotaryKnob(sNoiseLevel_, lNoiseLevel_, "noise_level", "NOISE");

    // 6. Dual Filters
    addRotaryKnob(sHpfCutoff_, lHpfCutoff_, "hpf_cutoff",     "CUTOFF");
    addRotaryKnob(sHpfRes_,    lHpfRes_,    "hpf_resonance",  "RESONANCE");
    addRotaryKnob(sHpfEnvAmt_, lHpfEnvAmt_, "hpf_env_amount", "ENV AMT");

    addRotaryKnob(sLpfCutoff_,   lLpfCutoff_,   "lpf_cutoff",     "CUTOFF");
    addRotaryKnob(sLpfRes_,      lLpfRes_,      "lpf_resonance",  "RESONANCE");
    addRotaryKnob(sLpfEnvAmt_,   lLpfEnvAmt_,   "lpf_env_amount", "ENV AMT");
    addRotaryKnob(sLpfKeyTrack_, lLpfKeyTrack_, "lpf_key_track",  "KEYBOARD");

    // 7. Envelopes & Vector Displays
    addRotaryKnob(sFiltAtt_, lFiltAtt_, "filt_attack",  "ATTACK");
    addRotaryKnob(sFiltDec_, lFiltDec_, "filt_decay",   "DECAY");
    addRotaryKnob(sFiltSus_, lFiltSus_, "filt_sustain", "SUSTAIN");
    addRotaryKnob(sFiltRel_, lFiltRel_, "filt_release", "RELEASE");
    addAndMakeVisible(adsrFilter_);

    addRotaryKnob(sAmpAtt_, lAmpAtt_, "amp_attack",  "ATTACK");
    addRotaryKnob(sAmpDec_, lAmpDec_, "amp_decay",   "DECAY");
    addRotaryKnob(sAmpSus_, lAmpSus_, "amp_sustain", "SUSTAIN");
    addRotaryKnob(sAmpRel_, lAmpRel_, "amp_release", "RELEASE");
    addToggleButton(bVelSens_, "velocity_sensitivity", "VELOCITY");
    addAndMakeVisible(adsrAmp_);

    // 8. Aftertouch Controls
    sAftertouchAmt_.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    sAftertouchAmt_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sAftertouchAmt_.setRange(0.0, 1.0, 0.01);
    sAftertouchAmt_.setValue(0.24);
    addAndMakeVisible(sAftertouchAmt_);
    lAftertouchAmt_.setText("AMOUNT", juce::dontSendNotification);
    lAftertouchAmt_.setFont(juce::Font(10.0f, juce::Font::bold));
    lAftertouchAmt_.setJustificationType(juce::Justification::centred);
    lAftertouchAmt_.setColour(juce::Label::textColourId, juce::Colour(0xffe8d8b8));
    addAndMakeVisible(lAftertouchAmt_);

    bAtFreq1_.setButtonText("FREQ 1");   addAndMakeVisible(bAtFreq1_);
    bAtFreq2_.setButtonText("FREQ 2");   addAndMakeVisible(bAtFreq2_);
    bAtLfoAmt_.setButtonText("LFO AMT"); addAndMakeVisible(bAtLfoAmt_);
    bAtAmp_.setButtonText("AMP");       addAndMakeVisible(bAtAmp_);
    bAtLPF_.setButtonText("LPF");       addAndMakeVisible(bAtLPF_);
    bAtHPF_.setButtonText("HPF");       addAndMakeVisible(bAtHPF_);

    // 9. LFO
    addRotaryKnob(sLfoFreq_, lLfoFreq_, "lfo_freq",   "FREQUENCY");
    addRotaryKnob(sLfoAmt_,  lLfoAmt_,  "lfo_amount", "INITIAL AMT");

    cLfoShape_.addItemList({"TRIANGLE", "SAWTOOTH", "REV SAW", "SQUARE", "RANDOM"}, 1);
    addAndMakeVisible(cLfoShape_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "lfo_shape", cLfoShape_));

    addToggleButton(bLfoOsc1_, "lfo_dest_osc1_freq", "FREQ 1");
    addToggleButton(bLfoOsc2_, "lfo_dest_osc2_freq", "FREQ 2");
    addToggleButton(bLfoLpf_,  "lfo_dest_lpf",       "LP FILTER");
    addToggleButton(bLfoHpf_,  "lfo_dest_hpf",       "HP FILTER");
    addToggleButton(bLfoAmp_,  "lfo_dest_amp",       "AMP");

    // 10. Master, Distortion, Glide, Unison
    addRotaryKnob(sDistortion_,   lDistortion_,   "distortion_amount", "AMOUNT");
    addRotaryKnob(sMasterVol_,    lMasterVol_,    "master_volume",     "PROG VOL");
    addRotaryKnob(sPanSpread_,    lPanSpread_,    "pan_spread",        "PAN SPREAD");
    addRotaryKnob(sGlide_,        lGlide_,        "glide",             "GLIDE RATE");
    addRotaryKnob(sUnisonDetune_, lUnisonDetune_, "unison_detune",     "DETUNE");
    addToggleButton(bUnison_, "unison", "UNISON");

    // 11. Dual Stereo Effects & Pre-FX Oscilloscope
    addToggleButton(bFxOn_, "fx_on", "ON/OFF");

    const juce::StringArray fxList = {
        "BBD DELAY", "DIGITAL DELAY", "TAPE DELAY", "CHORUS", "FLANGER",
        "PHASER", "RING MOD", "PLATE REVERB", "HALL REVERB", "OFF"
    };

    cFx1Type_.addItemList(fxList, 1);
    addAndMakeVisible(cFx1Type_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "fx1_type", cFx1Type_));

    addRotaryKnob(sFx1Mix_,    lFx1Mix_,    "fx1_mix",    "MIX");
    addRotaryKnob(sFx1Param1_, lFx1Param1_, "fx1_param1", "PARAM 1");
    addRotaryKnob(sFx1Param2_, lFx1Param2_, "fx1_param2", "PARAM 2");

    cFx2Type_.addItemList(fxList, 1);
    addAndMakeVisible(cFx2Type_);
    comboAttachments_.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef_.getAPVTS(), "fx2_type", cFx2Type_));

    addRotaryKnob(sFx2Mix_,    lFx2Mix_,    "fx2_mix",    "MIX");
    addRotaryKnob(sFx2Param1_, lFx2Param1_, "fx2_param1", "PARAM 1");
    addRotaryKnob(sFx2Param2_, lFx2Param2_, "fx2_param2", "PARAM 2");

    addAndMakeVisible(oscilloscope_);

    // 12. Preset Navigation & Hardware Librarian
    bProgDec_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
    bProgDec_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8d8b8));
    bProgDec_.onClick = [this] {
        if (processorRef_.isWriteArmed()) {
            int dest = processorRef_.getWriteDestination();
            int next = (dest <= 100) ? 999 : dest - 1;
            processorRef_.setWriteDestination(next);
            repaint(40, 8, 380, 72);
        } else {
            const int maxProg = static_cast<int>(processorRef_.getPresetManager().getNumPresets()) - 1;
            int cur = processorRef_.getCurrentProgram();
            int next = (cur <= 0) ? maxProg : cur - 1;
            processorRef_.setCurrentProgram(next);
            lastProgramIndex_ = next;
            cPresetSelect_.setSelectedId(next + 1, juce::dontSendNotification);
            repaint(40, 8, 380, 72);
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
            repaint(40, 8, 380, 72);
        } else {
            const int maxProg = static_cast<int>(processorRef_.getPresetManager().getNumPresets()) - 1;
            int cur = processorRef_.getCurrentProgram();
            int next = (cur >= maxProg) ? 0 : cur + 1;
            processorRef_.setCurrentProgram(next);
            lastProgramIndex_ = next;
            cPresetSelect_.setSelectedId(next + 1, juce::dontSendNotification);
            repaint(40, 8, 380, 72);
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
        repaint(40, 8, 380, 72);
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
        repaint(40, 8, 380, 72);
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
                    repaint(40, 8, 380, 72);
                }
            } else {
                if (progIndex != processorRef_.getCurrentProgram()) {
                    processorRef_.setCurrentProgram(progIndex);
                    lastProgramIndex_ = progIndex;
                    repaint(40, 8, 380, 72);
                }
            }
        }
    };
    addAndMakeVisible(cPresetSelect_);

    // 13. Performance Strip & Virtual Keyboard
    addAndMakeVisible(performanceWheels_);

    keyboardComponent_.setAvailableRange(24, 96); // C1 to C7 (6 Octaves)
    keyboardComponent_.setKeyWidth(16.5f);
    addAndMakeVisible(keyboardComponent_);

    // SoundTower Master Layout Dimension: 1200 x 820 px
    setSize(1200, 820);
    startTimerHz(40); // 40 Hz refresh
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

    // Real-Time Visualizers: Oscilloscope
    std::array<float, 512> preFxMono{};
    processorRef_.getSynthEngine().copyRecentPreFx(preFxMono.data(), preFxMono.size());
    const float fundamental = processorRef_.getSynthEngine().getLowestActiveNoteFreq();
    const float sr = processorRef_.getSynthEngine().getSampleRate();
    oscilloscope_.updateWaveform(preFxMono.data(), preFxMono.size(), fundamental, sr);

    // Real-Time Mini Waves
    miniWaveOsc1_.setParams(static_cast<float>(sOsc1Shape_.getValue()), static_cast<float>(sOsc1PW_.getValue()));
    miniWaveOsc2_.setParams(static_cast<float>(sOsc2Shape_.getValue()), static_cast<float>(sOsc2PW_.getValue()));

    // Real-Time Vector ADSRs
    adsrFilter_.setAdsr(static_cast<float>(sFiltAtt_.getValue()), static_cast<float>(sFiltDec_.getValue()),
                        static_cast<float>(sFiltSus_.getValue()), static_cast<float>(sFiltRel_.getValue()));
    adsrAmp_.setAdsr(static_cast<float>(sAmpAtt_.getValue()), static_cast<float>(sAmpDec_.getValue()),
                     static_cast<float>(sAmpSus_.getValue()), static_cast<float>(sAmpRel_.getValue()));

    if (needRepaint) {
        repaint(40, 8, 480, 72);
    }
}

void Prophet6AudioProcessorEditor::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // 1. Central Matte Dark Aluminum Chassis
    g.setColour(juce::Colour(0xff121316));
    g.fillRect(0, 0, w, h);

    // Hairline brushed dark metal gradient
    juce::ColourGradient chassisGrad(juce::Colour(0xff1e2024), 0.0f, 0.0f,
                                     juce::Colour(0xff101114), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(chassisGrad);
    g.fillRect(40, 0, w - 80, h);

    // 2. Walnut Wood End Cheeks
    auto drawWoodCheek = [&](int x, int width) {
        juce::ColourGradient woodGrad(juce::Colour(0xff42210e), static_cast<float>(x), 0.0f,
                                      juce::Colour(0xff1c0c04), static_cast<float>(x + width), static_cast<float>(h), false);
        g.setGradientFill(woodGrad);
        g.fillRect(x, 0, width, h);

        // Natural wood grain striations
        for (int i = 0; i < h; i += 7) {
            g.setColour(juce::Colours::black.withAlpha(0.14f));
            g.drawHorizontalLine(i, static_cast<float>(x), static_cast<float>(x + width));
        }

        // Metal joining border
        g.setColour(juce::Colours::black.withAlpha(0.65f));
        g.fillRect(x == 0 ? width - 2 : x, 0, 2, h);
    };
    drawWoodCheek(0, 40);
    drawWoodCheek(w - 40, 40);

    // 3. Header: LCD Preset Display ($x = 44, y = 10, w = 216, h = 64$)
    const juce::Rectangle<int> lcdDisplay(44, 10, 216, 64);
    g.setColour(juce::Colour(0xff090c0f));
    g.fillRoundedRectangle(lcdDisplay.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff222a32));
    g.drawRoundedRectangle(lcdDisplay.toFloat(), 4.0f, 1.2f);

    const int curProg = processorRef_.getCurrentProgram();
    const auto& pm = processorRef_.getPresetManager();
    const auto& p = pm.getPreset(static_cast<size_t>(curProg));

    // Preset Category & Bank
    g.setColour(juce::Colour(0xff88a0b0));
    g.setFont(juce::Font(9.5f, juce::Font::bold));
    juce::String bankStr = (curProg < 100) ? "F" : "U";
    juce::String catStr = (p.category == PresetCategory::Brass)   ? "BRASS"   :
                          (p.category == PresetCategory::Keys)    ? "KEYS"    :
                          (p.category == PresetCategory::Pad)     ? "PADS"    :
                          (p.category == PresetCategory::Strings) ? "STRINGS" :
                          (p.category == PresetCategory::Bass)    ? "BASS"    :
                          (p.category == PresetCategory::Lead)    ? "LEAD"    :
                          (p.category == PresetCategory::Arp)     ? "ARP"     : "SFX";
    g.drawText(bankStr + " " + juce::String::formatted("%03d", curProg), lcdDisplay.getX() + 8, lcdDisplay.getY() + 6, 70, 14, juce::Justification::left);
    g.drawText(catStr, lcdDisplay.getX() + 110, lcdDisplay.getY() + 6, 96, 14, juce::Justification::right);

    // Preset Name in bright turquoise/white LCD text
    g.setColour(juce::Colour(0xffffffff));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(juce::String(p.getName().data()), lcdDisplay.getX() + 8, lcdDisplay.getY() + 24, 170, 22, juce::Justification::left);

    // Status indicator
    if (processorRef_.isWriteArmed()) {
        const bool blink = (blinkCounter_ / 5) % 2 == 0;
        if (blink) {
            g.setColour(juce::Colour(0xffff3333));
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            g.drawText("WRITE -> " + juce::String::formatted("%03d", processorRef_.getWriteDestination()),
                       lcdDisplay.getX() + 8, lcdDisplay.getY() + 46, 190, 14, juce::Justification::left);
        }
    } else if (processorRef_.isComparing()) {
        g.setColour(juce::Colour(0xffffaa33));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("[COMPARE MODE]", lcdDisplay.getX() + 8, lcdDisplay.getY() + 46, 120, 14, juce::Justification::left);
    } else {
        g.setColour(juce::Colour(0xffff4444));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("(A)", lcdDisplay.getX() + 186, lcdDisplay.getY() + 46, 20, 14, juce::Justification::right);
    }

    // 4. Header: Top-Right Brushed Metallic Badge "SEQUENTIAL Prophet~6"
    const juce::Rectangle<float> badgeRect(942.0f, 10.0f, 214.0f, 64.0f);
    // Aluminum badge plate
    juce::ColourGradient badgeGrad(juce::Colour(0xffdedfe2), badgeRect.getX(), badgeRect.getY(),
                                   juce::Colour(0xff9ea0a6), badgeRect.getX(), badgeRect.getBottom(), false);
    g.setGradientFill(badgeGrad);
    g.fillRoundedRectangle(badgeRect, 4.0f);
    // Chrome edge border
    g.setColour(juce::Colour(0xfff8f9fa));
    g.drawRoundedRectangle(badgeRect.reduced(1.0f), 3.0f, 1.0f);
    g.setColour(juce::Colour(0xff4a4c52));
    g.drawRoundedRectangle(badgeRect, 4.0f, 1.5f);

    // "SEQUENTIAL" uppercase logo in black
    g.setColour(juce::Colour(0xff18181b));
    g.setFont(juce::Font(16.5f, juce::Font::bold));
    g.drawText("SEQUENTIAL", badgeRect.getX(), badgeRect.getY() + 6.0f, badgeRect.getWidth(), 22.0f, juce::Justification::centred);

    // "Prophet~6" script badge
    const juce::Rectangle<float> innerScriptBox(badgeRect.getX() + 12.0f, badgeRect.getY() + 30.0f, badgeRect.getWidth() - 24.0f, 26.0f);
    g.setColour(juce::Colour(0xff303238));
    g.fillRoundedRectangle(innerScriptBox, 2.0f);
    g.setColour(juce::Colour(0xffe8eaed));
    g.setFont(juce::Font(16.0f, juce::Font::italic | juce::Font::bold));
    g.drawText("Prophet~6", innerScriptBox, juce::Justification::centred);

    // 5. Section Panel Outlines matching SoundTower PlugSE layout
    auto drawSectionBox = [&](int sx, int sy, int sw, int sh, const juce::String& title,
                              juce::Colour accent = juce::Colour(0xffecd8ba)) {
        const auto b = juce::Rectangle<int>(sx, sy, sw, sh).toFloat();
        g.setColour(juce::Colour(0x24ffffff));
        g.drawRoundedRectangle(b, 4.0f, 1.0f);

        // Header cutout label
        const int titleW = juce::Font(9.5f, juce::Font::bold).getStringWidth(title) + 16;
        g.setColour(juce::Colour(0xff181a1e));
        g.fillRect(sx + 10, sy - 7, titleW, 14);

        g.setColour(accent);
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText(title, sx + 10, sy - 7, titleW, 14, juce::Justification::centred);
    };

    // Upper Grid:
    drawSectionBox(44,  84, 346, 120, "OSCILLATOR 1");
    drawSectionBox(44, 208, 346, 122, "OSCILLATOR 2");
    drawSectionBox(394,  84,  84, 120, "SLOP");
    drawSectionBox(482,  84, 196, 246, "MIXER");
    drawSectionBox(684,  84, 472, 120, "HIGH PASS FILTER");
    drawSectionBox(684, 208, 472, 122, "LOW PASS FILTER");

    // Middle Grid:
    drawSectionBox(44,  334, 634, 82, "FILTER ENVELOPE");
    drawSectionBox(684, 334, 472, 82, "POLY MOD", juce::Colour(0xfff07070));
    drawSectionBox(44,  420, 634, 84, "AMPLIFIER ENVELOPE");
    drawSectionBox(684, 420, 472, 84, "AFTERTOUCH", juce::Colour(0xfff0a070));

    // Lower Grid:
    drawSectionBox(44,  508, 506, 80, "EFFECTS", juce::Colour(0xffe8c890));
    drawSectionBox(554, 508, 286, 80, "PRE-FX OSCILLOSCOPE", juce::Colour(0xff66ff88));
    drawSectionBox(844, 508, 312, 80, "MISC PARAMETERS");
    drawSectionBox(44,  592, 636, 76, "LOW FREQUENCY OSCILLATORS (LFO)");
    drawSectionBox(684, 592, 472, 76, "DISTORTION & UNISON / GLIDE");

    // 6. Voice Activity LEDs inside Mixer Panel
    const int vLedX = 502;
    const int vLedY = 302;
    g.setColour(juce::Colour(0xffa09080));
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.drawText("VOICES:", vLedX, vLedY - 12, 60, 10, juce::Justification::left);

    for (int i = 0; i < 6; ++i) {
        const float lampX = static_cast<float>(vLedX + i * 26);
        const float lampY = static_cast<float>(vLedY);
        const float lampSize = 10.0f;

        g.setColour(juce::Colour(0xff303030));
        g.fillEllipse(lampX - 1.0f, lampY - 1.0f, lampSize + 2.0f, lampSize + 2.0f);

        const bool active = voiceActiveState_[static_cast<size_t>(i)];
        if (active) {
            g.setColour(juce::Colour(0x55ff2222));
            g.fillEllipse(lampX - 2.0f, lampY - 2.0f, lampSize + 4.0f, lampSize + 4.0f);
            g.setColour(juce::Colour(0xffff2828));
            g.fillEllipse(lampX, lampY, lampSize, lampSize);
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.fillEllipse(lampX + 2.0f, lampY + 2.0f, 2.5f, 2.5f);
        } else {
            g.setColour(juce::Colour(0xff240606));
            g.fillEllipse(lampX, lampY, lampSize, lampSize);
        }
    }
}

void Prophet6AudioProcessorEditor::resized()
{
    // --- Header Bar ---
    // Preset Librarian controls
    bProgDec_.setBounds(265, 12, 24, 22);
    bProgInc_.setBounds(265, 36, 24, 22);
    bWrite_.setBounds(293, 12, 54, 22);
    bCompare_.setBounds(293, 36, 54, 22);
    cCategoryFilter_.setBounds(351, 12, 90, 22);
    cPresetSelect_.setBounds(351, 36, 90, 22);

    // Clock & Tempo
    sSeqBpm_.setBounds(452, 14, 44, 44);
    lSeqBpm_.setBounds(452, 58, 44, 14);
    cArpDivision_.setBounds(500, 26, 68, 22);

    // Arpeggiator
    bArpEnabled_.setBounds(578, 14, 60, 20);
    bArpHold_.setBounds(578, 38, 60, 20);
    cArpMode_.setBounds(642, 26, 80, 22);
    sArpOctave_.setBounds(728, 14, 38, 38);
    lArpOctave_.setBounds(724, 52, 46, 12);

    // Sequencer
    bSeqPlay_.setBounds(778, 14, 64, 24);
    bSeqRecord_.setBounds(778, 42, 64, 20);
    bSeqTie_.setBounds(846, 14, 46, 22);
    bSeqRest_.setBounds(846, 40, 46, 22);
    sSeqLength_.setBounds(896, 14, 38, 38);
    lSeqLength_.setBounds(892, 52, 46, 12);

    // Standalone Settings Button
    bSettings_.setBounds(1164, 14, 32, 22);

    // --- Row 1: Upper Middle Grid ---
    // Oscillator 1
    sOsc1Freq_.setBounds(52, 102, 46, 46);
    lOsc1Freq_.setBounds(52, 148, 46, 13);
    bOsc1Sync_.setBounds(104, 116, 56, 20);
    sOsc1Shape_.setBounds(164, 102, 46, 46);
    lOsc1Shape_.setBounds(164, 148, 46, 13);
    miniWaveOsc1_.setBounds(214, 112, 44, 26);
    sOsc1PW_.setBounds(264, 102, 46, 46);
    lOsc1PW_.setBounds(264, 148, 46, 13);
    sOsc1SubLevel_.setBounds(328, 102, 46, 46);
    lOsc1SubLevel_.setBounds(328, 148, 46, 13);

    // Oscillator 2
    sOsc2Freq_.setBounds(52, 226, 46, 46);
    lOsc2Freq_.setBounds(52, 272, 46, 13);
    sOsc2Fine_.setBounds(104, 226, 46, 46);
    lOsc2Fine_.setBounds(104, 272, 46, 13);
    sOsc2Shape_.setBounds(156, 226, 46, 46);
    lOsc2Shape_.setBounds(156, 272, 46, 13);
    miniWaveOsc2_.setBounds(206, 236, 44, 26);
    sOsc2PW_.setBounds(254, 226, 46, 46);
    lOsc2PW_.setBounds(254, 272, 46, 13);
    bOsc2LowFreq_.setBounds(308, 226, 76, 20);
    bOsc2KeyTrack_.setBounds(308, 252, 76, 20);

    // Slop Panel
    sSlop_.setBounds(412, 106, 48, 48);
    lSlop_.setBounds(412, 154, 48, 13);

    // Mixer Panel
    sOsc1Level_.setBounds(500, 106, 50, 50);
    lOsc1Level_.setBounds(500, 156, 50, 13);
    sOsc2Level_.setBounds(608, 106, 50, 50);
    lOsc2Level_.setBounds(608, 156, 50, 13);
    sNoiseLevel_.setBounds(554, 204, 50, 50);
    lNoiseLevel_.setBounds(554, 254, 50, 13);

    // High Pass Filter
    sHpfCutoff_.setBounds(706, 104, 52, 52);
    lHpfCutoff_.setBounds(706, 156, 52, 13);
    sHpfRes_.setBounds(786, 104, 52, 52);
    lHpfRes_.setBounds(786, 156, 52, 13);
    sHpfEnvAmt_.setBounds(866, 104, 52, 52);
    lHpfEnvAmt_.setBounds(866, 156, 52, 13);

    // Low Pass Filter
    sLpfCutoff_.setBounds(706, 228, 52, 52);
    lLpfCutoff_.setBounds(706, 280, 52, 13);
    sLpfRes_.setBounds(786, 228, 52, 52);
    lLpfRes_.setBounds(786, 280, 52, 13);
    sLpfEnvAmt_.setBounds(866, 228, 52, 52);
    lLpfEnvAmt_.setBounds(866, 280, 52, 13);
    sLpfKeyTrack_.setBounds(946, 228, 52, 52);
    lLpfKeyTrack_.setBounds(946, 280, 52, 13);

    // --- Row 2: Envelopes, Poly Mod, Aftertouch ---
    // Filter Envelope
    sFiltAtt_.setBounds(56, 350, 42, 42);
    lFiltAtt_.setBounds(56, 392, 42, 12);
    sFiltDec_.setBounds(108, 350, 42, 42);
    lFiltDec_.setBounds(108, 392, 42, 12);
    sFiltSus_.setBounds(160, 350, 42, 42);
    lFiltSus_.setBounds(160, 392, 42, 12);
    sFiltRel_.setBounds(212, 350, 42, 42);
    lFiltRel_.setBounds(212, 392, 42, 12);
    adsrFilter_.setBounds(268, 344, 180, 62);

    // Poly Mod
    sPolyModFiltEnv_.setBounds(696, 344, 44, 44);
    lPolyModFiltEnv_.setBounds(696, 388, 44, 12);
    sPolyModOsc2_.setBounds(754, 344, 44, 44);
    lPolyModOsc2_.setBounds(754, 388, 44, 12);
    bPolyModFreq1_.setBounds(814, 342, 64, 18);
    bPolyModShape1_.setBounds(814, 364, 64, 18);
    bPolyModPW1_.setBounds(814, 386, 64, 18);
    bPolyModLPF_.setBounds(888, 348, 56, 18);
    bPolyModHPF_.setBounds(888, 376, 56, 18);

    // Amplifier Envelope
    bVelSens_.setBounds(52, 444, 76, 20);
    sAmpAtt_.setBounds(136, 436, 42, 42);
    lAmpAtt_.setBounds(136, 478, 42, 12);
    sAmpDec_.setBounds(188, 436, 42, 42);
    lAmpDec_.setBounds(188, 478, 42, 12);
    sAmpSus_.setBounds(240, 436, 42, 42);
    lAmpSus_.setBounds(240, 478, 42, 12);
    sAmpRel_.setBounds(292, 436, 42, 42);
    lAmpRel_.setBounds(292, 478, 42, 12);
    adsrAmp_.setBounds(350, 430, 180, 62);

    // Aftertouch Panel
    sAftertouchAmt_.setBounds(696, 432, 44, 44);
    lAftertouchAmt_.setBounds(696, 476, 44, 12);
    bAtFreq1_.setBounds(754, 430, 60, 18);
    bAtFreq2_.setBounds(754, 452, 60, 18);
    bAtLfoAmt_.setBounds(754, 474, 64, 18);
    bAtAmp_.setBounds(826, 430, 52, 18);
    bAtLPF_.setBounds(826, 452, 52, 18);
    bAtHPF_.setBounds(826, 474, 52, 18);

    // --- Row 3: Effects, Oscilloscope, Misc, LFO, Distortion ---
    // Effects Panel
    bFxOn_.setBounds(52, 532, 64, 22);
    cFx1Type_.setBounds(120, 532, 86, 22);
    sFx1Mix_.setBounds(210, 520, 40, 40);
    lFx1Mix_.setBounds(210, 560, 40, 12);
    sFx1Param1_.setBounds(254, 520, 40, 40);
    lFx1Param1_.setBounds(254, 560, 40, 12);
    sFx1Param2_.setBounds(298, 520, 40, 40);
    lFx1Param2_.setBounds(298, 560, 40, 12);

    cFx2Type_.setBounds(346, 532, 86, 22);
    sFx2Mix_.setBounds(436, 520, 40, 40);
    lFx2Mix_.setBounds(436, 560, 40, 12);
    sFx2Param1_.setBounds(480, 520, 40, 40);
    lFx2Param1_.setBounds(480, 560, 40, 12);
    sFx2Param2_.setBounds(524, 520, 40, 40);
    lFx2Param2_.setBounds(524, 560, 40, 12);

    // Pre-FX Oscilloscope
    oscilloscope_.setBounds(558, 514, 278, 68);

    // Misc Parameters
    sPanSpread_.setBounds(862, 520, 42, 42);
    lPanSpread_.setBounds(862, 562, 42, 12);
    sMasterVol_.setBounds(930, 520, 42, 42);
    lMasterVol_.setBounds(930, 562, 42, 12);

    // LFO Panel
    sLfoFreq_.setBounds(56, 604, 42, 42);
    lLfoFreq_.setBounds(56, 646, 42, 12);
    cLfoShape_.setBounds(108, 616, 92, 22);
    sLfoAmt_.setBounds(208, 604, 42, 42);
    lLfoAmt_.setBounds(208, 646, 42, 12);
    bLfoOsc1_.setBounds(262, 606, 56, 18);
    bLfoOsc2_.setBounds(262, 630, 56, 18);
    bLfoLpf_.setBounds(324, 606, 56, 18);
    bLfoHpf_.setBounds(324, 630, 56, 18);
    bLfoAmp_.setBounds(384, 618, 52, 18);

    // Distortion, Unison, Glide
    sDistortion_.setBounds(700, 604, 44, 44);
    lDistortion_.setBounds(700, 648, 44, 12);
    sGlide_.setBounds(760, 604, 44, 44);
    lGlide_.setBounds(760, 648, 44, 12);
    bUnison_.setBounds(822, 614, 68, 22);
    sUnisonDetune_.setBounds(896, 604, 44, 44);
    lUnisonDetune_.setBounds(896, 648, 44, 12);

    // --- Row 4: Performance Wheels & Virtual Keyboard Strip ---
    performanceWheels_.setBounds(44, 672, 160, 142);
    keyboardComponent_.setBounds(208, 672, 948, 142);
}

} // namespace prophet6
