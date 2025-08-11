#include "Eglof/AddCsv.h"
#include "Eglof/PluginEditor.h"
#include "Eglof/PluginProcessor.h"
#include <juce_core/juce_core.h>
#include <vector>

namespace audio_plugin{

AddCsv::AddCsv(const juce::String &fileExtension,
               const juce::String &fileWildCard,
               const juce::String &openFileDialogTitle,
               const juce::String &saveFileDialogTitle,
               EglofAudioProcessor& p): juce::FileBasedDocument(fileExtension,
                                                                                 fileWildCard,
                                                                                 openFileDialogTitle,
                                                                                 saveFileDialogTitle),
fileExtName ("csv"),
wildcardExtName("*.csv"),
openDialogText("Select a CSV file..."),
saveDialogText("Save file"),
processorRef(p)


{
    chooser = std::make_unique<juce::FileChooser> (openDialogText,
                                                   juce::File{},
                                                   wildcardExtName);
    auto chooserFlags = juce::FileBrowserComponent::openMode
    | juce::FileBrowserComponent::canSelectFiles;
    
    this->onClick = [this, chooserFlags] {chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)     // [8]
                                                                {
        auto file = fc.getResult();
        
        if (file != juce::File{} && file.hasReadAccess())                                                // [9]
        {
            loadDocument(file);
        }
    });};
}

AddCsv::~AddCsv(){}

void AddCsv::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (source == reinterpret_cast<juce::ChangeBroadcaster*>(chooser.get())){
        if(state == Empty && !state){
            state = Nonempty;
        }
    }
}

//
juce::String AddCsv::getDocumentTitle()
{
    return csvDocumentTitle;
}

juce::Result AddCsv::loadDocument(const juce::File &file)
{
    csvColumns = getCsvColumns(file);
    csvDocumentTitle = file.getFileName();
    if(csvColumns.isEmpty() && !file.exists()){
        return juce::Result::fail("Could not open: " + file.getFullPathName());
    }
    setLastDocumentOpened(file);
    documentModified = false;
    if(menu1 != nullptr){
        menu1->clear();
        menu1->setText("Q Mapping");
        menu2->clear();
        menu2->setText("Gain Mapping");
        menu3->clear();
        menu3->setText("Cutoff Mapping");
        menu4->clear();
        menu4->setText("Resonance Mapping");
        
        for(int i = 0; i < csvColumns.size(); ++i)
        {
            auto menuItemId = i + 1;
            menu1->addItem(csvColumns[i], menuItemId);
            menu2->addItem(csvColumns[i], menuItemId);
            menu3->addItem(csvColumns[i], menuItemId);
            menu4->addItem(csvColumns[i], menuItemId);
        }
        
        menu2->onChange = [this] {csvColumnMenuChanged();};
    }
    
    return juce::Result::ok();
}
//
juce::Result AddCsv::saveDocument(const juce::File &file)
{
    juce::String saved = file.loadFileAsString();
    if(saved.isEmpty() && !file.exists());
    {
        return juce::Result::fail("Could not save: " + file.getFullPathName());
    }
    return juce::Result::ok();
}


void AddCsv::setLastDocumentOpened(const juce::File &file)
{
    lastCsvFileOpened = file;
}

juce::File AddCsv::getLastDocumentOpened()
{
    return lastCsvFileOpened;
}

juce::StringArray AddCsv::getCsvRows(auto file)
{
    juce::StringArray csvRows;
    file.readLines(csvRows);
    return csvRows;
}

juce::StringArray AddCsv::getCsvColumns(auto file)
{
    juce::StringArray csvRows;
    file.readLines(csvRows);
    juce::String csvColumnString = csvRows[0];
    juce::String currentColumn;
    juce::StringArray columns;
    juce::String comma(",");
    juce::String space(" ");
    juce::String singleQuote("'");
    
    for (int i = 0; i < csvColumnString.length(); i++)
    {
        juce::String currentChar = juce::String::charToString(csvColumnString[i]);
        if(currentChar == comma){
            columns.add(currentColumn);
            currentColumn = "";
        }else if (currentChar == singleQuote || currentChar == space)
            continue;
        else{
            currentColumn += currentChar;
        }
    }
    if(currentColumn.isNotEmpty()){
        columns.add(currentColumn);
    }

    return columns;
}

juce::StringArray AddCsv::getCsvRowCells(juce::String csvRow)
{
    juce::StringArray cells;
    juce::String currentCell;
    juce::String comma(",");
    juce::String space(" ");
    juce::String singleQuote("'");
    
    for (int i = 0; i < csvRow.length(); i++)
    {
        juce::String currentChar = juce::String::charToString(csvRow[i]);
        if(currentChar == comma){
            cells.add(currentCell);
            currentCell = "";
        }else if (currentChar == singleQuote || currentChar == space)
            continue;
        else{
            currentCell += currentChar;
        }
    }
    if(currentCell.isNotEmpty()){
        cells.add(currentCell);
    }

    return cells;
}

