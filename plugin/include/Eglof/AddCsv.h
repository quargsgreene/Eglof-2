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

    class AddCsv:
    public juce::TextButton,
    public juce::ChangeListener,
    public juce::LookAndFeel_V4
    {
    public:
        explicit AddCsv();
        ~AddCsv() override;
        enum TransportState
        {
            Stopped,
            Starting,
            Playing,
            Stopping
        };
    
     void openButtonClicked();
     void changeListenerCallback(juce::ChangeBroadcaster* source) override;
     void playButtonClicked();
     void stopButtonClicked();
     void changeState(TransportState newState);
        
    private:
        TransportState state;
        juce::TextButton playButton;
        juce::TextButton stopButton;
        std::unique_ptr<juce::FileChooser> chooser;
        juce::AudioFormatManager formatManager;
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
        juce::AudioTransportSource transportSource;
        
    };

//    class AddCsvLookAndFeel: public juce::LookAndFeel_V4
//    {
//    public:
//        AddCsvLookAndFeel();
//    };
}
