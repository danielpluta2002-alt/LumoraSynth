#include "SynthParams.h"

namespace lumora
{
namespace
{
    std::atomic<float>* raw (juce::AudioProcessorValueTreeState& s, const juce::String& id)
    {
        auto* p = s.getRawParameterValue (id);
        jassert (p != nullptr);
        return p;
    }

    inline float f (const std::atomic<float>* p) { return p->load (std::memory_order_relaxed); }
    inline int i (const std::atomic<float>* p) { return (int) std::lround (f (p)); }
    inline bool b (const std::atomic<float>* p) { return f (p) >= 0.5f; }
}

ParamRefs::ParamRefs (juce::AudioProcessorValueTreeState& s)
{
    for (int n = 0; n < numOscs; ++n)
    {
        auto id = [n] (const char* name) { return pid::osc (n, name); };
        osc[n] = { raw (s, id ("wave")), raw (s, id ("voices")), raw (s, id ("detune")), raw (s, id ("stereo")),
                   raw (s, id ("phase")), raw (s, id ("retrig")), raw (s, id ("invert")), raw (s, id ("octave")),
                   raw (s, id ("note")), raw (s, id ("fine")), raw (s, id ("volume")), raw (s, id ("pan")) };
    }

    for (int n = 0; n < numFilters; ++n)
    {
        auto id = [n] (const char* name) { return pid::filt (n, name); };
        filt[n] = { raw (s, id ("type")), raw (s, id ("slope")), raw (s, id ("input")),
                    raw (s, id ("cutoff")), raw (s, id ("reso")), raw (s, id ("drive")) };
    }

    ctlCutoff = raw (s, "fctl_cutoff");
    ctlReso = raw (s, "fctl_reso");
    ctlWarm = raw (s, "fctl_warm");
    ctlKeytrack = raw (s, "fctl_keytrack");

    for (int n = 0; n < 2; ++n)
    {
        ampEnv[n] = { raw (s, pid::amp (n, "attack")), raw (s, pid::amp (n, "decay")),
                      raw (s, pid::amp (n, "sustain")), raw (s, pid::amp (n, "release")) };
        modEnv[n] = { raw (s, pid::menv (n, "attack")), raw (s, pid::menv (n, "decay")),
                      raw (s, pid::menv (n, "sustain")), raw (s, pid::menv (n, "release")) };
        lfo[n] = { raw (s, pid::lfo (n, "wave")), raw (s, pid::lfo (n, "rate")), raw (s, pid::lfo (n, "sync")),
                   raw (s, pid::lfo (n, "div")), raw (s, pid::lfo (n, "gain")), raw (s, pid::lfo (n, "free")) };
    }

    for (int n = 0; n < numModSlots; ++n)
        mods[n] = { raw (s, pid::mod (n, "src")), raw (s, pid::mod (n, "dst")), raw (s, pid::mod (n, "amt")) };

    master = raw (s, "master");
    mixA = raw (s, "mixA");
    mixB = raw (s, "mixB");
    poly = raw (s, "poly");
    voiceMode = raw (s, "voiceMode");
    porta = raw (s, "porta");
    bendRange = raw (s, "bendRange");
    velSens = raw (s, "velSens");

    arpOn = raw (s, "arp_on");
    arpMode = raw (s, "arp_mode");
    arpOctaves = raw (s, "arp_octaves");
    arpDiv = raw (s, "arp_div");
    arpGate = raw (s, "arp_gate");
    arpSwing = raw (s, "arp_swing");
    arpVelMode = raw (s, "arp_velMode");
    arpHold = raw (s, "arp_hold");
    arpSteps = raw (s, "arp_steps");
    for (int n = 0; n < numArpSteps; ++n)
        steps[n] = { raw (s, pid::arpStep (n, "on")), raw (s, pid::arpStep (n, "trans")), raw (s, pid::arpStep (n, "vel")) };

    distOn = raw (s, "dist_on"); distType = raw (s, "dist_type"); distAmount = raw (s, "dist_amount"); distMix = raw (s, "dist_mix");

    phaserOn = raw (s, "phaser_on"); phaserRate = raw (s, "phaser_rate"); phaserDepth = raw (s, "phaser_depth");
    phaserFreq = raw (s, "phaser_freq"); phaserFb = raw (s, "phaser_fb"); phaserMix = raw (s, "phaser_mix");

    chorusOn = raw (s, "chorus_on"); chorusMode = raw (s, "chorus_mode"); chorusRate = raw (s, "chorus_rate");
    chorusDepth = raw (s, "chorus_depth"); chorusDelay = raw (s, "chorus_delay"); chorusFb = raw (s, "chorus_fb");
    chorusMix = raw (s, "chorus_mix");

    eqOn = raw (s, "eq_on"); eqLowGain = raw (s, "eq_lowGain"); eqLowFreq = raw (s, "eq_lowFreq");
    eqHighGain = raw (s, "eq_highGain"); eqHighFreq = raw (s, "eq_highFreq");

    delayOn = raw (s, "delay_on"); delaySync = raw (s, "delay_sync"); delayDivL = raw (s, "delay_divL");
    delayDivR = raw (s, "delay_divR"); delayTimeL = raw (s, "delay_timeL"); delayTimeR = raw (s, "delay_timeR");
    delayFb = raw (s, "delay_fb"); delayPingPong = raw (s, "delay_pingpong"); delayLowCut = raw (s, "delay_lowcut");
    delayHighCut = raw (s, "delay_highcut"); delayMix = raw (s, "delay_mix");

    reverbOn = raw (s, "reverb_on"); reverbSize = raw (s, "reverb_size"); reverbDamp = raw (s, "reverb_damp");
    reverbWidth = raw (s, "reverb_width"); reverbMix = raw (s, "reverb_mix");

    compOn = raw (s, "comp_on"); compThresh = raw (s, "comp_thresh"); compRatio = raw (s, "comp_ratio");
    compAttack = raw (s, "comp_attack"); compRelease = raw (s, "comp_release"); compMakeup = raw (s, "comp_makeup");
}

void ParamRefs::fill (VoiceParams& v, ArpParams& a, FxParams& x, float& masterDb) const
{
    for (int n = 0; n < numOscs; ++n)
    {
        auto& o = v.osc[n];
        const auto& r = osc[n];
        o.wave = i (r.wave); o.voices = juce::jlimit (1, maxUnison, i (r.voices));
        o.detune = f (r.detune); o.stereo = f (r.stereo); o.phase = f (r.phase);
        o.retrig = b (r.retrig); o.invert = b (r.invert);
        o.octave = i (r.octave); o.note = i (r.note); o.fine = f (r.fine);
        o.volume = f (r.volume); o.pan = f (r.pan);
    }

    for (int n = 0; n < numFilters; ++n)
    {
        auto& fp = v.filter[n];
        const auto& r = filt[n];
        fp.type = i (r.type); fp.slope = i (r.slope); fp.input = i (r.input);
        fp.cutoff = f (r.cutoff); fp.reso = f (r.reso); fp.drive = f (r.drive);
    }

    v.ctlCutoff = f (ctlCutoff); v.ctlReso = f (ctlReso); v.ctlWarm = f (ctlWarm); v.ctlKeytrack = f (ctlKeytrack);

    auto readEnv = [] (const Env& r, EnvParams& e)
    {
        e.attack = f (r.attack); e.decay = f (r.decay); e.sustain = f (r.sustain); e.release = f (r.release);
    };

    for (int n = 0; n < 2; ++n)
    {
        readEnv (ampEnv[n], v.ampEnv[n]);
        readEnv (modEnv[n], v.modEnv[n]);
        auto& l = v.lfo[n];
        l.wave = i (lfo[n].wave); l.rate = f (lfo[n].rate); l.sync = b (lfo[n].sync);
        l.division = i (lfo[n].division); l.gain = f (lfo[n].gain); l.freeRun = b (lfo[n].freeRun);
    }

    for (int n = 0; n < numModSlots; ++n)
        v.mods[n] = { i (mods[n].source), i (mods[n].dest), f (mods[n].amount) };

    v.mixA = f (mixA); v.mixB = f (mixB); v.velSens = f (velSens); v.porta = f (porta);
    v.bendRange = i (bendRange); v.polyphony = juce::jlimit (1, maxPolyphony, i (poly)); v.voiceMode = i (voiceMode);
    masterDb = f (master);

    a.enabled = b (arpOn); a.mode = i (arpMode); a.octaves = i (arpOctaves); a.division = i (arpDiv);
    a.gate = f (arpGate); a.swing = f (arpSwing); a.velMode = i (arpVelMode); a.hold = b (arpHold);
    a.numSteps = juce::jlimit (1, numArpSteps, i (arpSteps));
    for (int n = 0; n < numArpSteps; ++n)
        a.steps[n] = { b (steps[n].on), i (steps[n].transpose), f (steps[n].velocity) };

    x.distOn = b (distOn); x.distType = i (distType); x.distAmount = f (distAmount); x.distMix = f (distMix);
    x.phaserOn = b (phaserOn); x.phaserRate = f (phaserRate); x.phaserDepth = f (phaserDepth);
    x.phaserFreq = f (phaserFreq); x.phaserFb = f (phaserFb); x.phaserMix = f (phaserMix);
    x.chorusOn = b (chorusOn); x.chorusMode = i (chorusMode); x.chorusRate = f (chorusRate); x.chorusDepth = f (chorusDepth);
    x.chorusDelay = f (chorusDelay); x.chorusFb = f (chorusFb); x.chorusMix = f (chorusMix);
    x.eqOn = b (eqOn); x.eqLowGain = f (eqLowGain); x.eqLowFreq = f (eqLowFreq); x.eqHighGain = f (eqHighGain); x.eqHighFreq = f (eqHighFreq);
    x.delayOn = b (delayOn); x.delaySync = b (delaySync); x.delayDivL = i (delayDivL); x.delayDivR = i (delayDivR);
    x.delayTimeL = f (delayTimeL); x.delayTimeR = f (delayTimeR); x.delayFb = f (delayFb); x.delayPingPong = b (delayPingPong);
    x.delayLowCut = f (delayLowCut); x.delayHighCut = f (delayHighCut); x.delayMix = f (delayMix);
    x.reverbOn = b (reverbOn); x.reverbSize = f (reverbSize); x.reverbDamp = f (reverbDamp);
    x.reverbWidth = f (reverbWidth); x.reverbMix = f (reverbMix);
    x.compOn = b (compOn); x.compThresh = f (compThresh); x.compRatio = f (compRatio);
    x.compAttack = f (compAttack); x.compRelease = f (compRelease); x.compMakeup = f (compMakeup);
}
} // namespace lumora
