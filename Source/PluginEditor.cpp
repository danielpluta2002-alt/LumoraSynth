#include "PluginEditor.h"
#include "Parameters.h"
#include "gui/Controls.h"

using namespace lumora;
using namespace lumora::gui;

namespace
{
/** Section that owns its child controls. */
class Panel : public Section
{
public:
    Panel (APVTS& s, const juce::String& panelTitle, juce::Colour panelAccent) : Section (panelTitle, panelAccent), state (s) {}

protected:
    Knob* knob (const juce::String& id, const juce::String& caption, juce::Colour colour)
    {
        return add (std::make_unique<Knob> (state, id, caption, colour));
    }
    Knob* knob (const juce::String& id, const juce::String& caption) { return knob (id, caption, getAccent()); }

    ChoiceBox* choice (const juce::String& id, const juce::String& caption = {})
    {
        return add (std::make_unique<ChoiceBox> (state, id, caption));
    }

    Toggle* toggle (const juce::String& id, const juce::String& text)
    {
        return add (std::make_unique<Toggle> (state, id, text, getAccent()));
    }

    Toggle* headerToggle (const juce::String& id)
    {
        auto t = std::make_unique<Toggle> (state, id, "ON", getAccent());
        auto* raw = t.get();
        owned.add (t.release());
        setHeaderComponent (raw);
        return raw;
    }

    template <typename T>
    T* add (std::unique_ptr<T> c)
    {
        auto* raw = c.get();
        addAndMakeVisible (raw);
        owned.add (c.release());
        return raw;
    }

    bool paramOn (const juce::String& id) const { return state.getRawParameterValue (id)->load() >= 0.5f; }

    APVTS& state;

private:
    juce::OwnedArray<juce::Component> owned;
};

constexpr int kw = Knob::width;
constexpr int kh = Knob::height;

//==============================================================================
class OscPanel : public Panel
{
public:
    OscPanel (APVTS& s, int i)
        : Panel (s, "Oscillator " + pid::oscLabel (i), i < 2 ? colours::partA : colours::partB)
    {
        wave = choice (pid::osc (i, "wave"));
        retrig = toggle (pid::osc (i, "retrig"), "RETRIG");
        invert = toggle (pid::osc (i, "invert"), "INV");
        octave = knob (pid::osc (i, "octave"), "OCTAVE");
        note = knob (pid::osc (i, "note"), "NOTE");
        fine = knob (pid::osc (i, "fine"), "FINE");
        phase = knob (pid::osc (i, "phase"), "PHASE");
        volume = knob (pid::osc (i, "volume"), "VOLUME");
        voices = knob (pid::osc (i, "voices"), "VOICES");
        detune = knob (pid::osc (i, "detune"), "DETUNE");
        stereo = knob (pid::osc (i, "stereo"), "STEREO");
        pan = knob (pid::osc (i, "pan"), "PAN");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        auto top = c.removeFromTop (24);
        wave->setBounds (top.removeFromLeft (120));
        invert->setBounds (top.removeFromRight (44));
        top.removeFromRight (4);
        retrig->setBounds (top.removeFromRight (64));
        c.removeFromTop (4);
        layoutRow (c.removeFromTop (kh), { octave, note, fine, phase, volume }, kw + 4);
        layoutRow (c.removeFromTop (kh), { voices, detune, stereo, pan, nullptr }, kw + 4);
    }

private:
    ChoiceBox* wave;
    Toggle *retrig, *invert;
    Knob *octave, *note, *fine, *phase, *volume, *voices, *detune, *stereo, *pan;
};

//==============================================================================
class FilterPanel : public Panel
{
public:
    FilterPanel (APVTS& s, int i)
        : Panel (s, i == 0 ? "Filter A" : "Filter B", i == 0 ? colours::partA : colours::partB)
    {
        type = choice (pid::filt (i, "type"), "TYPE");
        slope = choice (pid::filt (i, "slope"), "SLOPE");
        input = choice (pid::filt (i, "input"), "INPUT");
        cutoff = knob (pid::filt (i, "cutoff"), "CUTOFF");
        reso = knob (pid::filt (i, "reso"), "RESO");
        drive = knob (pid::filt (i, "drive"), "DRIVE");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        auto top = c.removeFromTop (38);
        type->setBounds (top.removeFromLeft (94));
        top.removeFromLeft (4);
        slope->setBounds (top.removeFromLeft (64));
        top.removeFromLeft (4);
        input->setBounds (top);
        c.removeFromTop (4);
        layoutRow (c, { cutoff, reso, drive }, 66, 6);
    }

private:
    ChoiceBox *type, *slope, *input;
    Knob *cutoff, *reso, *drive;
};

//==============================================================================
class FilterControlPanel : public Panel
{
public:
    explicit FilterControlPanel (APVTS& s) : Panel (s, "Filter Ctl", colours::global)
    {
        cutoff = knob ("fctl_cutoff", "CUTOFF");
        reso = knob ("fctl_reso", "RESO");
        warm = knob ("fctl_warm", "WARM");
        keytrack = knob ("fctl_keytrack", "KEYTRK");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        layoutRow (c.removeFromTop (kh), { cutoff, reso }, kw + 4);
        layoutRow (c.removeFromTop (kh), { warm, keytrack }, kw + 4);
    }

private:
    Knob *cutoff, *reso, *warm, *keytrack;
};

//==============================================================================
class AmpEnvPanel : public Panel
{
public:
    explicit AmpEnvPanel (APVTS& s) : Panel (s, "Amp Envelopes", colours::partA)
    {
        for (int i = 0; i < 2; ++i)
        {
            const auto colour = i == 0 ? colours::partA : colours::partB;
            rows[i][0] = knob (pid::amp (i, "attack"), "ATTACK", colour);
            rows[i][1] = knob (pid::amp (i, "decay"), "DECAY", colour);
            rows[i][2] = knob (pid::amp (i, "sustain"), "SUSTAIN", colour);
            rows[i][3] = knob (pid::amp (i, "release"), "RELEASE", colour);
        }
    }

