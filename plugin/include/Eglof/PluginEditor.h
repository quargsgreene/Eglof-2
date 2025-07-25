#pragma once

#include "PluginProcessor.h"
#include "Knob.h"
#include "Menu.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace audio_plugin {

class EglofAudioProcessorEditor : public juce::AudioProcessorEditor,
public juce::ChangeListener
{
public:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    explicit EglofAudioProcessorEditor(EglofAudioProcessor&);
    ~EglofAudioProcessorEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    void openButtonClicked();
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void playButtonClicked();
    void stopButtonClicked();
    void changeState(TransportState newState);
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    
    TransportState state;

    juce::Slider qRangeSlider;
    juce::Slider gainRangeSlider;
    juce::Slider cutoffRangeSlider;
    juce::Slider resonanceRangeSlider;
    
    juce::ComboBox presetMenu;
    juce::ComboBox dataColumnMenu1;
    juce::ComboBox dataColumnMenu2;
    juce::ComboBox dataColumnMenu3;
    juce::ComboBox dataColumnMenu4;
    
    juce::ShapeButton powerButton{"Power", juce::Colours::red, juce::Colours::green, juce::Colours::blue};
    juce::TextButton helpButton{"Help"};
    juce::TextButton forwardPresetButton{"->"};
    juce::TextButton backwardPresetButton{"<-"};
    juce::TextButton compareButton{"Compare"};
    juce::TextButton copyButton{"Copy"};
    juce::TextButton pasteButton{"Paste"};
    juce::TextButton chooseRandomDataButton{"Choose data for me!"};
    juce::TextButton downloadCSVButton{"Download CSV"};
    juce::Image background;
//    juce::Rectangle<int> dragDrop {1000, 35, 190, 200};
    
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    std::unique_ptr<juce::FileChooser> chooser;
    
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

  EglofAudioProcessor& processorRef;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EglofAudioProcessorEditor)
};
}  // namespace audio_plugin
