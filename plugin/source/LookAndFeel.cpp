#include "../include/Eglof/LookAndFeel.h"

namespace audio_plugin {
    LookAndFeel::LookAndFeel()
    {
        juce::Colour eglofPurple = juce::Colour(140u, 82u, 255u);
        juce::Colour eglofOrange = juce::Colour(255u, 87u, 87u);
        juce::Colour eglofGray = juce::Colour(39u, 39u, 39u);
        juce::Colour eglofGreen = juce::Colour(0u, 191u, 99u);
        juce::Colour eglofWhite = juce::Colours::white;
        
        setColour(juce::ResizableWindow::backgroundColourId, eglofGray);
        
        setColour(juce::TextButton::buttonColourId, eglofGreen);
        setColour(juce::TextButton::buttonOnColourId, eglofPurple);
        setColour(juce::TextButton::textColourOffId, eglofGray);
        setColour(juce::TextButton::textColourOnId, eglofWhite);
        
        setColour(juce::ComboBox::backgroundColourId,eglofOrange);
        setColour(juce::ComboBox::buttonColourId, eglofOrange);
        setColour(juce::ComboBox::textColourId, eglofGray);
        setColour(juce::ComboBox::arrowColourId, eglofPurple);
        setColour(juce::ComboBox::outlineColourId, eglofGreen);
        setColour(juce::ComboBox::focusedOutlineColourId, eglofPurple);
        
        setColour(juce::PopupMenu::backgroundColourId, eglofOrange);
        setColour(juce::PopupMenu::textColourId, eglofGray);
        setColour(juce::PopupMenu::highlightedTextColourId, eglofGreen);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, eglofPurple);
        
    }

    LookAndFeel::~LookAndFeel(){}

void LookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& menu) {
    
    auto background = menu.findColour(juce::ComboBox::backgroundColourId);
    auto focused = menu.findColour(juce::ComboBox::focusedOutlineColourId);
    auto arrow = menu.findColour(juce::ComboBox::arrowColourId);
    g.setColour(background);
    g.fillRoundedRectangle(0, 0, float(width), float(height), 8.0);
    juce::Path drawArrow;
    
    if (isButtonDown){
        g.setColour(focused);
        g.drawRoundedRectangle(juce::Rectangle<float>(0.0, 0.0, float (width), float (height)), 8.0, 1.5);
    }
    
    drawArrow.addTriangle (
        {
            static_cast<float>(buttonX + buttonW * 0.1),
            static_cast<float>(buttonY + buttonH * 0.2)
        },
        {
            static_cast<float>(buttonX + buttonW * 0.35),
            static_cast<float>(buttonY + buttonH * 0.2)
        },
        {
            static_cast<float>(buttonX + buttonW * 0.25),
            static_cast<float>(buttonY + buttonH * 0.3)
        }
                       );
    
    g.setColour(arrow);
    g.fillPath(drawArrow);
    
}

void LookAndFeel::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    g.fillAll(findColour(juce::PopupMenu::backgroundColourId));
    juce::Colour shadow = juce::Colours::black.withAlpha (0.0f);
    g.setColour (shadow);
    g.fillRect (-4, -4, width + 8, height + 8);
}

void LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool mouseOver, bool buttonDown){
    
        auto buttonArea = button.getLocalBounds();
        auto focusedButton = button.findColour(juce::TextButton::buttonOnColourId);
        auto focusedText = button.findColour(juce::TextButton::textColourOnId);
        auto corner = 8;
        g.setColour(backgroundColour);
        g.fillRoundedRectangle(buttonArea.toFloat(), corner);
        
        if(mouseOver || buttonDown)
        {
            g.setColour(focusedButton);
            g.setColour(focusedText);
        }
    }

//void LookAndFeel::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String &text, const juce::String &shortcutKeyText, const juce::Drawable *icon, const juce::Colour *textColour)
//{
//    (void) icon;
//    (void) textColour;
//    (void) shortcutKeyText;
//    (void) text;
//    (void) isActive;
//    (void) isHighlighted;
//    (void) hasSubMenu;
//    (void) isTicked;
//    (void) area;
//    
//    if (isSeparator){
//        g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
//    }

}

//void LookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar &scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOverButton, bool isButtonDown){}
// slider
// response window
// response curve
// help bubble
// power button