    void paint (juce::Graphics& g) override
    {
        Section::paint (g);
        auto c = getContentArea();
        g.setFont (uiFont (13.0f, true));
        for (int i = 0; i < 2; ++i)
        {
            g.setColour (i == 0 ? colours::partA : colours::partB);
            g.drawText (i == 0 ? "A" : "B", c.removeFromTop (kh).removeFromLeft (16), juce::Justification::centred);
        }
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        c.removeFromLeft (16);
        for (auto& row : rows)
            layoutRow (c.removeFromTop (kh), { row[0], row[1], row[2], row[3] }, kw + 2);
    }

private:
    Knob* rows[2][4] {};
};

//==============================================================================
class VoicePanel : public Panel
{
public:
    explicit VoicePanel (APVTS& s) : Panel (s, "Voice", colours::global)
    {
        mode = choice ("voiceMode", "MODE");
        poly = knob ("poly", "POLY");
        porta = knob ("porta", "PORTA");
        bend = knob ("bendRange", "BEND");
        vel = knob ("velSens", "VEL SENS");
        mixA = knob ("mixA", "MIX A", colours::partA);
        mixB = knob ("mixB", "MIX B", colours::partB);
        master = knob ("master", "MASTER");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        auto row1 = c.removeFromTop (kh);
        mode->setBounds (row1.removeFromLeft (100).withSizeKeepingCentre (100, 40));
        layoutRow (row1, { poly, porta, bend }, kw + 10);
        layoutRow (c.removeFromTop (kh), { vel, mixA, mixB, master }, kw + 10);
    }

private:
    ChoiceBox* mode;
    Knob *poly, *porta, *bend, *vel, *mixA, *mixB, *master;
};

//==============================================================================
class ModEnvPanel : public Panel
{
public:
    ModEnvPanel (APVTS& s, int i) : Panel (s, "Mod Envelope " + juce::String (i + 1), colours::modulation)
    {
        a = knob (pid::menv (i, "attack"), "ATTACK");
        d = knob (pid::menv (i, "decay"), "DECAY");
        su = knob (pid::menv (i, "sustain"), "SUSTAIN");
        r = knob (pid::menv (i, "release"), "RELEASE");
    }

    void resized() override
    {
        Section::resized();
        layoutRow (getContentArea().withSizeKeepingCentre (getContentArea().getWidth(), kh), { a, d, su, r }, kw + 2);
    }

private:
    Knob *a, *d, *su, *r;
};

class LfoPanel : public Panel
{
public:
    LfoPanel (APVTS& s, int i) : Panel (s, "LFO " + juce::String (i + 1), colours::modulation), index (i)
    {
        wave = choice (pid::lfo (i, "wave"), "WAVE");
        sync = toggle (pid::lfo (i, "sync"), "SYNC");
        freeRun = toggle (pid::lfo (i, "free"), "FREE");
        rate = knob (pid::lfo (i, "rate"), "RATE");
        division = choice (pid::lfo (i, "div"), "TIME");
        gain = knob (pid::lfo (i, "gain"), "GAIN");
        updateVisibility();
    }

