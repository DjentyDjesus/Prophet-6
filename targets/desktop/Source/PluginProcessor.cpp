#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace prophet6 {

Prophet6AudioProcessor::Prophet6AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache atomic parameter pointers
    pPolyModFiltEnv_     = apvts_.getRawParameterValue("polymod_filter_env");
    pPolyModOsc2_        = apvts_.getRawParameterValue("polymod_osc2");
    pPolyModDestOsc1Freq_= apvts_.getRawParameterValue("polymod_dest_osc1_freq");
    pPolyModDestOsc1Shape_= apvts_.getRawParameterValue("polymod_dest_osc1_shape");
    pPolyModDestOsc1PW_  = apvts_.getRawParameterValue("polymod_dest_osc1_pw");
    pPolyModDestLPF_     = apvts_.getRawParameterValue("polymod_dest_lpf");
    pPolyModDestHPF_     = apvts_.getRawParameterValue("polymod_dest_hpf");

    pOsc1Freq_           = apvts_.getRawParameterValue("osc1_freq");
    pOsc1Wave_           = apvts_.getRawParameterValue("osc1_wave");
    pOsc1PW_             = apvts_.getRawParameterValue("osc1_pw");
    pOsc1SubLevel_       = apvts_.getRawParameterValue("osc1_sub_level");
    pOsc1Sync_           = apvts_.getRawParameterValue("sync");

    pOsc2Freq_           = apvts_.getRawParameterValue("osc2_freq");
    pOsc2Fine_           = apvts_.getRawParameterValue("osc2_fine");
    pOsc2Wave_           = apvts_.getRawParameterValue("osc2_wave");
    pOsc2PW_             = apvts_.getRawParameterValue("osc2_pw");
    pOsc2LowFreq_        = apvts_.getRawParameterValue("osc2_low_freq");
    pOsc2KeyTrack_       = apvts_.getRawParameterValue("osc2_key_track");

    pOsc1Level_          = apvts_.getRawParameterValue("osc1_level");
    pOsc2Level_          = apvts_.getRawParameterValue("osc2_level");
    pNoiseLevel_         = apvts_.getRawParameterValue("noise_level");

    pLpfCutoff_          = apvts_.getRawParameterValue("lpf_cutoff");
    pLpfResonance_       = apvts_.getRawParameterValue("lpf_resonance");
    pLpfEnvAmount_       = apvts_.getRawParameterValue("lpf_env_amount");
    pLpfKeyTrack_        = apvts_.getRawParameterValue("lpf_key_track");

    pHpfCutoff_          = apvts_.getRawParameterValue("hpf_cutoff");
    pHpfResonance_       = apvts_.getRawParameterValue("hpf_resonance");
    pHpfEnvAmount_       = apvts_.getRawParameterValue("hpf_env_amount");

    pFiltAttack_         = apvts_.getRawParameterValue("filt_attack");
    pFiltDecay_          = apvts_.getRawParameterValue("filt_decay");
    pFiltSustain_        = apvts_.getRawParameterValue("filt_sustain");
    pFiltRelease_        = apvts_.getRawParameterValue("filt_release");

    pAmpAttack_          = apvts_.getRawParameterValue("amp_attack");
    pAmpDecay_           = apvts_.getRawParameterValue("amp_decay");
    pAmpSustain_         = apvts_.getRawParameterValue("amp_sustain");
    pAmpRelease_         = apvts_.getRawParameterValue("amp_release");
    pVelSens_            = apvts_.getRawParameterValue("velocity_sensitivity");

    pLfoShape_           = apvts_.getRawParameterValue("lfo_shape");
    pLfoFreq_            = apvts_.getRawParameterValue("lfo_freq");
    pLfoAmount_          = apvts_.getRawParameterValue("lfo_amount");
    pLfoDestOsc1Freq_    = apvts_.getRawParameterValue("lfo_dest_osc1_freq");
    pLfoDestOsc2Freq_    = apvts_.getRawParameterValue("lfo_dest_osc2_freq");
    pLfoDestLPF_         = apvts_.getRawParameterValue("lfo_dest_lpf");
    pLfoDestHPF_         = apvts_.getRawParameterValue("lfo_dest_hpf");
    pLfoDestAmp_         = apvts_.getRawParameterValue("lfo_dest_amp");

    pDistortion_         = apvts_.getRawParameterValue("distortion_amount");
    pMasterVolume_       = apvts_.getRawParameterValue("master_volume");
    pSlop_               = apvts_.getRawParameterValue("slop");
    pPanSpread_          = apvts_.getRawParameterValue("pan_spread");
    pUnison_             = apvts_.getRawParameterValue("unison");
    pUnisonDetune_       = apvts_.getRawParameterValue("unison_detune");
    pGlide_              = apvts_.getRawParameterValue("glide");

    pArpEnabled_         = apvts_.getRawParameterValue("arp_enabled");
    pArpMode_            = apvts_.getRawParameterValue("arp_mode");
    pArpOctave_          = apvts_.getRawParameterValue("arp_octave");
    pArpDivision_        = apvts_.getRawParameterValue("arp_division");
    pArpHold_            = apvts_.getRawParameterValue("arp_hold");

    pSeqEnabled_         = apvts_.getRawParameterValue("seq_enabled");
    pSeqRecord_          = apvts_.getRawParameterValue("seq_record");
    pSeqPlay_            = apvts_.getRawParameterValue("seq_play");
    pSeqLength_          = apvts_.getRawParameterValue("seq_length");
    pSeqBpm_             = apvts_.getRawParameterValue("seq_bpm");

    // Effects Engine Parameters (Phase 4)
    pFxOn_               = apvts_.getRawParameterValue("fx_on");
    pFx1Type_            = apvts_.getRawParameterValue("fx1_type");
    pFx1Param1_          = apvts_.getRawParameterValue("fx1_param1");
    pFx1Param2_          = apvts_.getRawParameterValue("fx1_param2");
    pFx1Mix_             = apvts_.getRawParameterValue("fx1_mix");
    pFx2Type_            = apvts_.getRawParameterValue("fx2_type");
    pFx2Param1_          = apvts_.getRawParameterValue("fx2_param1");
    pFx2Param2_          = apvts_.getRawParameterValue("fx2_param2");
    pFx2Mix_             = apvts_.getRawParameterValue("fx2_mix");

    // Initialize default preset 0 (Jump Brass)
    presetManager_.loadPreset(0, synth_);
}

