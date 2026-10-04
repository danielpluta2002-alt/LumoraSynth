#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace lumora
{
constexpr int numOscs       = 4;   // A1, A2, B1, B2
constexpr int numFilters    = 2;   // A, B
constexpr int numModSlots   = 8;
constexpr int numArpSteps   = 16;
constexpr int maxPolyphony  = 16;
constexpr int maxUnison     = 8;

enum OscWave { waveSine, waveTriangle, waveSaw, waveSquare, wavePulse, waveNoise };
enum FilterType { filterBypass, filterLowpass, filterBandpass, filterHighpass, filterNotch };
enum LfoWave { lfoSine, lfoTriangle, lfoSawUp, lfoSawDown, lfoSquare, lfoSampleHold, lfoSmoothRandom };
enum VoiceMode { modePoly, modeMono, modeLegato };

enum ModSource
{
    srcOff, srcModEnv1, srcModEnv2, srcLfo1, srcLfo2, srcVelocity,
    srcModWheel, srcAftertouch, srcKeyTrack, srcRandom, numModSources
};

enum ModDest
{
    dstOff, dstPitchAll, dstPitchA, dstPitchB, dstCutoffAB, dstCutoffA, dstCutoffB,
    dstResoAB, dstDrive, dstVolumeAll, dstVolumeA, dstVolumeB, dstMixAB, dstPan,
    dstDetune, dstStereo, dstLfo1Rate, dstLfo2Rate, dstLfo1Gain, dstLfo2Gain, numModDests
};

enum ArpMode { arpUp, arpDown, arpUpDown, arpDownUp, arpUpDown2, arpRandom, arpOrdered, arpChord };
enum ArpVelMode { arpVelKey, arpVelStep, arpVelBoth };
enum DistType { distOverdrive, distHardClip, distFoldback, distBitcrush, distDecimate };

const juce::StringArray& oscWaveNames();
const juce::StringArray& filterTypeNames();
const juce::StringArray& lfoWaveNames();
const juce::StringArray& syncDivisionNames();
const juce::StringArray& modSourceNames();
const juce::StringArray& modDestNames();
const juce::StringArray& voiceModeNames();
const juce::StringArray& arpModeNames();
const juce::StringArray& arpVelModeNames();
const juce::StringArray& distTypeNames();

/** Length of a tempo-synced division in quarter-note beats. */
double syncDivisionBeats (int index);

/** Parameter ID helpers, so the processor, the GUI and the presets agree on names. */
namespace pid
{
    juce::String osc (int index, const char* name);     // oscA1_wave ...
    juce::String filt (int index, const char* name);    // filtA_cutoff ...
    juce::String amp (int index, const char* name);     // ampA_attack ...
    juce::String menv (int index, const char* name);    // menv1_attack ...
    juce::String lfo (int index, const char* name);     // lfo1_rate ...
    juce::String mod (int index, const char* name);     // mod1_src ...
    juce::String arpStep (int index, const char* name); // arp_s1_trans ...

    juce::String oscLabel (int index);                  // "A1" ...
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
} // namespace lumora
