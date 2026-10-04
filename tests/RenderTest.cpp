// Offline smoke test: renders every factory preset and checks the output is
// finite, audible and not clipping wildly. Also exercises the arpeggiator,
// voice modes, state save/restore and every modulation destination.

#include "PluginProcessor.h"
#include "Parameters.h"

#include <iostream>

namespace
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;

    struct Stats
    {
        float peak = 0.0f;
        double rms = 0.0;
        bool finite = true;
    };

    Stats render (LumoraSynthProcessor& proc, double seconds, const std::vector<int>& chord, double noteSeconds,
                  juce::AudioBuffer<float>* capture = nullptr)
    {
        Stats st;
        juce::AudioBuffer<float> buffer (2, blockSize);
        const int totalBlocks = (int) (seconds * sampleRate / blockSize);
        const int offBlock = (int) (noteSeconds * sampleRate / blockSize);
        double sumSq = 0.0;
        long long count = 0;

        if (capture != nullptr)
            capture->setSize (2, totalBlocks * blockSize);

        for (int b = 0; b < totalBlocks; ++b)
        {
            juce::MidiBuffer midi;
            if (b == 0)
                for (int n : chord)
                    midi.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 100), 10);
            if (b == offBlock)
                for (int n : chord)
                    midi.addEvent (juce::MidiMessage::noteOff (1, n), 5);

            proc.processBlock (buffer, midi);

            for (int ch = 0; ch < 2; ++ch)
            {
                const auto* d = buffer.getReadPointer (ch);
                for (int i = 0; i < blockSize; ++i)
                {
                    if (! std::isfinite (d[i]))
                        st.finite = false;
                    st.peak = juce::jmax (st.peak, std::abs (d[i]));
                    sumSq += (double) d[i] * d[i];
                    ++count;
                }
                if (capture != nullptr)
                    capture->copyFrom (ch, b * blockSize, buffer, ch, 0, blockSize);
            }
        }

        st.rms = std::sqrt (sumSq / (double) juce::jmax (1LL, count));
        return st;
    }

    void setParam (LumoraSynthProcessor& p, const juce::String& id, float value)
    {
        auto* param = p.getState().getParameter (id);
        jassert (param != nullptr);
        param->setValueNotifyingHost (param->convertTo0to1 (value));
    }

    int failures = 0;

    void check (bool ok, const juce::String& what)
    {
        std::cout << (ok ? "  ok    " : "  FAIL  ") << what << "\n";
        if (! ok) ++failures;
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    LumoraSynthProcessor proc;
    proc.setPlayConfigDetails (0, 2, sampleRate, blockSize);
    proc.prepareToPlay (sampleRate, blockSize);

    const juce::File outDir = argc > 1 ? juce::File (argv[1]) : juce::File();
    if (outDir != juce::File())
        outDir.createDirectory();

    std::cout << "Factory presets:\n";
    for (int i = 0; i < proc.getNumPrograms(); ++i)
    {
        proc.setCurrentProgram (i);
        proc.prepareToPlay (sampleRate, blockSize); // clears tails from the previous preset

        juce::AudioBuffer<float> audio;
        const auto st = render (proc, 3.0, { 48, 55, 60, 63 }, 1.5, outDir != juce::File() ? &audio : nullptr);
        std::cout << "  " << juce::String (proc.getProgramName (i)).paddedRight (' ', 18)
                  << " peak " << juce::String (st.peak, 3) << "  rms " << juce::String (st.rms, 4) << "\n";
        check (st.finite, "finite output");
        check (st.peak > 0.01f, "audible");
        check (st.peak < 4.0f, "no runaway level");

        if (outDir != juce::File())
        {
            auto file = outDir.getChildFile (juce::File::createLegalFileName (proc.getProgramName (i)) + ".wav");
            file.deleteFile();
            juce::WavAudioFormat wav;
            if (auto stream = std::unique_ptr<juce::OutputStream> (file.createOutputStream()))
                if (auto writer = std::unique_ptr<juce::AudioFormatWriter> (wav.createWriterFor (stream.get(), sampleRate, 2, 24, {}, 0)))
                {
                    stream.release();
                    writer->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples());
                }
        }
    }

    std::cout << "Silence after release:\n";
    {
        proc.setCurrentProgram (0);
        proc.prepareToPlay (sampleRate, blockSize);
        render (proc, 1.0, { 60 }, 0.3);
        const auto tail = render (proc, 0.5, {}, 0.0);
        check (tail.peak < 1.0e-4f, "voices end after release (peak " + juce::String (tail.peak, 6) + ")");
        check (proc.getActiveVoiceCount() == 0, "no active voices left");
    }

    std::cout << "Polyphony limit and stealing:\n";
    {
        proc.setCurrentProgram (0);
        setParam (proc, "poly", 4);
        proc.prepareToPlay (sampleRate, blockSize);
        render (proc, 0.3, { 48, 50, 52, 53, 55, 57, 59, 60 }, 10.0);
        check (proc.getActiveVoiceCount() <= 4, "at most 4 voices sound (" + juce::String (proc.getActiveVoiceCount()) + ")");
    }

    std::cout << "Mono / legato:\n";
    for (int mode : { lumora::modeMono, lumora::modeLegato })
    {
        proc.setCurrentProgram (0);
        setParam (proc, "voiceMode", (float) mode);
        setParam (proc, "porta", 0.1f);
        proc.prepareToPlay (sampleRate, blockSize);
        const auto st = render (proc, 1.0, { 48, 52, 55 }, 0.8);
        check (st.finite && st.peak > 0.01f, lumora::voiceModeNames()[mode] + " renders");
        check (proc.getActiveVoiceCount() <= 1, lumora::voiceModeNames()[mode] + " uses one voice");
    }

    std::cout << "Arpeggiator (all modes):\n";
    for (int mode = 0; mode < lumora::arpModeNames().size(); ++mode)
    {
        proc.setCurrentProgram (0);
        setParam (proc, "arp_on", 1.0f);
        setParam (proc, "arp_mode", (float) mode);
        setParam (proc, "arp_octaves", 2);
        setParam (proc, "arp_swing", 0.4f);
        setParam (proc, "arp_hold", mode % 2 == 0 ? 1.0f : 0.0f);
        proc.prepareToPlay (sampleRate, blockSize);
        const auto st = render (proc, 2.0, { 60, 64, 67 }, 1.0);
        check (st.finite && st.peak > 0.01f, lumora::arpModeNames()[mode] + " plays");
    }

    std::cout << "Every modulation destination with every source:\n";
    {
        bool allFinite = true;
        for (int dst = 1; dst < lumora::numModDests; ++dst)
        {
            proc.setCurrentProgram (1);
            for (int slot = 0; slot < lumora::numModSlots; ++slot)
            {
                const int src = 1 + slot % (lumora::numModSources - 1);
                setParam (proc, lumora::pid::mod (slot, "src"), (float) src);
                setParam (proc, lumora::pid::mod (slot, "dst"), (float) dst);
                setParam (proc, lumora::pid::mod (slot, "amt"), slot % 2 == 0 ? 1.0f : -1.0f);
            }
            proc.prepareToPlay (sampleRate, blockSize);
            const auto st = render (proc, 0.6, { 36, 72 }, 0.4);
            allFinite = allFinite && st.finite;
        }
        check (allFinite, "extreme modulation stays finite");
    }

    std::cout << "Extreme filter settings:\n";
    {
        proc.setCurrentProgram (0);
        for (int f = 0; f < 2; ++f)
        {
            setParam (proc, lumora::pid::filt (f, "reso"), 1.0f);
            setParam (proc, lumora::pid::filt (f, "drive"), 1.0f);
        }
        setParam (proc, "fctl_warm", 1.0f);
        bool ok = true;
        for (int type = 0; type < lumora::filterTypeNames().size(); ++type)
            for (float cutoff : { 20.0f, 1000.0f, 20000.0f })
            {
                setParam (proc, lumora::pid::filt (0, "type"), (float) type);
                setParam (proc, lumora::pid::filt (0, "cutoff"), cutoff);
                proc.prepareToPlay (sampleRate, blockSize);
                const auto st = render (proc, 0.4, { 24, 96 }, 0.3);
                ok = ok && st.finite && st.peak < 8.0f;
            }
        check (ok, "all filter types at full resonance stay bounded");
    }

    std::cout << "All effects on:\n";
    {
        proc.setCurrentProgram (1);
        for (auto* id : { "dist_on", "phaser_on", "chorus_on", "eq_on", "delay_on", "reverb_on", "comp_on" })
            setParam (proc, id, 1.0f);
        for (int type = 0; type < lumora::distTypeNames().size(); ++type)
        {
            setParam (proc, "dist_type", (float) type);
            proc.prepareToPlay (sampleRate, blockSize);
            const auto st = render (proc, 1.0, { 60, 67 }, 0.5);
            check (st.finite && st.peak > 0.01f && st.peak < 4.0f, "full chain with " + lumora::distTypeNames()[type]);
        }
    }

    std::cout << "State save / restore:\n";
    {
        proc.setCurrentProgram (5);
        setParam (proc, "fctl_cutoff", 12.0f);
        juce::MemoryBlock block;
        proc.getStateInformation (block);

        LumoraSynthProcessor other;
        other.setStateInformation (block.getData(), (int) block.getSize());
        const float restored = other.getState().getRawParameterValue ("fctl_cutoff")->load();
        check (std::abs (restored - 12.0f) < 0.01f, "parameter restored (" + juce::String (restored) + ")");
        check (other.getPresetName() == proc.getPresetName(), "preset name restored (" + other.getPresetName() + ")");
    }

    std::cout << "Editor:\n";
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
        check (editor != nullptr && editor->getWidth() > 0, "editor constructs");

        if (outDir != juce::File())
        {
            auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
            auto file = outDir.getChildFile ("editor.png");
            file.deleteFile();
            juce::PNGImageFormat png;
            if (auto stream = file.createOutputStream())
                png.writeImageToStream (image, *stream);

            // Snapshot the other tabs too.
            std::function<juce::TextButton* (juce::Component&, const juce::String&)> findButton;
            findButton = [&findButton] (juce::Component& c, const juce::String& text) -> juce::TextButton*
            {
                for (auto* child : c.getChildren())
                {
                    if (auto* b = dynamic_cast<juce::TextButton*> (child); b != nullptr && b->getButtonText() == text)
                        return b;
                    if (auto* found = findButton (*child, text))
                        return found;
                }
                return nullptr;
            };

            for (auto* tab : { "ARPEGGIATOR", "EFFECTS" })
            {
                if (auto* b = findButton (*editor, tab))
                {
                    b->setToggleState (true, juce::dontSendNotification);
                    b->onClick();
                    auto shot = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
                    auto tabFile = outDir.getChildFile (juce::String ("editor_") + tab + ".png");
                    tabFile.deleteFile();
                    if (auto s2 = tabFile.createOutputStream())
                        png.writeImageToStream (shot, *s2);
                }
            }
        }
    }

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nFAILURES: " + juce::String (failures).toStdString() + "\n");
    return failures == 0 ? 0 : 1;
}
