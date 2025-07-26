#include "../include/Eglof/CsvColumnSelectionDropdown.h"

namespace audio_plugin
{
    CsvColumnSelectionDropdown::CsvColumnSelectionDropdown(){}

    CsvColumnSelectionDropdown::~CsvColumnSelectionDropdown(){}

void CsvColumnSelectionDropdown::changeListenerCallback(juce::ChangeBroadcaster *selectedData){
    (void) selectedData;
    // Detect when csv is uploaded
    // Set menu options to each entryin first line
}

void CsvColumnSelectionDropdown::dataColumnSelected()
{
    // Retrieve each value in corresponding column of csv
    // Map values to corresponding parameter
}

}

