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

#include "LookAndFeel.h"

namespace lumora::gui
{
juce::Font uiFont (float height, bool bold)
{
    return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain));
}

LumoraLookAndFeel::LumoraLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, colours::background);
    setColour (juce::Label::textColourId, colours::text);
    setColour (juce::Slider::textBoxTextColourId, colours::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::rotarySliderFillColourId, colours::partA);
    setColour (juce::Slider::trackColourId, colours::partA);
    setColour (juce::ComboBox::backgroundColourId, colours::knobBody);
    setColour (juce::ComboBox::textColourId, colours::text);
    setColour (juce::ComboBox::outlineColourId, colours::outline);
    setColour (juce::ComboBox::arrowColourId, colours::textDim);
    setColour (juce::PopupMenu::backgroundColourId, colours::panel);
    setColour (juce::PopupMenu::textColourId, colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, colours::partA.withAlpha (0.35f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::PopupMenu::headerTextColourId, colours::textDim);
    setColour (juce::TextButton::buttonColourId, colours::knobBody);
    setColour (juce::TextButton::textColourOffId, colours::text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::BubbleComponent::backgroundColourId, colours::panelHeader);
    setColour (juce::BubbleComponent::outlineColourId, colours::outline);
    setColour (juce::TooltipWindow::textColourId, colours::text);
    setColour (juce::AlertWindow::backgroundColourId, colours::panel);
    setColour (juce::AlertWindow::textColourId, colours::text);
}

void LumoraLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          float startAngle, float endAngle, juce::Slider& slider)
{
    const auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float trackWidth = juce::jmax (2.5f, radius * 0.16f);
    const float arcRadius = radius - trackWidth * 0.5f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Track
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour (colours::track);
    g.strokePath (track, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc: bipolar parameters grow from the centre.
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float fromAngle = bipolar ? startAngle + (float) slider.valueToProportionOfLength (0.0) * (endAngle - startAngle)
                                    : startAngle;
    if (std::abs (angle - fromAngle) > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             juce::jmin (fromAngle, angle), juce::jmax (fromAngle, angle), true);
        g.setColour (slider.isEnabled() ? accent : accent.withSaturation (0.1f));
        g.strokePath (value, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Body
    const float bodyRadius = arcRadius - trackWidth * 1.1f;
    g.setGradientFill (juce::ColourGradient (colours::knobBody.brighter (0.25f), centre.x, centre.y - bodyRadius,
                                             colours::knobBody.darker (0.35f), centre.x, centre.y + bodyRadius, false));
    g.fillEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.drawEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);

    // Pointer
    juce::Path pointer;
    const float pointerLength = bodyRadius * 0.75f;
    pointer.addRoundedRectangle (-1.25f, -bodyRadius + 2.0f, 2.5f, pointerLength, 1.2f);
    g.setColour (colours::text);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void LumoraLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
                                          juce::Slider& slider)
{
    const auto accent = slider.findColour (juce::Slider::trackColourId);
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();

    if (style == juce::Slider::LinearBar || style == juce::Slider::LinearBarVertical)
    {
        g.setColour (colours::knobBody);
        g.fillRoundedRectangle (bounds, 3.0f);

        const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
        juce::Rectangle<float> fill;

        if (style == juce::Slider::LinearBar)
        {
            const float zero = bipolar ? (float) x + (float) slider.valueToProportionOfLength (0.0) * (float) width : (float) x;
            fill = juce::Rectangle<float>::leftTopRightBottom (juce::jmin (zero, sliderPos), bounds.getY(),
                                                               juce::jmax (zero, sliderPos), bounds.getBottom());
        }
        else
        {
            const float zero = bipolar ? (float) (y + height) - (float) slider.valueToProportionOfLength (0.0) * (float) height
                                       : (float) (y + height);
            fill = juce::Rectangle<float>::leftTopRightBottom (bounds.getX(), juce::jmin (zero, sliderPos),
                                                               bounds.getRight(), juce::jmax (zero, sliderPos));
        }

        g.setColour (accent.withAlpha (slider.isEnabled() ? 0.75f : 0.25f));
        g.fillRoundedRectangle (fill, 3.0f);

        if (bipolar)
        {
            g.setColour (colours::textDim.withAlpha (0.6f));
            if (style == juce::Slider::LinearBar)
            {
                const float zx = (float) x + (float) slider.valueToProportionOfLength (0.0) * (float) width;
                g.drawVerticalLine ((int) zx, bounds.getY(), bounds.getBottom());
            }
            else
            {
                const float zy = (float) (y + height) - (float) slider.valueToProportionOfLength (0.0) * (float) height;
                g.drawHorizontalLine ((int) zy, bounds.getX(), bounds.getRight());
            }
        }

        g.setColour (colours::outline);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);
        return;
    }

    LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

void LumoraLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool)
{
    const auto accent = button.findColour (juce::ToggleButton::tickColourId);
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = button.getToggleState();

    g.setColour (on ? accent.withAlpha (0.85f) : colours::knobBody.brighter (highlighted ? 0.12f : 0.0f));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (on ? accent.brighter (0.3f) : colours::outline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    g.setColour (on ? juce::Colours::black.withAlpha (0.85f) : colours::textDim);
    g.setFont (uiFont (juce::jmin (12.0f, bounds.getHeight() * 0.62f), true));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred, false);
}

void LumoraLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& background,
                                              bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto c = background;
    if (down) c = c.brighter (0.2f);
    else if (highlighted) c = c.brighter (0.1f);
    g.setColour (c);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (colours::outline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

juce::Font LumoraLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return uiFont (juce::jmin (13.0f, (float) buttonHeight * 0.55f), true);
}

void LumoraLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (box.hasKeyboardFocus (true) ? colours::textDim : box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    juce::Path arrow;
    const float ax = (float) width - 11.0f, ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

juce::Font LumoraLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return uiFont (juce::jmin (13.0f, (float) box.getHeight() * 0.58f));
}

void LumoraLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (4, 1, box.getWidth() - 20, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

juce::Font LumoraLookAndFeel::getPopupMenuFont() { return uiFont (14.0f); }
juce::Font LumoraLookAndFeel::getLabelFont (juce::Label& label) { return label.getFont(); }
juce::Font LumoraLookAndFeel::getSliderPopupFont (juce::Slider&) { return uiFont (13.0f, true); }
int LumoraLookAndFeel::getSliderPopupPlacement (juce::Slider&) { return juce::BubbleComponent::above; }
} // namespace lumora::gui
