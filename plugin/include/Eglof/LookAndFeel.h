#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace audio_plugin {

    class LookAndFeel: public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel();
        ~LookAndFeel() override;
        struct PowerButton : juce::ToggleButton { };
        
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& menu) override;
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool mouseOver, bool buttonDown) override;
        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
        void drawToggleButton (juce::Graphics &g,
                               juce::ToggleButton & toggleButton,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
        juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

        struct AnalyzerButton : juce::ToggleButton
        {
            void resized() override
            {
                auto bounds = getLocalBounds();
                auto insetRect = bounds.reduced(4);
                
                randomPath.clear();
                
                juce::Random r;
                
                randomPath.startNewSubPath(insetRect.getX(),
                                           insetRect.getY() + insetRect.getHeight() * r.nextFloat());
                
                for( auto x = insetRect.getX() + 1; x < insetRect.getRight(); x += 2 )
                {
                    randomPath.lineTo(x,
                                      insetRect.getY() + insetRect.getHeight() * r.nextFloat());
                }
            }
            
            juce::Path randomPath;
        };
        
        struct RotarySliderWithLabels : juce::Slider
        {
            RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
            juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                         juce::Slider::TextEntryBoxPosition::NoTextBox),
            param(&rap),
            suffix(unitSuffix)
            {
                
            }
            
            ~RotarySliderWithLabels() override
            {
                
            }
            
            struct LabelPos
            {
                float pos;
                juce::String label;
            };
            
            juce::Array<LabelPos> labels;
            
            void paint(juce::Graphics& g) override;
            juce::Rectangle<int> getSliderBounds() const;
            int getTextHeight() const { return 14; }
            juce::String getDisplayString() const;
        private:
            
            juce::RangedAudioParameter* param;
            juce::String suffix;
        };
    };
}