    void updateVisibility()
    {
        const bool synced = paramOn (pid::lfo (index, "sync"));
        rate->setVisible (! synced);
        division->setVisible (synced);
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        auto left = c.removeFromLeft (110);
        wave->setBounds (left.removeFromTop (40));
        left.removeFromTop (8);
        auto toggles = left.removeFromTop (22);
        sync->setBounds (toggles.removeFromLeft (52));
        toggles.removeFromLeft (6);
        freeRun->setBounds (toggles.removeFromLeft (52));

        c.removeFromLeft (6);
        auto knobs = c.withSizeKeepingCentre (c.getWidth(), kh);
        auto rateArea = knobs.removeFromLeft (80);
        rate->setBounds (rateArea.withSizeKeepingCentre (kw, kh));
        division->setBounds (rateArea.withSizeKeepingCentre (76, 40));
        gain->setBounds (knobs.removeFromLeft (kw + 4).withSizeKeepingCentre (kw, kh));
    }

private:
    int index;
    ChoiceBox *wave, *division;
    Toggle *sync, *freeRun;
    Knob *rate, *gain;
};

class ModMatrixPanel : public Panel
{
public:
    explicit ModMatrixPanel (APVTS& s) : Panel (s, "Modulation Matrix", colours::modulation)
    {
        for (int i = 0; i < numModSlots; ++i)
        {
            slots[i].source = choice (pid::mod (i, "src"));
            slots[i].dest = choice (pid::mod (i, "dst"));
            slots[i].amount = add (std::make_unique<BarSlider> (state, pid::mod (i, "amt"), colours::modulation, false));
        }
    }

    void paint (juce::Graphics& g) override
    {
        Section::paint (g);
        g.setFont (uiFont (11.0f, true));
        g.setColour (colours::textDim);
        for (int col = 0; col < 2; ++col)
        {
            auto header = columnArea (col).removeFromTop (16);
            header.removeFromLeft (22);
            g.drawText ("SOURCE", header.removeFromLeft (124), juce::Justification::centredLeft);
            g.drawText ("DESTINATION", header.removeFromLeft (134), juce::Justification::centredLeft);
            g.drawText ("AMOUNT", header, juce::Justification::centredLeft);
        }

        for (int i = 0; i < numModSlots; ++i)
        {
            g.setColour (colours::modulation.withAlpha (0.8f));
            g.drawText (juce::String (i + 1), rowArea (i).removeFromLeft (18), juce::Justification::centred);
        }
    }

    void resized() override
    {
        Section::resized();
        for (int i = 0; i < numModSlots; ++i)
        {
            auto r = rowArea (i).reduced (0, 4);
            r.removeFromLeft (22);
            slots[i].source->setBounds (r.removeFromLeft (120));
            r.removeFromLeft (4);
            slots[i].dest->setBounds (r.removeFromLeft (130));
            r.removeFromLeft (4);
            slots[i].amount->setBounds (r.withSizeKeepingCentre (r.getWidth(), 22));
        }
    }

private:
    juce::Rectangle<int> columnArea (int col) const
    {
        auto c = getContentArea();
        const int w = (c.getWidth() - 12) / 2;
        return col == 0 ? c.removeFromLeft (w) : c.removeFromRight (w);
    }

    juce::Rectangle<int> rowArea (int i) const
    {
        auto col = columnArea (i / 4);
        col.removeFromTop (18);
        const int h = col.getHeight() / 4;
        return col.withTrimmedTop (h * (i % 4)).withHeight (h);
    }

    struct Slot { ChoiceBox* source; ChoiceBox* dest; BarSlider* amount; };
    Slot slots[numModSlots] {};
};

class ModPage : public juce::Component
{
public:
    explicit ModPage (APVTS& s)
    {
        for (int i = 0; i < 2; ++i)
        {
            modEnv[i] = std::make_unique<ModEnvPanel> (s, i);
            lfo[i] = std::make_unique<LfoPanel> (s, i);
            addAndMakeVisible (*modEnv[i]);
            addAndMakeVisible (*lfo[i]);
        }
        matrix = std::make_unique<ModMatrixPanel> (s);
        addAndMakeVisible (*matrix);
    }

    void update() { for (auto& l : lfo) l->updateVisibility(); }

