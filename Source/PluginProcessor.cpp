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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"

namespace
{
    const juce::Identifier presetNameId { "presetName" };
    constexpr const char* presetFileExtension = ".lumora";
}

LumoraSynthProcessor::LumoraSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      state (*this, nullptr, "LumoraSynth", lumora::createParameterLayout()),
      params (state)
{
    state.state.setProperty (presetNameId, "Init", nullptr);
}

bool LumoraSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void LumoraSynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    arp.prepare (sampleRate);
    effects.prepare (sampleRate, samplesPerBlock);
    masterGain.reset (sampleRate, 0.02);
    keyboardState.reset();
}

void LumoraSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const int numSamples = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    float masterDb = -6.0f;
    params.fill (voiceParams, arpParams, fxParams, masterDb);

    lumora::dsp::Arpeggiator::Transport transport;
    if (auto* ph = getPlayHead())
    {
        if (const auto pos = ph->getPosition())
        {
            if (const auto bpm = pos->getBpm())
                transport.bpm = *bpm;
            if (const auto ppq = pos->getPpqPosition())
            {
                transport.ppqPosition = *ppq;
                transport.hasPosition = true;
            }
            transport.isPlaying = pos->getIsPlaying();
        }
    }
    voiceParams.bpm = transport.bpm;
    fxParams.bpm = transport.bpm;

    arp.process (midi, numSamples, arpParams, transport);
    engine.process (buffer, midi, voiceParams);
    effects.process (buffer, fxParams);

    masterGain.setTargetValue (masterDb <= -47.9f ? 0.0f : juce::Decibels::decibelsToGain (masterDb));
    if (buffer.getNumChannels() > 1)
    {
        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);
        for (int i = 0; i < numSamples; ++i)
        {
            const float g = masterGain.getNextValue();
            l[i] *= g;
            r[i] *= g;
        }
    }
    else
    {
        masterGain.applyGain (buffer, numSamples);
    }

    activeVoices = engine.getNumActiveVoices();
    midi.clear();
}

//==============================================================================
int LumoraSynthProcessor::getNumPrograms()
{
    return (int) lumora::getFactoryPresets().size();
}

void LumoraSynthProcessor::setCurrentProgram (int index)
{
    const auto& presets = lumora::getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    currentProgram = index;
    lumora::applyFactoryPreset (state, presets[(size_t) index]);
    setPresetName (presets[(size_t) index].name);
}

const juce::String LumoraSynthProcessor::getProgramName (int index)
{
    const auto& presets = lumora::getFactoryPresets();
    return juce::isPositiveAndBelow (index, (int) presets.size()) ? presets[(size_t) index].name : juce::String();
}

juce::String LumoraSynthProcessor::getPresetName() const
{
    return state.state.getProperty (presetNameId, "Init").toString();
}

void LumoraSynthProcessor::setPresetName (const juce::String& name)
{
    state.state.setProperty (presetNameId, name, nullptr);
}

//==============================================================================
void LumoraSynthProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto copy = state.copyState();
    if (auto xml = copy.createXml())
        copyXmlToBinary (*xml, destData);
}

void LumoraSynthProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (state.state.getType()))
            state.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::File LumoraSynthProcessor::getUserPresetFolder()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                   .getChildFile ("Lumora Synth").getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

bool LumoraSynthProcessor::saveUserPreset (const juce::File& file)
{
    auto target = file.hasFileExtension (presetFileExtension) ? file : file.withFileExtension (presetFileExtension);
    setPresetName (target.getFileNameWithoutExtension());

    auto copy = state.copyState();
    if (auto xml = copy.createXml())
        return xml->writeTo (target);
    return false;
}

bool LumoraSynthProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName (state.state.getType()))
        return false;

    state.replaceState (juce::ValueTree::fromXml (*xml));
    setPresetName (file.getFileNameWithoutExtension());
    return true;
}

//==============================================================================
juce::AudioProcessorEditor* LumoraSynthProcessor::createEditor()
{
    return new LumoraSynthEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LumoraSynthProcessor();
}
