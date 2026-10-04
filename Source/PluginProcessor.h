#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "SynthParams.h"
#include "Presets.h"
#include "dsp/SynthEngine.h"
#include "dsp/Arpeggiator.h"
#include "dsp/Effects.h"

class LumoraSynthProcessor : public juce::AudioProcessor
{
public:
    LumoraSynthProcessor();
    ~LumoraSynthProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Lumora Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getState() noexcept { return state; }
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    juce::String getPresetName() const;
    void setPresetName (const juce::String& name);

    /** User presets: plain XML files holding the parameter state. */
    static juce::File getUserPresetFolder();
    bool saveUserPreset (const juce::File& file);
    bool loadUserPreset (const juce::File& file);

    int getArpStep() const noexcept { return arp.getCurrentStep(); }
    int getActiveVoiceCount() const noexcept { return activeVoices.load(); }

private:
    juce::AudioProcessorValueTreeState state;
    lumora::ParamRefs params;
    juce::MidiKeyboardState keyboardState;

    lumora::dsp::SynthEngine engine;
    lumora::dsp::Arpeggiator arp;
    lumora::dsp::EffectsChain effects;

    lumora::VoiceParams voiceParams;
    lumora::ArpParams arpParams;
    lumora::FxParams fxParams;
    juce::SmoothedValue<float> masterGain;

    int currentProgram = 0;
    std::atomic<int> activeVoices { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumoraSynthProcessor)
};
