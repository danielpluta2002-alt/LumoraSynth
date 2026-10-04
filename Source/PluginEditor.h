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