juce::AudioProcessorValueTreeState::ParameterLayout Prophet6AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Poly Mod
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"polymod_filter_env", 1}, "PolyMod Filter Env",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"polymod_osc2", 1}, "PolyMod Osc 2",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"polymod_dest_osc1_freq", 1}, "PolyMod Dest Freq 1", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"polymod_dest_osc1_shape", 1}, "PolyMod Dest Shape 1", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"polymod_dest_osc1_pw", 1}, "PolyMod Dest PW 1", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"polymod_dest_lpf", 1}, "PolyMod Dest LPF", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"polymod_dest_hpf", 1}, "PolyMod Dest HPF", false));

    // Oscillators
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_freq", 1}, "Osc 1 Freq",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_wave", 1}, "Osc 1 Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_pw", 1}, "Osc 1 Pulse Width",
        juce::NormalisableRange<float>(0.01f, 0.99f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_sub_level", 1}, "Osc 1 Sub Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"sync", 1}, "Hard Sync", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_freq", 1}, "Osc 2 Freq",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_fine", 1}, "Osc 2 Fine Tune",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_wave", 1}, "Osc 2 Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_pw", 1}, "Osc 2 Pulse Width",
        juce::NormalisableRange<float>(0.01f, 0.99f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"osc2_low_freq", 1}, "Osc 2 Low Freq", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"osc2_key_track", 1}, "Osc 2 Key Track", true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"slop", 1}, "Vintage Slop",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    // Mixer
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_level", 1}, "Osc 1 Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_level", 1}, "Osc 2 Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"noise_level", 1}, "Noise Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // Dual Filters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lpf_cutoff", 1}, "LPF Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f), 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lpf_resonance", 1}, "LPF Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lpf_env_amount", 1}, "LPF Env Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lpf_key_track", 1}, "LPF Key Track",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.5f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"hpf_cutoff", 1}, "HPF Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"hpf_resonance", 1}, "HPF Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"hpf_env_amount", 1}, "HPF Env Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    // Envelopes
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filt_attack", 1}, "Filter Attack",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filt_decay", 1}, "Filter Decay",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f, 0.3f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filt_sustain", 1}, "Filter Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filt_release", 1}, "Filter Release",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f, 0.3f), 0.4f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_attack", 1}, "Amp Attack",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_decay", 1}, "Amp Decay",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f, 0.3f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_sustain", 1}, "Amp Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_release", 1}, "Amp Release",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f, 0.3f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"velocity_sensitivity", 1}, "Velocity Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // LFO
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"lfo_shape", 1}, "LFO Waveform",
        juce::StringArray{"Triangle", "Sawtooth", "Reverse Saw", "Square", "Random (S&H)"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lfo_freq", 1}, "LFO Frequency",
        juce::NormalisableRange<float>(0.05f, 500.0f, 0.01f, 0.3f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lfo_amount", 1}, "LFO Initial Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfo_dest_osc1_freq", 1}, "LFO Dest Osc 1 Freq", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfo_dest_osc2_freq", 1}, "LFO Dest Osc 2 Freq", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfo_dest_lpf", 1}, "LFO Dest LPF", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfo_dest_hpf", 1}, "LFO Dest HPF", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfo_dest_amp", 1}, "LFO Dest Amp", false));

    // Master / Distortion / Performance
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distortion_amount", 1}, "Distortion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"master_volume", 1}, "Master Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pan_spread", 1}, "Pan Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"unison", 1}, "Unison", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"unison_detune", 1}, "Unison Detune",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"glide", 1}, "Glide",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f, 0.4f), 0.0f));

    // Arpeggiator Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"arp_enabled", 1}, "Arp Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"arp_mode", 1}, "Arp Mode",
        juce::StringArray{"UP", "DOWN", "UP+DOWN", "RANDOM", "ASSIGN"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"arp_octave", 1}, "Arp Octave Range", 1, 3, 1));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"arp_division", 1}, "Arp Division",
        juce::StringArray{"1/2", "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"}, 4));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"arp_hold", 1}, "Arp Hold / Latch", false));

    // Sequencer Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"seq_enabled", 1}, "Seq Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"seq_record", 1}, "Seq Record", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"seq_play", 1}, "Seq Play", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"seq_length", 1}, "Seq Length", 1, 64, 16));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"seq_bpm", 1}, "Seq BPM",
        juce::NormalisableRange<float>(30.0f, 250.0f, 0.1f), 120.0f));

    // Effects Engine Parameters (Phase 4)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"fx_on", 1}, "FX On / Bypass", false));

    const juce::StringArray fxChoices = {
        "BBD DELAY", "DIGITAL DELAY", "TAPE DELAY", "CHORUS", "FLANGER",
        "PHASER", "RING MOD", "PLATE REVERB", "HALL REVERB", "OFF"
    };

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"fx1_type", 1}, "FX 1 Type", fxChoices, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fx1_param1", 1}, "FX 1 Param 1",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fx1_param2", 1}, "FX 1 Param 2",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fx1_mix", 1}, "FX 1 Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"fx2_type", 1}, "FX 2 Type", fxChoices, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fx2_param1", 1}, "FX 2 Param 1",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fx2_param2", 1}, "FX 2 Param 2",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fx2_mix", 1}, "FX 2 Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    return { params.begin(), params.end() };
}

