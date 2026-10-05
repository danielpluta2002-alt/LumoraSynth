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

#include "Presets.h"
#include "Parameters.h"

namespace lumora
{
namespace
{
    using Values = std::vector<std::pair<juce::String, float>>;

    /** Small builder so the preset table below stays readable. */
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

        P& mono (bool legato, float porta)
        {
            return set ("voiceMode", (float) (legato ? modeLegato : modeMono)).set ("porta", porta);
        }
    };

    // Division indices into syncDivisionNames():
    constexpr int div1_4 = 7, div1_8D = 9, div1_8 = 10, div1_8T = 11, div1_16 = 13, div1_4D = 6, div1_2 = 4;

    std::vector<FactoryPreset> buildPresets()
    {
        std::vector<FactoryPreset> list;
        auto add = [&list] (const char* name, const char* category, const P& p) { list.push_back ({ name, category, p.v }); };

        add ("Init", "Init", P());

        add ("Supersaw Lead", "Lead", P()
             .osc (0, waveSaw, 0.75f, 8, 0.35f, 1.0f)
             .osc (1, waveSaw, 0.45f, 8, 0.30f, 1.0f, 1)
             .filters (filterLowpass, 1, 7000.0f, 0.15f)
             .ampEnvs (0.004f, 0.6f, 0.8f, 0.35f)
             .lfo (0, lfoSine, 5.5f, 1.0f)
             .mod (0, srcModWheel, dstPitchAll, 0.012f)
             .set ("porta", 0.03f)
             .delay (0.22f, div1_8D, div1_4)
             .reverb (0.22f, 0.75f));

        add ("Trance Pluck", "Pluck", P()
             .osc (0, waveSaw, 0.75f, 6, 0.25f, 0.9f)
             .osc (1, waveSquare, 0.35f, 4, 0.2f, 0.8f, 1)
             .filters (filterLowpass, 1, 300.0f, 0.25f)
             .modEnv (0, 0.0f, 0.28f, 0.0f, 0.25f)
             .mod (0, srcModEnv1, dstCutoffAB, 0.42f)
             .mod (1, srcVelocity, dstCutoffAB, 0.08f)
             .ampEnvs (0.0f, 0.45f, 0.0f, 0.35f)
             .delay (0.28f, div1_8D, div1_4D, 0.4f)
             .reverb (0.25f, 0.8f));

        add ("Deep Bass", "Bass", P()
             .osc (0, waveSaw, 0.7f, 1, 0.0f, 0.0f, -1)
             .osc (1, waveSquare, 0.5f, 1, 0.0f, 0.0f, -2)
             .filters (filterLowpass, 1, 260.0f, 0.3f, 0.3f)
             .modEnv (0, 0.0f, 0.3f, 0.1f, 0.1f)
             .mod (0, srcModEnv1, dstCutoffAB, 0.3f)
             .ampEnvs (0.001f, 0.3f, 0.8f, 0.08f)
             .mono (false, 0.03f)
             .set ("master", 0.0f));

        add ("Warm Pad", "Pad", P()
             .osc (0, waveSaw, 0.6f, 4, 0.22f, 1.0f)
             .osc (1, waveTriangle, 0.4f, 2, 0.1f, 0.6f, 1)
             .osc (2, waveSaw, 0.45f, 4, 0.18f, 1.0f, -1)
             .filters (filterLowpass, 0, 2200.0f, 0.1f)
             .ampEnvs (1.2f, 1.0f, 0.8f, 2.2f)
             .modEnv (0, 1.5f, 2.0f, 0.4f, 2.0f)
             .mod (0, srcModEnv1, dstCutoffAB, 0.15f)
             .lfo (0, lfoSine, 0.25f)
             .mod (1, srcLfo1, dstCutoffAB, 0.05f)
             .chorus (0.4f)
             .reverb (0.4f, 0.88f, 0.4f));

        P arp;
        arp.osc (0, waveSaw, 0.7f, 4, 0.2f, 0.8f)
           .osc (1, wavePulse, 0.35f, 2, 0.15f, 0.6f, 1)
           .filters (filterLowpass, 1, 900.0f, 0.3f)
           .modEnv (0, 0.0f, 0.18f, 0.0f, 0.15f)
           .mod (0, srcModEnv1, dstCutoffAB, 0.3f)
           .lfo (0, lfoTriangle, 0.15f, 1.0f, false, 7, true)
           .mod (1, srcLfo1, dstCutoffAB, 0.12f)
           .ampEnvs (0.0f, 0.25f, 0.0f, 0.2f)
           .set ("arp_on", 1).set ("arp_mode", arpUpDown).set ("arp_octaves", 2)
           .set ("arp_div", div1_16).set ("arp_gate", 0.6f).set ("arp_velMode", arpVelStep)
           .delay (0.25f, div1_8D, div1_8)
           .reverb (0.2f);
        const int transposes[] { 0, 0, 12, 0, 7, 0, 12, 0, 0, 0, 12, 0, 5, 0, 12, 7 };
        const float vels[] { 1.0f, 0.6f, 0.8f, 0.6f, 0.9f, 0.6f, 0.8f, 0.5f, 1.0f, 0.6f, 0.8f, 0.6f, 0.9f, 0.6f, 0.8f, 0.7f };
        for (int s = 0; s < numArpSteps; ++s)
            arp.set (pid::arpStep (s, "trans"), (float) transposes[s]).set (pid::arpStep (s, "vel"), vels[s]);
        add ("Arp Sequence", "Arp", arp);

        add ("Hoover", "Lead", P()
             .osc (0, waveSaw, 0.7f, 8, 0.6f, 1.0f)
             .osc (1, waveSaw, 0.5f, 8, 0.5f, 1.0f, -1)
             .osc (2, wavePulse, 0.35f, 4, 0.4f, 1.0f, -1, 0, 7.0f)
             .filters (filterLowpass, 0, 4500.0f, 0.15f)
             .modEnv (1, 0.0f, 0.25f, 0.0f, 0.3f)
             .mod (0, srcModEnv2, dstPitchAll, -0.06f)
             .lfo (0, lfoSine, 5.0f)
             .mod (1, srcModWheel, dstPitchAll, 0.01f)
             .ampEnvs (0.01f, 0.5f, 0.9f, 0.3f)
             .mono (true, 0.12f)
             .set ("master", -2.0f)
             .set ("phaser_on", 1).set ("phaser_mix", 0.35f)
             .reverb (0.18f));

        add ("Sub Bass", "Bass", P()
             .osc (0, waveSine, 0.9f, 1, 0.0f, 0.0f, -1)
             .osc (1, waveTriangle, 0.25f, 1, 0.0f, 0.0f, 0)
             .filters (filterLowpass, 0, 900.0f)
             .ampEnvs (0.002f, 0.2f, 1.0f, 0.08f)
             .mono (false, 0.0f)
             .set ("master", 0.0f));

        add ("Brass Section", "Brass", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.6f)
             .osc (1, waveSaw, 0.55f, 2, 0.12f, 0.7f, 0, 0, 7.0f)
             .filters (filterLowpass, 1, 650.0f, 0.1f)
             .modEnv (0, 0.08f, 0.5f, 0.4f, 0.3f)
             .mod (0, srcModEnv1, dstCutoffAB, 0.28f)
             .mod (1, srcVelocity, dstCutoffAB, 0.1f)
             .ampEnvs (0.05f, 0.3f, 0.9f, 0.25f)
             .reverb (0.2f));

