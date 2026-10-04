#include "SynthEngine.h"

namespace lumora::dsp
{
void SynthEngine::prepare (double newSampleRate, int)
{
    sampleRate = newSampleRate;
    for (auto& v : voices)
        v.prepare (sampleRate);
    held.reserve (128);
    sustained.reserve (128);
    reset();
}

void SynthEngine::reset()
{
    for (auto& v : voices)
        v.forceIdle();
    held.clear();
    sustained.clear();
    sustainDown = false;
    pitchBend = 0.0f;
    expression = {};
    lastNotePitch = -1.0f;
}

int SynthEngine::getNumActiveVoices() const
{
    int n = 0;
    for (const auto& v : voices)
        n += v.isActive() ? 1 : 0;
    return n;
}

void SynthEngine::process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi, const VoiceParams& p)
{
    if (p.voiceMode != lastVoiceMode)
    {
        allNotesOff (true);
        lastVoiceMode = p.voiceMode;
    }

    expression.bendSemis = pitchBend * (float) p.bendRange;

    int pos = 0;
    for (const auto meta : midi)
    {
        const int at = juce::jlimit (0, buffer.getNumSamples(), meta.samplePosition);
        if (at > pos)
        {
            render (buffer, pos, at - pos, p);
            pos = at;
        }
        handleMidi (meta.getMessage(), p);
    }

    if (pos < buffer.getNumSamples())
        render (buffer, pos, buffer.getNumSamples() - pos, p);

    // Free-running LFO phases: new notes pick these up when "Free" is enabled.
    for (int i = 0; i < 2; ++i)
    {
        const auto& lp = p.lfo[i];
        const double rate = lp.sync ? p.bpm / 60.0 / syncDivisionBeats (lp.division) : (double) lp.rate;
        freeLfoPhase[i] = (float) std::fmod ((double) freeLfoPhase[i] + rate * buffer.getNumSamples() / sampleRate, 1.0);
    }
}

void SynthEngine::render (juce::AudioBuffer<float>& buffer, int start, int num, const VoiceParams& p)
{
    auto* left = buffer.getWritePointer (0) + start;
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) + start : nullptr;

    if (right == nullptr)
    {
        // Mono output: render stereo into a scratch buffer would be cleaner, but hosts
        // practically always give a synth two channels. Sum both sides into channel 0.
        float scratch[256];
        int done = 0;
        while (done < num)
        {
            const int n = juce::jmin (128, num - done);
            std::fill (scratch, scratch + 256, 0.0f);
            for (auto& v : voices)
                v.render (scratch, scratch + 128, n, p, expression);
            for (int i = 0; i < n; ++i)
                left[done + i] += 0.5f * (scratch[i] + scratch[128 + i]);
            done += n;
        }
        return;
    }

    for (auto& v : voices)
        v.render (left, right, num, p, expression);
}

void SynthEngine::handleMidi (const juce::MidiMessage& m, const VoiceParams& p)
{
    if (m.isNoteOn())
    {
        noteOn (m.getNoteNumber(), m.getFloatVelocity(), p);
    }
    else if (m.isNoteOff())
    {
        noteOff (m.getNoteNumber(), p);
    }
    else if (m.isPitchWheel())
    {
        pitchBend = juce::jlimit (-1.0f, 1.0f, (float) (m.getPitchWheelValue() - 8192) / 8192.0f);
        expression.bendSemis = pitchBend * (float) p.bendRange;
    }
    else if (m.isChannelPressure())
    {
        expression.aftertouch = (float) m.getChannelPressureValue() / 127.0f;
    }
    else if (m.isAftertouch())
    {
        expression.aftertouch = (float) m.getAfterTouchValue() / 127.0f;
    }
    else if (m.isController())
    {
        const int cc = m.getControllerNumber();
        const int value = m.getControllerValue();

        if (cc == 1)
        {
            expression.modWheel = (float) value / 127.0f;
        }
        else if (cc == 64)
        {
            sustainDown = value >= 64;
            if (! sustainDown)
            {
                auto pending = sustained;
                sustained.clear();
                for (int n : pending)
                    noteOff (n, p);
            }
        }
        else if (cc == 120)
        {
            allNotesOff (true);
        }
        else if (cc == 123)
        {
            allNotesOff (false);
        }
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        allNotesOff (m.isAllSoundOff());
    }
}

