#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <prophet6/SynthEngine.hpp>
#include <prophet6/PresetManager.hpp>
#include <prophet6/SysEx.hpp>
#include <array>
#include <atomic>

namespace prophet6 {

class Prophet6AudioProcessor : public juce::AudioProcessor {
public:
    Prophet6AudioProcessor();
    ~Prophet6AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Real-time voice state for hardware GUI LEDs
    [[nodiscard]] bool isVoiceActive(int voiceIndex) const noexcept;

    // Real-time sequencer & arp state for hardware GUI
    [[nodiscard]] size_t getCurrentSequenceStep() const noexcept { return synth_.getSequencer().getCurrentStep(); }
    [[nodiscard]] size_t getRecordSequenceStep() const noexcept { return synth_.getSequencer().getRecordStep(); }
    [[nodiscard]] bool isSequencerRecording() const noexcept { return synth_.getSequencer().isRecording(); }
    void recordTie() noexcept { synth_.getSequencer().recordTie(); }
    void recordRest() noexcept { synth_.getSequencer().recordRest(); }

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }

    [[nodiscard]] PresetManager& getPresetManager() noexcept { return presetManager_; }
    [[nodiscard]] const PresetManager& getPresetManager() const noexcept { return presetManager_; }
    void syncApvtsWithCurrentPreset();

    // Hardware Compare & Write Workflow (Phase 6)
    void toggleCompare() {
        presetManager_.toggleCompare(synth_);
        syncApvtsWithCurrentPreset();
    }
    [[nodiscard]] bool isComparing() const noexcept { return presetManager_.isComparing(); }