        add ("Glass Bell", "Keys", P()
             .osc (0, waveSine, 0.6f)
             .osc (1, waveSine, 0.3f, 1, 0.0f, 0.0f, 2, 7)
             .osc (2, waveTriangle, 0.25f, 2, 0.1f, 1.0f, 1)
             .filters (filterLowpass, 0, 12000.0f)
             .ampEnvs (0.0f, 2.5f, 0.0f, 2.5f)
             .ampEnv (1, 0.0f, 0.6f, 0.0f, 0.6f)
             .delay (0.2f, div1_4D, div1_4)
             .reverb (0.35f, 0.85f, 0.3f));

        add ("Wobble Bass", "Bass", P()
             .osc (0, waveSaw, 0.7f, 2, 0.1f, 0.4f, -1)
             .osc (1, waveSquare, 0.5f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 180.0f, 0.5f, 0.4f)
             .lfo (0, lfoSine, 2.0f, 1.0f, true, div1_8, false)
             .mod (0, srcLfo1, dstCutoffAB, 0.32f)
             .ampEnvs (0.001f, 0.3f, 1.0f, 0.1f)
             .mono (false, 0.02f)
             .set ("master", 0.0f)
             .set ("dist_on", 1).set ("dist_amount", 0.35f).set ("dist_mix", 0.6f));

