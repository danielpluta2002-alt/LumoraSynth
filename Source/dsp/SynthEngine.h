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

#include "Voice.h"
#include <vector>

namespace lumora::dsp
{
/** Voice allocation (poly / mono / legato), portamento, sustain pedal and MIDI controllers. */
class SynthEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    /** Renders into the (already cleared) buffer, handling MIDI sample-accurately. */
    void process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi, const VoiceParams& params);

    int getNumActiveVoices() const;

private:
    void handleMidi (const juce::MidiMessage& m, const VoiceParams& p);
    void noteOn (int note, float velocity, const VoiceParams& p);
    void noteOff (int note, const VoiceParams& p);
    void allNotesOff (bool hard);
    void render (juce::AudioBuffer<float>& buffer, int start, int num, const VoiceParams& p);

    Voice* findFreeVoice();
    Voice* findVoiceToSteal();
    int countSoundingVoices() const;
    void startVoice (Voice& v, int note, float velocity, float glideFrom, const VoiceParams& p);

    static constexpr int numVoiceSlots = maxPolyphony + 8; // spare slots for voices fading out

    Voice voices[numVoiceSlots];
    Expression expression;
    double sampleRate = 44100.0;

    struct HeldNote { int note; float velocity; };
    std::vector<HeldNote> held;       // keys currently held, in press order
    std::vector<int> sustained;       // released keys kept alive by the sustain pedal
    bool sustainDown = false;
    int lastVoiceMode = modePoly;
    float lastNotePitch = -1.0f;
    float pitchBend = 0.0f;            // -1 .. 1
    uint64_t noteCounter = 0;
    uint32_t seedCounter = 12345;
    float freeLfoPhase[2] {};
};
} // namespace lumora::dsp