    void resized() override
    {
        auto r = getLocalBounds();
        const int half = (r.getHeight() - 8) / 2;
        auto envCol = r.removeFromLeft (250);
        modEnv[0]->setBounds (envCol.removeFromTop (half));
        modEnv[1]->setBounds (envCol.removeFromBottom (half));
        r.removeFromLeft (8);
        auto lfoCol = r.removeFromLeft (290);
        lfo[0]->setBounds (lfoCol.removeFromTop (half));
        lfo[1]->setBounds (lfoCol.removeFromBottom (half));
        r.removeFromLeft (8);
        matrix->setBounds (r);
    }

private:
    std::unique_ptr<ModEnvPanel> modEnv[2];
    std::unique_ptr<LfoPanel> lfo[2];
    std::unique_ptr<ModMatrixPanel> matrix;
};

//==============================================================================
class ArpSettingsPanel : public Panel
{
public:
    explicit ArpSettingsPanel (APVTS& s) : Panel (s, "Arpeggiator", colours::global)
    {
        headerToggle ("arp_on");
        mode = choice ("arp_mode", "MODE");
        time = choice ("arp_div", "TIME");
        vel = choice ("arp_velMode", "VELOCITY");
        hold = toggle ("arp_hold", "HOLD");
        octaves = knob ("arp_octaves", "OCTAVES");
        gate = knob ("arp_gate", "GATE");
        swing = knob ("arp_swing", "SWING");
        steps = knob ("arp_steps", "STEPS");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea().reduced (4, 6);
        auto row = c.removeFromTop (40);
        mode->setBounds (row.removeFromLeft (110));
        row.removeFromLeft (8);
        time->setBounds (row.removeFromLeft (80));
        row.removeFromLeft (8);
        vel->setBounds (row);
        c.removeFromTop (14);
        hold->setBounds (c.removeFromTop (24).removeFromLeft (80));
        c.removeFromTop (14);
        layoutRow (c.removeFromTop (kh), { octaves, gate, swing, steps }, kw + 8);
    }

private:
    ChoiceBox *mode, *time, *vel;
    Toggle* hold;
    Knob *octaves, *gate, *swing, *steps;
};

class ArpPatternPanel : public Panel
{
public:
    explicit ArpPatternPanel (APVTS& s) : Panel (s, "Pattern", colours::global)
    {
        for (int i = 0; i < numArpSteps; ++i)
        {
            steps[i].on = toggle (pid::arpStep (i, "on"), juce::String (i + 1));
            steps[i].transpose = add (std::make_unique<BarSlider> (state, pid::arpStep (i, "trans"), colours::partA, true));
            steps[i].velocity = add (std::make_unique<BarSlider> (state, pid::arpStep (i, "vel"), colours::partB, true));
        }
    }

    void setCurrentStep (int step, int numSteps)
    {
        if (step != currentStep || numSteps != activeSteps)
        {
            currentStep = step;
            activeSteps = numSteps;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        Section::paint (g);
        auto c = getContentArea();
        auto labels = c.removeFromLeft (labelWidth);
        g.setFont (uiFont (11.0f, true));
        g.setColour (colours::textDim);
        labels.removeFromTop (30);
        g.drawText ("TRANSPOSE", labels.removeFromTop (transposeHeight), juce::Justification::centredLeft);
        labels.removeFromTop (8);
        g.drawText ("VELOCITY", labels.removeFromTop (velocityHeight), juce::Justification::centredLeft);

        for (int i = 0; i < numArpSteps; ++i)
        {
            auto col = columnArea (i).toFloat();
            if (i >= activeSteps)
            {
                g.setColour (juce::Colours::black.withAlpha (0.25f));
                g.fillRoundedRectangle (col, 4.0f);
            }
            if (i == currentStep)
            {
                g.setColour (colours::global.withAlpha (0.18f));
                g.fillRoundedRectangle (col, 4.0f);
                g.setColour (colours::global);
                g.fillRoundedRectangle (col.removeFromBottom (4.0f), 2.0f);
            }
        }
    }

    void resized() override
    {
        Section::resized();
        for (int i = 0; i < numArpSteps; ++i)
        {
            auto col = columnArea (i).reduced (4, 0);
            steps[i].on->setBounds (col.removeFromTop (24));
            col.removeFromTop (6);
            steps[i].transpose->setBounds (col.removeFromTop (transposeHeight));
            col.removeFromTop (8);
            steps[i].velocity->setBounds (col.removeFromTop (velocityHeight));
        }
    }

private:
    juce::Rectangle<int> columnArea (int i) const
    {
        auto c = getContentArea();
        c.removeFromLeft (labelWidth);
        const int w = c.getWidth() / numArpSteps;
        return { c.getX() + i * w, c.getY(), w, c.getHeight() };
    }

    static constexpr int labelWidth = 76, transposeHeight = 150, velocityHeight = 110;

    struct Step { Toggle* on; BarSlider* transpose; BarSlider* velocity; };
    Step steps[numArpSteps] {};
    int currentStep = -1, activeSteps = numArpSteps;
};

class ArpPage : public juce::Component
{
public:
    explicit ArpPage (APVTS& s) : settings (s), pattern (s), state (s)
    {
        addAndMakeVisible (settings);
        addAndMakeVisible (pattern);
    }

    void update (int currentStep)
    {
        pattern.setCurrentStep (currentStep, (int) state.getRawParameterValue ("arp_steps")->load());
    }

