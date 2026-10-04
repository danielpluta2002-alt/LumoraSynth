#pragma once

#include "Parameters.h"

namespace lumora
{
/** Plain snapshots of the parameter values, taken once per audio block. */
struct OscParams
{
    int wave = waveSaw, voices = 1, octave = 0, note = 0;
    float detune = 0.0f, stereo = 0.0f, phase = 0.0f, fine = 0.0f, volume = 0.0f, pan = 0.0f;
    bool retrig = false, invert = false;
};

struct FilterParams
{
    int type = filterLowpass, slope = 1, input = 0;
    float cutoff = 8000.0f, reso = 0.0f, drive = 0.0f;
};

struct EnvParams
{
    float attack = 0.0f, decay = 0.3f, sustain = 1.0f, release = 0.1f;
};

struct LfoParams
{
    int wave = lfoSine, division = 7;
    float rate = 1.0f, gain = 1.0f;
    bool sync = false, freeRun = false;
};

struct ModSlot
{
    int source = srcOff, dest = dstOff;
    float amount = 0.0f;
};

struct VoiceParams
{
    OscParams osc[numOscs];
    FilterParams filter[numFilters];
    float ctlCutoff = 0.0f, ctlReso = 0.0f, ctlWarm = 0.0f, ctlKeytrack = 0.0f;
    EnvParams ampEnv[2], modEnv[2];
    LfoParams lfo[2];
    ModSlot mods[numModSlots];
    float mixA = 1.0f, mixB = 1.0f, velSens = 0.5f, porta = 0.0f;
    int bendRange = 2, polyphony = 8, voiceMode = modePoly;
    double bpm = 120.0;
};

struct ArpStep
{
    bool on = true;
    int transpose = 0;
    float velocity = 1.0f;
};

struct ArpParams
{
    bool enabled = false, hold = false;
    int mode = arpUp, octaves = 1, division = 13, velMode = arpVelKey, numSteps = 16;
    float gate = 0.5f, swing = 0.0f;
    ArpStep steps[numArpSteps];
};

struct FxParams
{
    bool distOn = false; int distType = distOverdrive; float distAmount = 0.3f, distMix = 1.0f;
    bool phaserOn = false; float phaserRate = 0.4f, phaserDepth = 0.6f, phaserFreq = 900.0f, phaserFb = 0.4f, phaserMix = 0.5f;
    bool chorusOn = false; int chorusMode = 0; float chorusRate = 0.6f, chorusDepth = 0.4f, chorusDelay = 8.0f, chorusFb = 0.0f, chorusMix = 0.5f;
    bool eqOn = false; float eqLowGain = 0.0f, eqLowFreq = 150.0f, eqHighGain = 0.0f, eqHighFreq = 6000.0f;
    bool delayOn = false, delaySync = true, delayPingPong = true; int delayDivL = 9, delayDivR = 7;
    float delayTimeL = 0.375f, delayTimeR = 0.5f, delayFb = 0.35f, delayLowCut = 150.0f, delayHighCut = 8000.0f, delayMix = 0.25f;
    bool reverbOn = false; float reverbSize = 0.7f, reverbDamp = 0.5f, reverbWidth = 1.0f, reverbMix = 0.25f;
    bool compOn = false; float compThresh = -12.0f, compRatio = 4.0f, compAttack = 5.0f, compRelease = 120.0f, compMakeup = 0.0f;
    double bpm = 120.0;
};

/** Caches the raw atomic pointers of every parameter and fills the snapshots above. */
class ParamRefs
{
public:
    explicit ParamRefs (juce::AudioProcessorValueTreeState& state);

    void fill (VoiceParams& voice, ArpParams& arp, FxParams& fx, float& masterDb) const;

private:
    struct Osc { std::atomic<float> *wave, *voices, *detune, *stereo, *phase, *retrig, *invert, *octave, *note, *fine, *volume, *pan; };
    struct Filt { std::atomic<float> *type, *slope, *input, *cutoff, *reso, *drive; };
    struct Env { std::atomic<float> *attack, *decay, *sustain, *release; };
    struct Lfo { std::atomic<float> *wave, *rate, *sync, *division, *gain, *freeRun; };
    struct Mod { std::atomic<float> *source, *dest, *amount; };
    struct Step { std::atomic<float> *on, *transpose, *velocity; };

    Osc osc[numOscs];
    Filt filt[numFilters];
    std::atomic<float> *ctlCutoff, *ctlReso, *ctlWarm, *ctlKeytrack;
    Env ampEnv[2], modEnv[2];
    Lfo lfo[2];
    Mod mods[numModSlots];
    std::atomic<float> *master, *mixA, *mixB, *poly, *voiceMode, *porta, *bendRange, *velSens;

    std::atomic<float> *arpOn, *arpMode, *arpOctaves, *arpDiv, *arpGate, *arpSwing, *arpVelMode, *arpHold, *arpSteps;
    Step steps[numArpSteps];

    std::atomic<float> *distOn, *distType, *distAmount, *distMix;
    std::atomic<float> *phaserOn, *phaserRate, *phaserDepth, *phaserFreq, *phaserFb, *phaserMix;
    std::atomic<float> *chorusOn, *chorusMode, *chorusRate, *chorusDepth, *chorusDelay, *chorusFb, *chorusMix;
    std::atomic<float> *eqOn, *eqLowGain, *eqLowFreq, *eqHighGain, *eqHighFreq;
    std::atomic<float> *delayOn, *delaySync, *delayDivL, *delayDivR, *delayTimeL, *delayTimeR, *delayFb,
                       *delayPingPong, *delayLowCut, *delayHighCut, *delayMix;
    std::atomic<float> *reverbOn, *reverbSize, *reverbDamp, *reverbWidth, *reverbMix;
    std::atomic<float> *compOn, *compThresh, *compRatio, *compAttack, *compRelease, *compMakeup;
};
} // namespace lumora
