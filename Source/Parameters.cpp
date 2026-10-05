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

#include "Parameters.h"

namespace lumora
{
const juce::StringArray& oscWaveNames()
{
    static const juce::StringArray names { "Sine", "Triangle", "Saw", "Square", "Pulse", "Noise" };
    return names;
}

const juce::StringArray& filterTypeNames()
{
    static const juce::StringArray names { "Bypass", "Lowpass", "Bandpass", "Highpass", "Notch" };
    return names;
}

const juce::StringArray& lfoWaveNames()
{
    static const juce::StringArray names { "Sine", "Triangle", "Saw Up", "Saw Down", "Square", "S&H", "Smooth Rnd" };
    return names;
}

const juce::StringArray& syncDivisionNames()
{
    static const juce::StringArray names { "4/1", "2/1", "1/1", "1/2D", "1/2", "1/2T", "1/4D", "1/4", "1/4T",
                                           "1/8D", "1/8", "1/8T", "1/16D", "1/16", "1/16T", "1/32" };
    return names;
}

double syncDivisionBeats (int index)
{
    static const double beats[] { 16.0, 8.0, 4.0, 3.0, 2.0, 4.0 / 3.0, 1.5, 1.0, 2.0 / 3.0,
                                  0.75, 0.5, 1.0 / 3.0, 0.375, 0.25, 1.0 / 6.0, 0.125 };
    return beats[juce::jlimit (0, (int) std::size (beats) - 1, index)];
}

const juce::StringArray& modSourceNames()
{
    static const juce::StringArray names { "Off", "Mod Env 1", "Mod Env 2", "LFO 1", "LFO 2", "Velocity",
                                           "Mod Wheel", "Aftertouch", "Key Track", "Random" };
    return names;
}

const juce::StringArray& modDestNames()
{
    static const juce::StringArray names { "Off", "Pitch All", "Pitch A", "Pitch B", "Cutoff A+B", "Cutoff A",
                                           "Cutoff B", "Reso A+B", "Drive", "Volume All", "Volume A", "Volume B",
                                           "Mix A/B", "Pan", "Detune", "Stereo", "LFO 1 Rate", "LFO 2 Rate",
                                           "LFO 1 Gain", "LFO 2 Gain" };
    return names;
}

const juce::StringArray& voiceModeNames()
{
    static const juce::StringArray names { "Poly", "Mono", "Legato" };
    return names;
}

const juce::StringArray& arpModeNames()
{
    static const juce::StringArray names { "Up", "Down", "Up/Down", "Down/Up", "Up/Down 2", "Random", "Ordered", "Chord" };
    return names;
}

const juce::StringArray& arpVelModeNames()
{
    static const juce::StringArray names { "Key", "Step", "Key x Step" };
    return names;
}

const juce::StringArray& distTypeNames()
{
    static const juce::StringArray names { "Overdrive", "Hard Clip", "Foldback", "Bitcrush", "Decimate" };
    return names;
}

namespace pid
{
    juce::String oscLabel (int index)
    {
        static const char* labels[] { "A1", "A2", "B1", "B2" };
        return labels[index];
    }

    juce::String osc (int index, const char* name)     { return "osc" + oscLabel (index) + "_" + name; }
    juce::String filt (int index, const char* name)    { return juce::String ("filt") + (index == 0 ? "A" : "B") + "_" + name; }
    juce::String amp (int index, const char* name)     { return juce::String ("amp") + (index == 0 ? "A" : "B") + "_" + name; }
    juce::String menv (int index, const char* name)    { return "menv" + juce::String (index + 1) + "_" + name; }
    juce::String lfo (int index, const char* name)     { return "lfo" + juce::String (index + 1) + "_" + name; }
    juce::String mod (int index, const char* name)     { return "mod" + juce::String (index + 1) + "_" + name; }
    juce::String arpStep (int index, const char* name) { return "arp_s" + juce::String (index + 1) + "_" + name; }
}

//==============================================================================
namespace
{
    using Layout = juce::AudioProcessorValueTreeState::ParameterLayout;
    using Group = juce::AudioProcessorParameterGroup;
    using StringFn = std::function<juce::String (float, int)>;
    using ValueFn = std::function<float (const juce::String&)>;

