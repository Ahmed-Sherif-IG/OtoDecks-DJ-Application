#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 panelColourValue      = 0xFF161A22;
    static constexpr juce::uint32 panelAltColourValue   = 0xFF1C212B;
    static constexpr juce::uint32 outlineColourValue    = 0xFF31394A;
    static constexpr juce::uint32 textColourValue       = 0xFFF3F6FB;
    static constexpr juce::uint32 mutedTextColourValue  = 0xFF93A0B8;
    static constexpr juce::uint32 accentBlueValue       = 0xFF3A7BD5;
    static constexpr juce::uint32 accentOrangeValue     = 0xFFF5A623;
    static constexpr juce::uint32 accentGreenValue      = 0xFF2BD97F;
    static constexpr juce::uint32 accentRedValue        = 0xFFFF5D73;

    CustomLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, colour(panelColourValue));
        setColour(juce::TextButton::buttonColourId, colour(panelAltColourValue));
        setColour(juce::TextButton::buttonOnColourId, colour(accentBlueValue));
        setColour(juce::TextButton::textColourOffId, colour(textColourValue));
        setColour(juce::TextButton::textColourOnId, colour(textColourValue));
        setColour(juce::Slider::thumbColourId, colour(textColourValue));
        setColour(juce::Slider::trackColourId, colour(accentBlueValue));
        setColour(juce::Slider::rotarySliderFillColourId, colour(accentBlueValue));
        setColour(juce::Slider::rotarySliderOutlineColourId, colour(outlineColourValue));
        setColour(juce::Label::textColourId, colour(textColourValue));
        setColour(juce::ListBox::backgroundColourId, colour(panelColourValue));
        setColour(juce::TableHeaderComponent::backgroundColourId, colour(panelAltColourValue));
        setColour(juce::TableHeaderComponent::textColourId, colour(textColourValue));
        setColour(juce::TextEditor::backgroundColourId, colour(panelAltColourValue));
        setColour(juce::TextEditor::textColourId, colour(textColourValue));
        setColour(juce::TextEditor::outlineColourId, colour(outlineColourValue));
        setColour(juce::CaretComponent::caretColourId, colour(textColourValue));
    }

    static juce::Colour colour(juce::uint32 argb) { return juce::Colour(argb); }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font(juce::FontOptions(static_cast<float>(juce::jmax(13, buttonHeight / 3 + 3))).withStyle("Bold"));
    }

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
    {
        return juce::LookAndFeel_V4::getTypefaceForFont(font);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto baseColour = backgroundColour;

        if (!button.isEnabled())
            baseColour = baseColour.darker(0.35f).withAlpha(0.55f);
        else if (button.getToggleState())
            baseColour = baseColour.brighter(0.18f);
        else if (isButtonDown)
            baseColour = baseColour.brighter(0.12f);
        else if (isMouseOverButton)
            baseColour = baseColour.brighter(0.05f);

        juce::ColourGradient fill(baseColour.brighter(0.15f), bounds.getTopLeft(),
                                  baseColour.darker(0.18f), bounds.getBottomRight(), false);
        g.setGradientFill(fill);
        g.fillRoundedRectangle(bounds, 9.0f);

        g.setColour(baseColour.brighter(button.hasKeyboardFocus(true) ? 0.35f : 0.18f).withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 9.0f, button.getToggleState() ? 2.0f : 1.0f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height));

        if (style == juce::Slider::LinearHorizontal)
        {
            auto track = bounds.withHeight(6.0f).withCentre(bounds.getCentre());
            g.setColour(colour(outlineColourValue));
            g.fillRoundedRectangle(track, 3.0f);

            auto active = track.withWidth(juce::jmax(8.0f, sliderPos - track.getX()));
            g.setColour(slider.findColour(juce::Slider::trackColourId).brighter(0.15f));
            g.fillRoundedRectangle(active, 3.0f);

            g.setColour(colour(textColourValue));
            g.fillEllipse(sliderPos - 8.0f, track.getCentreY() - 8.0f, 16.0f, 16.0f);
            return;
        }

        if (style == juce::Slider::LinearVertical)
        {
            auto track = bounds.withWidth(8.0f).withCentre(bounds.getCentre());
            g.setColour(colour(outlineColourValue));
            g.fillRoundedRectangle(track, 4.0f);

            auto active = juce::Rectangle<float>(track.getX(), sliderPos,
                                                 track.getWidth(), track.getBottom() - sliderPos);
            juce::Colour activeColour = slider.findColour(juce::Slider::trackColourId);
            g.setColour(activeColour.withAlpha(0.95f));
            g.fillRoundedRectangle(active, 4.0f);

            g.setColour(colour(textColourValue));
            g.fillEllipse(track.getCentreX() - 10.0f, sliderPos - 10.0f, 20.0f, 20.0f);
            return;
        }

        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
                                               sliderPos, minSliderPos, maxSliderPos,
                                               style, slider);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        auto bounds = label.getLocalBounds();
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());
        g.drawFittedText(label.getText(), bounds.reduced(1, 0),
                         label.getJustificationType(), 1, 0.95f);
    }

    void drawTableHeaderBackground(juce::Graphics& g, juce::TableHeaderComponent& header) override
    {
        auto area = header.getLocalBounds().toFloat();
        juce::ColourGradient gradient(colour(panelAltColourValue).brighter(0.08f), area.getTopLeft(),
                                      colour(panelAltColourValue).darker(0.2f), area.getBottomLeft(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(area.reduced(0.5f), 8.0f);

        g.setColour(colour(outlineColourValue).brighter(0.12f));
        g.drawRoundedRectangle(area.reduced(0.5f), 8.0f, 1.0f);
    }

    void drawTableHeaderColumn(juce::Graphics& g, juce::TableHeaderComponent&, const juce::String& columnName,
                               int, int width, int height, bool isMouseOver, bool isMouseDown,
                               int columnFlags) override
    {
        auto area = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(4.0f, 2.0f);
        auto textColour = colour(textColourValue).withAlpha(isMouseDown ? 1.0f : (isMouseOver ? 0.95f : 0.88f));
        g.setColour(textColour);
        g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
        g.drawText(columnName.toUpperCase(), area.toNearestInt(), juce::Justification::centredLeft, true);

        if ((columnFlags & juce::TableHeaderComponent::sortedForwards) != 0
            || (columnFlags & juce::TableHeaderComponent::sortedBackwards) != 0)
        {
            juce::Path arrow;
            const float cx = area.getRight() - 10.0f;
            const float cy = area.getCentreY();
            const bool forwards = (columnFlags & juce::TableHeaderComponent::sortedForwards) != 0;
            if (forwards)
                arrow.addTriangle(cx - 4.0f, cy + 2.5f, cx + 4.0f, cy + 2.5f, cx, cy - 3.5f);
            else
                arrow.addTriangle(cx - 4.0f, cy - 2.5f, cx + 4.0f, cy - 2.5f, cx, cy + 3.5f);
            g.fillPath(arrow);
        }
    }
};
