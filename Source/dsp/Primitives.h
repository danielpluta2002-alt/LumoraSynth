#pragma once

#include "../SynthParams.h"
#include <array>
#include <cmath>

namespace lumora::dsp
{
constexpr float twoPi = 6.28318530717958647692f;

inline float fastTanh (float x) noexcept
{
    if (x > 3.0f) return 1.0f;
    if (x < -3.0f) return -1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline float semitonesToRatio (float semis) noexcept { return std::exp2 (semis * (1.0f / 12.0f)); }

inline float noteToHz (float note) noexcept { return 440.0f * semitonesToRatio (note - 69.0f); }

/** Small, fast per-instance random generator (xorshift32). */
struct Rng
{
    uint32_t state = 0x9e3779b9u;
    void seed (uint32_t s) noexcept { state = s != 0 ? s : 0x9e3779b9u; }
    uint32_t next() noexcept { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
    float unipolar() noexcept { return (float) (next() >> 8) * (1.0f / 16777216.0f); }
    float bipolar() noexcept { return unipolar() * 2.0f - 1.0f; }
};

/** Sine lookup table with linear interpolation, phase in [0, 1). */
struct SineTable
{
    static constexpr int size = 2048;
    std::array<float, size + 1> table {};

    SineTable()
    {
        for (int i = 0; i <= size; ++i)
            table[(size_t) i] = std::sin (twoPi * (float) i / (float) size);
    }

    static const SineTable& get() { static const SineTable t; return t; }

    inline float lookup (float phase) const noexcept
    {
        const float pos = phase * (float) size;
        const int idx = (int) pos;
        const float frac = pos - (float) idx;
        const float a = table[(size_t) idx];
        return a + (table[(size_t) idx + 1] - a) * frac;
    }
};

inline float polyBlep (float t, float dt) noexcept
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

//==============================================================================
/** One oscillator with up to eight detuned unison voices, spread across the stereo field. */
class Oscillator
{
public:
    void resetPhases (const OscParams& p, bool hardReset, Rng& rng) noexcept
    {
        if (! (p.retrig || hardReset))
            return;

        for (int i = 0; i < maxUnison; ++i)
        {
            if (p.retrig)
                phases[i] = std::fmod (p.phase + (float) i * 0.1357f, 1.0f);
            else
                phases[i] = rng.unipolar();
        }
        noise.seed (rng.next());
    }

    /** Called at control rate: sets frequency, unison spread and gains. */
    void setControl (float freqHz, float sampleRate, const OscParams& p, float detune, float stereo, float gain) noexcept
    {
        wave = p.wave;
        numVoices = wave == waveNoise ? 2 : p.voices;
        const float sign = p.invert ? -1.0f : 1.0f;
        const float norm = gain * sign / std::sqrt ((float) numVoices);
        const float maxInc = 0.45f;

        for (int i = 0; i < numVoices; ++i)
        {
            const float t = numVoices > 1 ? -1.0f + 2.0f * (float) i / (float) (numVoices - 1) : 0.0f;
            const float cents = t * detune * detune * 80.0f + t * detune * 20.0f;
            incs[i] = juce::jmin (maxInc, freqHz * std::exp2 (cents / 1200.0f) / sampleRate);

            // Alternate the stereo side so neighbouring detune values land on opposite sides.
            const float side = (i % 2 == 0 ? t : -t);
            const float pan = juce::jlimit (-1.0f, 1.0f, p.pan + side * stereo);
            const float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
            gainL[i] = std::cos (angle) * juce::MathConstants<float>::sqrt2 * norm;
            gainR[i] = std::sin (angle) * juce::MathConstants<float>::sqrt2 * norm;
        }
    }

    inline void process (float& outL, float& outR) noexcept
    {
        float l = 0.0f, r = 0.0f;

        if (wave == waveNoise)
        {
            l += noise.bipolar() * gainL[0] + noise.bipolar() * gainL[1];
            r += noise.bipolar() * gainR[0] + noise.bipolar() * gainR[1];
            outL += l;
            outR += r;
            return;
        }

        const auto& sine = SineTable::get();

        for (int i = 0; i < numVoices; ++i)
        {
            const float t = phases[i];
            const float dt = incs[i];
            float v;

            switch (wave)
            {
                case waveSine:      v = sine.lookup (t); break;
                case waveTriangle:  v = t < 0.5f ? 4.0f * t - 1.0f : 3.0f - 4.0f * t; break;
                case waveSquare:    v = pulse (t, dt, 0.5f); break;
                case wavePulse:     v = pulse (t, dt, 0.2f); break;
                case waveSaw:
                default:            v = 2.0f * t - 1.0f - polyBlep (t, dt); break;
            }

            l += v * gainL[i];
            r += v * gainR[i];

            float next = t + dt;
            if (next >= 1.0f) next -= 1.0f;
            phases[i] = next;
        }

        outL += l;
        outR += r;
    }

private:
    static inline float pulse (float t, float dt, float width) noexcept
    {
        float v = t < width ? 1.0f : -1.0f;
        v += polyBlep (t, dt);
        float t2 = t + 1.0f - width;
        if (t2 >= 1.0f) t2 -= 1.0f;
        v -= polyBlep (t2, dt);
        return v - (2.0f * width - 1.0f); // remove DC
    }

    float phases[maxUnison] {};
    float incs[maxUnison] {};
    float gainL[maxUnison] {}, gainR[maxUnison] {};
    int numVoices = 1, wave = waveSaw;
    Rng noise;
};

//==============================================================================
/** Stereo state-variable filter (topology-preserving transform), 12 or 24 dB/oct. */
class VoiceFilter
{
public:
    void reset() noexcept
    {
        for (auto& s : stages)
            s = {};
    }

    void setParams (const FilterParams& p, float cutoffHz, float reso, float drive, float sampleRate) noexcept
    {
        type = p.type;
        cascade = p.slope == 1;
        driveGain = 1.0f + drive * 7.0f;
        driveComp = 1.0f / (1.0f + drive * 1.5f);
        useDrive = drive > 0.001f;

        const float fc = juce::jlimit (16.0f, sampleRate * 0.45f, cutoffHz);
        const float g = std::tan (juce::MathConstants<float>::pi * fc / sampleRate);

        // In 24 dB mode the second stage is less resonant, so the peak stays controllable.
        const float k1 = 2.0f - 1.96f * juce::jlimit (0.0f, 1.0f, reso);
        const float k2 = cascade ? juce::jmax (k1, 1.2f) : k1;
        stages[0].setCoefs (g, cascade ? juce::jmin (2.0f, k1 * 1.15f) : k1);
        stages[1].setCoefs (g, k2);
    }

    inline void process (float& l, float& r) noexcept
    {
        if (type == filterBypass)
            return;

        if (useDrive)
        {
            l = fastTanh (l * driveGain) * driveComp;
            r = fastTanh (r * driveGain) * driveComp;
        }

        l = stages[0].process (l, 0, type);
        r = stages[0].process (r, 1, type);

        if (cascade)
        {
            l = stages[1].process (l, 0, type);
            r = stages[1].process (r, 1, type);
        }
    }

private:
    struct Stage
    {
        float ic1[2] {}, ic2[2] {};
        float a1 = 0, a2 = 0, a3 = 0, k = 2;

        void setCoefs (float g, float newK) noexcept
        {
            k = newK;
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }

        inline float process (float v0, int ch, int filterType) noexcept
        {
            const float v3 = v0 - ic2[ch];
            const float v1 = a1 * ic1[ch] + a2 * v3;
            const float v2 = ic2[ch] + a2 * ic1[ch] + a3 * v3;
            ic1[ch] = 2.0f * v1 - ic1[ch];
            ic2[ch] = 2.0f * v2 - ic2[ch];

            switch (filterType)
            {
                case filterBandpass: return v1 * k;
                case filterHighpass: return v0 - k * v1 - v2;
                case filterNotch:    return v0 - k * v1;
                case filterLowpass:
                default:             return v2;
            }
        }
    };

    Stage stages[2];
    int type = filterLowpass;
    bool cascade = true, useDrive = false;
    float driveGain = 1.0f, driveComp = 1.0f;
};

//==============================================================================
/** ADSR with linear attack and exponential decay/release. Advances by a fixed step of samples. */
class Envelope
{
public:
    void prepare (float sampleRate, int samplesPerStep) noexcept
    {
        sr = sampleRate;
        step = samplesPerStep;
        cached = {};
        cached.attack = -1.0f;
    }

    void setParams (const EnvParams& p) noexcept
    {
        sustain = p.sustain;
        if (juce::exactlyEqual (p.attack, cached.attack) && juce::exactlyEqual (p.decay, cached.decay)
            && juce::exactlyEqual (p.release, cached.release))
            return;

        cached = p;
        const float stepF = (float) step;
        attackInc = stepF / (juce::jmax (0.0005f, p.attack) * sr);
        decayCoef = std::exp (-5.0f * stepF / (juce::jmax (0.0005f, p.decay) * sr));
        releaseCoef = std::exp (-5.0f * stepF / (juce::jmax (0.0005f, p.release) * sr));
    }

    void noteOn() noexcept { stage = Stage::attack; }
    void noteOff() noexcept { if (stage != Stage::idle) stage = Stage::release; }
    void reset() noexcept { stage = Stage::idle; level = 0.0f; }

    bool isActive() const noexcept { return stage != Stage::idle; }
    float getLevel() const noexcept { return level; }

    inline float next() noexcept
    {
        switch (stage)
        {
            case Stage::attack:
                level += attackInc;
                if (level >= 1.0f) { level = 1.0f; stage = Stage::decay; }
                break;
            case Stage::decay:
                level = sustain + (level - sustain) * decayCoef;
                if (level - sustain < 1.0e-4f) { level = sustain; stage = Stage::sustain; }
                break;
            case Stage::sustain:
                level += (sustain - level) * 0.01f * (float) step;
                break;
            case Stage::release:
                level *= releaseCoef;
                if (level < 1.0e-4f) { level = 0.0f; stage = Stage::idle; }
                break;
            case Stage::idle:
            default:
                break;
        }
        return level;
    }

private:
    enum class Stage { idle, attack, decay, sustain, release };
    Stage stage = Stage::idle;
    float level = 0.0f, sustain = 1.0f;
    float attackInc = 1.0f, decayCoef = 0.0f, releaseCoef = 0.0f;
    float sr = 44100.0f;
    int step = 1;
    EnvParams cached;
};

//==============================================================================
class Lfo
{
public:
    void reset (float startPhase, uint32_t seed) noexcept
    {
        phase = startPhase - std::floor (startPhase);
        rng.seed (seed);
        current = rng.bipolar();
        previous = current;
    }

    /** Advances by @p inc cycles and returns a bipolar value. */
    inline float advance (float inc, int wave) noexcept
    {
        phase += inc;
        if (phase >= 1.0f)
        {
            phase -= std::floor (phase);
            previous = current;
            current = rng.bipolar();
        }

        switch (wave)
        {
            case lfoTriangle:     return phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase;
            case lfoSawUp:        return 2.0f * phase - 1.0f;
            case lfoSawDown:      return 1.0f - 2.0f * phase;
            case lfoSquare:       return phase < 0.5f ? 1.0f : -1.0f;
            case lfoSampleHold:   return current;
            case lfoSmoothRandom:
            {
                const float s = phase * phase * (3.0f - 2.0f * phase);
                return previous + (current - previous) * s;
            }
            case lfoSine:
            default:              return SineTable::get().lookup (phase);
        }
    }

private:
    float phase = 0.0f, current = 0.0f, previous = 0.0f;
    Rng rng;
};
} // namespace lumora::dsp
