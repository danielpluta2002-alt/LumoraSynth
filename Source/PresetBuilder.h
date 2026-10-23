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

#include "Presets.h"
#include "Parameters.h"

#include <functional>

namespace lumora::presetbuilder
{
using Values = std::vector<std::pair<juce::String, float>>;

/** Small builder so the preset tables stay readable. Later calls override earlier ones. */
struct P
{
    Values v;

    P& set (const juce::String& id, float value) { v.emplace_back (id, value); return *this; }

    P& osc (int i, int wave, float volume, int voices = 1, float detune = 0.2f, float stereo = 0.5f,
            int octave = 0, int note = 0, float fine = 0.0f)
    {
        return set (pid::osc (i, "wave"), (float) wave).set (pid::osc (i, "volume"), volume)
              .set (pid::osc (i, "voices"), (float) voices).set (pid::osc (i, "detune"), detune)
              .set (pid::osc (i, "stereo"), stereo).set (pid::osc (i, "octave"), (float) octave)
              .set (pid::osc (i, "note"), (float) note).set (pid::osc (i, "fine"), fine);
    }

    /** Restart the oscillator at a fixed phase on every note (punchier attacks). */
    P& retrig (int i, float phase = 0.0f)
    {
        return set (pid::osc (i, "retrig"), 1.0f).set (pid::osc (i, "phase"), phase);
    }

    P& pan (int i, float p) { return set (pid::osc (i, "pan"), p); }
    P& invert (int i) { return set (pid::osc (i, "invert"), 1.0f); }

    P& filter (int i, int type, int slope, float cutoff, float reso = 0.0f, float drive = 0.0f, int input = 0)
    {
        return set (pid::filt (i, "type"), (float) type).set (pid::filt (i, "slope"), (float) slope)
              .set (pid::filt (i, "cutoff"), cutoff).set (pid::filt (i, "reso"), reso)
              .set (pid::filt (i, "drive"), drive).set (pid::filt (i, "input"), (float) input);
    }

    P& filters (int type, int slope, float cutoff, float reso = 0.0f, float drive = 0.0f)
    {
        return filter (0, type, slope, cutoff, reso, drive).filter (1, type, slope, cutoff, reso, drive);
    }

    /** Filter control section: global cutoff offset (semitones), resonance, warm drive, key tracking. */
    P& ctl (float cutoffSemis, float reso = 0.0f, float warm = 0.0f, float keytrack = 0.0f)
    {
        return set ("fctl_cutoff", cutoffSemis).set ("fctl_reso", reso)
              .set ("fctl_warm", warm).set ("fctl_keytrack", keytrack);
    }

    P& ampEnv (int i, float a, float d, float s, float r)
    {
        return set (pid::amp (i, "attack"), a).set (pid::amp (i, "decay"), d)
              .set (pid::amp (i, "sustain"), s).set (pid::amp (i, "release"), r);
    }

    P& ampEnvs (float a, float d, float s, float r) { return ampEnv (0, a, d, s, r).ampEnv (1, a, d, s, r); }

    P& modEnv (int i, float a, float d, float s, float r)
    {
        return set (pid::menv (i, "attack"), a).set (pid::menv (i, "decay"), d)
              .set (pid::menv (i, "sustain"), s).set (pid::menv (i, "release"), r);
    }

    P& lfo (int i, int wave, float rate, float gain = 1.0f, bool sync = false, int division = 7, bool freeRun = false)
    {
        return set (pid::lfo (i, "wave"), (float) wave).set (pid::lfo (i, "rate"), rate)
              .set (pid::lfo (i, "gain"), gain).set (pid::lfo (i, "sync"), sync ? 1.0f : 0.0f)
              .set (pid::lfo (i, "div"), (float) division).set (pid::lfo (i, "free"), freeRun ? 1.0f : 0.0f);
    }

    /** Tempo-synced LFO. */
    P& lfoSync (int i, int wave, int division, float gain = 1.0f, bool freeRun = true)
    {
        return lfo (i, wave, 1.0f, gain, true, division, freeRun);
    }

    P& mod (int slot, int src, int dst, float amount)
    {
        return set (pid::mod (slot, "src"), (float) src).set (pid::mod (slot, "dst"), (float) dst)
              .set (pid::mod (slot, "amt"), amount);
    }