void Prophet6AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    synth_.init(static_cast<float>(sampleRate));
    isSustainPedalDown_ = false;
    sustainedNotes_.fill(false);
}

void Prophet6AudioProcessor::releaseResources()
{
    synth_.reset();
}

bool Prophet6AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void Prophet6AudioProcessor::updateParameters() noexcept
{
    auto& p = synth_.getParameters();

    if (pPolyModFiltEnv_)      p.polyModFiltEnvAmt    = pPolyModFiltEnv_->load(std::memory_order_relaxed);
    if (pPolyModOsc2_)         p.polyModOsc2Amt       = pPolyModOsc2_->load(std::memory_order_relaxed);
    if (pPolyModDestOsc1Freq_) p.polyModDestOsc1Freq  = pPolyModDestOsc1Freq_->load(std::memory_order_relaxed) > 0.5f;
    if (pPolyModDestOsc1Shape_)p.polyModDestOsc1Shape = pPolyModDestOsc1Shape_->load(std::memory_order_relaxed) > 0.5f;
    if (pPolyModDestOsc1PW_)   p.polyModDestOsc1PW    = pPolyModDestOsc1PW_->load(std::memory_order_relaxed) > 0.5f;
    if (pPolyModDestLPF_)      p.polyModDestLPF       = pPolyModDestLPF_->load(std::memory_order_relaxed) > 0.5f;
    if (pPolyModDestHPF_)      p.polyModDestHPF       = pPolyModDestHPF_->load(std::memory_order_relaxed) > 0.5f;

    if (pOsc1Freq_)            p.osc1Octave           = static_cast<int>(pOsc1Freq_->load(std::memory_order_relaxed) / 12.0f);
    if (pOsc1Wave_)            p.osc1Shape            = pOsc1Wave_->load(std::memory_order_relaxed);
    if (pOsc1PW_)              p.osc1PulseWidth       = pOsc1PW_->load(std::memory_order_relaxed);
    if (pOsc1SubLevel_)        p.osc1SubLevel         = pOsc1SubLevel_->load(std::memory_order_relaxed);
    if (pOsc1Sync_)            p.osc1Sync             = pOsc1Sync_->load(std::memory_order_relaxed) > 0.5f;

    if (pOsc2Freq_)            p.osc2Octave           = static_cast<int>(pOsc2Freq_->load(std::memory_order_relaxed) / 12.0f);
    if (pOsc2Fine_)            p.osc2FineTune         = pOsc2Fine_->load(std::memory_order_relaxed) * 0.01f; // cents to semitones
    if (pOsc2Wave_)            p.osc2Shape            = pOsc2Wave_->load(std::memory_order_relaxed);
    if (pOsc2PW_)              p.osc2PulseWidth       = pOsc2PW_->load(std::memory_order_relaxed);
    if (pOsc2LowFreq_)         p.osc2LowFreq          = pOsc2LowFreq_->load(std::memory_order_relaxed) > 0.5f;
    if (pOsc2KeyTrack_)        p.osc2KbdTrack         = pOsc2KeyTrack_->load(std::memory_order_relaxed) > 0.5f;

    if (pSlop_)                p.slop                 = pSlop_->load(std::memory_order_relaxed);

    if (pOsc1Level_)           p.osc1Level            = pOsc1Level_->load(std::memory_order_relaxed);
    if (pOsc2Level_)           p.osc2Level            = pOsc2Level_->load(std::memory_order_relaxed);
    if (pNoiseLevel_)          p.noiseLevel           = pNoiseLevel_->load(std::memory_order_relaxed);

    if (pLpfCutoff_)           p.lpfCutoff            = pLpfCutoff_->load(std::memory_order_relaxed);
    if (pLpfResonance_)        p.lpfResonance         = pLpfResonance_->load(std::memory_order_relaxed);
    if (pLpfEnvAmount_)        p.lpfEnvAmount         = pLpfEnvAmount_->load(std::memory_order_relaxed);
    if (pLpfKeyTrack_)         p.lpfKbdTrack          = pLpfKeyTrack_->load(std::memory_order_relaxed);

    if (pHpfCutoff_)           p.hpfCutoff            = pHpfCutoff_->load(std::memory_order_relaxed);
    if (pHpfResonance_)        p.hpfResonance         = pHpfResonance_->load(std::memory_order_relaxed);
    if (pHpfEnvAmount_)        p.hpfEnvAmount         = pHpfEnvAmount_->load(std::memory_order_relaxed);

    if (pFiltAttack_)          p.filtAttack           = pFiltAttack_->load(std::memory_order_relaxed);
    if (pFiltDecay_)           p.filtDecay            = pFiltDecay_->load(std::memory_order_relaxed);
    if (pFiltSustain_)         p.filtSustain          = pFiltSustain_->load(std::memory_order_relaxed);
    if (pFiltRelease_)         p.filtRelease          = pFiltRelease_->load(std::memory_order_relaxed);

    if (pAmpAttack_)           p.ampAttack            = pAmpAttack_->load(std::memory_order_relaxed);
    if (pAmpDecay_)            p.ampDecay             = pAmpDecay_->load(std::memory_order_relaxed);
    if (pAmpSustain_)          p.ampSustain           = pAmpSustain_->load(std::memory_order_relaxed);
    if (pAmpRelease_)          p.ampRelease           = pAmpRelease_->load(std::memory_order_relaxed);
    if (pVelSens_)             p.ampVelocityAmount    = pVelSens_->load(std::memory_order_relaxed);

    if (pLfoShape_)            p.lfoShape             = static_cast<LfoWaveform>(static_cast<int>(pLfoShape_->load(std::memory_order_relaxed)));
    if (pLfoFreq_)             p.lfoFrequency         = pLfoFreq_->load(std::memory_order_relaxed);
    if (pLfoAmount_)           p.lfoInitialAmount     = pLfoAmount_->load(std::memory_order_relaxed);
    if (pLfoDestOsc1Freq_)     p.lfoDestOsc1Freq      = pLfoDestOsc1Freq_->load(std::memory_order_relaxed) > 0.5f;
    if (pLfoDestOsc2Freq_)     p.lfoDestOsc2Freq      = pLfoDestOsc2Freq_->load(std::memory_order_relaxed) > 0.5f;
    if (pLfoDestLPF_)          p.lfoDestLPF           = pLfoDestLPF_->load(std::memory_order_relaxed) > 0.5f;
    if (pLfoDestHPF_)          p.lfoDestHPF           = pLfoDestHPF_->load(std::memory_order_relaxed) > 0.5f;
    if (pLfoDestAmp_)          p.lfoDestAmp           = pLfoDestAmp_->load(std::memory_order_relaxed) > 0.5f;

    if (pDistortion_)          p.distortion           = pDistortion_->load(std::memory_order_relaxed);
    if (pMasterVolume_)        p.masterVolume         = pMasterVolume_->load(std::memory_order_relaxed);
    if (pPanSpread_)           p.panSpread            = pPanSpread_->load(std::memory_order_relaxed);
    if (pUnison_)              p.unison               = pUnison_->load(std::memory_order_relaxed) > 0.5f;
    if (pUnisonDetune_)        p.unisonDetune         = pUnisonDetune_->load(std::memory_order_relaxed);
    if (pGlide_)               p.glide                = pGlide_->load(std::memory_order_relaxed);

    // Arpeggiator & Step Sequencer
    if (pArpEnabled_)  synth_.getArpeggiator().setEnabled(pArpEnabled_->load(std::memory_order_relaxed) > 0.5f);
    if (pArpMode_)     synth_.getArpeggiator().setMode(static_cast<ArpMode>(static_cast<int>(pArpMode_->load(std::memory_order_relaxed))));
    if (pArpOctave_)   synth_.getArpeggiator().setOctaveRange(static_cast<int>(pArpOctave_->load(std::memory_order_relaxed)));
    if (pArpDivision_) synth_.getArpeggiator().setDivision(static_cast<ClockDivision>(static_cast<int>(pArpDivision_->load(std::memory_order_relaxed))));
    if (pArpHold_)     synth_.getArpeggiator().setHold(pArpHold_->load(std::memory_order_relaxed) > 0.5f);

    if (pSeqEnabled_)  synth_.getSequencer().setEnabled(pSeqEnabled_->load(std::memory_order_relaxed) > 0.5f);
    if (pSeqRecord_)   synth_.getSequencer().setRecording(pSeqRecord_->load(std::memory_order_relaxed) > 0.5f);
    if (pSeqPlay_)     synth_.getSequencer().setPlaying(pSeqPlay_->load(std::memory_order_relaxed) > 0.5f);
    if (pSeqLength_)   synth_.getSequencer().setSequenceLength(static_cast<int>(pSeqLength_->load(std::memory_order_relaxed)));
    if (pSeqBpm_)      synth_.setTempo(pSeqBpm_->load(std::memory_order_relaxed));

    // Effects Engine Parameters (Phase 4)
    if (pFxOn_)       p.fxOn       = (pFxOn_->load(std::memory_order_relaxed) > 0.5f);
    if (pFx1Type_)    p.fx1Type    = static_cast<int>(pFx1Type_->load(std::memory_order_relaxed));
    if (pFx1Param1_)  p.fx1Param1  = pFx1Param1_->load(std::memory_order_relaxed);
    if (pFx1Param2_)  p.fx1Param2  = pFx1Param2_->load(std::memory_order_relaxed);
    if (pFx1Mix_)     p.fx1Mix     = pFx1Mix_->load(std::memory_order_relaxed);
    if (pFx2Type_)    p.fx2Type    = static_cast<int>(pFx2Type_->load(std::memory_order_relaxed));
    if (pFx2Param1_)  p.fx2Param1  = pFx2Param1_->load(std::memory_order_relaxed);
    if (pFx2Param2_)  p.fx2Param2  = pFx2Param2_->load(std::memory_order_relaxed);
    if (pFx2Mix_)     p.fx2Mix     = pFx2Mix_->load(std::memory_order_relaxed);
}

