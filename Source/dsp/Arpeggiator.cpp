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

#include "Arpeggiator.h"

namespace lumora::dsp
{
void Arpeggiator::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
    pressed.reserve (128);
    latched.reserve (128);
    sequence.reserve (512);
    sounding.reserve (128);
    outBuffer.ensureSize (4096);
    reset();
}

void Arpeggiator::reset()
{
    pressed.clear();
    latched.clear();
    sequence.clear();
    sounding.clear();
    latchArmed = false;
    stepPosition = 0.0;
    sequenceIndex = 0;
    noteOffCountdown = -1.0;
    wasEnabled = false;
    wasRunning = false;
    currentStepForGui = -1;
}

void Arpeggiator::stopSounding (juce::MidiBuffer& out, int pos)
{
    for (int n : sounding)
        out.addEvent (juce::MidiMessage::noteOff (1, n), pos);
    sounding.clear();
    noteOffCountdown = -1.0;
}

void Arpeggiator::buildSequence (const ArpParams& p)
{
    sequence.clear();
    if (latched.empty())
        return;

    std::vector<Note> base (latched.begin(), latched.end());
    if (p.mode != arpOrdered)
        std::sort (base.begin(), base.end(), [] (const Note& a, const Note& b) { return a.note < b.note; });

    for (int oct = 0; oct < p.octaves; ++oct)
        for (const auto& n : base)
            if (n.note + oct * 12 <= 127)
                sequence.push_back ({ n.note + oct * 12, n.velocity });
}

void Arpeggiator::handleInput (const juce::MidiMessage& m, const ArpParams& p, juce::MidiBuffer& out, int pos)
{
    if (m.isNoteOn())
    {
        const int note = m.getNoteNumber();
        pressed.erase (std::remove_if (pressed.begin(), pressed.end(), [note] (const Note& n) { return n.note == note; }), pressed.end());
        pressed.push_back ({ note, m.getFloatVelocity() });

        if (p.hold)
        {
            if (latchArmed)
            {
                latched.clear();
                latchArmed = false;
            }
            latched.erase (std::remove_if (latched.begin(), latched.end(), [note] (const Note& n) { return n.note == note; }), latched.end());
            latched.push_back ({ note, m.getFloatVelocity() });
        }
        else
        {
            latched = pressed;
        }
        buildSequence (p);
    }
    else if (m.isNoteOff())
    {
        const int note = m.getNoteNumber();
        pressed.erase (std::remove_if (pressed.begin(), pressed.end(), [note] (const Note& n) { return n.note == note; }), pressed.end());

        if (p.hold)
            latchArmed = pressed.empty();
        else
            latched = pressed;

        buildSequence (p);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        pressed.clear();
        latched.clear();
        sequence.clear();
        stopSounding (out, pos);
        out.addEvent (m, pos);
    }
    else
    {
        out.addEvent (m, pos);
    }
}

void Arpeggiator::triggerStep (const ArpParams& p, juce::MidiBuffer& out, int pos, double stepSamples)
{
    stopSounding (out, pos);

    const int numSteps = juce::jmax (1, p.numSteps);
    const int stepNum = (int) (((stepIndex % numSteps) + numSteps) % numSteps);
    ++stepIndex;
    currentStepForGui = stepNum;

    const auto& step = p.steps[stepNum];
    if (! step.on || sequence.empty())
        return;

    auto velocityFor = [&] (float keyVel)
    {
        switch (p.velMode)
        {
            case arpVelStep: return step.velocity;
            case arpVelBoth: return step.velocity * keyVel;
            case arpVelKey:
            default:         return keyVel;
        }
    };

    auto play = [&] (int note, float vel)
    {
        note = juce::jlimit (0, 127, note + step.transpose);
        vel = juce::jlimit (0.0f, 1.0f, vel);
        if (vel < 1.0f / 127.0f || std::find (sounding.begin(), sounding.end(), note) != sounding.end())
            return;
        out.addEvent (juce::MidiMessage::noteOn (1, note, vel), pos);
        sounding.push_back (note);
    };

    const int count = (int) sequence.size();

    if (p.mode == arpChord)
    {
        // Whole chord on every step, climbing through the octave range.
        const int octave = sequenceIndex % juce::jmax (1, p.octaves);
        ++sequenceIndex;
        for (const auto& n : latched)
            play (n.note + octave * 12, velocityFor (n.velocity));
    }
    else
    {
        int idx = 0;
        const int i = sequenceIndex++;

        switch (p.mode)
        {
            case arpDown:
                idx = count - 1 - (i % count);
                break;
            case arpUpDown:
            case arpDownUp:
            {
                const int period = juce::jmax (1, 2 * count - 2);
                const int ph = i % period;
                idx = count == 1 ? 0 : (ph < count ? ph : period - ph);
                if (p.mode == arpDownUp)
                    idx = count - 1 - idx;
                break;
            }
            case arpUpDown2:
            {
                const int period = 2 * count;
                const int ph = i % period;
                idx = ph < count ? ph : period - 1 - ph;
                break;
            }
            case arpRandom:
                idx = (int) (rng.next() % (uint32_t) count);
                break;
            case arpUp:
            case arpOrdered:
            default:
                idx = i % count;
                break;
        }

        const auto& n = sequence[(size_t) juce::jlimit (0, count - 1, idx)];
        play (n.note, velocityFor (n.velocity));
    }

    if (! sounding.empty())
        noteOffCountdown = (double) pos + juce::jmax (1.0, p.gate * stepSamples);
}

