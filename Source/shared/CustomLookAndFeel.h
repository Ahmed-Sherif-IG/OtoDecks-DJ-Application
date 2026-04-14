#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 panelColourValue      = 0xFF0B1018;
    static constexpr juce::uint32 panelAltColourValue   = 0xFF151D29;
    static constexpr juce::uint32 panelRaisedColourValue= 0xFF1B2636;
    static constexpr juce::uint32 outlineColourValue    = 0xFF2D3C51;
    static constexpr juce::uint32 textColourValue       = 0xFFF4F7FC;
    static constexpr juce::uint32 mutedTextColourValue  = 0xFF8EA0B9;
    static constexpr juce::uint32 accentBlueValue       = 0xFF49A3FF;
    static constexpr juce::uint32 accentOrangeValue     = 0xFFFFB53D;
    static constexpr juce::uint32 accentGreenValue      = 0xFF31E38A;
    static constexpr juce::uint32 accentRedValue        = 0xFFFF5D73;

    CustomLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, colour(panelColourValue));
        setColour(juce::TextButton::buttonColourId, colour(panelRaisedColourValue));
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
        setColour(juce::TextEditor::backgroundColourId, colour(panelRaisedColourValue));
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

        juce::DropShadow shadow(juce::Colours::black.withAlpha(0.35f), 8, { 0, 3 });
        shadow.drawForRectangle(g, bounds.toNearestInt());

        juce::ColourGradient fill(baseColour.brighter(0.10f), bounds.getTopLeft(),
                                  baseColour.darker(0.12f), bounds.getBottomLeft(), false);
        g.setGradientFill(fill);
        g.fillRoundedRectangle(bounds, 9.0f);

        g.setColour(juce::Colours::white.withAlpha(button.isEnabled() ? 0.025f : 0.015f));
        g.drawRoundedRectangle(bounds.reduced(0.8f), 8.0f, 1.0f);

        g.setColour(baseColour.brighter(button.hasKeyboardFocus(true) ? 0.28f : 0.16f).withAlpha(0.92f));
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
            g.setColour(colour(outlineColourValue).darker(0.2f));
            g.fillRoundedRectangle(track, 3.0f);

            auto active = track.withWidth(juce::jmax(8.0f, sliderPos - track.getX()));
            juce::ColourGradient activeFill(slider.findColour(juce::Slider::trackColourId).brighter(0.2f), active.getTopLeft(),
                                            slider.findColour(juce::Slider::trackColourId).darker(0.15f), active.getTopRight(), false);
            g.setGradientFill(activeFill);
            g.fillRoundedRectangle(active, 3.0f);

            g.setColour(juce::Colours::black.withAlpha(0.35f));
            g.fillEllipse(sliderPos - 8.0f, track.getCentreY() - 7.0f, 16.0f, 16.0f);
            g.setColour(colour(textColourValue));
            g.fillEllipse(sliderPos - 8.0f, track.getCentreY() - 8.0f, 16.0f, 16.0f);
            return;
        }

        if (style == juce::Slider::LinearVertical)
        {
            auto track = bounds.withWidth(8.0f).withCentre(bounds.getCentre());
            g.setColour(colour(outlineColourValue).darker(0.2f));
            g.fillRoundedRectangle(track, 4.0f);

            auto active = juce::Rectangle<float>(track.getX(), sliderPos,
                                                 track.getWidth(), track.getBottom() - sliderPos);
            juce::Colour activeColour = slider.findColour(juce::Slider::trackColourId);
            juce::ColourGradient activeFill(activeColour.brighter(0.18f), active.getTopLeft(),
                                            activeColour.darker(0.18f), active.getBottomLeft(), false);
            g.setGradientFill(activeFill);
            g.fillRoundedRectangle(active, 4.0f);

            g.setColour(juce::Colours::black.withAlpha(0.35f));
            g.fillEllipse(track.getCentreX() - 10.0f, sliderPos - 9.0f, 20.0f, 20.0f);
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