void Prophet6AudioProcessor::handleNoteOn(int noteNumber, float velocity) noexcept
{
    if (noteNumber >= 0 && noteNumber < 128) {
        sustainedNotes_[static_cast<size_t>(noteNumber)] = false;
        synth_.noteOn(noteNumber, velocity);
    }
}

void Prophet6AudioProcessor::handleNoteOff(int noteNumber) noexcept
{
    if (noteNumber >= 0 && noteNumber < 128) {
        if (isSustainPedalDown_) {
            sustainedNotes_[static_cast<size_t>(noteNumber)] = true;
        } else {
            synth_.noteOff(noteNumber);
        }
    }
}

void Prophet6AudioProcessor::handleSustainPedal(int controllerValue) noexcept
{
    const bool isDown = (controllerValue >= 64);
    if (isSustainPedalDown_ && !isDown) {
        // Pedal released: release all sustained notes
        for (int note = 0; note < 128; ++note) {
            if (sustainedNotes_[static_cast<size_t>(note)]) {
                synth_.noteOff(note);
                sustainedNotes_[static_cast<size_t>(note)] = false;
            }
        }
    }
    isSustainPedalDown_ = isDown;
}

void Prophet6AudioProcessor::allNotesOff() noexcept
{
    synth_.allNotesOff();
    sustainedNotes_.fill(false);
}