    P& delay (float mix, int divL = 9, int divR = 7, float fb = 0.35f)
    {
        return set ("delay_on", 1).set ("delay_mix", mix).set ("delay_divL", (float) divL)
              .set ("delay_divR", (float) divR).set ("delay_fb", fb);
    }

    P& reverb (float mix, float size = 0.7f, float damp = 0.5f)
    {
        return set ("reverb_on", 1).set ("reverb_mix", mix).set ("reverb_size", size).set ("reverb_damp", damp);
    }

    P& chorus (float mix, float rate = 0.6f, float depth = 0.4f)
    {
        return set ("chorus_on", 1).set ("chorus_mix", mix).set ("chorus_rate", rate).set ("chorus_depth", depth);
    }

    P& flanger (float mix, float rate = 0.25f, float depth = 0.6f, float feedback = 0.5f)
    {
        return set ("chorus_on", 1).set ("chorus_mode", 1).set ("chorus_mix", mix).set ("chorus_rate", rate)
              .set ("chorus_depth", depth).set ("chorus_fb", feedback);
    }

    P& phaser (float mix, float rate = 0.4f, float depth = 0.6f, float feedback = 0.4f)
    {
        return set ("phaser_on", 1).set ("phaser_mix", mix).set ("phaser_rate", rate)
              .set ("phaser_depth", depth).set ("phaser_fb", feedback);
    }

    P& dist (int type, float amount, float mix = 1.0f)
    {
        return set ("dist_on", 1).set ("dist_type", (float) type).set ("dist_amount", amount).set ("dist_mix", mix);
    }

    P& eq (float lowDb, float highDb)
    {
        return set ("eq_on", 1).set ("eq_lowGain", lowDb).set ("eq_highGain", highDb);
    }

    P& comp (float thresholdDb, float ratio = 4.0f, float makeupDb = 0.0f)
    {
        return set ("comp_on", 1).set ("comp_thresh", thresholdDb).set ("comp_ratio", ratio).set ("comp_makeup", makeupDb);
    }

    P& mono (bool legato, float porta)
    {
        return set ("voiceMode", (float) (legato ? modeLegato : modeMono)).set ("porta", porta);
    }

    P& master (float db) { return set ("master", db); }
    P& velSens (float s) { return set ("velSens", s); }
    P& poly (int voices) { return set ("poly", (float) voices); }

    P& arp (int mode, int octaves, int division, float gate = 0.5f, int velMode = arpVelKey, float swing = 0.0f)
    {
        return set ("arp_on", 1).set ("arp_mode", (float) mode).set ("arp_octaves", (float) octaves)
              .set ("arp_div", (float) division).set ("arp_gate", gate)
              .set ("arp_velMode", (float) velMode).set ("arp_swing", swing);
    }

    /** Step pattern: transpose per step, optional velocities, steps with velocity 0 become rests. */
    P& steps (std::initializer_list<int> transposes, std::initializer_list<float> velocities = {})
    {
        int s = 0;
        for (int t : transposes)
            set (pid::arpStep (s++, "trans"), (float) t);
        set ("arp_steps", (float) juce::jmax (1, s));

        s = 0;
        for (float vel : velocities)
        {
            if (vel <= 0.0f)
                set (pid::arpStep (s, "on"), 0.0f);
            else
                set (pid::arpStep (s, "vel"), vel);
            ++s;
        }
        return *this;
    }
};

// Division indices into syncDivisionNames().
constexpr int div2_1 = 1, div1_1 = 2, div1_2D = 3, div1_2 = 4, div1_2T = 5, div1_4D = 6, div1_4 = 7, div1_4T = 8,
              div1_8D = 9, div1_8 = 10, div1_8T = 11, div1_16D = 12, div1_16 = 13, div1_16T = 14, div1_32 = 15;

// Modulation amount helpers: pitch destinations span ±36 semitones, cutoff ±120 semitones.
constexpr float semis (float s) { return s / 36.0f; }
constexpr float cutOct (float o) { return o * 12.0f / 120.0f; }

/** Appends the large factory library (defined in PresetLibrary.cpp). */
void addPresetLibrary (std::vector<FactoryPreset>& list);
} // namespace lumora::presetbuilder
