#include "Eglof/PluginEditor.h"
#include "Eglof/PluginProcessor.h"
#include "../include/Eglof/csvDragDrop.h"
#include <sstream>

namespace audio_plugin {
EglofAudioProcessorEditor::EglofAudioProcessorEditor(
    EglofAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
        int textBoxSizeX = 75;
        int textBoxSizeY = 25;
        bool readOnly = false;
        
        setSize(1200, 800);
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Choose a CSV File!");
        openButton.onClick = [this] {openButton.openButtonClicked(); };
        
        addAndMakeVisible(&qRangeSlider);
        
        qRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        qRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
        qRangeSlider.setRange(0.1, 10, 0.1);
        qRangeSlider.setValue(1);
        
        addAndMakeVisible(&gainRangeSlider);
        gainRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        gainRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
        gainRangeSlider.setRange(-6, 6, 0.1);
        gainRangeSlider.setValue(0);
        gainRangeSlider.setTextValueSuffix("Db");

        addAndMakeVisible(&cutoffRangeSlider);
        cutoffRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        cutoffRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
        cutoffRangeSlider.setRange(20, 20000, 1);
        cutoffRangeSlider.setValue(20);
        cutoffRangeSlider.setTextValueSuffix("Hz");
        
        addAndMakeVisible(&resonanceRangeSlider);
        resonanceRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        resonanceRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
        resonanceRangeSlider.setRange(0, 1, 0.01);
        resonanceRangeSlider.setValue(0);
        
        addAndMakeVisible(&presetMenu);
        addAndMakeVisible(&dataColumnMenu1);
        addAndMakeVisible(&dataColumnMenu2);
        addAndMakeVisible(&dataColumnMenu3);
        addAndMakeVisible(&dataColumnMenu4);
        
        float powerButtonSize = 50.f;
        juce::Path powerButtonShape;
        powerButtonShape.addRectangle(0, 0, powerButtonSize, powerButtonSize);
        powerButton.setShape(powerButtonShape, true, true, false);
        addAndMakeVisible(&powerButton);
        
        addAndMakeVisible(&forwardPresetButton);
        addAndMakeVisible(&backwardPresetButton);
        addAndMakeVisible(&compareButton);
        addAndMakeVisible(&copyButton);
        addAndMakeVisible(&pasteButton);
        addAndMakeVisible(&helpButton);
        addAndMakeVisible(&chooseRandomDataButton);
        addAndMakeVisible(&downloadCSVButton);

}

EglofAudioProcessorEditor::~EglofAudioProcessorEditor() {}

void EglofAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  
  auto bounds = getLocalBounds();
  auto responseArea = bounds.removeFromBottom(2 * bounds.getHeight()/3);
  auto responseWidth = responseArea.getWidth();
  std::vector<float> magnitudes;
    
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
  g.drawFittedText("Eglof FILTER", getLocalBounds(),
                   juce::Justification::centredTop, 1);
    
  magnitudes.resize(static_cast<std::vector<int>::size_type>(responseWidth));

  for(int i = 0; i < responseWidth; ++i)
  {
    double magnitude = 1;
    magnitudes[static_cast<std::vector<int>::size_type>(i)] = static_cast<std::vector<int>::size_type>(magnitude);
  }
  juce::Path responseCurve;
  responseCurve.startNewSubPath(responseArea.getX(), magnitudes.front());
      
  for(size_t i = 1; i < magnitudes.size(); ++i)
  {
    responseCurve.lineTo(static_cast<size_t>(responseArea.getX() - 3) + i, magnitudes[i] + getWidth()/2);
  }
    

  g.setColour(juce::Colours::black);
  g.fillRoundedRectangle(responseArea.toFloat(),4);
  g.setColour(juce::Colours::orange);
  g.drawRoundedRectangle(responseArea.toFloat(), 4, 1);
  g.setColour(juce::Colours::white);
  g.strokePath(responseCurve, juce::PathStrokeType(2));
    
  g.drawImage(background, getLocalBounds().toFloat());
    
  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
  g.drawText("Q", getWidth()/40, 15, 60, 175, juce::Justification::left);
  g.drawText("Gain", getWidth()/40 + 150, 15, 60, 175, juce::Justification::left);
  g.drawText("Cutoff", getWidth()/40 + 300, 15, 60, 175, juce::Justification::left);
  g.drawText("Resonance", getWidth()/40 + 440, 15, 100, 175, juce::Justification::left);
}

