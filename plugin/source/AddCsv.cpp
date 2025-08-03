#include "../include/Eglof/AddCsv.h"
#include "../include/Eglof/PluginEditor.h"
#include <juce_core/juce_core.h>


namespace audio_plugin{

AddCsv::AddCsv(const juce::String &fileExtension,
               const juce::String &fileWildCard,
               const juce::String &openFileDialogTitle,
               const juce::String &saveFileDialogTitle): juce::FileBasedDocument(fileExtension,
                                                                                 fileWildCard,
                                                                                 openFileDialogTitle,
                                                                                 saveFileDialogTitle),
fileExtName ("csv"),
wildcardExtName("*.csv"),
openDialogText("Select a CSV file..."),
saveDialogText("Save file")
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

juce::StringArray AddCsv::getCsvColumns(auto file)
{
    juce::StringArray csvRows;
    file.readLines(csvRows);
    juce::String csvColumnString = csvRows[0];
    std::cout<<"Column String Length:"<<std::endl;
    std::cout<<csvColumnString.length()<<std::endl;

    juce::String currentColumn;
    juce::StringArray columns;
    juce::String comma(",");
    juce::String space(" ");
    juce::String singleQuote("'");
    
    for (int i = 0; i < csvColumnString.length(); i++)
    {
        std::cout <<"i:"<<std::endl;
        std::cout<<i<<std::endl;
        std::cout<<"Current Column before if else:"<<std::endl;
        std::cout<<currentColumn<<std::endl;
        juce::String currentChar = juce::String::charToString(csvColumnString[i]);
        if(currentChar == comma){
            columns.add(currentColumn);
            currentColumn = "";
        }else if (currentChar == singleQuote || currentChar == space)
            continue;
        else{
            currentColumn += currentChar;
        }
        std::cout <<"Current Char:"<<std::endl;
        std::cout<<currentChar << std::endl;
        std::cout<<"Current Column after if else:"<<std::endl;
        std::cout<<currentColumn<<std::endl;
    }
    if(currentColumn.isNotEmpty()){
        columns.add(currentColumn);
    }
    for(int i = 0; i < columns.size(); i++){
        std::cout<<columns[i]<<std::endl;
    }
    return columns;
}

void AddCsv::setColumnMenus(CsvColumnSelectionDropdown *m1, CsvColumnSelectionDropdown *m2, CsvColumnSelectionDropdown *m3, CsvColumnSelectionDropdown *m4)
    {
  
        menu1 = m1;
        menu2 = m2;
        menu3 = m3;
        menu4 = m4;
    }


}