void Prophet6AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, numSamples);
    }

    // Lock-free parameter update from APVTS
    updateParameters();

    // Extract DAW host tempo & transport state
    bool isPlayActive = (pSeqPlay_ && pSeqPlay_->load(std::memory_order_relaxed) > 0.5f);
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            if (pos->getBpm().hasValue()) {
                synth_.setTempo(*pos->getBpm());
            }
            if (pos->getIsPlaying()) {
                isPlayActive = true;
            }
            const double ppq = pos->getPpqPosition().orFallback(0.0);
            synth_.setTransport(isPlayActive, ppq);
        }
    } else {
        synth_.setTransport(isPlayActive, 0.0);
    }

    // Process Virtual Keyboard & External MIDI
    keyboardState_.processNextMidiBuffer(midiMessages, 0, numSamples, true);

    // Process MIDI events
    for (const auto metadata : midiMessages) {
        const auto m = metadata.getMessage();

        if (m.isNoteOn()) {
            handleNoteOn(m.getNoteNumber(), m.getFloatVelocity());
        } else if (m.isNoteOff()) {
            handleNoteOff(m.getNoteNumber());
        } else if (m.isPitchWheel()) {
            const float pitchBendNorm = static_cast<float>(m.getPitchWheelValue() - 8192) * (1.0f / 8192.0f);
            synth_.pitchBend(pitchBendNorm);
        } else if (m.isProgramChange()) {
            setCurrentProgram(m.getProgramChangeNumber());
        } else if (m.isSysEx()) {
            const auto* syxData = reinterpret_cast<const uint8_t*>(m.getSysExData());
            const size_t syxSize = static_cast<size_t>(m.getSysExDataSize());
            Preset p;
            int bank = 0, prog = 0;
            if (SysEx::decodeProgramDump(syxData, syxSize, p, bank, prog)) {
                const int fullProg = bank * 100 + prog;
                presetManager_.setPreset(static_cast<size_t>(fullProg), p);
                presetManager_.loadPreset(static_cast<size_t>(fullProg), synth_);
                syncApvtsWithCurrentPreset();
            } else if (SysEx::decodeEditBufferDump(syxData, syxSize, p)) {
                synth_.loadPreset(p);
                syncApvtsWithCurrentPreset();
            }
        } else if (m.isController()) {
            const int ccNum = m.getControllerNumber();
            const int ccVal = m.getControllerValue();

            if (ccNum == 64) {
                handleSustainPedal(ccVal);
            } else if (ccNum == 1) {
                synth_.controlChange(1, ccVal);
            } else if (ccNum == 120 || ccNum == 123) {
                allNotesOff();
            } else {
                synth_.controlChange(ccNum, ccVal);
            }
        }
    }

    // Real-time zero-allocation block rendering
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = (totalNumOutputChannels > 1) ? buffer.getWritePointer(1) : leftChannel;

    synth_.process(leftChannel, rightChannel, static_cast<size_t>(numSamples));
}

