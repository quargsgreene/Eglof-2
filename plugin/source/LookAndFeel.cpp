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
    
    setColour(juce::Slider::thumbColourId, eglofPurple);
    setColour(juce::Slider::trackColourId, eglofGreen);
    setColour(juce::Slider::textBoxTextColourId, eglofOrange);
    setColour(juce::Slider::textBoxBackgroundColourId, eglofGreen);
    setColour(juce::Slider::textBoxOutlineColourId, eglofPurple);
    setColour(juce::Slider::textBoxHighlightColourId, eglofPurple);
    
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
    g.setFont(juce::FontOptions("Courier", 15.0f, juce::Font::bold));
}

void LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool mouseOver, bool buttonDown){
    
    auto buttonArea = button.getLocalBounds();
    auto focusedButton = button.findColour(juce::TextButton::buttonOnColourId);
    auto focusedText = button.findColour(juce::TextButton::textColourOnId);
    auto corner = 8;
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(buttonArea.toFloat(), corner);
    g.setFont(juce::FontOptions("Courier", 15.0f, juce::Font::bold));
    
    if(mouseOver || buttonDown)
    {
        g.setColour(focusedButton);
        g.setColour(focusedText);
    }
}

juce::Font LookAndFeel::getTextButtonFont(juce::TextButton &, int buttonHeight)
{
    juce::ignoreUnused(buttonHeight);
    return juce::FontOptions("Courier", 10.0f, juce::Font::bold);
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    
    juce::ignoreUnused(shouldDrawButtonAsDown);
    juce::ignoreUnused(shouldDrawButtonAsHighlighted);
    
    if( auto* pb = dynamic_cast<PowerButton*>(&toggleButton) )
    {
        Path powerButton;
        juce::ignoreUnused(pb);
        
        auto bounds = toggleButton.getLocalBounds().toFloat();
        
        auto size = static_cast<float>(jmin(bounds.getWidth()/2, bounds.getHeight()/2) - 6);
        auto r = bounds.withSizeKeepingCentre(size/2, size/2);
        
        float ang = 30.f; //30.f;
        
        size -= 6;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size/2,
                                  size/2,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(4.f, PathStrokeType::JointStyle::mitered);
        
        auto color = toggleButton.getToggleState() ? Colour(140u, 82u, 255u) : Colour(0u, 191u, 99u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 4);
    }
    else if( auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton) )
    {
        auto color = ! toggleButton.getToggleState() ? Colour(140u, 82u, 255u) : Colour(0u, 191u, 99u);
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
    
}


}
