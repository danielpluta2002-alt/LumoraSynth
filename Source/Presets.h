#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace lumora
{
struct FactoryPreset
{
    juce::String name;
    juce::String category;
    std::vector<std::pair<juce::String, float>> values; // parameter id -> real value (choice index, 0/1 for bools)
};

const std::vector<FactoryPreset>& getFactoryPresets();

/** Resets every parameter to its default, then applies the preset's values. */
void applyFactoryPreset (juce::AudioProcessorValueTreeState& state, const FactoryPreset& preset);
} // namespace lumora