bool Prophet6AudioProcessor::isVoiceActive(int voiceIndex) const noexcept
{
    if (voiceIndex < 0 || voiceIndex >= 6) return false;
    const auto& voice = synth_.getVoiceManager().getVoices()[static_cast<size_t>(voiceIndex)];
    return voice.isActive() && (voice.getAmpLevel() > 0.005f);
}

bool Prophet6AudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* Prophet6AudioProcessor::createEditor() { return new Prophet6AudioProcessorEditor(*this); }

const juce::String Prophet6AudioProcessor::getName() const { return "Prophet-6"; }
bool Prophet6AudioProcessor::acceptsMidi() const { return true; }
bool Prophet6AudioProcessor::producesMidi() const { return false; }
bool Prophet6AudioProcessor::isMidiEffect() const { return false; }
double Prophet6AudioProcessor::getTailLengthSeconds() const { return 0.0; }

int Prophet6AudioProcessor::getNumPrograms() {
    return static_cast<int>(presetManager_.getNumPresets());
}

int Prophet6AudioProcessor::getCurrentProgram() {
    return static_cast<int>(presetManager_.getCurrentProgramIndex());
}

void Prophet6AudioProcessor::setCurrentProgram(int index) {
    if (index >= 0 && index < static_cast<int>(presetManager_.getNumPresets())) {
        presetManager_.loadPreset(static_cast<size_t>(index), synth_);
        syncApvtsWithCurrentPreset();
    }
}