    juce::String formatTime (float seconds)
    {
        if (seconds < 1.0f)
            return juce::String (seconds * 1000.0f, seconds < 0.01f ? 1 : 0) + " ms";
        return juce::String (seconds, 2) + " s";
    }

    juce::String formatHz (float hz)
    {
        if (hz < 1000.0f)
            return juce::String (hz, hz < 10.0f ? 2 : (hz < 100.0f ? 1 : 0)) + " Hz";
        return juce::String (hz / 1000.0f, 2) + " kHz";
    }

    const StringFn percentText = [] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; };
    const ValueFn percentValue = [] (const juce::String& t) { return t.getFloatValue() / 100.0f; };
    const StringFn timeText = [] (float v, int) { return formatTime (v); };
    const StringFn hzText = [] (float v, int) { return formatHz (v); };
    const StringFn dbText = [] (float v, int) { return v <= -47.9f ? juce::String ("-inf dB") : juce::String (v, 1) + " dB"; };
    const StringFn semiText = [] (float v, int) { return (v > 0 ? "+" : "") + juce::String (v, 1) + " st"; };
    const StringFn centText = [] (float v, int) { return (v > 0 ? "+" : "") + juce::String (juce::roundToInt (v)) + " ct"; };
    const StringFn bipolarText = [] (float v, int) { return (v > 0 ? "+" : "") + juce::String (juce::roundToInt (v * 100.0f)) + " %"; };
    const StringFn panText = [] (float v, int)
    {
        const int p = juce::roundToInt (v * 100.0f);
        if (p == 0) return juce::String ("C");
        return (p < 0 ? "L " : "R ") + juce::String (std::abs (p));
    };
    const StringFn degreeText = [] (float v, int) { return juce::String (juce::roundToInt (v * 360.0f)) + juce::String::fromUTF8 ("\xc2\xb0"); };

    juce::NormalisableRange<float> skewed (float min, float max, float centre, float interval = 0.0f)
    {
        juce::NormalisableRange<float> r (min, max, interval);
        r.setSkewForCentre (centre);
        return r;
    }

