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