const juce::String Prophet6AudioProcessor::getProgramName(int index) {
    if (index >= 0 && index < static_cast<int>(presetManager_.getNumPresets())) {
        const auto& p = presetManager_.getPreset(static_cast<size_t>(index));
        return juce::String::formatted("%03d: ", index) + juce::String(p.getName().data());
    }
    return {};
}

void Prophet6AudioProcessor::changeProgramName(int index, const juce::String& newName) {
    if (index >= 0 && index < static_cast<int>(presetManager_.getNumPresets())) {
        auto p = presetManager_.getPreset(static_cast<size_t>(index));
        p.setName(newName.toStdString());
        presetManager_.setPreset(static_cast<size_t>(index), p);
    }
}

void Prophet6AudioProcessor::syncApvtsWithCurrentPreset()
{
    const auto& p = presetManager_.getCurrentPreset();
    const auto& pm = p.params;

    auto setFloat = [this](const char* id, float val) {
        if (auto* param = apvts_.getParameter(id)) {
            param->setValueNotifyingHost(param->convertTo0to1(val));
        }
    };
    auto setBool = [this](const char* id, bool val) {
        if (auto* param = apvts_.getParameter(id)) {
            param->setValueNotifyingHost(val ? 1.0f : 0.0f);
        }
    };
    auto setChoice = [this](const char* id, int idx) {
        if (auto* param = apvts_.getParameter(id)) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        }
    };

    // Poly Mod
    setFloat("polymod_filter_env", pm.polyModFiltEnvAmt);
    setFloat("polymod_osc2", pm.polyModOsc2Amt);
    setBool("polymod_dest_osc1_freq", pm.polyModDestOsc1Freq);
    setBool("polymod_dest_osc1_shape", pm.polyModDestOsc1Shape);
    setBool("polymod_dest_osc1_pw", pm.polyModDestOsc1PW);
    setBool("polymod_dest_lpf", pm.polyModDestLPF);
    setBool("polymod_dest_hpf", pm.polyModDestHPF);

    // Oscillators
    setFloat("osc1_freq", static_cast<float>(pm.osc1Octave * 12));
    setFloat("osc1_wave", pm.osc1Shape);
    setFloat("osc1_pw", pm.osc1PulseWidth);
    setFloat("osc1_sub_level", pm.osc1SubLevel);
    setBool("sync", pm.osc1Sync);

    setFloat("osc2_freq", static_cast<float>(pm.osc2Octave * 12));
    setFloat("osc2_fine", pm.osc2FineTune * 100.0f / 7.0f);
    setFloat("osc2_wave", pm.osc2Shape);
    setFloat("osc2_pw", pm.osc2PulseWidth);
    setBool("osc2_low_freq", pm.osc2LowFreq);
    setBool("osc2_key_track", pm.osc2KbdTrack);

    setFloat("slop", pm.slop);

    // Mixer
    setFloat("osc1_level", pm.osc1Level);
    setFloat("osc2_level", pm.osc2Level);
    setFloat("noise_level", pm.noiseLevel);

    // Filters
    setFloat("lpf_cutoff", pm.lpfCutoff);
    setFloat("lpf_resonance", pm.lpfResonance);
    setFloat("lpf_env_amount", pm.lpfEnvAmount);
    setFloat("lpf_key_track", pm.lpfKbdTrack);

    setFloat("hpf_cutoff", pm.hpfCutoff);
    setFloat("hpf_resonance", pm.hpfResonance);
    setFloat("hpf_env_amount", pm.hpfEnvAmount);

    // Envelopes
    setFloat("filt_attack", pm.filtAttack);
    setFloat("filt_decay", pm.filtDecay);
    setFloat("filt_sustain", pm.filtSustain);
    setFloat("filt_release", pm.filtRelease);

    setFloat("amp_attack", pm.ampAttack);
    setFloat("amp_decay", pm.ampDecay);
    setFloat("amp_sustain", pm.ampSustain);
    setFloat("amp_release", pm.ampRelease);
    setFloat("velocity_sensitivity", pm.ampVelocityAmount);

    // LFO
    setChoice("lfo_shape", static_cast<int>(pm.lfoShape));
    setFloat("lfo_freq", pm.lfoFrequency);
    setFloat("lfo_amount", pm.lfoInitialAmount);
    setBool("lfo_dest_osc1_freq", pm.lfoDestOsc1Freq);
    setBool("lfo_dest_osc2_freq", pm.lfoDestOsc2Freq);
    setBool("lfo_dest_lpf", pm.lfoDestLPF);
    setBool("lfo_dest_hpf", pm.lfoDestHPF);
    setBool("lfo_dest_amp", pm.lfoDestAmp);

    // Master
    setFloat("distortion_amount", pm.distortion);
    setFloat("master_volume", pm.masterVolume);
    setFloat("pan_spread", pm.panSpread);
    setBool("unison", pm.unison);
    setFloat("unison_detune", pm.unisonDetune);
    setFloat("glide", pm.glide);

    // Arpeggiator & Sequencer
    setBool("arp_enabled", p.arpEnabled);
    setChoice("arp_mode", static_cast<int>(p.arpMode));
    setFloat("arp_octave", static_cast<float>(p.arpOctave));
    setChoice("arp_division", static_cast<int>(p.arpDivision));
    setBool("arp_hold", p.arpHold);

    setBool("seq_enabled", p.seqEnabled);
    setFloat("seq_length", static_cast<float>(p.seqLength));
    setFloat("seq_bpm", p.seqBpm);

    // Effects Engine
    setBool("fx_on", pm.fxOn);
    setChoice("fx1_type", pm.fx1Type);
    setFloat("fx1_param1", pm.fx1Param1);
    setFloat("fx1_param2", pm.fx1Param2);
    setFloat("fx1_mix", pm.fx1Mix);
    setChoice("fx2_type", pm.fx2Type);
    setFloat("fx2_param1", pm.fx2Param1);
    setFloat("fx2_param2", pm.fx2Param2);
    setFloat("fx2_mix", pm.fx2Mix);
}

void Prophet6AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    state.setProperty("currentProgram", getCurrentProgram(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Prophet6AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts_.state.getType())) {
        auto newState = juce::ValueTree::fromXml(*xmlState);
        apvts_.replaceState(newState);
        if (newState.hasProperty("currentProgram")) {
            const int prog = newState.getProperty("currentProgram");
            presetManager_.setCurrentProgramIndex(static_cast<size_t>(prog));
        }
    }
}

} // namespace prophet6

// Entry point required by JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new prophet6::Prophet6AudioProcessor();
}
