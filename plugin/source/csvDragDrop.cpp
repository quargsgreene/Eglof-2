#include "../include/Eglof/csvDragDrop.h"


namespace audio_plugin{

csvDragDrop::csvDragDrop(){
    formatManager.registerBasicFormats();       // [1]
    transportSource.addChangeListener (this);   // [2]
}

csvDragDrop::~csvDragDrop(){}

void csvDragDrop::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a CSV file...",
                                                   juce::File{},
                                                   "*.csv");                     // [7]
    auto chooserFlags = juce::FileBrowserComponent::openMode
    | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)     // [8]
                          {
        auto file = fc.getResult();
        
        if (file != juce::File{})                                                // [9]
        {
            auto* reader = formatManager.createReaderFor (file);                 // [10]
            
            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);   // [11]
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                playButton.setEnabled (true);                                                      // [13]
                readerSource.reset (newSource.release());                                          // [14]
            }
        }
    });
}

void csvDragDrop::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }
}

void csvDragDrop::changeState (TransportState newState)
{
    if (state != newState)
    {
        state = newState;
        
        switch (state)
        {
            case Stopped:                           // [3]
                stopButton.setEnabled (false);
                playButton.setEnabled (true);
                transportSource.setPosition (0.0);
                break;
                
            case Starting:                          // [4]
                playButton.setEnabled (false);
                transportSource.start();
                break;
                
            case Playing:                           // [5]
                stopButton.setEnabled (true);
                break;
                
            case Stopping:                          // [6]
                transportSource.stop();
                break;
        }
    }
}

void csvDragDrop::playButtonClicked()
{
    changeState (Starting);
}

void csvDragDrop::stopButtonClicked()
{
    changeState (Stopping);
}

}