void Arpeggiator::process (juce::MidiBuffer& midi, int numSamples, const ArpParams& p, const Transport& t)
{
    auto& out = outBuffer;
    out.clear();

    if (! p.enabled)
    {
        if (wasEnabled)
        {
            stopSounding (out, 0);
            pressed.clear();
            latched.clear();
            sequence.clear();
            wasEnabled = false;
            wasRunning = false;
            currentStepForGui = -1;
            for (const auto meta : midi)
                out.addEvent (meta.getMessage(), meta.samplePosition);
            midi.swapWith (out);
        }
        return;
    }

    if (! wasEnabled)
    {
        // Keys held while the arp gets switched on should be released from the synth.
        out.addEvent (juce::MidiMessage::allNotesOff (1), 0);
        wasEnabled = true;
    }

    if (! p.hold && latched.size() != pressed.size())
    {
        latched = pressed;
        buildSequence (p);
    }

    const double bpm = juce::jlimit (20.0, 999.0, t.bpm);
    const double stepBeats = syncDivisionBeats (p.division);
    const double stepSamples = juce::jmax (1.0, 60.0 / bpm * stepBeats * sampleRate);
    const bool hostClock = t.isPlaying && t.hasPosition;
    const double swingOffset = juce::jlimit (0.0, 1.0, (double) p.swing) * 0.5;

    if (hostClock)
        stepPosition = t.ppqPosition / stepBeats;

    auto runSegment = [&] (int from, int to)
    {
        if (to <= from)
            return;

        const bool running = ! latched.empty();
        const double x0 = stepPosition + (double) from / stepSamples;
        const double x1 = stepPosition + (double) to / stepSamples;

        auto flushNoteOff = [&] (double beforeSample)
        {
            if (noteOffCountdown >= 0.0 && noteOffCountdown < beforeSample)
                stopSounding (out, juce::jlimit (0, numSamples - 1, (int) noteOffCountdown));
        };

        if (running)
        {
            const long long firstPair = (long long) std::floor (x0 / 2.0) - 1;
            const long long lastPair = (long long) std::floor (x1 / 2.0) + 1;

            for (long long pair = firstPair; pair <= lastPair; ++pair)
            {
                for (int odd = 0; odd < 2; ++odd)
                {
                    const double trig = (double) (pair * 2 + odd) + (odd ? swingOffset : 0.0);
                    if (trig < x0 || trig >= x1)
                        continue;

                    const int pos = juce::jlimit (from, to - 1, from + (int) std::floor ((trig - x0) * stepSamples));
                    flushNoteOff ((double) pos);
                    if (hostClock)
                        stepIndex = pair * 2 + odd;
                    triggerStep (p, out, pos, stepSamples);
                }
            }
        }

        flushNoteOff ((double) to);
    };

    int pos = 0;
    for (const auto meta : midi)
    {
        const int at = juce::jlimit (0, numSamples, meta.samplePosition);
        runSegment (pos, at);
        pos = juce::jmax (pos, at);

        const bool wasEmpty = latched.empty();
        handleInput (meta.getMessage(), p, out, juce::jmin (at, numSamples - 1));

        if (wasEmpty && ! latched.empty() && ! hostClock)
        {
            // Free-running clock: the first key starts the pattern right away.
            stepPosition = -(double) at / stepSamples;
            stepIndex = 0;
            sequenceIndex = 0;
        }
        else if (! wasEmpty && latched.empty())
        {
            stopSounding (out, juce::jmin (at, numSamples - 1));
            sequenceIndex = 0;
            currentStepForGui = -1;
        }
    }
    runSegment (pos, numSamples);

    if (! hostClock)
        stepPosition += (double) numSamples / stepSamples;

    if (noteOffCountdown >= 0.0)
        noteOffCountdown -= numSamples;

    midi.swapWith (out);
}
} // namespace lumora::dsp
