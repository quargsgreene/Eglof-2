#pragma once

#include "PluginProcessor.h"
#include "Knob.h"
#include "Menu.h"
#include "AddCsv.h"
#include "CsvColumnSelectionDropdown.h"
#include "LookAndFeel.h"

namespace audio_plugin {

class EglofAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit EglofAudioProcessorEditor(EglofAudioProcessor&);
    ~EglofAudioProcessorEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    CsvColumnSelectionDropdown dataColumnMenu1;
    CsvColumnSelectionDropdown dataColumnMenu2;
    CsvColumnSelectionDropdown dataColumnMenu3;
    CsvColumnSelectionDropdown dataColumnMenu4;
    AddCsv openButton;
    LookAndFeel uiAesthetic;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::Slider qRangeSlider;
    juce::Slider gainRangeSlider;
    juce::Slider cutoffRangeSlider;
    juce::Slider resonanceRangeSlider;
    
    CsvColumnSelectionDropdown presetMenu;

    
    juce::ShapeButton powerButton{"Power", juce::Colours::red, juce::Colours::green, juce::Colours::blue};
    juce::TextButton helpButton{"Help"};
    juce::TextButton forwardPresetButton{"->"};
    juce::TextButton backwardPresetButton{"<-"};
    juce::TextButton compareButton{"Compare"};
    juce::TextButton copyButton{"Copy"};
    juce::TextButton pasteButton{"Paste"};
    juce::TextButton chooseRandomDataButton{"Choose data for me!"};
    juce::TextButton downloadCSVButton{"Download CSV"};
//    AddCsv openButton;
    juce::Image background;

  EglofAudioProcessor& processorRef;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EglofAudioProcessorEditor)
};
}  // namespace audio_plugin