    void resized() override
    {
        auto r = getLocalBounds();
        settings.setBounds (r.removeFromLeft (320));
        r.removeFromLeft (8);
        pattern.setBounds (r);
    }

private:
    ArpSettingsPanel settings;
    ArpPatternPanel pattern;
    APVTS& state;
};

//==============================================================================
class DistortionPanel : public Panel
{
public:
    explicit DistortionPanel (APVTS& s) : Panel (s, "Distortion", colours::effects)
    {
        headerToggle ("dist_on");
        type = choice ("dist_type", "TYPE");
        amount = knob ("dist_amount", "AMOUNT");
        mix = knob ("dist_mix", "MIX");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        type->setBounds (c.removeFromTop (40).reduced (4, 0));
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { amount, mix }, kw + 4);
    }

private:
    ChoiceBox* type;
    Knob *amount, *mix;
};

class PhaserPanel : public Panel
{
public:
    explicit PhaserPanel (APVTS& s) : Panel (s, "Phaser", colours::effects)
    {
        headerToggle ("phaser_on");
        rate = knob ("phaser_rate", "RATE");
        depth = knob ("phaser_depth", "DEPTH");
        freq = knob ("phaser_freq", "CENTRE");
        fb = knob ("phaser_fb", "FEEDBACK");
        mix = knob ("phaser_mix", "MIX");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { rate, depth }, kw + 4);
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { freq, fb }, kw + 4);
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { mix }, kw + 4);
    }

private:
    Knob *rate, *depth, *freq, *fb, *mix;
};

class ChorusPanel : public Panel
{
public:
    explicit ChorusPanel (APVTS& s) : Panel (s, "Chorus / Flanger", colours::effects)
    {
        headerToggle ("chorus_on");
        mode = choice ("chorus_mode", "MODE");
        rate = knob ("chorus_rate", "RATE");
        depth = knob ("chorus_depth", "DEPTH");
        delay = knob ("chorus_delay", "DELAY");
        fb = knob ("chorus_fb", "FEEDBACK");
        mix = knob ("chorus_mix", "MIX");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        mode->setBounds (c.removeFromTop (40).reduced (4, 0));
        c.removeFromTop (8);
        layoutRow (c.removeFromTop (kh), { rate, depth }, kw + 4);
        c.removeFromTop (6);
        layoutRow (c.removeFromTop (kh), { delay, fb }, kw + 4);
        c.removeFromTop (6);
        layoutRow (c.removeFromTop (kh), { mix }, kw + 4);
    }

private:
    ChoiceBox* mode;
    Knob *rate, *depth, *delay, *fb, *mix;
};

class EqPanel : public Panel
{
public:
    explicit EqPanel (APVTS& s) : Panel (s, "Equalizer", colours::effects)
    {
        headerToggle ("eq_on");
        lowGain = knob ("eq_lowGain", "BASS");
        lowFreq = knob ("eq_lowFreq", "FREQ");
        highGain = knob ("eq_highGain", "TREBLE");
        highFreq = knob ("eq_highFreq", "FREQ");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { lowGain, lowFreq }, kw + 4);
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { highGain, highFreq }, kw + 4);
    }

private:
    Knob *lowGain, *lowFreq, *highGain, *highFreq;
};

class DelayPanel : public Panel
{
public:
    explicit DelayPanel (APVTS& s) : Panel (s, "Delay", colours::effects)
    {
        headerToggle ("delay_on");
        sync = toggle ("delay_sync", "SYNC");
        pingPong = toggle ("delay_pingpong", "PING-PONG");
        divL = choice ("delay_divL", "TIME L");
        divR = choice ("delay_divR", "TIME R");
        timeL = knob ("delay_timeL", "TIME L");
        timeR = knob ("delay_timeR", "TIME R");
        fb = knob ("delay_fb", "FEEDBACK");
        mix = knob ("delay_mix", "MIX");
        lowCut = knob ("delay_lowcut", "LOW CUT");
        highCut = knob ("delay_highcut", "HIGH CUT");
        updateVisibility();
    }

    void updateVisibility()
    {
        const bool synced = paramOn ("delay_sync");
        divL->setVisible (synced);
        divR->setVisible (synced);
        timeL->setVisible (! synced);
        timeR->setVisible (! synced);
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        auto toggles = c.removeFromTop (22);
        sync->setBounds (toggles.removeFromLeft (52));
        toggles.removeFromLeft (6);
        pingPong->setBounds (toggles);
        c.removeFromTop (6);

        auto times = c.removeFromTop (kh);
        layoutRow (times, { timeL, timeR }, kw + 4);
        auto left = times.removeFromLeft (times.getWidth() / 2).reduced (2, 0);
        divL->setBounds (left.withSizeKeepingCentre (left.getWidth(), 40));
        auto right = times.reduced (2, 0);
        divR->setBounds (right.withSizeKeepingCentre (right.getWidth(), 40));

        c.removeFromTop (6);
        layoutRow (c.removeFromTop (kh), { fb, mix }, kw + 4);
        c.removeFromTop (6);
        layoutRow (c.removeFromTop (kh), { lowCut, highCut }, kw + 4);
    }

private:
    Toggle *sync, *pingPong;
    ChoiceBox *divL, *divR;
    Knob *timeL, *timeR, *fb, *mix, *lowCut, *highCut;
};