int SynthEngine::countSoundingVoices() const
{
    int n = 0;
    for (const auto& v : voices)
        if (v.isActive() && ! v.isKilling())
            ++n;
    return n;
}

Voice* SynthEngine::findFreeVoice()
{
    for (auto& v : voices)
        if (! v.isActive())
            return &v;

    // Every slot is busy (many voices fading out): reuse the oldest fading one.
    Voice* oldest = nullptr;
    for (auto& v : voices)
        if (v.isKilling() && (oldest == nullptr || v.getOrder() < oldest->getOrder()))
            oldest = &v;

    if (oldest == nullptr)
        oldest = &voices[0];

    oldest->forceIdle();
    return oldest;
}

Voice* SynthEngine::findVoiceToSteal()
{
    Voice* best = nullptr;

    // Prefer the oldest released voice, then the oldest held one.
    for (auto& v : voices)
        if (v.isActive() && ! v.isKilling() && v.isReleased() && (best == nullptr || v.getOrder() < best->getOrder()))
            best = &v;

    if (best == nullptr)
        for (auto& v : voices)
            if (v.isActive() && ! v.isKilling() && (best == nullptr || v.getOrder() < best->getOrder()))
                best = &v;

    return best;
}

void SynthEngine::startVoice (Voice& v, int note, float velocity, float glideFrom, const VoiceParams& p)
{
    seedCounter = seedCounter * 1664525u + 1013904223u;
    v.start (note, velocity, glideFrom, p, seedCounter, freeLfoPhase, ++noteCounter);
}

void SynthEngine::noteOn (int note, float velocity, const VoiceParams& p)
{
    held.erase (std::remove_if (held.begin(), held.end(), [note] (const HeldNote& h) { return h.note == note; }), held.end());
    sustained.erase (std::remove (sustained.begin(), sustained.end(), note), sustained.end());
    const bool otherKeysHeld = ! held.empty();
    held.push_back ({ note, velocity });

    if (p.voiceMode == modePoly)
    {
        const float glideFrom = lastNotePitch >= 0.0f ? lastNotePitch : (float) note;
        lastNotePitch = (float) note;

        // Same key again: retrigger the voice that still plays it.
        for (auto& v : voices)
        {
            if (v.isActive() && ! v.isKilling() && v.getNote() == note)
            {
                startVoice (v, note, velocity, glideFrom, p);
                return;
            }
        }

        while (countSoundingVoices() >= p.polyphony)
        {
            if (auto* victim = findVoiceToSteal())
                victim->kill();
            else
                break;
        }

        startVoice (*findFreeVoice(), note, velocity, glideFrom, p);
        return;
    }

    // Mono / legato: one voice, voices[0].
    auto& v = voices[0];
    const bool sounding = v.isActive() && ! v.isKilling();
    const float glideFrom = sounding ? v.getCurrentPitch() : (float) note;
    lastNotePitch = (float) note;

    if (p.voiceMode == modeLegato && sounding && otherKeysHeld)
        v.glideTo (note, ++noteCounter);
    else
        startVoice (v, note, velocity, glideFrom, p);
}

void SynthEngine::noteOff (int note, const VoiceParams& p)
{
    const auto it = std::find_if (held.begin(), held.end(), [note] (const HeldNote& h) { return h.note == note; });
    if (it == held.end())
        return;

    if (sustainDown)
    {
        if (std::find (sustained.begin(), sustained.end(), note) == sustained.end())
            sustained.push_back (note);
        return;
    }

    held.erase (it);

    if (p.voiceMode == modePoly)
    {
        for (auto& v : voices)
            if (v.isActive() && ! v.isKilling() && ! v.isReleased() && v.getNote() == note)
                v.release();
        return;
    }

    auto& v = voices[0];
    if (! v.isActive() || v.getNote() != note)
        return;

    if (! held.empty())
    {
        // Fall back to the most recently pressed key that is still down.
        const auto& back = held.back();
        if (p.voiceMode == modeLegato)
            v.glideTo (back.note, ++noteCounter);
        else
            startVoice (v, back.note, back.velocity, v.getCurrentPitch(), p);
    }
    else
    {
        v.release();
    }
}

void SynthEngine::allNotesOff (bool hard)
{
    held.clear();
    sustained.clear();
    for (auto& v : voices)
    {
        if (hard)
            v.kill();
        else if (v.isActive())
            v.release();
    }
}
} // namespace lumora::dsp
