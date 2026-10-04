#pragma once

#include "Primitives.h"

namespace lumora::dsp
{
/** Performance controllers shared by all voices. */
struct Expression
{
    float bendSemis = 0.0f;
    float modWheel = 0.0f;
    float aftertouch = 0.0f;
};

/**
    One synth voice: four oscillators in two parts (A1+A2, B1+B2), one filter and one
    amp envelope per part, two mod envelopes, two LFOs and the modulation matrix.
*/
class Voice
{
public:
    static constexpr int controlInterval = 16;

    void prepare (double sampleRate);

    /** Starts a note. If the voice is still sounding, envelopes continue from their current level. */
    void start (int note, float velocity, float glideFromNote, const VoiceParams& p,
                uint32_t seed, const float* freeLfoPhases, uint64_t order);

    /** Moves to a new pitch without retriggering (legato). */
    void glideTo (int note, uint64_t order);

    void release();
    void kill();          // short fade-out, used when a voice gets stolen
    void forceIdle();

    bool isActive() const noexcept { return active; }
    bool isReleased() const noexcept { return released; }
    bool isKilling() const noexcept { return killing; }
    int getNote() const noexcept { return note; }
    uint64_t getOrder() const noexcept { return order; }
    float getCurrentPitch() const noexcept { return currentPitch; }

    void render (float* left, float* right, int numSamples, const VoiceParams& p, const Expression& ex);

private:
    void updateControl (const VoiceParams& p, const Expression& ex);

    Oscillator osc[numOscs];
    VoiceFilter filter[numFilters];
    Envelope ampEnv[2], modEnv[2];
    Lfo lfo[2];
    Rng rng;

    float sr = 44100.0f;
    int note = 60;
    float velocity = 1.0f, randomValue = 0.0f;
    float currentPitch = 60.0f, targetPitch = 60.0f;
    bool active = false, released = false, killing = false, firstControl = true;
    float killGain = 1.0f, killStep = 0.0f;
    uint64_t order = 0;

    int controlCounter = 0;
    float dest[numModDests] {};
    bool oscOn[numOscs] {};
    int filterInput[numFilters] {};
    float warm = 0.0f, warmGain = 1.0f, warmComp = 1.0f;

    float gainA = 0.0f, gainB = 0.0f, gainAInc = 0.0f, gainBInc = 0.0f;
    float panL = 1.0f, panR = 1.0f, panLInc = 0.0f, panRInc = 0.0f;
};
} // namespace lumora::dsp
