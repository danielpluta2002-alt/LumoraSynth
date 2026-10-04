#pragma once

#include "../SynthParams.h"
#include <juce_dsp/juce_dsp.h>

namespace lumora::dsp
{
class Distortion
{
public:
    void reset() { held[0] = held[1] = 0.0f; counter = 0.0f; }
    void process (juce::AudioBuffer<float>& buffer, int type, float amount, float mix);

private:
    float held[2] {};
    float counter = 0.0f;
};

class StereoDelay
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void process (juce::AudioBuffer<float>& buffer, float timeLSec, float timeRSec, float feedback,
                  bool pingPong, float lowCutHz, float highCutHz, float mix);

private:
    juce::AudioBuffer<float> line;
    int writePos = 0;
    double sr = 44100.0;
    juce::SmoothedValue<float> timeL, timeR;
    float lpState[2] {}, hpState[2] {};
    bool first = true;
};

/** The effect rack: Distortion > Phaser > Chorus/Flanger > EQ > Delay > Reverb > Compressor. */
class EffectsChain
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void process (juce::AudioBuffer<float>& buffer, const FxParams& p);

private:
    using StereoIIR = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

    double sr = 44100.0;
    Distortion distortion;
    juce::dsp::Phaser<float> phaser;
    juce::dsp::Chorus<float> chorus;
    StereoIIR lowShelf, highShelf;
    StereoDelay delay;
    juce::Reverb reverb;
    juce::dsp::Compressor<float> compressor;
    juce::AudioBuffer<float> dryBuffer;

    float lastLowGain = 999.0f, lastLowFreq = 0.0f, lastHighGain = 999.0f, lastHighFreq = 0.0f;
    bool wasPhaserOn = false, wasChorusOn = false, wasEqOn = false, wasDelayOn = false, wasReverbOn = false, wasCompOn = false, wasDistOn = false;
};
} // namespace lumora::dsp
