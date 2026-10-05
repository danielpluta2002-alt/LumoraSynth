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
#include "PluginProcessor.h"
#include "gui/LookAndFeel.h"

class MainPanel;

class LumoraSynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit LumoraSynthEditor (LumoraSynthProcessor&);
    ~LumoraSynthEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int designWidth = 1320;
    static constexpr int designHeight = 860;

private:
    lumora::gui::LumoraLookAndFeel lookAndFeel;
    std::unique_ptr<MainPanel> mainPanel;
    juce::TooltipWindow tooltips { this, 700 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumoraSynthEditor)
};