    std::unique_ptr<juce::AudioParameterFloat> makeFloat (const juce::String& id, const juce::String& name,
                                                          juce::NormalisableRange<float> range, float def,
                                                          StringFn toText, ValueFn fromText = nullptr)
    {
        auto attrs = juce::AudioParameterFloatAttributes().withStringFromValueFunction (std::move (toText));
        if (fromText != nullptr)
            attrs = attrs.withValueFromStringFunction (std::move (fromText));
        return std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name, range, def, attrs);
    }

    std::unique_ptr<juce::AudioParameterFloat> makePercent (const juce::String& id, const juce::String& name, float def)
    {
        return makeFloat (id, name, { 0.0f, 1.0f }, def, percentText, percentValue);
    }

    std::unique_ptr<juce::AudioParameterFloat> makeBipolar (const juce::String& id, const juce::String& name, float def)
    {
        return makeFloat (id, name, { -1.0f, 1.0f }, def, bipolarText, percentValue);
    }

    std::unique_ptr<juce::AudioParameterFloat> makeTime (const juce::String& id, const juce::String& name, float max, float def)
    {
        return makeFloat (id, name, skewed (0.0f, max, max * 0.08f), def, timeText);
    }

    std::unique_ptr<juce::AudioParameterChoice> makeChoice (const juce::String& id, const juce::String& name,
                                                            const juce::StringArray& choices, int def)
    {
        return std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 }, name, choices, def);
    }

    std::unique_ptr<juce::AudioParameterInt> makeInt (const juce::String& id, const juce::String& name, int min, int max, int def)
    {
        return std::make_unique<juce::AudioParameterInt> (juce::ParameterID { id, 1 }, name, min, max, def);
    }

    std::unique_ptr<juce::AudioParameterBool> makeBool (const juce::String& id, const juce::String& name, bool def)
    {
        return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def);
    }

    void addOscillators (Layout& layout)
    {
        for (int i = 0; i < numOscs; ++i)
        {
            const auto label = "Osc " + pid::oscLabel (i);
            auto group = std::make_unique<Group> ("osc" + pid::oscLabel (i), label, "|");

            group->addChild (makeChoice (pid::osc (i, "wave"), label + " Wave", oscWaveNames(), waveSaw));
            group->addChild (makeInt (pid::osc (i, "voices"), label + " Voices", 1, maxUnison, 1));
            group->addChild (makePercent (pid::osc (i, "detune"), label + " Detune", 0.2f));
            group->addChild (makePercent (pid::osc (i, "stereo"), label + " Stereo", 0.5f));
            group->addChild (makeFloat (pid::osc (i, "phase"), label + " Phase", { 0.0f, 1.0f }, 0.0f, degreeText));
            group->addChild (makeBool (pid::osc (i, "retrig"), label + " Retrigger", false));
            group->addChild (makeBool (pid::osc (i, "invert"), label + " Invert", false));
            group->addChild (makeInt (pid::osc (i, "octave"), label + " Octave", -3, 3, 0));
            group->addChild (makeInt (pid::osc (i, "note"), label + " Note", -12, 12, 0));
            group->addChild (makeFloat (pid::osc (i, "fine"), label + " Fine", { -100.0f, 100.0f }, 0.0f, centText));
            group->addChild (makePercent (pid::osc (i, "volume"), label + " Volume", i == 0 ? 0.8f : 0.0f));
            group->addChild (makeFloat (pid::osc (i, "pan"), label + " Pan", { -1.0f, 1.0f }, 0.0f, panText));

            layout.add (std::move (group));
        }
    }

    void addFilters (Layout& layout)
    {
        for (int i = 0; i < numFilters; ++i)
        {
            const juce::String label = i == 0 ? "Filter A" : "Filter B";
            auto group = std::make_unique<Group> (i == 0 ? "filtA" : "filtB", label, "|");

            group->addChild (makeChoice (pid::filt (i, "type"), label + " Type", filterTypeNames(), filterLowpass));
            group->addChild (makeChoice (pid::filt (i, "slope"), label + " Slope", { "12 dB", "24 dB" }, 1));
            group->addChild (makeChoice (pid::filt (i, "input"), label + " Input", { i == 0 ? "A" : "B", "A + B" }, 0));
            group->addChild (makeFloat (pid::filt (i, "cutoff"), label + " Cutoff", skewed (20.0f, 20000.0f, 1000.0f), 8000.0f, hzText));
            group->addChild (makePercent (pid::filt (i, "reso"), label + " Resonance", 0.0f));
            group->addChild (makePercent (pid::filt (i, "drive"), label + " Drive", 0.0f));

            layout.add (std::move (group));
        }

        auto ctl = std::make_unique<Group> ("filtCtl", "Filter Control", "|");
        ctl->addChild (makeFloat ("fctl_cutoff", "Filter Ctl Cutoff", { -60.0f, 60.0f }, 0.0f, semiText));
        ctl->addChild (makePercent ("fctl_reso", "Filter Ctl Resonance", 0.0f));
        ctl->addChild (makePercent ("fctl_warm", "Filter Ctl Warm Drive", 0.0f));
        ctl->addChild (makePercent ("fctl_keytrack", "Filter Ctl Key Track", 0.0f));
        layout.add (std::move (ctl));
    }

    void addEnvelope (Group& group, const juce::String& label, const std::function<juce::String (const char*)>& id,
                      float a, float d, float s, float r)
    {
        group.addChild (makeTime (id ("attack"), label + " Attack", 10.0f, a));
        group.addChild (makeTime (id ("decay"), label + " Decay", 10.0f, d));
        group.addChild (makePercent (id ("sustain"), label + " Sustain", s));
        group.addChild (makeTime (id ("release"), label + " Release", 10.0f, r));
    }

    void addEnvelopes (Layout& layout)
    {
        for (int i = 0; i < 2; ++i)
        {
            const juce::String label = i == 0 ? "Amp Env A" : "Amp Env B";
            auto group = std::make_unique<Group> (i == 0 ? "ampA" : "ampB", label, "|");
            addEnvelope (*group, label, [i] (const char* n) { return pid::amp (i, n); }, 0.002f, 0.3f, 1.0f, 0.15f);
            layout.add (std::move (group));
        }

        for (int i = 0; i < 2; ++i)
        {
            const juce::String label = "Mod Env " + juce::String (i + 1);
            auto group = std::make_unique<Group> ("menv" + juce::String (i + 1), label, "|");
            addEnvelope (*group, label, [i] (const char* n) { return pid::menv (i, n); }, 0.002f, 0.4f, 0.0f, 0.2f);
            layout.add (std::move (group));
        }
    }

    void addLfos (Layout& layout)
    {
        for (int i = 0; i < 2; ++i)
        {
            const juce::String label = "LFO " + juce::String (i + 1);
            auto group = std::make_unique<Group> ("lfo" + juce::String (i + 1), label, "|");
            group->addChild (makeChoice (pid::lfo (i, "wave"), label + " Wave", lfoWaveNames(), lfoSine));
            group->addChild (makeFloat (pid::lfo (i, "rate"), label + " Rate", skewed (0.01f, 40.0f, 2.0f), 2.0f, hzText));
            group->addChild (makeBool (pid::lfo (i, "sync"), label + " Sync", false));
            group->addChild (makeChoice (pid::lfo (i, "div"), label + " Division", syncDivisionNames(), 7));
            group->addChild (makePercent (pid::lfo (i, "gain"), label + " Gain", 1.0f));
            group->addChild (makeBool (pid::lfo (i, "free"), label + " Free Run", false));
            layout.add (std::move (group));
        }
    }

    void addModMatrix (Layout& layout)
    {
        auto group = std::make_unique<Group> ("modMatrix", "Mod Matrix", "|");
        for (int i = 0; i < numModSlots; ++i)
        {
            const juce::String label = "Mod " + juce::String (i + 1);
            group->addChild (makeChoice (pid::mod (i, "src"), label + " Source", modSourceNames(), srcOff));
            group->addChild (makeChoice (pid::mod (i, "dst"), label + " Destination", modDestNames(), dstOff));
            group->addChild (makeBipolar (pid::mod (i, "amt"), label + " Amount", 0.0f));
        }
        layout.add (std::move (group));
    }

    void addGlobal (Layout& layout)
    {
        auto group = std::make_unique<Group> ("global", "Global", "|");
        group->addChild (makeFloat ("master", "Master Volume", { -48.0f, 6.0f }, -6.0f, dbText));
        group->addChild (makePercent ("mixA", "Mix A", 1.0f));
        group->addChild (makePercent ("mixB", "Mix B", 1.0f));
        group->addChild (makeInt ("poly", "Polyphony", 1, maxPolyphony, 8));
        group->addChild (makeChoice ("voiceMode", "Voice Mode", voiceModeNames(), modePoly));
        group->addChild (makeFloat ("porta", "Portamento", skewed (0.0f, 2.0f, 0.2f), 0.0f, timeText));
        group->addChild (makeInt ("bendRange", "Bend Range", 0, 24, 2));
        group->addChild (makePercent ("velSens", "Velocity Sensitivity", 0.5f));
        layout.add (std::move (group));
    }

    void addArp (Layout& layout)
    {
        auto group = std::make_unique<Group> ("arp", "Arpeggiator", "|");
        group->addChild (makeBool ("arp_on", "Arp On", false));
        group->addChild (makeChoice ("arp_mode", "Arp Mode", arpModeNames(), arpUp));
        group->addChild (makeInt ("arp_octaves", "Arp Octaves", 1, 4, 1));
        group->addChild (makeChoice ("arp_div", "Arp Time", syncDivisionNames(), 13));
        group->addChild (makeFloat ("arp_gate", "Arp Gate", { 0.05f, 1.0f }, 0.5f, percentText, percentValue));
        group->addChild (makeFloat ("arp_swing", "Arp Swing", { 0.0f, 1.0f }, 0.0f, percentText, percentValue));
        group->addChild (makeChoice ("arp_velMode", "Arp Velocity", arpVelModeNames(), arpVelKey));
        group->addChild (makeBool ("arp_hold", "Arp Hold", false));
        group->addChild (makeInt ("arp_steps", "Arp Steps", 1, numArpSteps, 16));

        for (int s = 0; s < numArpSteps; ++s)
        {
            const juce::String label = "Arp Step " + juce::String (s + 1);
            group->addChild (makeBool (pid::arpStep (s, "on"), label + " On", true));
            group->addChild (makeInt (pid::arpStep (s, "trans"), label + " Transpose", -24, 24, 0));
            group->addChild (makePercent (pid::arpStep (s, "vel"), label + " Velocity", 1.0f));
        }
        layout.add (std::move (group));
    }

    void addEffects (Layout& layout)
    {
        auto dist = std::make_unique<Group> ("fxDist", "Distortion", "|");
        dist->addChild (makeBool ("dist_on", "Distortion On", false));
        dist->addChild (makeChoice ("dist_type", "Distortion Type", distTypeNames(), distOverdrive));
        dist->addChild (makePercent ("dist_amount", "Distortion Amount", 0.3f));
        dist->addChild (makePercent ("dist_mix", "Distortion Mix", 1.0f));
        layout.add (std::move (dist));

        auto phaser = std::make_unique<Group> ("fxPhaser", "Phaser", "|");
        phaser->addChild (makeBool ("phaser_on", "Phaser On", false));
        phaser->addChild (makeFloat ("phaser_rate", "Phaser Rate", skewed (0.02f, 10.0f, 1.0f), 0.4f, hzText));
        phaser->addChild (makePercent ("phaser_depth", "Phaser Depth", 0.6f));
        phaser->addChild (makeFloat ("phaser_freq", "Phaser Centre", skewed (100.0f, 8000.0f, 1000.0f), 900.0f, hzText));
        phaser->addChild (makeBipolar ("phaser_fb", "Phaser Feedback", 0.4f));
        phaser->addChild (makePercent ("phaser_mix", "Phaser Mix", 0.5f));
        layout.add (std::move (phaser));

        auto chorus = std::make_unique<Group> ("fxChorus", "Chorus", "|");
        chorus->addChild (makeBool ("chorus_on", "Chorus On", false));
        chorus->addChild (makeChoice ("chorus_mode", "Chorus Mode", { "Chorus", "Flanger" }, 0));
        chorus->addChild (makeFloat ("chorus_rate", "Chorus Rate", skewed (0.02f, 10.0f, 1.0f), 0.6f, hzText));
        chorus->addChild (makePercent ("chorus_depth", "Chorus Depth", 0.4f));
        chorus->addChild (makeFloat ("chorus_delay", "Chorus Delay", { 1.0f, 40.0f }, 8.0f,
                                     [] (float v, int) { return juce::String (v, 1) + " ms"; }));
        chorus->addChild (makeBipolar ("chorus_fb", "Chorus Feedback", 0.0f));
        chorus->addChild (makePercent ("chorus_mix", "Chorus Mix", 0.5f));
        layout.add (std::move (chorus));

        auto eq = std::make_unique<Group> ("fxEq", "Equalizer", "|");
        eq->addChild (makeBool ("eq_on", "EQ On", false));
        eq->addChild (makeFloat ("eq_lowGain", "EQ Bass", { -15.0f, 15.0f }, 0.0f, dbText));
        eq->addChild (makeFloat ("eq_lowFreq", "EQ Bass Freq", skewed (30.0f, 800.0f, 150.0f), 150.0f, hzText));
        eq->addChild (makeFloat ("eq_highGain", "EQ Treble", { -15.0f, 15.0f }, 0.0f, dbText));
        eq->addChild (makeFloat ("eq_highFreq", "EQ Treble Freq", skewed (1000.0f, 16000.0f, 5000.0f), 6000.0f, hzText));
        layout.add (std::move (eq));

        auto delay = std::make_unique<Group> ("fxDelay", "Delay", "|");
        delay->addChild (makeBool ("delay_on", "Delay On", false));
        delay->addChild (makeBool ("delay_sync", "Delay Sync", true));
        delay->addChild (makeChoice ("delay_divL", "Delay Time L (Sync)", syncDivisionNames(), 9));
        delay->addChild (makeChoice ("delay_divR", "Delay Time R (Sync)", syncDivisionNames(), 7));
        delay->addChild (makeFloat ("delay_timeL", "Delay Time L", skewed (0.001f, 2.0f, 0.35f), 0.375f, timeText));
        delay->addChild (makeFloat ("delay_timeR", "Delay Time R", skewed (0.001f, 2.0f, 0.35f), 0.5f, timeText));
        delay->addChild (makePercent ("delay_fb", "Delay Feedback", 0.35f));
        delay->addChild (makeBool ("delay_pingpong", "Delay Ping-Pong", true));
        delay->addChild (makeFloat ("delay_lowcut", "Delay Low Cut", skewed (20.0f, 2000.0f, 200.0f), 150.0f, hzText));
        delay->addChild (makeFloat ("delay_highcut", "Delay High Cut", skewed (1000.0f, 20000.0f, 6000.0f), 8000.0f, hzText));
        delay->addChild (makePercent ("delay_mix", "Delay Mix", 0.25f));
        layout.add (std::move (delay));

        auto reverb = std::make_unique<Group> ("fxReverb", "Reverb", "|");
        reverb->addChild (makeBool ("reverb_on", "Reverb On", false));
        reverb->addChild (makePercent ("reverb_size", "Reverb Size", 0.7f));
        reverb->addChild (makePercent ("reverb_damp", "Reverb Damping", 0.5f));
        reverb->addChild (makePercent ("reverb_width", "Reverb Width", 1.0f));
        reverb->addChild (makePercent ("reverb_mix", "Reverb Mix", 0.25f));
        layout.add (std::move (reverb));

        auto comp = std::make_unique<Group> ("fxComp", "Compressor", "|");
        comp->addChild (makeBool ("comp_on", "Compressor On", false));
        comp->addChild (makeFloat ("comp_thresh", "Comp Threshold", { -48.0f, 0.0f }, -12.0f, dbText));
        comp->addChild (makeFloat ("comp_ratio", "Comp Ratio", skewed (1.0f, 20.0f, 4.0f), 4.0f,
                                   [] (float v, int) { return juce::String (v, 1) + ":1"; }));
        comp->addChild (makeFloat ("comp_attack", "Comp Attack", skewed (0.1f, 100.0f, 10.0f), 5.0f,
                                   [] (float v, int) { return juce::String (v, 1) + " ms"; }));
        comp->addChild (makeFloat ("comp_release", "Comp Release", skewed (10.0f, 1000.0f, 150.0f), 120.0f,
                                   [] (float v, int) { return juce::String (juce::roundToInt (v)) + " ms"; }));
        comp->addChild (makeFloat ("comp_makeup", "Comp Makeup", { 0.0f, 24.0f }, 0.0f, dbText));
        layout.add (std::move (comp));
    }
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    Layout layout;
    addGlobal (layout);
    addOscillators (layout);
    addFilters (layout);
    addEnvelopes (layout);
    addLfos (layout);
    addModMatrix (layout);
    addArp (layout);
    addEffects (layout);
    return layout;
}
} // namespace lumora