        add ("Minor Chord Stab", "Chord", P()
             .osc (0, waveSaw, 0.55f, 4, 0.2f, 0.9f)
             .osc (1, waveSaw, 0.5f, 4, 0.2f, 0.9f, 0, 7)
             .osc (2, waveSaw, 0.5f, 4, 0.2f, 0.9f, 0, 3)
             .filters (filterLowpass, 1, 700.0f, 0.2f)
             .modEnv (0, 0.0f, 0.22f, 0.0f, 0.2f)
             .mod (0, srcModEnv1, dstCutoffAB, 0.35f)
             .ampEnvs (0.0f, 0.35f, 0.0f, 0.3f)
             .delay (0.25f, div1_8D, div1_4)
             .reverb (0.3f, 0.8f));

        add ("Lush Strings", "Pad", P()
             .osc (0, waveSaw, 0.6f, 6, 0.28f, 1.0f)
             .osc (1, waveSaw, 0.4f, 6, 0.24f, 1.0f, 1)
             .filters (filterLowpass, 0, 3500.0f, 0.05f)
             .ampEnvs (0.6f, 1.0f, 0.85f, 1.4f)
             .lfo (0, lfoTriangle, 4.8f, 1.0f)
             .mod (0, srcLfo1, dstPitchAll, 0.003f)
             .chorus (0.45f, 0.35f, 0.5f)
             .reverb (0.38f, 0.85f));

        add ("Acid Line", "Bass", P()
             .osc (0, waveSaw, 0.8f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 250.0f, 0.8f, 0.45f)
             .modEnv (0, 0.0f, 0.2f, 0.0f, 0.1f)
             .mod (0, srcModEnv1, dstCutoffAB, 0.35f)
             .mod (1, srcVelocity, dstCutoffAB, 0.15f)
             .ampEnvs (0.001f, 0.25f, 0.6f, 0.06f)
             .mono (true, 0.06f)
             .set ("master", 0.0f)
             .set ("dist_on", 1).set ("dist_amount", 0.3f).set ("dist_mix", 0.5f)
             .delay (0.15f, div1_8D, div1_8D));

        add ("Noise Sweep", "FX", P()
             .osc (0, waveSaw, 0.0f)
             .osc (2, waveNoise, 0.7f, 1, 0.0f, 1.0f)
             .filter (1, filterBandpass, 0, 400.0f, 0.75f)
             .ampEnvs (2.0f, 2.0f, 0.8f, 3.0f)
             .lfo (0, lfoSine, 0.08f, 1.0f, false, 7, true)
             .mod (0, srcLfo1, dstCutoffB, 0.45f)
             .reverb (0.5f, 0.92f, 0.3f));

        add ("Ambient Keys", "Keys", P()
             .osc (0, waveTriangle, 0.65f, 2, 0.12f, 0.8f)
             .osc (1, waveSine, 0.4f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 5000.0f)
             .ampEnvs (0.005f, 1.6f, 0.3f, 1.2f)
             .lfo (0, lfoSine, 4.0f, 1.0f)
             .mod (0, srcLfo1, dstVolumeAll, 0.18f)
             .chorus (0.3f)
             .delay (0.2f, div1_4D, div1_2)
             .reverb (0.4f, 0.85f));

        add ("Detuned Organ", "Keys", P()
             .osc (0, waveSine, 0.55f)
             .osc (1, waveSine, 0.35f, 1, 0.0f, 0.0f, 1)
             .osc (2, waveSquare, 0.15f, 2, 0.15f, 0.8f, 2)
             .osc (3, waveSine, 0.25f, 1, 0.0f, 0.0f, 1, 7)
             .filters (filterLowpass, 0, 6000.0f)
             .ampEnvs (0.004f, 0.1f, 1.0f, 0.08f)
             .set ("phaser_on", 1).set ("phaser_rate", 0.8f).set ("phaser_mix", 0.3f)
             .reverb (0.25f, 0.6f));

        return list;
    }
} // namespace

const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = buildPresets();
    return presets;
}

void applyFactoryPreset (juce::AudioProcessorValueTreeState& state, const FactoryPreset& preset)
{
    for (auto* p : state.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, value] : preset.values)
    {
        if (auto* rp = state.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));
        else
            jassertfalse; // unknown parameter id in the preset table
    }
}
} // namespace lumora