    void armWrite() noexcept { presetManager_.armWrite(); }
    void cancelWrite() noexcept { presetManager_.cancelWrite(); }
    bool confirmWrite(std::string_view name = {}) {
        const bool ok = presetManager_.confirmWrite(synth_, name);
        if (ok) syncApvtsWithCurrentPreset();
        return ok;
    }
    [[nodiscard]] bool isWriteArmed() const noexcept { return presetManager_.isWriteArmed(); }
    [[nodiscard]] int getWriteDestination() const noexcept { return presetManager_.getWriteDestination(); }
    void setWriteDestination(int slot) noexcept { presetManager_.setWriteDestination(slot); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateParameters() noexcept;
    void handleNoteOn(int noteNumber, float velocity) noexcept;
    void handleNoteOff(int noteNumber) noexcept;
    void handleSustainPedal(int controllerValue) noexcept;
    void allNotesOff() noexcept;

    SynthEngine synth_;
    PresetManager presetManager_{};

    juce::AudioProcessorValueTreeState apvts_;

    // Sustain pedal state
    bool isSustainPedalDown_ = false;
    std::array<bool, 128> sustainedNotes_{};

    // Cached atomic parameter pointers for lock-free, zero-allocation audio thread updates
    std::atomic<float>* pPolyModFiltEnv_ = nullptr;
    std::atomic<float>* pPolyModOsc2_ = nullptr;
    std::atomic<float>* pPolyModDestOsc1Freq_ = nullptr;
    std::atomic<float>* pPolyModDestOsc1Shape_ = nullptr;
    std::atomic<float>* pPolyModDestOsc1PW_ = nullptr;
    std::atomic<float>* pPolyModDestLPF_ = nullptr;
    std::atomic<float>* pPolyModDestHPF_ = nullptr;

    std::atomic<float>* pOsc1Freq_ = nullptr;
    std::atomic<float>* pOsc1Wave_ = nullptr;
    std::atomic<float>* pOsc1PW_ = nullptr;
    std::atomic<float>* pOsc1SubLevel_ = nullptr;
    std::atomic<float>* pOsc1Sync_ = nullptr;

    std::atomic<float>* pOsc2Freq_ = nullptr;
    std::atomic<float>* pOsc2Fine_ = nullptr;
    std::atomic<float>* pOsc2Wave_ = nullptr;
    std::atomic<float>* pOsc2PW_ = nullptr;
    std::atomic<float>* pOsc2LowFreq_ = nullptr;
    std::atomic<float>* pOsc2KeyTrack_ = nullptr;

    std::atomic<float>* pOsc1Level_ = nullptr;
    std::atomic<float>* pOsc2Level_ = nullptr;
    std::atomic<float>* pNoiseLevel_ = nullptr;

    std::atomic<float>* pLpfCutoff_ = nullptr;
    std::atomic<float>* pLpfResonance_ = nullptr;
    std::atomic<float>* pLpfEnvAmount_ = nullptr;
    std::atomic<float>* pLpfKeyTrack_ = nullptr;

    std::atomic<float>* pHpfCutoff_ = nullptr;
    std::atomic<float>* pHpfResonance_ = nullptr;
    std::atomic<float>* pHpfEnvAmount_ = nullptr;

    std::atomic<float>* pFiltAttack_ = nullptr;
    std::atomic<float>* pFiltDecay_ = nullptr;
    std::atomic<float>* pFiltSustain_ = nullptr;
    std::atomic<float>* pFiltRelease_ = nullptr;

    std::atomic<float>* pAmpAttack_ = nullptr;
    std::atomic<float>* pAmpDecay_ = nullptr;
    std::atomic<float>* pAmpSustain_ = nullptr;
    std::atomic<float>* pAmpRelease_ = nullptr;
    std::atomic<float>* pVelSens_ = nullptr;

    std::atomic<float>* pLfoShape_ = nullptr;
    std::atomic<float>* pLfoFreq_ = nullptr;
    std::atomic<float>* pLfoAmount_ = nullptr;
    std::atomic<float>* pLfoDestOsc1Freq_ = nullptr;
    std::atomic<float>* pLfoDestOsc2Freq_ = nullptr;
    std::atomic<float>* pLfoDestLPF_ = nullptr;
    std::atomic<float>* pLfoDestHPF_ = nullptr;
    std::atomic<float>* pLfoDestAmp_ = nullptr;

    std::atomic<float>* pDistortion_ = nullptr;
    std::atomic<float>* pMasterVolume_ = nullptr;
    std::atomic<float>* pSlop_ = nullptr;
    std::atomic<float>* pPanSpread_ = nullptr;
    std::atomic<float>* pUnison_ = nullptr;
    std::atomic<float>* pUnisonDetune_ = nullptr;
    std::atomic<float>* pGlide_ = nullptr;

    // Arpeggiator & Step Sequencer Parameters
    std::atomic<float>* pArpEnabled_ = nullptr;
    std::atomic<float>* pArpMode_ = nullptr;
    std::atomic<float>* pArpOctave_ = nullptr;
    std::atomic<float>* pArpDivision_ = nullptr;
    std::atomic<float>* pArpHold_ = nullptr;

    std::atomic<float>* pSeqEnabled_ = nullptr;
    std::atomic<float>* pSeqRecord_ = nullptr;
    std::atomic<float>* pSeqPlay_ = nullptr;
    std::atomic<float>* pSeqLength_ = nullptr;
    std::atomic<float>* pSeqBpm_ = nullptr;

    // Effects Engine Parameters (Phase 4)
    std::atomic<float>* pFxOn_ = nullptr;
    std::atomic<float>* pFx1Type_ = nullptr;
    std::atomic<float>* pFx1Param1_ = nullptr;
    std::atomic<float>* pFx1Param2_ = nullptr;
    std::atomic<float>* pFx1Mix_ = nullptr;
    std::atomic<float>* pFx2Type_ = nullptr;
    std::atomic<float>* pFx2Param1_ = nullptr;
    std::atomic<float>* pFx2Param2_ = nullptr;
    std::atomic<float>* pFx2Mix_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Prophet6AudioProcessor)
};

} // namespace prophet6
