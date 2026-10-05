/*
    Lumora Synth - polyphonic synthesizer plugin
    Copyright (C) 2026 danielpluta2002-alt

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    SPDX-License-Identifier: AGPL-3.0-or-later
*/

#include "Effects.h"
#include "Primitives.h"

namespace lumora::dsp
{
namespace
{
    /** Keeps the dry signal at full level up to 50 % mix, then fades it out. */
    inline void wetDryGains (float mix, float& dry, float& wet)
    {
        dry = juce::jmin (1.0f, 2.0f * (1.0f - mix));
        wet = juce::jmin (1.0f, 2.0f * mix);
    }

    inline float foldback (float x)
    {
        // Reflects the signal back into [-1, 1].
        x = std::fmod (x + 1.0f, 4.0f);
        if (x < 0.0f) x += 4.0f;
        return x < 2.0f ? x - 1.0f : 3.0f - x;
    }
}

//==============================================================================
void Distortion::process (juce::AudioBuffer<float>& buffer, int type, float amount, float mix)
{
    const int numCh = juce::jmin (2, buffer.getNumChannels());
    const int n = buffer.getNumSamples();
    const float drive = 1.0f + amount * amount * 30.0f;
    const float levels = std::exp2 (16.0f - amount * 13.5f);
    const float decimate = 1.0f + amount * amount * 40.0f;
    const float makeup = type == distOverdrive || type == distHardClip ? 1.0f / std::sqrt (drive) * 1.6f : 1.0f;
    const float wet = mix, dry = 1.0f - mix;

    for (int s = 0; s < n; ++s)
    {
        bool takeNew = true;
        if (type == distDecimate)
        {
            counter += 1.0f;
            takeNew = counter >= decimate;
            if (takeNew) counter -= decimate;
        }

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            const float x = d[s];
            float y;

            switch (type)
            {
                case distHardClip: y = juce::jlimit (-1.0f, 1.0f, x * drive) * makeup; break;
                case distFoldback: y = foldback (x * (1.0f + amount * 6.0f)) * 0.8f; break;
                case distBitcrush: y = std::round (x * levels) / levels; break;
                case distDecimate:
                    if (takeNew) held[ch] = x;
                    y = held[ch];
                    break;
                case distOverdrive:
                default:           y = fastTanh (x * drive) * makeup; break;
            }

            d[s] = x * dry + y * wet;
        }
    }
}

//==============================================================================
void StereoDelay::prepare (double sampleRate, int)
{
    sr = sampleRate;
    line.setSize (2, (int) (sampleRate * 2.5) + 4);
    timeL.reset (sampleRate, 0.08);
    timeR.reset (sampleRate, 0.08);
    reset();
}

void StereoDelay::reset()
{
    line.clear();
    writePos = 0;
    lpState[0] = lpState[1] = hpState[0] = hpState[1] = 0.0f;
    first = true;
}

void StereoDelay::process (juce::AudioBuffer<float>& buffer, float timeLSec, float timeRSec, float feedback,
                           bool pingPong, float lowCutHz, float highCutHz, float mix)
{
    const int size = line.getNumSamples();
    const float maxDelay = (float) size - 2.0f;
    const float targetL = juce::jlimit (1.0f, maxDelay, timeLSec * (float) sr);
    const float targetR = juce::jlimit (1.0f, maxDelay, timeRSec * (float) sr);

    if (first)
    {
        timeL.setCurrentAndTargetValue (targetL);
        timeR.setCurrentAndTargetValue (targetR);
        first = false;
    }
    timeL.setTargetValue (targetL);
    timeR.setTargetValue (targetR);

    const float lpCoef = 1.0f - std::exp (-twoPi * highCutHz / (float) sr);
    const float hpCoef = 1.0f - std::exp (-twoPi * lowCutHz / (float) sr);
    const float fb = juce::jlimit (0.0f, 0.98f, feedback);
    float dryGain, wetGain;
    wetDryGains (mix, dryGain, wetGain);

    auto* bufL = line.getWritePointer (0);
    auto* bufR = line.getWritePointer (1);
    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : outL;

    auto read = [size] (const float* data, float readPos)
    {
        if (readPos < 0.0f) readPos += (float) size;
        const int i0 = (int) readPos;
        const int i1 = (i0 + 1) % size;
        const float frac = readPos - (float) i0;
        return data[i0] + (data[i1] - data[i0]) * frac;
    };

    for (int s = 0; s < buffer.getNumSamples(); ++s)
    {
        const float inL = outL[s];
        const float inR = outR[s];

        float dl = read (bufL, (float) writePos - timeL.getNextValue());
        float dr = read (bufR, (float) writePos - timeR.getNextValue());

        // Tone shaping inside the feedback path.
        lpState[0] += (dl - lpState[0]) * lpCoef;
        lpState[1] += (dr - lpState[1]) * lpCoef;
        hpState[0] += (lpState[0] - hpState[0]) * hpCoef;
        hpState[1] += (lpState[1] - hpState[1]) * hpCoef;
        dl = lpState[0] - hpState[0];
        dr = lpState[1] - hpState[1];

        if (pingPong)
        {
            bufL[writePos] = 0.5f * (inL + inR) + dr * fb;
            bufR[writePos] = dl * fb;
        }
        else
        {
            bufL[writePos] = inL + dl * fb;
            bufR[writePos] = inR + dr * fb;
        }

        if (++writePos >= size)
            writePos = 0;

        outL[s] = inL * dryGain + dl * wetGain;
        if (outR != outL)
            outR[s] = inR * dryGain + dr * wetGain;
    }
}

