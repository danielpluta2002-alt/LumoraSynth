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

#pragma once

#include "../SynthParams.h"
#include "Primitives.h"
#include <vector>

namespace lumora::dsp
{
/**
    Tempo-synced arpeggiator with a 16-step pattern (gate, transpose and velocity per step).
    Replaces incoming notes in the MIDI buffer with the generated sequence.
*/
class Arpeggiator
{
public:
    struct Transport
    {
        double bpm = 120.0;
        double ppqPosition = 0.0;
        bool isPlaying = false;
        bool hasPosition = false;
    };

    void prepare (double sampleRate);
    void reset();

    void process (juce::MidiBuffer& midi, int numSamples, const ArpParams& p, const Transport& transport);

    /** Index of the step that played last, for the GUI (-1 when idle). */
    int getCurrentStep() const noexcept { return currentStepForGui.load(); }

private:
    struct Note { int note; float velocity; };

    void handleInput (const juce::MidiMessage& m, const ArpParams& p, juce::MidiBuffer& out, int pos);
    void buildSequence (const ArpParams& p);
    void triggerStep (const ArpParams& p, juce::MidiBuffer& out, int pos, double stepSamples);
    void stopSounding (juce::MidiBuffer& out, int pos);

    double sampleRate = 44100.0;
    std::vector<Note> pressed;      // keys physically down, in press order
    std::vector<Note> latched;      // notes the arp plays (== pressed unless Hold is on)
    std::vector<Note> sequence;     // expanded over octaves and sorted per mode
    std::vector<int> sounding;      // notes currently on
    bool latchArmed = false;        // next key press starts a fresh latched chord

    double stepPosition = 0.0;      // in steps, for the free-running clock
    long long stepIndex = 0;
    int sequenceIndex = 0;
    int direction = 1;
    double noteOffCountdown = -1.0;
    bool wasEnabled = false, wasRunning = false;
    Rng rng;
    juce::MidiBuffer outBuffer;
    std::atomic<int> currentStepForGui { -1 };
};
} // namespace lumora::dsp
