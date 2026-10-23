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

// The factory library: original sounds in the style of classic VA synth banks,
// grouped by category. Values are real parameter units (Hz, seconds, choice
// indices); helpers are in PresetBuilder.h.

#include "PresetBuilder.h"

namespace lumora::presetbuilder
{
namespace
{
    using Add = std::function<void (const char*, const P&)>;

    //==========================================================================
    void leads (const Add& add)
    {
        add ("Anthem Saw Lead", P()
             .osc (0, waveSaw, 0.7f, 8, 0.4f, 1.0f)
             .osc (1, waveSaw, 0.45f, 8, 0.35f, 1.0f, 1)
             .osc (2, waveSaw, 0.35f, 4, 0.3f, 0.8f, -1)
             .filters (filterLowpass, 1, 9000.0f, 0.1f)
             .ampEnvs (0.005f, 0.8f, 0.85f, 0.4f)
             .lfo (0, lfoSine, 5.2f).mod (0, srcModWheel, dstPitchAll, semis (0.4f))
             .mono (true, 0.04f)
             .delay (0.2f, div1_8D, div1_4).reverb (0.25f, 0.8f).master (-8.0f));

        add ("Square Lead", P()
             .osc (0, waveSquare, 0.7f, 2, 0.08f, 0.4f)
             .osc (1, waveSquare, 0.35f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 1, 3500.0f, 0.2f)
             .ampEnvs (0.003f, 0.4f, 0.8f, 0.2f)
             .lfo (0, lfoSine, 5.5f).mod (0, srcModWheel, dstPitchAll, semis (0.5f))
             .mono (true, 0.06f).delay (0.15f, div1_8D, div1_8D));

        add ("Sync-ish Lead", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.5f)
             .osc (1, waveSaw, 0.45f, 2, 0.1f, 0.5f, 1, 7)
             .filters (filterLowpass, 1, 1800.0f, 0.45f, 0.3f)
             .modEnv (0, 0.0f, 0.35f, 0.3f, 0.3f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.002f, 0.3f, 0.9f, 0.25f)
             .mono (true, 0.05f).dist (distOverdrive, 0.25f, 0.5f).delay (0.18f));

        add ("Screamer", P()
             .osc (0, waveSaw, 0.7f, 4, 0.3f, 0.8f)
             .osc (1, wavePulse, 0.5f, 2, 0.2f, 0.6f, 1)
             .filters (filterBandpass, 0, 1500.0f, 0.55f, 0.6f)
             .modEnv (0, 0.0f, 0.6f, 0.5f, 0.3f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.002f, 0.4f, 0.9f, 0.25f)
             .mono (true, 0.08f).dist (distHardClip, 0.5f, 0.7f).reverb (0.15f).master (-9.0f));

        add ("Soft Sine Lead", P()
             .osc (0, waveSine, 0.8f)
             .osc (1, waveTriangle, 0.3f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 6000.0f)
             .ampEnvs (0.03f, 0.5f, 0.9f, 0.4f)
             .lfo (0, lfoSine, 5.0f).mod (0, srcLfo1, dstPitchAll, semis (0.15f)).mod (1, srcModWheel, dstPitchAll, semis (0.3f))
             .mono (true, 0.1f).delay (0.25f, div1_4D, div1_4).reverb (0.3f, 0.8f));

        add ("Portamento Lead", P()
             .osc (0, waveSaw, 0.7f, 3, 0.15f, 0.6f)
             .osc (1, waveSquare, 0.4f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 2600.0f, 0.3f)
             .ampEnvs (0.005f, 0.5f, 0.85f, 0.3f)
             .mono (true, 0.25f).delay (0.2f, div1_8D, div1_4).reverb (0.2f));

        add ("Hollow Pulse Lead", P()
             .osc (0, wavePulse, 0.7f, 2, 0.12f, 0.7f)
             .osc (1, waveTriangle, 0.4f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 4000.0f, 0.15f)
             .lfo (0, lfoTriangle, 0.35f).mod (0, srcLfo1, dstCutoffAB, cutOct (0.6f))
             .ampEnvs (0.01f, 0.5f, 0.8f, 0.3f)
             .mono (true, 0.05f).chorus (0.3f).delay (0.2f));

        add ("Big Room Lead", P()
             .osc (0, waveSaw, 0.65f, 8, 0.5f, 1.0f)
             .osc (1, waveSquare, 0.4f, 4, 0.3f, 1.0f, -1)
             .osc (2, waveSaw, 0.35f, 8, 0.45f, 1.0f, 1)
             .filters (filterLowpass, 1, 12000.0f)
             .ampEnvs (0.002f, 0.3f, 1.0f, 0.2f)
             .dist (distOverdrive, 0.2f, 0.4f).eq (2.0f, 2.0f).reverb (0.2f, 0.7f).master (-9.0f));

        add ("Wah Lead", P()
             .osc (0, waveSaw, 0.7f, 2, 0.1f, 0.5f)
             .filters (filterBandpass, 1, 900.0f, 0.6f)
             .lfo (0, lfoSine, 4.0f, 1.0f, true, div1_8).mod (0, srcLfo1, dstCutoffAB, cutOct (1.5f))
             .ampEnvs (0.005f, 0.3f, 0.9f, 0.2f)
             .mono (true, 0.04f).dist (distOverdrive, 0.3f, 0.5f).master (-3.0f));

        add ("Retro Chip Lead", P()
             .osc (0, wavePulse, 0.6f).retrig (0)
             .osc (1, waveSquare, 0.3f, 1, 0.0f, 0.0f, 1)
             .filters (filterBypass, 0, 20000.0f)
             .lfo (0, lfoSquare, 12.0f).mod (0, srcModWheel, dstPitchAll, semis (1.0f))
             .ampEnvs (0.0f, 0.2f, 0.7f, 0.05f)
             .mono (false, 0.0f).dist (distBitcrush, 0.45f, 0.6f).master (-10.0f));

        add ("Dreamy Lead", P()
             .osc (0, waveTriangle, 0.7f, 4, 0.2f, 1.0f)
             .osc (1, waveSine, 0.4f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 5000.0f)
             .ampEnvs (0.08f, 0.8f, 0.8f, 0.8f)
             .lfo (0, lfoSine, 5.0f).mod (0, srcModWheel, dstPitchAll, semis (0.4f))
             .mono (true, 0.12f).chorus (0.4f).delay (0.3f, div1_4D, div1_2).reverb (0.4f, 0.9f));

        add ("Distorted Lead", P()
             .osc (0, waveSaw, 0.7f, 3, 0.2f, 0.7f)
             .osc (1, waveSaw, 0.5f, 3, 0.2f, 0.7f, -1)
             .filters (filterLowpass, 1, 3200.0f, 0.35f, 0.7f)
             .ctl (0.0f, 0.0f, 0.5f)
             .ampEnvs (0.002f, 0.3f, 0.9f, 0.2f)
             .mono (true, 0.04f).dist (distOverdrive, 0.6f, 0.8f).master (-10.0f));
    }

