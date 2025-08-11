#pragma once
#include "PluginProcessor.h"
#include "Knob.h"
#include "Menu.h"
#include "CsvColumnSelectionDropdown.h"
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
    public juce::LookAndFeel_V4,
    public juce::FileBasedDocument
    {
    public:
        juce::StringArray csvColumns;

        AddCsv(const juce::String &fileExtension,
               const juce::String &fileWildCard,
               const juce::String &openFileDialogTitle,
               const juce::String &saveFileDialogTitle,
               EglofAudioProcessor&);

        ~AddCsv() override;
        
         enum FileTransportState
         {
             Empty,
             Nonempty
         };
         void changeListenerCallback(juce::ChangeBroadcaster* source) override;
         void setColumnMenus (CsvColumnSelectionDropdown* m1, CsvColumnSelectionDropdown* m2, CsvColumnSelectionDropdown* m3, CsvColumnSelectionDropdown* m4);
         void csvColumnMenuChanged();
         juce::StringArray getCsvColumns(auto file);
         juce::StringArray getCsvRows(auto file);
         juce::StringArray getCsvRowCells(juce::String csvRow);
         std::vector<juce::String> getColumn(int columnIndex);
         int getSelectedColumnIndex(int selectedColumnId);
         std::vector<float> normalizeFrequenciesToAudibleRange (const std::vector<float> csvColumn);
        
    private:
        FileTransportState state;
        std::unique_ptr<juce::FileChooser> chooser;
        juce::String csvDocumentTitle;
        juce::File lastCsvFileOpened;
        bool documentModified;
        
        juce::String fileExtName;
        juce::String wildcardExtName;
        juce::String openDialogText;
        juce::String saveDialogText;
        
        EglofAudioProcessor& processorRef;
        
        CsvColumnSelectionDropdown* menu1 = nullptr;
        CsvColumnSelectionDropdown* menu2 = nullptr;
        CsvColumnSelectionDropdown* menu3 = nullptr;
        CsvColumnSelectionDropdown* menu4 = nullptr;
    protected:
        juce::String getDocumentTitle() override;
        juce::Result loadDocument(const juce::File &file) override;
        juce::Result saveDocument(const juce::File &file) override;
        juce::File getLastDocumentOpened() override;
        void setLastDocumentOpened (const juce::File &file) override;
        
    };

}