std::vector<juce::String> AddCsv::getColumn(int columnIndex)
{
    std::vector<juce::String> column;
    juce::StringArray allCsvData = getCsvRows(lastCsvFileOpened);
    juce::StringArray allCsvColumns = getCsvColumns(lastCsvFileOpened);
    
    for(int i = 0; i < allCsvData.size(); ++i)
    {
        juce::StringArray rowCells = getCsvRowCells(allCsvData[i]);
        juce::String currentColumnItem = rowCells[columnIndex];
        column.push_back(currentColumnItem);
    }
    
    return column;
}

int AddCsv::getSelectedColumnIndex(int selectedColumnId)
{
    return selectedColumnId - 1;
}

void AddCsv::setColumnMenus(CsvColumnSelectionDropdown *m1, CsvColumnSelectionDropdown *m2, CsvColumnSelectionDropdown *m3, CsvColumnSelectionDropdown *m4)
    {
  
        menu1 = m1;
        menu2 = m2;
        menu3 = m3;
        menu4 = m4;
    }

 std::vector<float> AddCsv::normalizeFrequenciesToAudibleRange (const std::vector<float> csvColumn)
{
    constexpr float MIN_FREQ = 20.f;
    constexpr float MAX_FREQ = 20000.f;
    
    std::vector<float> validFreqs;
    validFreqs.reserve(csvColumn.size());
    for(size_t i = 0; i < csvColumn.size(); ++i)
    {
        if(std::isfinite(csvColumn[i]) && csvColumn[i] > 0.0f)
        {
            validFreqs.push_back(csvColumn[i]);
        }
    }
    
    std::vector<float> normalized;
    normalized.reserve(csvColumn.size());
    
    if(validFreqs.empty())
    {
        normalized.assign(csvColumn.size(), 0.0f);
        return normalized;
    }
    
    auto [minIterator, maxIterator] = std::minmax_element(validFreqs.begin(), validFreqs.end());
    const float sourceDataMin = *minIterator;
    const float sourceDataMax = *maxIterator;
    
    if(sourceDataMin <= 0.f || !std::isfinite(sourceDataMin) || !std::isfinite(sourceDataMax))
    {
        normalized.assign(csvColumn.size(), 0.f);
        return normalized;
    }
    
    if(sourceDataMin >= MIN_FREQ && sourceDataMax <= MAX_FREQ)
    {
        return csvColumn;
    }
    
    const float ratio = MAX_FREQ/MIN_FREQ;
    const float sourceDataRatio = sourceDataMax/sourceDataMin;
    
    if(sourceDataRatio <= 0.0f || !std::isfinite(sourceDataRatio))
    {
        normalized.assign(static_cast<size_t>(csvColumns.size()), 0.0f);
        return normalized;
    }
     
    const float logRatio = std::log(ratio) / std::log(sourceDataRatio);
    const float minPower = MIN_FREQ/std::pow(sourceDataMin, logRatio);
    
    normalized.clear();
    normalized.reserve(csvColumn.size());
     
     for(size_t i = 0; i < csvColumn.size(); ++i)
    {
        if(!std::isfinite(csvColumn[i]) || csvColumn[i] <= 0.f)
        {
            normalized.push_back(0.f);
        } else {
            float gPow = minPower * std::pow(csvColumn[i], logRatio);
            gPow = juce::jlimit(MIN_FREQ, MAX_FREQ, gPow);
            normalized.push_back(gPow);
        }
    }
     
     
    return normalized;
}

void AddCsv::csvColumnMenuChanged()
{
    csvFreqs = normalizeFrequenciesToAudibleRange(csvFreqs);
    int selectedColumnId = menu2->getSelectedId();
    int selectedColumnIndex = getSelectedColumnIndex(selectedColumnId);
    std::vector<juce::String> selectedColumn = getColumn(selectedColumnIndex);
    auto& currentApvts = processorRef.apvts;
    const size_t limit = std::min(csvFreqs.size(), CSV_MAX_ROWS);

    csvFreqs.clear();

    for(size_t i = 0; i < std::min(selectedColumn.size(), CSV_MAX_ROWS); ++i)
    {
        float currentFreq = selectedColumn[i].getFloatValue();
        csvFreqs.push_back(currentFreq);
    }
    
    updateChainSettings();
    
    for(size_t i = 0; i < limit; ++i)
    {
        auto* param = currentApvts.getParameter(std::format("Peak Freq {}", i));
        if(auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            const float freqHz = juce::jlimit(20.0f, 20000.0f, csvFreqs[i]);
            const float norm = rangedParam->convertTo0to1(freqHz);
            rangedParam->setValueNotifyingHost(norm);
        }
        
        auto* bandEnabled = currentApvts.getParameter(std::format("Peak Bypassed {}", i));
        if(bandEnabled)
        {
            bandEnabled->setValueNotifyingHost(0.0f);
        }
        
    }
    
    for(size_t i = limit; i < CSV_MAX_ROWS; ++i)
    {
        auto* bandEnabled = currentApvts.getParameter(std::format("Peak Bypassed {}", i));
        if(bandEnabled)
        {
            bandEnabled->setValueNotifyingHost(1.0f);
        }
    }
    
}

}
