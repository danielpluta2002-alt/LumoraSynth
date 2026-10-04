#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace lumora::gui
{
namespace colours
{
    const juce::Colour background   { 0xff121419 };
    const juce::Colour panel        { 0xff1c2027 };
    const juce::Colour panelHeader  { 0xff242a33 };
    const juce::Colour outline      { 0xff2f3641 };
    const juce::Colour knobBody     { 0xff2a3039 };
    const juce::Colour track        { 0xff38404c };
    const juce::Colour text         { 0xffdde2ea };
    const juce::Colour textDim      { 0xff8b94a3 };
    const juce::Colour partA        { 0xff38c9c0 };
    const juce::Colour partB        { 0xfff29a3a };
    const juce::Colour modulation   { 0xffa77ef2 };
    const juce::Colour effects      { 0xff5aa6ff };
    const juce::Colour global       { 0xffe8d25a };
}

juce::Font uiFont (float height, bool bold = false);

class LumoraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    LumoraLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& background,
                               bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                       int buttonW, int buttonH, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    juce::Font getPopupMenuFont() override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getSliderPopupFont (juce::Slider&) override;
    int getSliderPopupPlacement (juce::Slider&) override;
};
} // namespace lumora::gui