    //==========================================================================
    void plucks (const Add& add)
    {
        add ("Crystal Pluck", P()
             .osc (0, waveSaw, 0.6f, 6, 0.25f, 1.0f, 1)
             .osc (1, waveSine, 0.5f, 1, 0.0f, 0.0f, 2)
             .filters (filterLowpass, 1, 500.0f, 0.2f)
             .modEnv (0, 0.0f, 0.2f, 0.0f, 0.15f).mod (0, srcModEnv1, dstCutoffAB, cutOct (5.0f))
             .ampEnvs (0.0f, 0.35f, 0.0f, 0.3f)
             .delay (0.3f, div1_8D, div1_4, 0.45f).reverb (0.3f, 0.85f));

        add ("Deep House Pluck", P()
             .osc (0, waveSquare, 0.6f, 2, 0.1f, 0.6f)
             .osc (1, waveSaw, 0.4f, 2, 0.1f, 0.6f, -1)
             .filters (filterLowpass, 1, 350.0f, 0.3f)
             .modEnv (0, 0.0f, 0.18f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.5f))
             .ampEnvs (0.0f, 0.3f, 0.0f, 0.2f)
             .chorus (0.25f).delay (0.2f, div1_8D, div1_8D).reverb (0.2f));

        add ("Marimba Pluck", P()
             .osc (0, waveSine, 0.8f).retrig (0)
             .osc (1, waveTriangle, 0.35f, 1, 0.0f, 0.0f, 2).retrig (1)
             .filters (filterLowpass, 0, 4000.0f)
             .ampEnvs (0.0f, 0.45f, 0.0f, 0.4f)
             .ampEnv (0, 0.0f, 0.6f, 0.0f, 0.5f)
             .reverb (0.25f, 0.6f));

        add ("Tight Saw Pluck", P()
             .osc (0, waveSaw, 0.7f, 4, 0.2f, 0.8f)
             .filters (filterLowpass, 1, 300.0f, 0.15f)
             .modEnv (0, 0.0f, 0.12f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (5.0f))
             .ampEnvs (0.0f, 0.2f, 0.0f, 0.15f)
             .delay (0.18f, div1_16, div1_8));

        add ("Harp Pluck", P()
             .osc (0, waveTriangle, 0.7f, 2, 0.08f, 0.7f)
             .osc (1, waveSaw, 0.25f, 2, 0.1f, 0.8f, 1)
             .filters (filterLowpass, 0, 1500.0f)
             .modEnv (0, 0.0f, 0.5f, 0.0f, 0.4f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.0f, 1.2f, 0.0f, 1.0f)
             .reverb (0.35f, 0.85f));