void EglofAudioProcessorEditor::resized() {
    int marginX = getWidth()/20;
    int marginY = 90;
    int dialWidth = (getWidth() - marginX)/8;
    int dialHeight = (getWidth() - marginX)/8;
    int menuWidth = (getWidth() - marginX)/4;
    int menuHeight = (getWidth() - marginX)/32;
    int gapX = 150;
    int gapY = gapX/3;
    background = juce::Image(juce::Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    juce::Graphics g(background);
    juce::Array<float> freqs
    {
        20, 30, 40, 50, 100,
        200, 300, 400, 500, 1000,
        2000, 3000, 4000, 5000, 10000,
        20000
    };
    juce::Array<float> gain
    {
        -240, -120, 0, 120, 240
    };
    
    g.setColour(juce::Colours::greenyellow);
    const int fontHeight = 10;
    g.setFont(fontHeight);
    
    for(auto f : freqs)
    {
        bool addK = false;
        std::string suff;
        auto normX = f;
        g.drawVerticalLine(static_cast<int>(normX), getHeight()/3, getHeight());
        if (f > 999)
        {
            addK = true;
            f /= 1000;
        }
        std::stringstream freqLabelSS;
        freqLabelSS << suff << f;
        if (addK)
        {
            freqLabelSS << suff << "k";
        }
        freqLabelSS << suff << "Hz";
        
        std::string freqLabel = freqLabelSS.str();
        
        g.drawText(freqLabel, static_cast<int>(normX), getHeight()/3, 100, 100, juce::Justification::centred, false);
    }
    
    for(auto gainDb : gain)
    {
        auto normY = gainDb + getWidth()/2;
        g.setColour(std::abs(gainDb) <= 0 ? juce::Colours::pink : juce::Colour(0u, 172u, 1u));
        g.drawHorizontalLine(static_cast<int>(normY), 0, getWidth());
        std::string pref;
        std::string suff;
        int addNeg = false;
        if (gainDb > 0)
        {
            addNeg = true;
        }
        std::stringstream gainLabelSS;
        if (addNeg)
        {
            gainLabelSS << "-";
            gainLabelSS << gainDb/10;
        } else {
            gainLabelSS << gainDb/10;
        }
        gainLabelSS << suff << "Db";
        std::string gainLabel = gainLabelSS.str();
        
        g.drawText(gainLabel, 0, static_cast<int>(normY), 100, 100, juce::Justification::centred, false);
    }
    qRangeSlider.setBounds(marginX, marginY, dialWidth, dialHeight);
    gainRangeSlider.setBounds(marginX + gapX, marginY, dialWidth, dialHeight);
    cutoffRangeSlider.setBounds(marginX + 2 * gapX, marginY, dialWidth, dialHeight);
    resonanceRangeSlider.setBounds(marginX + 3 * gapX, marginY, dialWidth, dialHeight);
    openButton.setBounds (1000, 35, 190, 200);
    
    presetMenu.setBounds(marginX + gapX/3, marginY - 80, menuWidth, menuHeight);
    dataColumnMenu1.setBounds(marginX + 4 * gapX + 15, marginY, menuWidth/2, menuHeight);
    dataColumnMenu2.setBounds(marginX + 5 * gapX + 30, marginY, menuWidth/2, menuHeight);
    dataColumnMenu3.setBounds(marginX + 4 * gapX + 15, marginY + gapY, menuWidth/2, menuHeight);
    dataColumnMenu4.setBounds(marginX + 5 * gapX + 30, marginY + gapY, menuWidth/2, menuHeight);
    
    powerButton.setBounds(0, 0, getWidth()/10, getWidth()/10);
    backwardPresetButton.setBounds(presetMenu.getX(), 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    forwardPresetButton.setBounds(presetMenu.getX() + gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    compareButton.setBounds(presetMenu.getX() + 2 * gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/3, presetMenu.getHeight());
    copyButton.setBounds(presetMenu.getX() + 4 * gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    pasteButton.setBounds(presetMenu.getX() + 5 * gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    helpButton.setBounds(dataColumnMenu1.getX(), dataColumnMenu1.getY() - gapY, dataColumnMenu1.getWidth(), dataColumnMenu1.getHeight());
    downloadCSVButton.setBounds(dataColumnMenu2.getX(), dataColumnMenu2.getY() - gapY, dataColumnMenu2.getWidth(), dataColumnMenu2.getHeight());
    chooseRandomDataButton.setBounds(dataColumnMenu1.getX(), dataColumnMenu1.getY() + 2 * gapY, 2 * dataColumnMenu1.getWidth() + 25, dataColumnMenu1.getHeight() + 10);
}

}  // namespace audio_plugin
