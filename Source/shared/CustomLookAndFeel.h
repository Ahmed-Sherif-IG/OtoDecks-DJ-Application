#pragma once

#include <JuceHeader.h>
#include <cmath>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 panelColourValue       = 0xFF050913;
    static constexpr juce::uint32 panelAltColourValue    = 0xFF0C1524;
    static constexpr juce::uint32 panelRaisedColourValue = 0xFF172235;
    static constexpr juce::uint32 outlineColourValue     = 0xFF223652;
    static constexpr juce::uint32 textColourValue        = 0xFFF2F6FF;
    static constexpr juce::uint32 mutedTextColourValue   = 0xFF708BAC;
    static constexpr juce::uint32 accentBlueValue        = 0xFF1AB3FF;
    static constexpr juce::uint32 accentOrangeValue      = 0xFFFF8A18;
    static constexpr juce::uint32 accentGreenValue       = 0xFF18D98C;
    static constexpr juce::uint32 accentRedValue         = 0xFFFF4A6C;

    CustomLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, colour(panelColourValue));

        setColour(juce::TextButton::buttonColourId, colour(panelRaisedColourValue));
        setColour(juce::TextButton::buttonOnColourId, colour(accentBlueValue));
        setColour(juce::TextButton::textColourOffId, colour(textColourValue));
        setColour(juce::TextButton::textColourOnId, colour(textColourValue));

        setColour(juce::Slider::thumbColourId, colour(textColourValue));
        setColour(juce::Slider::trackColourId, colour(accentBlueValue));
        setColour(juce::Slider::backgroundColourId, colour(outlineColourValue).darker(0.35f));
        setColour(juce::Slider::rotarySliderFillColourId, colour(accentBlueValue));
        setColour(juce::Slider::rotarySliderOutlineColourId, colour(outlineColourValue));

        setColour(juce::Label::textColourId, colour(textColourValue));
        setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        setColour(juce::ListBox::backgroundColourId, colour(panelColourValue));
        setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ListBox::textColourId, colour(textColourValue));

        setColour(juce::TableHeaderComponent::backgroundColourId, colour(panelAltColourValue));
        setColour(juce::TableHeaderComponent::textColourId, colour(textColourValue));
        setColour(juce::TableHeaderComponent::outlineColourId, colour(outlineColourValue));

        setColour(juce::TextEditor::backgroundColourId, colour(panelAltColourValue));
        setColour(juce::TextEditor::textColourId, colour(textColourValue));
        setColour(juce::TextEditor::highlightColourId, colour(accentBlueValue).withAlpha(0.28f));
        setColour(juce::TextEditor::highlightedTextColourId, colour(textColourValue));
        setColour(juce::TextEditor::outlineColourId, colour(outlineColourValue));
        setColour(juce::TextEditor::focusedOutlineColourId, colour(accentBlueValue));
        setColour(juce::CaretComponent::caretColourId, colour(textColourValue));

        setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::ScrollBar::thumbColourId, colour(outlineColourValue).brighter(0.22f));
    }

    static juce::Colour colour(juce::uint32 argb) { return juce::Colour(argb); }

    static constexpr float smallRadius() noexcept   { return 6.0f; }
    static constexpr float controlRadius() noexcept { return 7.5f; }
    static constexpr float panelRadius() noexcept   { return 10.0f; }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        const float size = juce::jlimit(8.5f, 15.0f, static_cast<float>(buttonHeight) * 0.42f);
        return juce::Font(juce::FontOptions(size).withStyle("Bold"));
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
        const float radius = controlRadius();

        auto baseColour = backgroundColour;
        if (!button.isEnabled())
            baseColour = colour(panelRaisedColourValue).darker(0.1f).withAlpha(0.52f);
        else if (button.getToggleState())
            baseColour = baseColour.brighter(0.12f).withSaturation(baseColour.getSaturation() + 0.05f);
        else if (isButtonDown)
            baseColour = baseColour.brighter(0.08f);
        else if (isMouseOverButton)
            baseColour = baseColour.brighter(0.035f);

        juce::DropShadow shadow(juce::Colours::black.withAlpha(button.isEnabled() ? 0.32f : 0.18f), 8, { 0, 3 });
        shadow.drawForRectangle(g, bounds.toNearestInt());

        juce::ColourGradient fill(baseColour.brighter(0.07f), bounds.getTopLeft(),
                                  baseColour.darker(0.16f), bounds.getBottomLeft(), false);
        g.setGradientFill(fill);
        g.fillRoundedRectangle(bounds, radius);

        auto innerGlow = bounds.reduced(1.0f);
        juce::ColourGradient gloss(juce::Colours::white.withAlpha(button.isEnabled() ? 0.06f : 0.025f),
                                   innerGlow.getTopLeft(),
                                   juce::Colours::transparentWhite,
                                   innerGlow.getCentre(), false);
        g.setGradientFill(gloss);
        g.fillRoundedRectangle(innerGlow.removeFromTop(bounds.getHeight() * 0.52f), radius - 1.0f);

        g.setColour(colour(outlineColourValue).withAlpha(button.isEnabled() ? 0.95f : 0.35f));
        g.drawRoundedRectangle(bounds, radius, 1.0f);

        const auto accentOutline = baseColour.brighter(button.getToggleState() ? 0.34f : 0.16f)
                                             .withAlpha(button.isEnabled() ? (button.hasKeyboardFocus(true) ? 0.95f : 0.72f) : 0.22f);
        g.setColour(accentOutline);
        g.drawRoundedRectangle(bounds.reduced(0.6f), radius - 0.6f, button.getToggleState() ? 1.6f : 0.9f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);

        const bool enabled = button.isEnabled();
        const auto textColour = button.findColour(button.getToggleState()
                                                      ? juce::TextButton::textColourOnId
                                                      : juce::TextButton::textColourOffId)
                                    .withAlpha(enabled ? 1.0f : 0.45f);

        g.setColour(textColour);
        g.drawFittedText(button.getButtonText(),
                         button.getLocalBounds().reduced(6, 0),
                         juce::Justification::centred, 1, 0.72f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height));

        if (style == juce::Slider::LinearHorizontal)
        {
            auto track = bounds.withHeight(5.0f).withCentre(bounds.getCentre());
            g.setColour(colour(panelAltColourValue).brighter(0.10f));
            g.fillRoundedRectangle(track, 2.5f);

            g.setColour(colour(outlineColourValue).withAlpha(0.95f));
            g.drawRoundedRectangle(track.expanded(0.25f, 0.25f), 2.5f, 0.8f);

            auto active = track.withWidth(juce::jlimit(8.0f, track.getWidth(), sliderPos - track.getX()));
            auto activeColour = slider.findColour(juce::Slider::trackColourId);
            juce::ColourGradient activeFill(activeColour.brighter(0.18f), active.getTopLeft(),
                                            activeColour.darker(0.12f), active.getTopRight(), false);
            g.setGradientFill(activeFill);
            g.fillRoundedRectangle(active, 2.5f);

            const float shadowX = sliderPos - 8.0f;
            const float shadowY = track.getCentreY() - 6.0f;
            g.setColour(juce::Colours::black.withAlpha(0.32f));
            g.fillEllipse(shadowX, shadowY + 1.2f, 16.0f, 16.0f);

            g.setColour(colour(textColourValue));
            g.fillEllipse(shadowX, shadowY, 16.0f, 16.0f);

            g.setColour(activeColour.withAlpha(0.55f));
            g.drawEllipse(shadowX, shadowY, 16.0f, 16.0f, 1.1f);
            return;
        }

        if (style == juce::Slider::LinearVertical)
        {
            auto track = bounds.withWidth(7.0f).withCentre(bounds.getCentre());
            g.setColour(colour(panelAltColourValue).brighter(0.08f));
            g.fillRoundedRectangle(track, 3.5f);

            g.setColour(colour(outlineColourValue).withAlpha(0.92f));
            g.drawRoundedRectangle(track.expanded(0.25f, 0.25f), 3.5f, 0.8f);

            auto active = juce::Rectangle<float>(track.getX(), sliderPos,
                                                 track.getWidth(), track.getBottom() - sliderPos);
            auto activeColour = slider.findColour(juce::Slider::trackColourId);
            juce::ColourGradient activeFill(activeColour.brighter(0.14f), active.getTopLeft(),
                                            activeColour.darker(0.18f), active.getBottomLeft(), false);
            g.setGradientFill(activeFill);
            g.fillRoundedRectangle(active, 3.5f);

            const float thumbX = track.getCentreX() - 9.0f;
            const float thumbY = sliderPos - 9.0f;
            g.setColour(juce::Colours::black.withAlpha(0.32f));
            g.fillEllipse(thumbX, thumbY + 1.2f, 18.0f, 18.0f);
            g.setColour(colour(textColourValue));
            g.fillEllipse(thumbX, thumbY, 18.0f, 18.0f);
            g.setColour(activeColour.withAlpha(0.55f));
            g.drawEllipse(thumbX, thumbY, 18.0f, 18.0f, 1.1f);
            return;
        }

        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
                                               sliderPos, minSliderPos, maxSliderPos,
                                               style, slider);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        auto bounds = label.getLocalBounds();
        g.setColour(label.findColour(juce::Label::textColourId).withAlpha(label.isEnabled() ? 1.0f : 0.45f));
        g.setFont(label.getFont());
        g.drawFittedText(label.getText(), bounds.reduced(1, 0),
                         label.getJustificationType(), 1, 0.95f);
    }

    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
        juce::ColourGradient fill(editor.findColour(juce::TextEditor::backgroundColourId).brighter(0.05f), bounds.getTopLeft(),
                                  editor.findColour(juce::TextEditor::backgroundColourId).darker(0.08f), bounds.getBottomLeft(), false);
        g.setGradientFill(fill);
        g.fillRoundedRectangle(bounds, controlRadius());
    }

    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
        const bool focused = editor.hasKeyboardFocus(true);
        g.setColour((focused ? editor.findColour(juce::TextEditor::focusedOutlineColourId)
                             : editor.findColour(juce::TextEditor::outlineColourId))
                        .withAlpha(focused ? 0.98f : 0.90f));
        g.drawRoundedRectangle(bounds, controlRadius(), focused ? 1.4f : 1.0f);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        const float radius = juce::jmin(width / 2.0f, height / 2.0f) - 4.0f;
        const float centreX = static_cast<float>(x) + width * 0.5f;
        const float centreY = static_cast<float>(y) + height * 0.5f;
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const bool enabled = slider.isEnabled();

        juce::Path track;
        track.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(colour(outlineColourValue).darker(0.08f));
        g.strokePath(track, juce::PathStrokeType(3.8f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        const float zeroAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * 0.5f;
        const float arcStart = juce::jmin(angle, zeroAngle);
        const float arcEnd = juce::jmax(angle, zeroAngle);
        if (arcEnd > arcStart + 0.01f)
        {
            juce::Path filled;
            filled.addCentredArc(centreX, centreY, radius, radius, 0.0f, arcStart, arcEnd, true);
            auto trackColour = slider.findColour(juce::Slider::trackColourId);
            if (!enabled)
                trackColour = trackColour.withAlpha(0.40f);
            g.setColour(trackColour);
            g.strokePath(filled, juce::PathStrokeType(3.8f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }

        const float knobRadius = radius * 0.62f;
        juce::ColourGradient knobFill(colour(panelRaisedColourValue).brighter(0.15f),
                                      centreX - knobRadius * 0.45f, centreY - knobRadius * 0.55f,
                                      colour(panelColourValue).darker(0.08f),
                                      centreX + knobRadius * 0.45f, centreY + knobRadius * 0.45f,
                                      true);
        g.setGradientFill(knobFill);
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        g.setColour(colour(outlineColourValue).brighter(0.10f));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.1f);

        const float pointerLen = knobRadius * 0.62f;
        const float pointerStart = knobRadius * 0.20f;
        const float px = centreX + std::sin(angle) * pointerLen;
        const float py = centreY - std::cos(angle) * pointerLen;
        const float px2 = centreX + std::sin(angle) * pointerStart;
        const float py2 = centreY - std::cos(angle) * pointerStart;
        g.setColour(enabled ? colour(textColourValue) : colour(mutedTextColourValue));
        g.drawLine(px2, py2, px, py, 2.0f);

        if (slider.isMouseOverOrDragging() && enabled)
        {
            g.setColour(slider.findColour(juce::Slider::trackColourId).withAlpha(0.18f));
            g.drawEllipse(centreX - radius - 2.0f, centreY - radius - 2.0f,
                          (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f, 1.8f);
        }
    }

    void drawTableHeaderBackground(juce::Graphics& g, juce::TableHeaderComponent& header) override
    {
        auto area = header.getLocalBounds().toFloat();
        juce::ColourGradient gradient(colour(panelAltColourValue).brighter(0.08f), area.getTopLeft(),
                                      colour(panelAltColourValue).darker(0.16f), area.getBottomLeft(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(area.reduced(0.5f), panelRadius());

        g.setColour(colour(outlineColourValue).brighter(0.08f));
        g.drawRoundedRectangle(area.reduced(0.5f), panelRadius(), 1.0f);
    }

    void drawTableHeaderColumn(juce::Graphics& g, juce::TableHeaderComponent&, const juce::String& columnName,
                               int, int width, int height, bool isMouseOver, bool isMouseDown,
                               int columnFlags) override
    {
        auto area = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(6.0f, 2.0f);
        auto textColour = colour(textColourValue).withAlpha(isMouseDown ? 1.0f : (isMouseOver ? 0.96f : 0.86f));
        g.setColour(textColour);
        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
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

        g.setColour(colour(outlineColourValue).withAlpha(0.35f));
        g.drawVerticalLine(width - 1, 6.0f, static_cast<float>(height - 6));
    }

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height,
                       bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                       bool isMouseOver, bool isMouseDown) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height));
        auto track = bounds.reduced(isScrollbarVertical ? 2.0f : 1.0f,
                                    isScrollbarVertical ? 1.0f : 2.0f);
        g.setColour(colour(panelAltColourValue).withAlpha(0.62f));
        g.fillRoundedRectangle(track, smallRadius());

        juce::Rectangle<float> thumb = isScrollbarVertical
            ? juce::Rectangle<float>(track.getX(), static_cast<float>(thumbStartPosition),
                                     track.getWidth(), static_cast<float>(thumbSize))
            : juce::Rectangle<float>(static_cast<float>(thumbStartPosition), track.getY(),
                                     static_cast<float>(thumbSize), track.getHeight());

        auto thumbColour = colour(outlineColourValue).brighter(isMouseDown ? 0.22f : (isMouseOver ? 0.14f : 0.06f));
        g.setColour(thumbColour.withAlpha(0.95f));
        g.fillRoundedRectangle(thumb.reduced(0.5f), smallRadius());
    }
};
