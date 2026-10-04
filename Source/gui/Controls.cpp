#include "Controls.h"

namespace lumora::gui
{
Knob::Knob (APVTS& state, const juce::String& paramId, const juce::String& caption, juce::Colour accent)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setPopupDisplayEnabled (true, true, nullptr, 1200);
    slider.setMouseDragSensitivity (180);
    addAndMakeVisible (slider);

    label.setText (caption, juce::dontSendNotification);
    label.setFont (uiFont (11.0f, true));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, colours::textDim);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    auto* param = state.getParameter (paramId);
    jassert (param != nullptr);
    attachment = std::make_unique<APVTS::SliderAttachment> (state, paramId, slider);
    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
        slider.setDoubleClickReturnValue (true, ranged->convertFrom0to1 (ranged->getDefaultValue()));
    slider.setTooltip (param->getName (64));
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromBottom (14));
    slider.setBounds (r);
}

//==============================================================================
BarSlider::BarSlider (APVTS& state, const juce::String& paramId, juce::Colour accent, bool vertical)
{
    slider.setSliderStyle (vertical ? juce::Slider::LinearBarVertical : juce::Slider::LinearBar);
    slider.setColour (juce::Slider::trackColourId, accent);
    slider.setColour (juce::Slider::textBoxTextColourId, colours::text);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxHighlightColourId, juce::Colours::transparentBlack);
    slider.setTextBoxIsEditable (false);
    addAndMakeVisible (slider);

    auto* param = state.getParameter (paramId);
    jassert (param != nullptr);
    attachment = std::make_unique<APVTS::SliderAttachment> (state, paramId, slider);
    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
        slider.setDoubleClickReturnValue (true, ranged->convertFrom0to1 (ranged->getDefaultValue()));
    slider.setTooltip (param->getName (64));
}

//==============================================================================
ChoiceBox::ChoiceBox (APVTS& state, const juce::String& paramId, const juce::String& caption)
{
    auto* param = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (paramId));
    jassert (param != nullptr);
    box.addItemList (param->choices, 1);
    box.setTooltip (param->getName (64));
    addAndMakeVisible (box);

    if (caption.isNotEmpty())
    {
        label.setText (caption, juce::dontSendNotification);
        label.setFont (uiFont (11.0f, true));
        label.setColour (juce::Label::textColourId, colours::textDim);
        label.setJustificationType (juce::Justification::centredLeft);
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);
    }

    attachment = std::make_unique<APVTS::ComboBoxAttachment> (state, paramId, box);
}

void ChoiceBox::resized()
{
    auto r = getLocalBounds();
    if (label.isVisible() && label.getText().isNotEmpty())
        label.setBounds (r.removeFromTop (14));
    box.setBounds (r.withSizeKeepingCentre (r.getWidth(), juce::jmin (r.getHeight(), 24)));
}

//==============================================================================
Toggle::Toggle (APVTS& state, const juce::String& paramId, const juce::String& text, juce::Colour accent)
{
    button.setButtonText (text);
    button.setColour (juce::ToggleButton::tickColourId, accent);
    if (auto* param = state.getParameter (paramId))
        button.setTooltip (param->getName (64));
    addAndMakeVisible (button);
    attachment = std::make_unique<APVTS::ButtonAttachment> (state, paramId, button);
}

//==============================================================================
Section::Section (const juce::String& t, juce::Colour a) : title (t), accent (a) {}

void Section::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (colours::panel);
    g.fillRoundedRectangle (bounds, 6.0f);

    auto header = bounds.withHeight ((float) headerHeight);
    juce::Path headerPath;
    headerPath.addRoundedRectangle (header.getX(), header.getY(), header.getWidth(), header.getHeight(),
                                    6.0f, 6.0f, true, true, false, false);
    g.setColour (colours::panelHeader);
    g.fillPath (headerPath);

    g.setColour (accent);
    g.fillRect (header.getX() + 10.0f, header.getBottom() - 2.0f, 28.0f, 2.0f);

    g.setColour (colours::text);
    g.setFont (uiFont (12.0f, true));
    g.drawText (title.toUpperCase(), header.reduced (10.0f, 0.0f).toNearestInt(), juce::Justification::centredLeft, false);

    g.setColour (colours::outline);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
}

void Section::resized()
{
    placeHeaderComponent();
}

void Section::placeHeaderComponent()
{
    if (headerComponent != nullptr)
        headerComponent->setBounds (getWidth() - 50, 3, 44, headerHeight - 6);
}

juce::Rectangle<int> Section::getContentArea() const
{
    return getLocalBounds().withTrimmedTop (headerHeight).reduced (6, 4);
}

void Section::setHeaderComponent (juce::Component* c)
{
    headerComponent = c;
    if (c != nullptr)
        addAndMakeVisible (c);
    placeHeaderComponent();
}

//==============================================================================
void layoutRow (juce::Rectangle<int> area, std::initializer_list<juce::Component*> items, int cellWidth, int gap)
{
    const int count = (int) items.size();
    const int total = count * cellWidth + (count - 1) * gap;
    int x = area.getX() + juce::jmax (0, (area.getWidth() - total) / 2);
    for (auto* c : items)
    {
        if (c != nullptr)
            c->setBounds (x, area.getY(), cellWidth, area.getHeight());
        x += cellWidth + gap;
    }
}
} // namespace lumora::gui