class ReverbPanel : public Panel
{
public:
    explicit ReverbPanel (APVTS& s) : Panel (s, "Reverb", colours::effects)
    {
        headerToggle ("reverb_on");
        size = knob ("reverb_size", "SIZE");
        damp = knob ("reverb_damp", "DAMP");
        width = knob ("reverb_width", "WIDTH");
        mix = knob ("reverb_mix", "MIX");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { size, damp }, kw + 4);
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { width, mix }, kw + 4);
    }

private:
    Knob *size, *damp, *width, *mix;
};

class CompressorPanel : public Panel
{
public:
    explicit CompressorPanel (APVTS& s) : Panel (s, "Compressor", colours::effects)
    {
        headerToggle ("comp_on");
        thresh = knob ("comp_thresh", "THRESH");
        ratio = knob ("comp_ratio", "RATIO");
        attack = knob ("comp_attack", "ATTACK");
        release = knob ("comp_release", "RELEASE");
        makeup = knob ("comp_makeup", "MAKEUP");
    }

    void resized() override
    {
        Section::resized();
        auto c = getContentArea();
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { thresh, ratio }, kw + 4);
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { attack, release }, kw + 4);
        c.removeFromTop (10);
        layoutRow (c.removeFromTop (kh), { makeup }, kw + 4);
    }

private:
    Knob *thresh, *ratio, *attack, *release, *makeup;
};

class FxPage : public juce::Component
{
public:
    explicit FxPage (APVTS& s)
        : dist (s), phaser (s), chorus (s), eq (s), delay (s), reverb (s), comp (s)
    {
        for (auto* c : panels())
            addAndMakeVisible (c);
    }

    void update() { delay.updateVisibility(); }

    void resized() override
    {
        auto r = getLocalBounds();
        const auto list = panels();
        const int gap = 8;
        const int w = (r.getWidth() - gap * ((int) list.size() - 1)) / (int) list.size();
        for (auto* c : list)
        {
            c->setBounds (r.removeFromLeft (w));
            r.removeFromLeft (gap);
        }
    }

private:
    std::vector<juce::Component*> panels() { return { &dist, &phaser, &chorus, &eq, &delay, &reverb, &comp }; }

    DistortionPanel dist;
    PhaserPanel phaser;
    ChorusPanel chorus;
    EqPanel eq;
    DelayPanel delay;
    ReverbPanel reverb;
    CompressorPanel comp;
};
} // namespace

//==============================================================================
class MainPanel : public juce::Component, private juce::Timer
{
public:
    explicit MainPanel (LumoraSynthProcessor& p)
        : processor (p), state (p.getState()),
          filterCtl (state), ampEnv (state), voice (state),
          modPage (state), arpPage (state), fxPage (state),
          keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        for (int i = 0; i < numOscs; ++i)
            addAndMakeVisible (*(oscs[i] = std::make_unique<OscPanel> (state, i)));
        for (int i = 0; i < numFilters; ++i)
            addAndMakeVisible (*(filters[i] = std::make_unique<FilterPanel> (state, i)));

        addAndMakeVisible (filterCtl);
        addAndMakeVisible (ampEnv);
        addAndMakeVisible (voice);
        addChildComponent (modPage);
        addChildComponent (arpPage);
        addChildComponent (fxPage);

        setupHeader();
        setupTabs();
        setupKeyboard();

        refreshPresetList();
        startTimerHz (15);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (colours::background);

        auto header = getLocalBounds().removeFromTop (headerHeight);
        g.setGradientFill (juce::ColourGradient (colours::panelHeader, 0.0f, 0.0f,
                                                 colours::background, 0.0f, (float) headerHeight, false));
        g.fillRect (header);

