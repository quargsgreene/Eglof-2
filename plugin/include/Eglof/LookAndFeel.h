#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

    class LookAndFeel: public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel();
        ~LookAndFeel() override;
        
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& menu) override;
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool mouseOver, bool buttonDown) override;
        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
//        void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>&area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable *icon, const juce::Colour *textColour) override;
//        void drawScrollbar (juce::Graphics& g, juce::ScrollBar &scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOverButton, bool isButtonDown) override;
        // slider
        // response window
        // response curve
        // help bubble
        // power button
    };
}