//==============================================================================
void EffectsChain::prepare (double sampleRate, int maxBlockSize)
{
    sr = sampleRate;
    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };

    phaser.prepare (spec);
    chorus.prepare (spec);
    lowShelf.prepare (spec);
    highShelf.prepare (spec);
    compressor.prepare (spec);
    delay.prepare (sampleRate, maxBlockSize);
    reverb.setSampleRate (sampleRate);
    dryBuffer.setSize (2, maxBlockSize);
    lastLowGain = lastHighGain = 999.0f;
    reset();
}

void EffectsChain::reset()
{
    distortion.reset();
    phaser.reset();
    chorus.reset();
    lowShelf.reset();
    highShelf.reset();
    compressor.reset();
    delay.reset();
    reverb.reset();
}

void EffectsChain::process (juce::AudioBuffer<float>& buffer, const FxParams& p)
{
    if (buffer.getNumChannels() < 2)
        return;

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    const int n = buffer.getNumSamples();

    if (p.distOn)
    {
        if (! wasDistOn) distortion.reset();
        distortion.process (buffer, p.distType, p.distAmount, p.distMix);
    }
    wasDistOn = p.distOn;

    if (p.phaserOn)
    {
        if (! wasPhaserOn) phaser.reset();
        phaser.setRate (p.phaserRate);
        phaser.setDepth (p.phaserDepth);
        phaser.setCentreFrequency (p.phaserFreq);
        phaser.setFeedback (juce::jlimit (-0.95f, 0.95f, p.phaserFb));
        phaser.setMix (p.phaserMix);
        phaser.process (ctx);
    }
    wasPhaserOn = p.phaserOn;

    if (p.chorusOn)
    {
        if (! wasChorusOn) chorus.reset();
        const bool flanger = p.chorusMode == 1;
        chorus.setRate (p.chorusRate);
        chorus.setDepth (flanger ? p.chorusDepth * 0.5f : p.chorusDepth);
        chorus.setCentreDelay (flanger ? juce::jlimit (1.0f, 6.0f, p.chorusDelay * 0.25f) : p.chorusDelay);
        chorus.setFeedback (juce::jlimit (-0.95f, 0.95f, p.chorusFb));
        chorus.setMix (p.chorusMix);
        chorus.process (ctx);
    }
    wasChorusOn = p.chorusOn;

    if (p.eqOn)
    {
        if (! wasEqOn) { lowShelf.reset(); highShelf.reset(); }
        if (! juce::exactlyEqual (p.eqLowGain, lastLowGain) || ! juce::exactlyEqual (p.eqLowFreq, lastLowFreq))
        {
            *lowShelf.state = juce::dsp::IIR::ArrayCoefficients<float>::makeLowShelf (sr, p.eqLowFreq, 0.7f,
                                                                                      juce::Decibels::decibelsToGain (p.eqLowGain));
            lastLowGain = p.eqLowGain;
            lastLowFreq = p.eqLowFreq;
        }
        if (! juce::exactlyEqual (p.eqHighGain, lastHighGain) || ! juce::exactlyEqual (p.eqHighFreq, lastHighFreq))
        {
            *highShelf.state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf (sr, juce::jmin (p.eqHighFreq, (float) sr * 0.45f), 0.7f,
                                                                                        juce::Decibels::decibelsToGain (p.eqHighGain));
            lastHighGain = p.eqHighGain;
            lastHighFreq = p.eqHighFreq;
        }
        lowShelf.process (ctx);
        highShelf.process (ctx);
    }
    wasEqOn = p.eqOn;

    if (p.delayOn)
    {
        if (! wasDelayOn) delay.reset();
        const float beat = (float) (60.0 / juce::jlimit (20.0, 999.0, p.bpm));
        const float tl = p.delaySync ? beat * (float) syncDivisionBeats (p.delayDivL) : p.delayTimeL;
        const float tr = p.delaySync ? beat * (float) syncDivisionBeats (p.delayDivR) : p.delayTimeR;
        delay.process (buffer, tl, tr, p.delayFb, p.delayPingPong, p.delayLowCut, p.delayHighCut, p.delayMix);
    }
    wasDelayOn = p.delayOn;

    if (p.reverbOn)
    {
        if (! wasReverbOn) reverb.reset();

        juce::Reverb::Parameters rp;
        rp.roomSize = p.reverbSize;
        rp.damping = p.reverbDamp;
        rp.width = p.reverbWidth;
        rp.wetLevel = 0.6f;
        rp.dryLevel = 0.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        for (int ch = 0; ch < 2; ++ch)
            dryBuffer.copyFrom (ch, 0, buffer, ch, 0, n);

        reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), n);

        float dryGain, wetGain;
        wetDryGains (p.reverbMix, dryGain, wetGain);
        for (int ch = 0; ch < 2; ++ch)
        {
            buffer.applyGain (ch, 0, n, wetGain);
            buffer.addFrom (ch, 0, dryBuffer, ch, 0, n, dryGain);
        }
    }
    wasReverbOn = p.reverbOn;

    if (p.compOn)
    {
        if (! wasCompOn) compressor.reset();
        compressor.setThreshold (p.compThresh);
        compressor.setRatio (juce::jmax (1.0f, p.compRatio));
        compressor.setAttack (p.compAttack);
        compressor.setRelease (p.compRelease);
        compressor.process (ctx);
        buffer.applyGain (juce::Decibels::decibelsToGain (p.compMakeup));
    }
    wasCompOn = p.compOn;
}
} // namespace lumora::dsp
