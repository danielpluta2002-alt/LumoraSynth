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

#include "Voice.h"

namespace lumora::dsp
{
namespace
{
    constexpr float pitchModRange = 36.0f;   // semitones at full amount
    constexpr float cutoffModRange = 120.0f; // semitones at full amount
    constexpr float voiceHeadroom = 0.5f;    // -6 dB per voice, so chords leave headroom
}

void Voice::prepare (double sampleRate)
{
    sr = (float) sampleRate;
    for (auto& e : ampEnv) e.prepare (sr, 1);
    for (auto& e : modEnv) e.prepare (sr, controlInterval);
    killStep = 1.0f / (0.003f * sr);
    forceIdle();
}

void Voice::start (int newNote, float newVelocity, float glideFromNote, const VoiceParams& p,
                   uint32_t seed, const float* freeLfoPhases, uint64_t newOrder)
{
    const bool hardReset = ! active || killing;

    note = newNote;
    velocity = newVelocity;
    order = newOrder;
    targetPitch = (float) newNote;
    currentPitch = p.porta > 0.0005f ? glideFromNote : targetPitch;

    rng.seed (seed);
    randomValue = rng.bipolar();

    for (int i = 0; i < numOscs; ++i)
        osc[i].resetPhases (p.osc[i], hardReset, rng);

    if (hardReset)
    {
        for (auto& f : filter) f.reset();
        for (auto& e : ampEnv) e.reset();
        for (auto& e : modEnv) e.reset();
        gainA = gainB = 0.0f;
        std::fill (std::begin (dest), std::end (dest), 0.0f);
        firstControl = true;
    }

    for (int i = 0; i < 2; ++i)
    {
        if (p.lfo[i].freeRun)
            lfo[i].reset (freeLfoPhases[i], rng.next());
        else
            lfo[i].reset (0.0f, rng.next());

        ampEnv[i].setParams (p.ampEnv[i]);
        modEnv[i].setParams (p.modEnv[i]);
        ampEnv[i].noteOn();
        modEnv[i].noteOn();
    }

    active = true;
    released = false;
    killing = false;
    killGain = 1.0f;
    controlCounter = 0;
}

void Voice::glideTo (int newNote, uint64_t newOrder)
{
    note = newNote;
    targetPitch = (float) newNote;
    order = newOrder;
    released = false;
}

void Voice::release()
{
    released = true;
    for (auto& e : ampEnv) e.noteOff();
    for (auto& e : modEnv) e.noteOff();
}

void Voice::kill()
{
    if (active)
        killing = true;
}

void Voice::forceIdle()
{
    active = false;
    released = false;
    killing = false;
    killGain = 1.0f;
    for (auto& e : ampEnv) e.reset();
    for (auto& e : modEnv) e.reset();
}

void Voice::updateControl (const VoiceParams& p, const Expression& ex)
{
    // --- modulation sources ---------------------------------------------------
    float src[numModSources] {};

    for (int i = 0; i < 2; ++i)
    {
        modEnv[i].setParams (p.modEnv[i]);
        src[srcModEnv1 + i] = modEnv[i].next();

        const auto& lp = p.lfo[i];
        float rate = lp.sync ? (float) (p.bpm / 60.0 / syncDivisionBeats (lp.division)) : lp.rate;
        rate *= std::exp2 (dest[dstLfo1Rate + i] * 4.0f);
        const float gain = juce::jlimit (0.0f, 1.0f, lp.gain + dest[dstLfo1Gain + i]);
        src[srcLfo1 + i] = lfo[i].advance (rate * (float) controlInterval / sr, lp.wave) * gain;
    }

    src[srcVelocity] = velocity;
    src[srcModWheel] = ex.modWheel;
    src[srcAftertouch] = ex.aftertouch;
    src[srcKeyTrack] = ((float) note - 60.0f) / 60.0f;
    src[srcRandom] = randomValue;

    std::fill (std::begin (dest), std::end (dest), 0.0f);
    for (const auto& slot : p.mods)
        if (slot.source > srcOff && slot.source < numModSources && slot.dest > dstOff && slot.dest < numModDests)
            dest[slot.dest] += src[slot.source] * slot.amount;

    // --- portamento -----------------------------------------------------------
    if (p.porta > 0.0005f)
    {
        const float coef = 1.0f - std::exp (-(float) controlInterval / (p.porta * 0.25f * sr));
        currentPitch += (targetPitch - currentPitch) * coef;
    }
    else
    {
        currentPitch = targetPitch;
    }

    // --- oscillators ----------------------------------------------------------
    const float basePitch = currentPitch + ex.bendSemis + dest[dstPitchAll] * pitchModRange;

    for (int i = 0; i < numOscs; ++i)
    {
        const auto& op = p.osc[i];
        oscOn[i] = op.volume > 0.0001f;
        if (! oscOn[i])
            continue;

        const float partPitch = (i < 2 ? dest[dstPitchA] : dest[dstPitchB]) * pitchModRange;
        const float pitch = basePitch + partPitch + (float) (op.octave * 12 + op.note) + op.fine * 0.01f;
        const float detune = juce::jlimit (0.0f, 1.0f, op.detune + dest[dstDetune]);
        const float stereo = juce::jlimit (0.0f, 1.0f, op.stereo + dest[dstStereo]);
        osc[i].setControl (noteToHz (pitch), sr, op, detune, stereo, op.volume);
    }

    // --- filters --------------------------------------------------------------
    const float keytrack = p.ctlKeytrack * (currentPitch - 60.0f);

    for (int i = 0; i < numFilters; ++i)
    {
        const auto& fp = p.filter[i];
        filterInput[i] = fp.input;
        const float mod = (dest[dstCutoffAB] + (i == 0 ? dest[dstCutoffA] : dest[dstCutoffB])) * cutoffModRange;
        const float cutoff = fp.cutoff * semitonesToRatio (p.ctlCutoff + keytrack + mod);
        const float reso = juce::jlimit (0.0f, 1.0f, fp.reso + p.ctlReso + dest[dstResoAB]);
        const float drive = juce::jlimit (0.0f, 1.0f, fp.drive + dest[dstDrive]);
        filter[i].setParams (fp, cutoff, reso, drive, sr);
    }

    warm = p.ctlWarm;
    warmGain = 1.0f + warm * 5.0f;
    warmComp = 1.0f / (1.0f + warm * 2.0f);

    // --- amp and pan (ramped across the control block) ------------------------
    for (int i = 0; i < 2; ++i)
        ampEnv[i].setParams (p.ampEnv[i]);

    const float velGain = (1.0f - p.velSens * (1.0f - velocity)) * voiceHeadroom;
    const float mixShift = dest[dstMixAB];
    const float volA = juce::jmax (0.0f, 1.0f + dest[dstVolumeAll] + dest[dstVolumeA]);
    const float volB = juce::jmax (0.0f, 1.0f + dest[dstVolumeAll] + dest[dstVolumeB]);
    const float targetA = volA * p.mixA * juce::jlimit (0.0f, 1.0f, 1.0f - mixShift) * velGain;
    const float targetB = volB * p.mixB * juce::jlimit (0.0f, 1.0f, 1.0f + mixShift) * velGain;

    const float pan = juce::jlimit (-1.0f, 1.0f, dest[dstPan]);
    const float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
    const float targetL = std::cos (angle) * juce::MathConstants<float>::sqrt2;
    const float targetR = std::sin (angle) * juce::MathConstants<float>::sqrt2;

    if (firstControl)
    {
        gainA = targetA; gainB = targetB; panL = targetL; panR = targetR;
        firstControl = false;
    }

    const float inv = 1.0f / (float) controlInterval;
    gainAInc = (targetA - gainA) * inv;
    gainBInc = (targetB - gainB) * inv;
    panLInc = (targetL - panL) * inv;
    panRInc = (targetR - panR) * inv;
}

void Voice::render (float* left, float* right, int numSamples, const VoiceParams& p, const Expression& ex)
{
    if (! active)
        return;

    int pos = 0;
    while (pos < numSamples)
    {
        if (controlCounter <= 0)
        {
            updateControl (p, ex);
            controlCounter = controlInterval;
        }

        const int chunk = juce::jmin (numSamples - pos, controlCounter);

        for (int s = 0; s < chunk; ++s)
        {
            float aL = 0.0f, aR = 0.0f, bL = 0.0f, bR = 0.0f;
            if (oscOn[0]) osc[0].process (aL, aR);
            if (oscOn[1]) osc[1].process (aL, aR);
            if (oscOn[2]) osc[2].process (bL, bR);
            if (oscOn[3]) osc[3].process (bL, bR);

            float fAL = aL, fAR = aR, fBL = bL, fBR = bR;
            if (filterInput[0] == 1) { fAL += bL; fAR += bR; }
            if (filterInput[1] == 1) { fBL += aL; fBR += aR; }

            filter[0].process (fAL, fAR);
            filter[1].process (fBL, fBR);

            const float envA = ampEnv[0].next() * gainA;
            const float envB = ampEnv[1].next() * gainB;
            gainA += gainAInc;
            gainB += gainBInc;

            float l = fAL * envA + fBL * envB;
            float r = fAR * envA + fBR * envB;

            if (warm > 0.001f)
            {
                l = fastTanh (l * warmGain) * warmComp;
                r = fastTanh (r * warmGain) * warmComp;
            }

            float g = 1.0f;
            if (killing)
            {
                killGain -= killStep;
                if (killGain <= 0.0f)
                {
                    forceIdle();
                    return;
                }
                g = killGain;
            }

            left[pos + s] += l * panL * g;
            right[pos + s] += r * panR * g;
            panL += panLInc;
            panR += panRInc;
        }

        controlCounter -= chunk;
        pos += chunk;

        if (! ampEnv[0].isActive() && ! ampEnv[1].isActive())
        {
            forceIdle();
            return;
        }
    }
}
} // namespace lumora::dsp