        auto logo = header.reduced (16, 0).removeFromLeft (220);
        g.setFont (uiFont (22.0f, true));
        g.setColour (colours::text);
        g.drawText ("LUMORA", logo.removeFromLeft (92), juce::Justification::centredLeft);
        g.setColour (colours::partA);
        g.drawText ("SYNTH", logo, juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto r = getLocalBounds();

        // Header
        auto header = r.removeFromTop (headerHeight).reduced (12, 8);
        header.removeFromLeft (230);
        voiceCount.setBounds (header.removeFromRight (110));
        header.removeFromRight (12);
        loadButton.setBounds (header.removeFromRight (64));
        header.removeFromRight (6);
        saveButton.setBounds (header.removeFromRight (64));
        header.removeFromRight (18);
        nextButton.setBounds (header.removeFromRight (30));
        header.removeFromRight (4);
        presetBox.setBounds (header.removeFromRight (300));
        header.removeFromRight (4);
        prevButton.setBounds (header.removeFromRight (30));

        r = r.reduced (12, 0);
        r.removeFromBottom (8);

        // Keyboard
        keyboard.setBounds (r.removeFromBottom (keyboardHeight));
        r.removeFromBottom (8);

        // Oscillators
        auto oscRow = r.removeFromTop (186);
        const int oscW = (oscRow.getWidth() - 3 * gap) / 4;
        for (auto& o : oscs)
        {
            o->setBounds (oscRow.removeFromLeft (oscW));
            oscRow.removeFromLeft (gap);
        }
        r.removeFromTop (gap);

        // Filters, envelopes, voice
        auto midRow = r.removeFromTop (156);
        filters[0]->setBounds (midRow.removeFromLeft (250));
        midRow.removeFromLeft (gap);
        filters[1]->setBounds (midRow.removeFromLeft (250));
        midRow.removeFromLeft (gap);
        filterCtl.setBounds (midRow.removeFromLeft (140));
        midRow.removeFromLeft (gap);
        ampEnv.setBounds (midRow.removeFromLeft (260));
        midRow.removeFromLeft (gap);
        voice.setBounds (midRow);
        r.removeFromTop (gap);

        // Tabs
        auto tabBar = r.removeFromTop (26);
        for (auto* b : { &modTab, &arpTab, &fxTab })
        {
            b->setBounds (tabBar.removeFromLeft (140));
            tabBar.removeFromLeft (4);
        }
        r.removeFromTop (6);
        modPage.setBounds (r);
        arpPage.setBounds (r);
        fxPage.setBounds (r);
    }

private:
    void setupHeader()
    {
        presetBox.setTextWhenNothingSelected ("Init");
        presetBox.onChange = [this] { presetChosen(); };
        addAndMakeVisible (presetBox);

        prevButton.setButtonText ("<");
        nextButton.setButtonText (">");
        prevButton.onClick = [this] { stepPreset (-1); };
        nextButton.onClick = [this] { stepPreset (1); };
        saveButton.setButtonText ("SAVE");
        loadButton.setButtonText ("LOAD");
        saveButton.onClick = [this] { savePreset(); };
        loadButton.onClick = [this] { loadPreset(); };
        for (auto* b : { &prevButton, &nextButton, &saveButton, &loadButton })
            addAndMakeVisible (b);

        voiceCount.setFont (uiFont (12.0f, true));
        voiceCount.setColour (juce::Label::textColourId, colours::textDim);
        voiceCount.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (voiceCount);
    }

    void setupTabs()
    {
        const std::pair<juce::TextButton*, const char*> tabs[] { { &modTab, "MODULATION" }, { &arpTab, "ARPEGGIATOR" }, { &fxTab, "EFFECTS" } };
        for (auto& [button, text] : tabs)
        {
            button->setButtonText (text);
            button->setClickingTogglesState (true);
            button->setRadioGroupId (1001);
            button->setColour (juce::TextButton::buttonOnColourId, colours::panelHeader.brighter (0.15f));
            button->onClick = [this] { updateTabs(); };
            addAndMakeVisible (button);
        }
        modTab.setToggleState (true, juce::dontSendNotification);
        updateTabs();
    }

    void updateTabs()
    {
        modPage.setVisible (modTab.getToggleState());
        arpPage.setVisible (arpTab.getToggleState());
        fxPage.setVisible (fxTab.getToggleState());
    }

