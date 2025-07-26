#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>


namespace audio_plugin {
    class Dropdown: public juce::ChangeListener,
                    public juce::ComboBox,
                    public juce::LookAndFeel_V4
    {
    public:
        explicit Dropdown();
        ~Dropdown() override;
        void changeListenerCallback(juce::ChangeBroadcaster* selectedData) override;
        void dataColumnSelected();
    };
}
