#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

namespace lumora::gui
{
using APVTS = juce::AudioProcessorValueTreeState;

/** Rotary knob with a caption underneath; the value shows in a popup while dragging. */
class Knob : public juce::Component
{
public:
    Knob (APVTS& state, const juce::String& paramId, const juce::String& caption, juce::Colour accent);

    void resized() override;

    juce::Slider slider;
    juce::Label label;

    static constexpr int width = 54;
    static constexpr int height = 62;

private:
    std::unique_ptr<APVTS::SliderAttachment> attachment;
};

/** Horizontal or vertical bar slider showing its value as text (mod amounts, arp steps). */
class BarSlider : public juce::Component
{
public:
    BarSlider (APVTS& state, const juce::String& paramId, juce::Colour accent, bool vertical);

    void resized() override { slider.setBounds (getLocalBounds()); }

    juce::Slider slider;

private:
    std::unique_ptr<APVTS::SliderAttachment> attachment;
};

/** Combo box bound to a choice parameter, with an optional caption on top. */
class ChoiceBox : public juce::Component
{
public:
    ChoiceBox (APVTS& state, const juce::String& paramId, const juce::String& caption = {});

    void resized() override;

    juce::ComboBox box;
    juce::Label label;

private:
    std::unique_ptr<APVTS::ComboBoxAttachment> attachment;
};

/** Toggle button bound to a bool parameter. */
class Toggle : public juce::Component
{
public:
    Toggle (APVTS& state, const juce::String& paramId, const juce::String& text, juce::Colour accent);

    void resized() override { button.setBounds (getLocalBounds()); }

    juce::ToggleButton button;

private:
    std::unique_ptr<APVTS::ButtonAttachment> attachment;
};

/** Rounded panel with a title bar. Children are positioned by the owner. */
class Section : public juce::Component
{
public:
    Section (const juce::String& title, juce::Colour accent);

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Area below the title bar. */
    juce::Rectangle<int> getContentArea() const;

    /** Optional component placed at the right end of the title bar (e.g. an On switch). */
    void setHeaderComponent (juce::Component* c);

    juce::Colour getAccent() const noexcept { return accent; }

    static constexpr int headerHeight = 22;

private:
    void placeHeaderComponent();

    juce::String title;
    juce::Colour accent;
    juce::Component* headerComponent = nullptr;
};

/** Places components left-to-right in a row of fixed-size cells, centred in @p area. */
void layoutRow (juce::Rectangle<int> area, std::initializer_list<juce::Component*> items, int cellWidth, int gap = 2);
} // namespace lumora::gui