    void setupKeyboard()
    {
        keyboard.setLowestVisibleKey (36);
        keyboard.setKeyWidth (22.0f);
        keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffd9dde3));
        keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff1a1d22));
        keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff6b7280));
        keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, colours::partA.withAlpha (0.7f));
        keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, colours::partA.withAlpha (0.25f));
        keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::black.withAlpha (0.3f));
        keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, colours::panel);
        keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, colours::textDim);
        keyboard.setWantsKeyboardFocus (false);
        addAndMakeVisible (keyboard);
    }

    //==============================================================================
    void refreshPresetList()
    {
        presetBox.clear (juce::dontSendNotification);
        userPresets.clear();

        const auto& factory = getFactoryPresets();
        juce::String lastCategory;
        for (size_t i = 0; i < factory.size(); ++i)
        {
            if (factory[i].category != lastCategory)
            {
                presetBox.addSectionHeading (factory[i].category);
                lastCategory = factory[i].category;
            }
            presetBox.addItem (factory[i].name, (int) i + 1);
        }

        userPresets = LumoraSynthProcessor::getUserPresetFolder().findChildFiles (juce::File::findFiles, false, "*.lumora");
        userPresets.sort();
        if (! userPresets.isEmpty())
        {
            presetBox.addSectionHeading ("User");
            for (int i = 0; i < userPresets.size(); ++i)
                presetBox.addItem (userPresets[i].getFileNameWithoutExtension(), userIdOffset + i);
        }

        showPresetName();
    }

    void showPresetName()
    {
        shownPresetName = processor.getPresetName();
        presetBox.setText (shownPresetName, juce::dontSendNotification);
    }

    void presetChosen()
    {
        const int id = presetBox.getSelectedId();
        if (id <= 0)
            return;

        if (id < userIdOffset)
            processor.setCurrentProgram (id - 1);
        else if (juce::isPositiveAndBelow (id - userIdOffset, userPresets.size()))
            processor.loadUserPreset (userPresets[id - userIdOffset]);

        showPresetName();
    }

    void stepPreset (int delta)
    {
        juce::Array<int> ids;
        for (int i = 0; i < presetBox.getNumItems(); ++i)
            ids.add (presetBox.getItemId (i));
        if (ids.isEmpty())
            return;

        int index = ids.indexOf (presetBox.getSelectedId());
        if (index < 0)
            index = delta > 0 ? -1 : 0;
        index = (index + delta + ids.size()) % ids.size();
        presetBox.setSelectedId (ids[index], juce::sendNotificationSync);
    }

    void savePreset()
    {
        chooser = std::make_unique<juce::FileChooser> ("Save preset",
                                                       LumoraSynthProcessor::getUserPresetFolder().getChildFile (processor.getPresetName() + ".lumora"),
                                                       "*.lumora");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting,
                              [this] (const juce::FileChooser& fc)
                              {
                                  const auto file = fc.getResult();
                                  if (file != juce::File() && processor.saveUserPreset (file))
                                      refreshPresetList();
                              });
    }

    void loadPreset()
    {
        chooser = std::make_unique<juce::FileChooser> ("Load preset", LumoraSynthProcessor::getUserPresetFolder(), "*.lumora");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
                              {
                                  const auto file = fc.getResult();
                                  if (file.existsAsFile() && processor.loadUserPreset (file))
                                      showPresetName();
                              });
    }

    void timerCallback() override
    {
        modPage.update();
        fxPage.update();
        arpPage.update (processor.getArpStep());
        voiceCount.setText ("VOICES " + juce::String (processor.getActiveVoiceCount()), juce::dontSendNotification);

        if (processor.getPresetName() != shownPresetName)
            showPresetName();
    }

    static constexpr int headerHeight = 48, keyboardHeight = 64, gap = 8, userIdOffset = 1000;

    LumoraSynthProcessor& processor;
    APVTS& state;

    std::unique_ptr<OscPanel> oscs[numOscs];
    std::unique_ptr<FilterPanel> filters[numFilters];
    FilterControlPanel filterCtl;
    AmpEnvPanel ampEnv;
    VoicePanel voice;
    ModPage modPage;
    ArpPage arpPage;
    FxPage fxPage;
    juce::TextButton modTab, arpTab, fxTab;

    juce::ComboBox presetBox;
    juce::TextButton prevButton, nextButton, saveButton, loadButton;
    juce::Label voiceCount;
    juce::Array<juce::File> userPresets;
    juce::String shownPresetName;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::MidiKeyboardComponent keyboard;
};

//==============================================================================
LumoraSynthEditor::LumoraSynthEditor (LumoraSynthProcessor& p)
    : AudioProcessorEditor (&p)
{
    setLookAndFeel (&lookAndFeel);
    mainPanel = std::make_unique<MainPanel> (p);
    addAndMakeVisible (*mainPanel);

    setResizable (true, true);
    setResizeLimits (designWidth / 2, designHeight / 2, designWidth * 2, designHeight * 2);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) designWidth / (double) designHeight);
    setSize (designWidth, designHeight);
}

LumoraSynthEditor::~LumoraSynthEditor()
{
    mainPanel = nullptr;
    setLookAndFeel (nullptr);
}

void LumoraSynthEditor::paint (juce::Graphics& g)
{
    g.fillAll (colours::background);
}

void LumoraSynthEditor::resized()
{
    const float scale = (float) getWidth() / (float) designWidth;
    mainPanel->setBounds (0, 0, designWidth, designHeight);
    mainPanel->setTransform (juce::AffineTransform::scale (scale));
}