        add ("Rubber Pluck", P()
             .osc (0, waveSquare, 0.7f)
             .filters (filterLowpass, 1, 250.0f, 0.55f)
             .modEnv (0, 0.0f, 0.15f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .modEnv (1, 0.0f, 0.06f, 0.0f, 0.05f).mod (1, srcModEnv2, dstPitchAll, semis (3.0f))
             .ampEnvs (0.0f, 0.25f, 0.0f, 0.2f)
             .delay (0.15f, div1_8, div1_8D));

        add ("Glass Pluck", P()
             .osc (0, waveSine, 0.6f, 1, 0.0f, 0.0f, 1)
             .osc (1, waveSine, 0.35f, 1, 0.0f, 0.0f, 2, 7)
             .osc (2, waveTriangle, 0.25f, 2, 0.1f, 1.0f, 3)
             .filters (filterLowpass, 0, 10000.0f)
             .ampEnvs (0.0f, 0.5f, 0.0f, 0.5f)
             .delay (0.3f, div1_8D, div1_4D, 0.5f).reverb (0.35f, 0.9f));

        add ("Supersaw Pluck", P()
             .osc (0, waveSaw, 0.65f, 8, 0.35f, 1.0f)
             .osc (1, waveSaw, 0.4f, 8, 0.3f, 1.0f, 1)
             .filters (filterLowpass, 1, 600.0f, 0.15f)
             .modEnv (0, 0.0f, 0.3f, 0.05f, 0.2f).mod (0, srcModEnv1, dstCutoffAB, cutOct (4.0f))
             .ampEnvs (0.0f, 0.5f, 0.0f, 0.35f)
             .delay (0.25f, div1_8D, div1_4).reverb (0.3f, 0.8f).master (-8.0f));

        add ("Kalimba Pluck", P()
             .osc (0, waveSine, 0.7f).retrig (0)
             .osc (1, waveSine, 0.25f, 1, 0.0f, 0.0f, 2, 4).retrig (1)
             .filters (filterLowpass, 0, 6000.0f)
             .modEnv (1, 0.0f, 0.02f, 0.0f, 0.02f).mod (0, srcModEnv2, dstPitchAll, semis (1.0f))
             .ampEnvs (0.0f, 0.7f, 0.0f, 0.6f)
             .ampEnv (1, 0.0f, 0.15f, 0.0f, 0.15f)
             .reverb (0.3f, 0.7f));

        add ("Noise Pluck", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.6f)
             .osc (2, waveNoise, 0.3f, 1, 0.0f, 1.0f)
             .filters (filterLowpass, 1, 400.0f, 0.2f)
             .modEnv (0, 0.0f, 0.12f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (5.0f))
             .ampEnvs (0.0f, 0.25f, 0.0f, 0.2f)
             .ampEnv (1, 0.0f, 0.08f, 0.0f, 0.08f)
             .delay (0.2f, div1_8D, div1_8).reverb (0.2f));
    }

    //==========================================================================
    void basses (const Add& add)
    {
        add ("Reese Bass", P()
             .osc (0, waveSaw, 0.7f, 2, 0.25f, 0.3f, -1)
             .osc (1, waveSaw, 0.6f, 2, 0.3f, 0.3f, -1, 0, 12.0f)
             .filters (filterLowpass, 1, 700.0f, 0.15f, 0.3f)
             .lfo (0, lfoSine, 0.2f).mod (0, srcLfo1, dstCutoffAB, cutOct (0.4f))
             .ampEnvs (0.005f, 0.3f, 1.0f, 0.15f)
             .mono (true, 0.03f).dist (distOverdrive, 0.25f, 0.5f).master (-4.0f));

        add ("Analog Mono Bass", P()
             .osc (0, waveSaw, 0.7f, 1, 0.0f, 0.0f, -1).retrig (0)
             .osc (1, waveSquare, 0.5f, 1, 0.0f, 0.0f, -2).retrig (1)
             .filters (filterLowpass, 1, 300.0f, 0.35f)
             .modEnv (0, 0.0f, 0.25f, 0.15f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .mod (1, srcVelocity, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (0.001f, 0.3f, 0.8f, 0.08f)
             .mono (false, 0.0f).master (0.0f));

        add ("Sub Sine Bass", P()
             .osc (0, waveSine, 0.9f, 1, 0.0f, 0.0f, -2).retrig (0)
             .filters (filterLowpass, 0, 600.0f)
             .ampEnvs (0.003f, 0.2f, 1.0f, 0.1f)
             .mono (false, 0.0f).master (0.0f));

        add ("FM-ish Bass", P()
             .osc (0, waveSine, 0.7f, 1, 0.0f, 0.0f, -1).retrig (0)
             .osc (1, waveSquare, 0.4f, 1, 0.0f, 0.0f, 0, 7)
             .filters (filterLowpass, 1, 450.0f, 0.4f, 0.4f)
             .modEnv (0, 0.0f, 0.12f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (4.0f))
             .ampEnvs (0.001f, 0.25f, 0.6f, 0.1f)
             .mono (false, 0.0f).master (-1.0f));

        add ("Acid Squelch", P()
             .osc (0, waveSquare, 0.8f, 1, 0.0f, 0.0f, -1).retrig (0)
             .filters (filterLowpass, 1, 220.0f, 0.85f, 0.5f)
             .modEnv (0, 0.0f, 0.18f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.5f))
             .mod (1, srcVelocity, dstCutoffAB, cutOct (1.5f))
             .ampEnvs (0.001f, 0.2f, 0.7f, 0.05f)
             .mono (true, 0.07f).dist (distOverdrive, 0.35f, 0.5f).delay (0.12f, div1_8D, div1_8D).master (0.0f));

        add ("Growl Bass", P()
             .osc (0, waveSaw, 0.7f, 2, 0.15f, 0.4f, -1)
             .osc (1, wavePulse, 0.5f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 400.0f, 0.6f, 0.5f)
             .lfoSync (0, lfoTriangle, div1_16).mod (0, srcLfo1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.002f, 0.3f, 1.0f, 0.1f)
             .mono (false, 0.02f).dist (distFoldback, 0.35f, 0.6f).master (-3.0f));

        add ("Wobble Slow", P()
             .osc (0, waveSaw, 0.7f, 2, 0.12f, 0.4f, -1)
             .osc (1, waveSquare, 0.5f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 160.0f, 0.55f, 0.4f)
             .lfoSync (0, lfoSine, div1_4).mod (0, srcLfo1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.002f, 0.3f, 1.0f, 0.1f)
             .mono (false, 0.02f).dist (distOverdrive, 0.35f, 0.6f).master (-2.0f));

        add ("Wobble Triplet", P()
             .osc (0, waveSaw, 0.7f, 2, 0.12f, 0.4f, -1)
             .osc (1, waveSaw, 0.5f, 2, 0.12f, 0.4f, -2)
             .filters (filterLowpass, 1, 180.0f, 0.5f, 0.4f)
             .lfoSync (0, lfoSine, div1_8T).mod (0, srcLfo1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.002f, 0.3f, 1.0f, 0.1f)
             .mono (false, 0.02f).dist (distHardClip, 0.3f, 0.5f).master (-3.0f));

        add ("Rolling Bass", P()
             .osc (0, waveSaw, 0.7f, 1, 0.0f, 0.0f, -1).retrig (0)
             .osc (1, waveSine, 0.6f, 1, 0.0f, 0.0f, -2).retrig (1)
             .filters (filterLowpass, 1, 350.0f, 0.25f)
             .modEnv (0, 0.0f, 0.12f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.5f))
             .ampEnvs (0.001f, 0.18f, 0.0f, 0.1f)
             .mono (false, 0.0f).master (0.0f));

        add ("Offbeat Pluck Bass", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.3f, -1)
             .osc (1, waveSquare, 0.5f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 500.0f, 0.3f)
             .modEnv (0, 0.0f, 0.15f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.001f, 0.22f, 0.0f, 0.12f)
             .mono (false, 0.0f).master (-1.0f));

        add ("Funk Bass", P()
             .osc (0, waveSquare, 0.7f, 1, 0.0f, 0.0f, -1).retrig (0)
             .osc (1, waveSaw, 0.4f, 1, 0.0f, 0.0f, -1).retrig (1)
             .filters (filterLowpass, 0, 400.0f, 0.5f)
             .modEnv (0, 0.0f, 0.15f, 0.2f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.5f))
             .mod (1, srcVelocity, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (0.001f, 0.3f, 0.5f, 0.08f)
             .mono (true, 0.03f).master (-1.0f));

        add ("Distorted 808", P()
             .osc (0, waveSine, 0.9f, 1, 0.0f, 0.0f, -2).retrig (0, 0.25f)
             .filters (filterLowpass, 0, 2000.0f)
             .modEnv (1, 0.0f, 0.06f, 0.0f, 0.05f).mod (0, srcModEnv2, dstPitchAll, semis (12.0f))
             .ampEnvs (0.001f, 1.2f, 0.0f, 0.6f)
             .mono (false, 0.05f).dist (distOverdrive, 0.45f, 0.7f).master (-3.0f));

        add ("Hoover Bass", P()
             .osc (0, waveSaw, 0.6f, 8, 0.55f, 1.0f, -1)
             .osc (1, wavePulse, 0.45f, 4, 0.4f, 1.0f, -2)
             .filters (filterLowpass, 0, 2500.0f, 0.2f)
             .modEnv (1, 0.0f, 0.2f, 0.0f, 0.2f).mod (0, srcModEnv2, dstPitchAll, semis (-2.0f))
             .ampEnvs (0.005f, 0.4f, 1.0f, 0.2f)
             .mono (true, 0.1f).phaser (0.3f).master (-5.0f));

        add ("Plucky Sub", P()
             .osc (0, waveTriangle, 0.8f, 1, 0.0f, 0.0f, -2).retrig (0)
             .osc (1, waveSaw, 0.3f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 300.0f)
             .modEnv (0, 0.0f, 0.1f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.001f, 0.4f, 0.5f, 0.1f)
             .mono (false, 0.0f).master (0.0f));
    }

    //==========================================================================
    void pads (const Add& add)
    {
        add ("Supersaw Pad", P()
             .osc (0, waveSaw, 0.6f, 8, 0.35f, 1.0f)
             .osc (1, waveSaw, 0.45f, 8, 0.3f, 1.0f, 1)
             .osc (2, waveSaw, 0.35f, 8, 0.3f, 1.0f, -1)
             .filters (filterLowpass, 0, 3000.0f, 0.1f)
             .ampEnvs (0.6f, 1.0f, 0.85f, 1.8f)
             .reverb (0.35f, 0.88f).master (-9.0f));

        add ("Analog Strings Pad", P()
             .osc (0, waveSaw, 0.6f, 4, 0.2f, 1.0f)
             .osc (1, wavePulse, 0.35f, 4, 0.2f, 1.0f, 1)
             .filters (filterLowpass, 1, 2200.0f, 0.1f)
             .lfo (0, lfoTriangle, 0.3f, 1.0f, false, 7, true).mod (0, srcLfo1, dstCutoffAB, cutOct (0.3f))
             .ampEnvs (0.8f, 1.0f, 0.9f, 1.5f)
             .chorus (0.45f, 0.4f, 0.5f).reverb (0.35f, 0.85f));

        add ("Glass Pad", P()
             .osc (0, waveTriangle, 0.6f, 4, 0.15f, 1.0f, 1)
             .osc (1, waveSine, 0.4f, 1, 0.0f, 0.0f, 2)
             .osc (2, waveSaw, 0.25f, 4, 0.2f, 1.0f)
             .filters (filterLowpass, 0, 6000.0f)
             .ampEnvs (1.2f, 1.5f, 0.8f, 2.5f)
             .delay (0.2f, div1_4D, div1_2).reverb (0.45f, 0.92f, 0.3f).master (-13.0f));

        add ("Dark Drone", P()
             .osc (0, waveSaw, 0.6f, 4, 0.3f, 1.0f, -1)
             .osc (1, waveSquare, 0.4f, 2, 0.2f, 1.0f, -2)
             .filters (filterLowpass, 1, 500.0f, 0.35f)
             .lfo (0, lfoSmoothRandom, 0.15f, 1.0f, false, 7, true).mod (0, srcLfo1, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (2.0f, 2.0f, 1.0f, 3.0f)
             .phaser (0.3f, 0.1f).reverb (0.5f, 0.95f, 0.4f).master (-6.0f));

        add ("Sweeping Pad", P()
             .osc (0, waveSaw, 0.6f, 6, 0.3f, 1.0f)
             .osc (1, waveSaw, 0.4f, 6, 0.25f, 1.0f, 1)
             .filters (filterLowpass, 1, 600.0f, 0.4f)
             .lfo (0, lfoSine, 0.08f, 1.0f, false, 7, true).mod (0, srcLfo1, dstCutoffAB, cutOct (2.5f))
             .ampEnvs (1.0f, 1.0f, 1.0f, 2.0f)
             .reverb (0.4f, 0.9f).master (-10.0f));

        add ("Warm Analog Pad", P()
             .osc (0, waveSaw, 0.55f, 2, 0.15f, 0.8f)
             .osc (1, waveSaw, 0.55f, 2, 0.15f, 0.8f, 0, 0, 7.0f)
             .filters (filterLowpass, 1, 1400.0f, 0.15f)
             .ctl (0.0f, 0.0f, 0.3f)
             .modEnv (0, 1.0f, 2.0f, 0.4f, 2.0f).mod (0, srcModEnv1, dstCutoffAB, cutOct (1.2f))
             .ampEnvs (0.7f, 1.0f, 0.85f, 1.6f)
             .chorus (0.35f).reverb (0.35f, 0.85f).master (-9.0f));

        add ("Choir Pad", P()
             .osc (0, wavePulse, 0.55f, 6, 0.2f, 1.0f)
             .osc (1, waveTriangle, 0.45f, 4, 0.15f, 1.0f, 1)
             .filters (filterBandpass, 0, 1200.0f, 0.3f)
             .lfo (0, lfoSine, 4.5f).mod (0, srcLfo1, dstPitchAll, semis (0.08f))
             .ampEnvs (0.6f, 1.0f, 0.9f, 1.5f)
             .chorus (0.4f).reverb (0.45f, 0.9f).master (-5.0f));

        add ("Shimmer Pad", P()
             .osc (0, waveSaw, 0.5f, 6, 0.25f, 1.0f)
             .osc (2, waveSine, 0.35f, 1, 0.0f, 0.0f, 2)
             .osc (3, waveTriangle, 0.3f, 2, 0.1f, 1.0f, 2, 7)
             .filters (filterLowpass, 0, 7000.0f)
             .ampEnvs (1.5f, 2.0f, 0.8f, 3.0f)
             .delay (0.3f, div1_4D, div1_2, 0.5f).reverb (0.5f, 0.95f, 0.2f).master (-13.0f));

        add ("Evolving Pad", P()
             .osc (0, waveSaw, 0.5f, 4, 0.25f, 1.0f)
             .osc (2, wavePulse, 0.45f, 4, 0.25f, 1.0f, 1)
             .filter (0, filterLowpass, 1, 1500.0f, 0.2f)
             .filter (1, filterBandpass, 0, 1000.0f, 0.4f)
             .lfo (0, lfoSine, 0.07f, 1.0f, false, 7, true).mod (0, srcLfo1, dstMixAB, 0.8f)
             .lfo (1, lfoTriangle, 0.13f, 1.0f, false, 7, true).mod (1, srcLfo2, dstCutoffB, cutOct (1.5f))
             .ampEnvs (1.5f, 1.0f, 1.0f, 2.5f)
             .reverb (0.45f, 0.9f));

        add ("Airy Pad", P()
             .osc (0, waveTriangle, 0.6f, 4, 0.2f, 1.0f)
             .osc (2, waveNoise, 0.15f, 1, 0.0f, 1.0f)
             .filter (0, filterLowpass, 0, 4000.0f)
             .filter (1, filterHighpass, 1, 5000.0f, 0.2f)
             .ampEnvs (1.0f, 1.0f, 0.9f, 2.0f)
             .chorus (0.3f).reverb (0.45f, 0.92f));

        add ("Retro Poly Pad", P()
             .osc (0, waveSquare, 0.55f, 2, 0.1f, 0.7f)
             .osc (1, waveSaw, 0.45f, 2, 0.12f, 0.7f, -1)
             .filters (filterLowpass, 1, 1800.0f, 0.2f)
             .lfo (0, lfoTriangle, 0.4f, 1.0f, false, 7, true).mod (0, srcLfo1, dstPitchAll, semis (0.06f))
             .ampEnvs (0.4f, 0.8f, 0.8f, 1.2f)
             .chorus (0.5f, 0.5f, 0.6f).reverb (0.3f));

        add ("Tremolo Pad", P()
             .osc (0, waveSaw, 0.55f, 4, 0.2f, 1.0f)
             .osc (1, waveTriangle, 0.4f, 2, 0.1f, 1.0f, 1)
             .filters (filterLowpass, 0, 3000.0f)
             .lfoSync (0, lfoSine, div1_8).mod (0, srcLfo1, dstVolumeAll, 0.4f)
             .ampEnvs (0.5f, 1.0f, 0.9f, 1.5f)
             .reverb (0.35f, 0.85f).master (-10.0f));
    }

    //==========================================================================
    void keys (const Add& add)
    {
        add ("E-Piano", P()
             .osc (0, waveSine, 0.7f).retrig (0)
             .osc (1, waveTriangle, 0.3f, 1, 0.0f, 0.0f, 1).retrig (1)
             .osc (2, waveSine, 0.15f, 1, 0.0f, 0.0f, 3).retrig (2)
             .filters (filterLowpass, 0, 5000.0f)
             .ampEnvs (0.002f, 1.5f, 0.2f, 0.4f)
             .ampEnv (1, 0.001f, 0.3f, 0.0f, 0.2f)
             .velSens (0.8f).chorus (0.3f, 0.8f, 0.3f).reverb (0.2f));

        add ("Wurly Keys", P()
             .osc (0, waveTriangle, 0.7f).retrig (0)
             .osc (1, wavePulse, 0.2f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 2500.0f, 0.15f)
             .mod (0, srcVelocity, dstCutoffAB, cutOct (1.5f))
             .ampEnvs (0.002f, 1.2f, 0.3f, 0.3f)
             .lfo (0, lfoSine, 5.0f).mod (1, srcLfo1, dstVolumeAll, 0.15f)
             .velSens (0.8f).dist (distOverdrive, 0.15f, 0.4f).reverb (0.2f));

        add ("Organ Drawbars", P()
             .osc (0, waveSine, 0.5f)
             .osc (1, waveSine, 0.4f, 1, 0.0f, 0.0f, 1)
             .osc (2, waveSine, 0.3f, 1, 0.0f, 0.0f, 1, 7)
             .osc (3, waveSine, 0.25f, 1, 0.0f, 0.0f, 2)
             .filters (filterBypass, 0, 20000.0f)
             .ampEnvs (0.005f, 0.1f, 1.0f, 0.05f)
             .velSens (0.0f).chorus (0.35f, 5.5f, 0.2f).dist (distOverdrive, 0.15f, 0.3f).reverb (0.2f).master (-8.0f));

        add ("Clav Funk", P()
             .osc (0, wavePulse, 0.7f).retrig (0)
             .osc (1, waveSaw, 0.3f, 1, 0.0f, 0.0f, 1).retrig (1)
             .filters (filterHighpass, 0, 250.0f, 0.2f)
             .ampEnvs (0.001f, 0.5f, 0.0f, 0.08f)
             .velSens (0.9f).phaser (0.25f, 0.7f).master (-3.0f));

        add ("Digital Piano", P()
             .osc (0, waveTriangle, 0.6f).retrig (0)
             .osc (1, waveSaw, 0.25f, 2, 0.05f, 0.5f)
             .filters (filterLowpass, 1, 1200.0f)
             .modEnv (0, 0.0f, 0.8f, 0.1f, 0.4f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .mod (1, srcVelocity, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (0.001f, 2.0f, 0.1f, 0.5f)
             .velSens (0.85f).reverb (0.25f, 0.7f));

        add ("Lofi Keys", P()
             .osc (0, waveTriangle, 0.7f, 2, 0.08f, 0.6f)
             .osc (1, waveSine, 0.3f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 2000.0f)
             .lfo (0, lfoSmoothRandom, 0.8f, 1.0f, false, 7, true).mod (0, srcLfo1, dstPitchAll, semis (0.1f))
             .ampEnvs (0.005f, 1.0f, 0.4f, 0.4f)
             .dist (distBitcrush, 0.35f, 0.4f).chorus (0.3f).reverb (0.25f));

        add ("Vintage Poly Keys", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.6f)
             .osc (1, waveSquare, 0.4f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 1, 900.0f, 0.15f)
             .modEnv (0, 0.0f, 0.6f, 0.3f, 0.4f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.5f))
             .ampEnvs (0.002f, 0.8f, 0.6f, 0.4f)
             .chorus (0.4f).reverb (0.2f));
    }

    //==========================================================================
    void bells (const Add& add)
    {
        add ("Music Box", P()
             .osc (0, waveSine, 0.6f, 1, 0.0f, 0.0f, 1).retrig (0)
             .osc (1, waveTriangle, 0.3f, 1, 0.0f, 0.0f, 3).retrig (1)
             .filters (filterLowpass, 0, 9000.0f)
             .ampEnvs (0.0f, 1.2f, 0.0f, 1.0f)
             .ampEnv (1, 0.0f, 0.4f, 0.0f, 0.4f)
             .reverb (0.35f, 0.8f));

        add ("Tubular Bell", P()
             .osc (0, waveSine, 0.6f).retrig (0)
             .osc (1, waveSine, 0.35f, 1, 0.0f, 0.0f, 1, 3, 20.0f)
             .osc (2, waveSine, 0.3f, 1, 0.0f, 0.0f, 2, 2, -14.0f)
             .filters (filterLowpass, 0, 8000.0f)
             .ampEnvs (0.0f, 3.5f, 0.0f, 3.0f)
             .reverb (0.4f, 0.9f).master (-11.0f));

        add ("Metallic Bell", P()
             .osc (0, waveSquare, 0.4f).retrig (0)
             .osc (1, waveSine, 0.4f, 1, 0.0f, 0.0f, 2, 1)
             .osc (2, waveTriangle, 0.3f, 1, 0.0f, 0.0f, 1, 6)
             .filters (filterBandpass, 0, 3000.0f, 0.3f)
             .ampEnvs (0.0f, 2.0f, 0.0f, 2.0f)
             .delay (0.2f, div1_4D, div1_4).reverb (0.35f, 0.85f));

        add ("Vibraphone", P()
             .osc (0, waveSine, 0.7f).retrig (0)
             .osc (1, waveSine, 0.2f, 1, 0.0f, 0.0f, 2).retrig (1)
             .filters (filterLowpass, 0, 6000.0f)
             .lfo (0, lfoSine, 5.5f).mod (0, srcLfo1, dstVolumeAll, 0.25f)
             .ampEnvs (0.0f, 2.5f, 0.0f, 1.5f)
             .velSens (0.8f).reverb (0.3f, 0.75f));

        add ("Ice Chimes", P()
             .osc (0, waveSine, 0.5f, 1, 0.0f, 0.0f, 2)
             .osc (1, waveSine, 0.35f, 1, 0.0f, 0.0f, 2, 7)
             .osc (2, waveTriangle, 0.3f, 2, 0.2f, 1.0f, 3)
             .filters (filterHighpass, 0, 800.0f)
             .ampEnvs (0.0f, 1.5f, 0.0f, 1.5f)
             .delay (0.35f, div1_8D, div1_4D, 0.5f).reverb (0.5f, 0.95f, 0.2f).master (-4.0f));
    }

    //==========================================================================
    void brass (const Add& add)
    {
        add ("Synth Brass", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.6f)
             .osc (1, waveSaw, 0.55f, 2, 0.12f, 0.6f, 0, 0, 6.0f)
             .filters (filterLowpass, 1, 700.0f, 0.1f)
             .modEnv (0, 0.06f, 0.4f, 0.5f, 0.3f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.5f))
             .mod (1, srcVelocity, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (0.04f, 0.3f, 0.9f, 0.25f)
             .reverb (0.2f));

        add ("Brass Stab", P()
             .osc (0, waveSaw, 0.6f, 4, 0.15f, 0.8f)
             .osc (1, waveSaw, 0.45f, 4, 0.15f, 0.8f, -1)
             .filters (filterLowpass, 1, 600.0f, 0.15f)
             .modEnv (0, 0.0f, 0.25f, 0.0f, 0.2f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.005f, 0.35f, 0.0f, 0.25f)
             .reverb (0.25f).master (-7.0f));

        add ("Horn Section", P()
             .osc (0, waveSaw, 0.55f, 3, 0.12f, 0.8f)
             .osc (1, wavePulse, 0.4f, 2, 0.1f, 0.8f)
             .filters (filterLowpass, 1, 900.0f, 0.1f)
             .modEnv (0, 0.1f, 0.6f, 0.6f, 0.4f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.0f))
             .lfo (0, lfoSine, 5.0f).mod (1, srcModWheel, dstPitchAll, semis (0.3f))
             .ampEnvs (0.08f, 0.4f, 0.9f, 0.3f)
             .chorus (0.25f).reverb (0.25f, 0.75f));

        add ("Fat Poly Brass", P()
             .osc (0, waveSaw, 0.55f, 6, 0.25f, 1.0f)
             .osc (1, waveSaw, 0.4f, 6, 0.25f, 1.0f, 1)
             .filters (filterLowpass, 1, 1100.0f, 0.15f)
             .ctl (0.0f, 0.0f, 0.3f)
             .modEnv (0, 0.03f, 0.5f, 0.5f, 0.3f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.03f, 0.4f, 0.85f, 0.3f)
             .reverb (0.25f).master (-8.0f));
    }

    //==========================================================================
    void strings (const Add& add)
    {
        add ("String Ensemble", P()
             .osc (0, waveSaw, 0.55f, 6, 0.25f, 1.0f)
             .osc (1, waveSaw, 0.4f, 6, 0.22f, 1.0f, 1)
             .filters (filterLowpass, 0, 3500.0f)
             .ampEnvs (0.4f, 1.0f, 0.9f, 1.0f)
             .chorus (0.5f, 0.5f, 0.6f).reverb (0.35f, 0.85f).master (-7.0f));

        add ("Pizzicato Strings", P()
             .osc (0, waveSaw, 0.6f, 4, 0.15f, 1.0f)
             .filters (filterLowpass, 1, 700.0f)
             .modEnv (0, 0.0f, 0.15f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.0f, 0.3f, 0.0f, 0.25f)
             .reverb (0.35f, 0.8f));

        add ("Cello Solo", P()
             .osc (0, waveSaw, 0.7f, 2, 0.06f, 0.4f, -1)
             .filters (filterLowpass, 1, 1400.0f, 0.15f)
             .lfo (0, lfoSine, 5.0f).mod (0, srcLfo1, dstPitchAll, semis (0.12f))
             .modEnv (0, 0.15f, 0.5f, 0.6f, 0.5f).mod (1, srcModEnv1, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (0.15f, 0.5f, 0.9f, 0.5f)
             .mono (true, 0.08f).reverb (0.3f, 0.8f));

        add ("Slow Strings", P()
             .osc (0, waveSaw, 0.55f, 8, 0.28f, 1.0f)
             .osc (2, wavePulse, 0.35f, 4, 0.2f, 1.0f, -1)
             .filters (filterLowpass, 1, 2500.0f, 0.1f)
             .ampEnvs (1.5f, 1.5f, 0.9f, 2.5f)
             .chorus (0.4f).reverb (0.45f, 0.9f).master (-8.0f));

        add ("Octave Strings", P()
             .osc (0, waveSaw, 0.5f, 6, 0.25f, 1.0f)
             .osc (1, waveSaw, 0.35f, 6, 0.25f, 1.0f, 1)
             .osc (2, waveSaw, 0.35f, 6, 0.25f, 1.0f, -1)
             .filters (filterLowpass, 0, 4000.0f)
             .ampEnvs (0.3f, 1.0f, 0.9f, 1.2f)
             .lfo (0, lfoSine, 4.8f).mod (0, srcModWheel, dstPitchAll, semis (0.25f))
             .chorus (0.4f).reverb (0.35f, 0.85f).master (-9.0f));
    }

    //==========================================================================
    void chords (const Add& add)
    {
        add ("Major Stab", P()
             .osc (0, waveSaw, 0.5f, 4, 0.2f, 0.9f)
             .osc (1, waveSaw, 0.45f, 4, 0.2f, 0.9f, 0, 4)
             .osc (2, waveSaw, 0.45f, 4, 0.2f, 0.9f, 0, 7)
             .filters (filterLowpass, 1, 900.0f, 0.2f)
             .modEnv (0, 0.0f, 0.25f, 0.0f, 0.2f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.0f, 0.4f, 0.0f, 0.3f)
             .delay (0.25f, div1_8D, div1_4).reverb (0.3f).master (-8.0f));

        add ("Minor 7 Stab", P()
             .osc (0, waveSaw, 0.45f, 4, 0.2f, 0.9f)
             .osc (1, waveSaw, 0.4f, 4, 0.2f, 0.9f, 0, 3)
             .osc (2, waveSaw, 0.4f, 4, 0.2f, 0.9f, 0, 7)
             .osc (3, waveSaw, 0.35f, 4, 0.2f, 0.9f, 0, 10)
             .filters (filterLowpass, 1, 800.0f, 0.2f)
             .modEnv (0, 0.0f, 0.3f, 0.0f, 0.2f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.0f, 0.45f, 0.0f, 0.3f)
             .chorus (0.25f).delay (0.25f, div1_8D, div1_4).reverb (0.3f).master (-9.0f));

        add ("Rave Organ Chord", P()
             .osc (0, waveSquare, 0.5f)
             .osc (1, waveSquare, 0.4f, 1, 0.0f, 0.0f, 0, 5)
             .osc (2, waveSquare, 0.4f, 1, 0.0f, 0.0f, 0, 9)
             .osc (3, waveSine, 0.4f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 0, 3000.0f, 0.1f)
             .ampEnvs (0.002f, 0.3f, 0.6f, 0.1f)
             .reverb (0.2f, 0.6f).master (-9.0f));

        add ("Dub Chord", P()
             .osc (0, waveSaw, 0.45f, 2, 0.1f, 0.7f)
             .osc (1, waveSaw, 0.4f, 2, 0.1f, 0.7f, 0, 3)
             .osc (2, waveSaw, 0.4f, 2, 0.1f, 0.7f, 0, 7)
             .filters (filterBandpass, 1, 800.0f, 0.3f)
             .ampEnvs (0.0f, 0.25f, 0.0f, 0.2f)
             .delay (0.4f, div1_4D, div1_4D, 0.6f).reverb (0.3f, 0.85f).master (-3.0f));

        add ("Sus4 Pad Chord", P()
             .osc (0, waveSaw, 0.4f, 4, 0.25f, 1.0f)
             .osc (1, waveSaw, 0.35f, 4, 0.25f, 1.0f, 0, 5)
             .osc (2, waveSaw, 0.35f, 4, 0.25f, 1.0f, 0, 7)
             .filters (filterLowpass, 0, 2500.0f)
             .ampEnvs (0.5f, 1.0f, 0.8f, 1.5f)
             .chorus (0.3f).reverb (0.4f, 0.9f).master (-9.0f));

        add ("Fifth Power Chord", P()
             .osc (0, waveSaw, 0.55f, 3, 0.15f, 0.7f)
             .osc (1, waveSaw, 0.5f, 3, 0.15f, 0.7f, 0, 7)
             .osc (2, waveSquare, 0.4f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 2500.0f, 0.2f, 0.3f)
             .ampEnvs (0.002f, 0.4f, 0.8f, 0.2f)
             .dist (distOverdrive, 0.45f, 0.7f).master (-10.0f));
    }

    //==========================================================================
    void arps (const Add& add)
    {
        add ("Trance Gate Arp", P()
             .osc (0, waveSaw, 0.6f, 6, 0.3f, 1.0f)
             .osc (1, waveSaw, 0.4f, 6, 0.25f, 1.0f, 1)
             .filters (filterLowpass, 1, 1500.0f, 0.2f)
             .ampEnvs (0.002f, 0.2f, 0.6f, 0.08f)
             .arp (arpUp, 2, div1_16, 0.5f)
             .delay (0.25f, div1_8D, div1_4).reverb (0.3f).master (-8.0f));

        add ("Bouncing Arp", P()
             .osc (0, waveSquare, 0.6f, 2, 0.1f, 0.7f)
             .filters (filterLowpass, 1, 700.0f, 0.35f)
             .modEnv (0, 0.0f, 0.15f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.0f, 0.25f, 0.0f, 0.15f)
             .arp (arpUpDown, 3, div1_16, 0.6f, arpVelBoth)
             .steps ({ 0, 0, 12, 0, 0, 0, 12, 0 }, { 1.0f, 0.6f, 0.9f, 0.6f, 1.0f, 0.6f, 0.9f, 0.7f })
             .delay (0.2f, div1_8D, div1_8));

        add ("Random Sparkle Arp", P()
             .osc (0, waveSine, 0.6f, 1, 0.0f, 0.0f, 1)
             .osc (1, waveTriangle, 0.3f, 1, 0.0f, 0.0f, 2)
             .filters (filterLowpass, 0, 8000.0f)
             .ampEnvs (0.0f, 0.3f, 0.0f, 0.3f)
             .arp (arpRandom, 3, div1_16, 0.4f)
             .delay (0.35f, div1_8D, div1_4D, 0.5f).reverb (0.4f, 0.9f));

        add ("Triplet Arp", P()
             .osc (0, waveSaw, 0.6f, 4, 0.2f, 0.8f)
             .filters (filterLowpass, 1, 900.0f, 0.25f)
             .modEnv (0, 0.0f, 0.18f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .ampEnvs (0.0f, 0.25f, 0.0f, 0.15f)
             .arp (arpUp, 2, div1_8T, 0.5f)
             .delay (0.25f, div1_8D, div1_4).reverb (0.25f));

        add ("Octave Jumper", P()
             .osc (0, waveSaw, 0.6f, 2, 0.1f, 0.6f)
             .osc (1, waveSquare, 0.35f, 1, 0.0f, 0.0f, -1)
             .filters (filterLowpass, 1, 1200.0f, 0.3f)
             .ampEnvs (0.0f, 0.2f, 0.3f, 0.1f)
             .arp (arpOrdered, 1, div1_16, 0.45f, arpVelStep)
             .steps ({ 0, 12, 0, 12, 0, 12, 24, 12 }, { 1.0f, 0.7f, 0.9f, 0.7f, 1.0f, 0.7f, 0.9f, 0.7f }));

        add ("Chord Gate", P()
             .osc (0, waveSaw, 0.5f, 4, 0.25f, 1.0f)
             .osc (1, waveSaw, 0.4f, 4, 0.25f, 1.0f, 1)
             .filters (filterLowpass, 1, 2000.0f, 0.15f)
             .ampEnvs (0.002f, 0.15f, 0.7f, 0.05f)
             .arp (arpChord, 1, div1_16, 0.5f, arpVelStep)
             .steps ({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                     { 1.0f, 0.0f, 0.7f, 0.0f, 1.0f, 0.6f, 0.0f, 0.8f, 1.0f, 0.0f, 0.7f, 0.0f, 1.0f, 0.6f, 0.8f, 0.0f })
             .reverb (0.3f).master (-9.0f));

        add ("Swing Arp", P()
             .osc (0, waveTriangle, 0.6f, 2, 0.1f, 0.7f)
             .osc (1, wavePulse, 0.3f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 3000.0f)
             .ampEnvs (0.0f, 0.3f, 0.0f, 0.2f)
             .arp (arpUpDown2, 2, div1_16, 0.5f, arpVelKey, 0.55f)
             .delay (0.2f, div1_8D, div1_8).reverb (0.25f));

        add ("Acid Arp", P()
             .osc (0, waveSaw, 0.8f, 1, 0.0f, 0.0f, -1).retrig (0)
             .filters (filterLowpass, 1, 300.0f, 0.8f, 0.5f)
             .modEnv (0, 0.0f, 0.15f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (3.0f))
             .mod (1, srcVelocity, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.001f, 0.2f, 0.5f, 0.05f)
             .arp (arpUp, 2, div1_16, 0.6f, arpVelStep)
             .steps ({ 0, 0, 12, 0, 3, 0, 12, 7 }, { 1.0f, 0.5f, 0.9f, 0.5f, 1.0f, 0.5f, 0.9f, 0.6f })
             .dist (distOverdrive, 0.3f, 0.5f).delay (0.15f, div1_8D, div1_8D).master (0.0f));
    }

    //==========================================================================
    void sequences (const Add& add)
    {
        add ("Berlin Sequence", P()
             .osc (0, waveSaw, 0.7f, 1, 0.0f, 0.0f, -1).retrig (0)
             .osc (1, waveSquare, 0.4f, 1, 0.0f, 0.0f, -2)
             .filters (filterLowpass, 1, 450.0f, 0.4f)
             .modEnv (0, 0.0f, 0.12f, 0.0f, 0.1f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.5f))
             .lfo (0, lfoSine, 0.05f, 1.0f, false, 7, true).mod (1, srcLfo1, dstCutoffAB, cutOct (1.0f))
             .ampEnvs (0.001f, 0.2f, 0.0f, 0.1f)
             .arp (arpOrdered, 1, div1_16, 0.5f)
             .steps ({ 0, 12, 7, 12, 0, 12, 5, 12 })
             .delay (0.2f, div1_8D, div1_8D).master (0.0f));

        add ("Pulse Sequence", P()
             .osc (0, wavePulse, 0.7f, 1, 0.0f, 0.0f, -1).retrig (0)
             .filters (filterLowpass, 1, 800.0f, 0.3f)
             .ampEnvs (0.001f, 0.15f, 0.0f, 0.08f)
             .arp (arpUp, 1, div1_16, 0.4f, arpVelStep)
             .steps ({ 0, 0, 0, 12, 0, 0, 10, 0 }, { 1.0f, 0.6f, 0.8f, 0.9f, 1.0f, 0.0f, 0.9f, 0.6f })
             .delay (0.25f, div1_8D, div1_4).master (-2.0f));

        add ("Minimal Blip", P()
             .osc (0, waveSine, 0.8f).retrig (0)
             .filters (filterLowpass, 0, 5000.0f)
             .modEnv (1, 0.0f, 0.03f, 0.0f, 0.03f).mod (0, srcModEnv2, dstPitchAll, semis (12.0f))
             .ampEnvs (0.0f, 0.12f, 0.0f, 0.1f)
             .arp (arpUp, 2, div1_16, 0.3f, arpVelStep)
             .steps ({ 0, 0, 7, 0, 12, 0, 7, 0 }, { 1.0f, 0.0f, 0.7f, 0.0f, 0.9f, 0.5f, 0.0f, 0.6f })
             .delay (0.3f, div1_8D, div1_4D, 0.45f).reverb (0.2f));

        add ("Gated Bass Line", P()
             .osc (0, waveSaw, 0.7f, 2, 0.1f, 0.3f, -1)
             .osc (1, waveSquare, 0.5f, 1, 0.0f, 0.0f, -2)
             .filters (filterLowpass, 1, 400.0f, 0.3f)
             .modEnv (0, 0.0f, 0.1f, 0.0f, 0.08f).mod (0, srcModEnv1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.001f, 0.15f, 0.3f, 0.05f)
             .arp (arpUp, 1, div1_16, 0.5f, arpVelStep)
             .steps ({ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 1.0f, 0.7f, 0.8f })
             .master (0.0f));

        add ("Arcade Melody", P()
             .osc (0, waveSquare, 0.6f).retrig (0)
             .osc (1, wavePulse, 0.3f, 1, 0.0f, 0.0f, 1)
             .filters (filterBypass, 0, 20000.0f)
             .ampEnvs (0.0f, 0.15f, 0.4f, 0.05f)
             .arp (arpOrdered, 1, div1_16, 0.6f)
             .steps ({ 0, 4, 7, 12, 7, 4, 0, -5, 0, 4, 7, 12, 16, 12, 7, 4 })
             .dist (distBitcrush, 0.4f, 0.5f).master (-10.0f));
    }

    //==========================================================================
    void effects (const Add& add)
    {
        add ("Riser", P()
             .osc (0, waveSaw, 0.5f, 8, 0.5f, 1.0f)
             .osc (2, waveNoise, 0.4f, 1, 0.0f, 1.0f)
             .filters (filterHighpass, 1, 200.0f, 0.3f)
             .modEnv (0, 6.0f, 1.0f, 1.0f, 1.0f)
             .mod (0, srcModEnv1, dstCutoffAB, cutOct (5.0f))
             .mod (1, srcModEnv1, dstPitchAll, semis (12.0f))
             .ampEnvs (3.0f, 1.0f, 1.0f, 1.5f)
             .reverb (0.45f, 0.92f).master (-8.0f));

        add ("Downlifter", P()
             .osc (0, waveSaw, 0.5f, 4, 0.4f, 1.0f, 1)
             .osc (2, waveNoise, 0.4f, 1, 0.0f, 1.0f)
             .filters (filterLowpass, 1, 12000.0f, 0.2f)
             .modEnv (0, 0.0f, 3.0f, 0.0f, 2.0f)
             .mod (0, srcModEnv1, dstCutoffAB, cutOct (-6.0f))
             .mod (1, srcModEnv1, dstPitchAll, semis (-24.0f))
             .ctl (60.0f)
             .ampEnvs (0.01f, 3.5f, 0.0f, 2.0f)
             .reverb (0.45f, 0.92f).master (-8.0f));

        add ("White Noise Wash", P()
             .osc (0, waveSaw, 0.0f)
             .osc (2, waveNoise, 0.6f, 1, 0.0f, 1.0f)
             .filter (1, filterBandpass, 0, 2000.0f, 0.5f)
             .lfo (0, lfoSine, 0.1f, 1.0f, false, 7, true).mod (0, srcLfo1, dstCutoffB, cutOct (3.0f))
             .ampEnvs (1.5f, 1.0f, 1.0f, 3.0f)
             .phaser (0.4f, 0.15f).reverb (0.5f, 0.95f).master (-5.0f));

        add ("Laser Zap", P()
             .osc (0, waveSaw, 0.7f, 1, 0.0f, 0.0f, 1).retrig (0)
             .filters (filterLowpass, 0, 6000.0f, 0.4f)
             .modEnv (1, 0.0f, 0.25f, 0.0f, 0.2f).mod (0, srcModEnv2, dstPitchAll, semis (30.0f))
             .ampEnvs (0.0f, 0.3f, 0.0f, 0.2f)
             .delay (0.3f, div1_8D, div1_8).master (-5.0f));

        add ("Sci-Fi Bleeps", P()
             .osc (0, waveSine, 0.7f, 1, 0.0f, 0.0f, 1)
             .filters (filterLowpass, 0, 9000.0f)
             .lfoSync (0, lfoSampleHold, div1_16).mod (0, srcLfo1, dstPitchAll, semis (12.0f))
             .ampEnvs (0.01f, 0.5f, 0.8f, 0.5f)
             .delay (0.35f, div1_8D, div1_4D, 0.5f).reverb (0.3f, 0.85f).master (-10.0f));

        add ("Alarm Siren", P()
             .osc (0, waveSquare, 0.5f).osc (1, waveSaw, 0.4f, 1, 0.0f, 0.0f, 0, 0, 10.0f)
             .filters (filterLowpass, 0, 4000.0f, 0.2f)
             .lfo (0, lfoTriangle, 1.2f).mod (0, srcLfo1, dstPitchAll, semis (5.0f))
             .ampEnvs (0.01f, 0.3f, 1.0f, 0.3f)
             .reverb (0.25f).master (-9.0f));

        add ("Bitcrushed Texture", P()
             .osc (0, waveSaw, 0.5f, 4, 0.4f, 1.0f)
             .osc (2, waveNoise, 0.2f, 1, 0.0f, 1.0f)
             .filters (filterLowpass, 0, 3000.0f, 0.3f)
             .lfo (0, lfoSmoothRandom, 0.5f, 1.0f, false, 7, true).mod (0, srcLfo1, dstCutoffAB, cutOct (2.0f))
             .ampEnvs (0.5f, 1.0f, 0.9f, 1.5f)
             .dist (distDecimate, 0.5f, 0.7f).reverb (0.35f, 0.9f).master (-6.0f));

        add ("Wind", P()
             .osc (0, waveSaw, 0.0f)
             .osc (2, waveNoise, 0.7f, 1, 0.0f, 1.0f)
             .filter (1, filterBandpass, 1, 700.0f, 0.6f)
             .lfo (0, lfoSmoothRandom, 0.3f, 1.0f, false, 7, true).mod (0, srcLfo1, dstCutoffB, cutOct (2.0f))
             .lfo (1, lfoSmoothRandom, 0.2f, 1.0f, false, 7, true).mod (1, srcLfo2, dstVolumeB, 0.4f)
             .ampEnvs (1.5f, 1.0f, 1.0f, 2.5f)
             .reverb (0.4f, 0.9f).master (-1.0f));
    }

    //==========================================================================
    void drums (const Add& add)
    {
        add ("Synth Kick", P()
             .osc (0, waveSine, 0.95f, 1, 0.0f, 0.0f, -2).retrig (0, 0.25f)
             .filters (filterLowpass, 0, 3000.0f)
             .modEnv (1, 0.0f, 0.05f, 0.0f, 0.05f).mod (0, srcModEnv2, dstPitchAll, semis (24.0f))
             .ampEnvs (0.0f, 0.35f, 0.0f, 0.2f)
             .velSens (0.3f).dist (distOverdrive, 0.2f, 0.4f).master (-4.0f));

        add ("Synth Snare", P()
             .osc (0, waveTriangle, 0.6f).retrig (0)
             .osc (2, waveNoise, 0.7f, 1, 0.0f, 0.6f)
             .filter (0, filterLowpass, 0, 3000.0f)
             .filter (1, filterHighpass, 0, 1500.0f, 0.2f)
             .modEnv (1, 0.0f, 0.04f, 0.0f, 0.03f).mod (0, srcModEnv2, dstPitchA, semis (12.0f))
             .ampEnv (0, 0.0f, 0.12f, 0.0f, 0.1f)
             .ampEnv (1, 0.0f, 0.2f, 0.0f, 0.15f)
             .velSens (0.4f).reverb (0.2f, 0.5f).master (-10.0f));

        add ("Closed Hi-Hat", P()
             .osc (0, waveSaw, 0.0f)
             .osc (2, waveNoise, 0.8f, 1, 0.0f, 0.4f)
             .filter (1, filterHighpass, 1, 8000.0f, 0.2f)
             .ampEnvs (0.0f, 0.05f, 0.0f, 0.04f)
             .velSens (0.5f).master (-2.0f));

        add ("Open Hi-Hat", P()
             .osc (0, waveSaw, 0.0f)
             .osc (2, waveNoise, 0.8f, 1, 0.0f, 0.5f)
             .filter (1, filterHighpass, 1, 7000.0f, 0.25f)
             .ampEnvs (0.0f, 0.35f, 0.0f, 0.3f)
             .velSens (0.5f).master (-3.0f));

        add ("Synth Tom", P()
             .osc (0, waveSine, 0.9f, 1, 0.0f, 0.0f, -1).retrig (0, 0.25f)
             .filters (filterLowpass, 0, 2000.0f)
             .modEnv (1, 0.0f, 0.15f, 0.0f, 0.1f).mod (0, srcModEnv2, dstPitchAll, semis (7.0f))
             .ampEnvs (0.0f, 0.4f, 0.0f, 0.3f)
             .velSens (0.4f).reverb (0.2f, 0.5f).master (-5.0f));

        add ("Clap", P()
             .osc (0, waveSaw, 0.0f)
             .osc (2, waveNoise, 0.8f, 1, 0.0f, 0.7f)
             .filter (1, filterBandpass, 0, 1200.0f, 0.4f)
             .lfo (0, lfoSquare, 40.0f).mod (0, srcLfo1, dstVolumeB, 0.6f)
             .ampEnvs (0.0f, 0.18f, 0.0f, 0.15f)
             .velSens (0.4f).reverb (0.3f, 0.5f).master (-1.0f));
    }
} // namespace

void addPresetLibrary (std::vector<FactoryPreset>& list)
{
    const std::pair<const char*, void (*) (const Add&)> categories[] {
        { "Lead", leads },     { "Pluck", plucks },  { "Bass", basses },
        { "Pad", pads },       { "Keys", keys },     { "Bell", bells },
        { "Brass", brass },    { "Strings", strings }, { "Chord", chords },
        { "Arp", arps },       { "Sequence", sequences }, { "FX", effects },
        { "Drums", drums },
    };

    for (const auto& [category, fill] : categories)
    {
        const juce::String cat (category);
        fill ([&list, &cat] (const char* name, const P& p) { list.push_back ({ name, cat, p.v }); });
    }
}
} // namespace lumora::presetbuilder
