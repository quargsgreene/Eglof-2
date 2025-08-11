#include "Eglof/PluginEditor.h"
#include "Eglof/PluginProcessor.h"
#include "../include/Eglof/AddCsv.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <sstream>
#include <tuple>
#include <utility>
audio_plugin::ResponseCurveComponent::ResponseCurveComponent(EglofAudioProcessor& p) :
audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params )
    {
        param->addListener(this);
    }

    updateChain();
    
    startTimerHz(60);
}

audio_plugin::ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params )
    {
        param->removeListener(this);
    }
}

template<std::size_t... I>
void audio_plugin::ResponseCurveComponent::updateResponseCurveImpl(std::index_sequence<I...>)
{
    using namespace juce;
    auto responseArea = getAnalysisArea();
    
    auto w = responseArea.getWidth();
    auto peaks = std::forward_as_tuple(monoChain.get<I>()...);
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<float> mags;
    
    mags.resize(static_cast<size_t>(w));
    
    for( int i = 0; i < w; ++i )
    {
        float mag = 1.f;

        auto freq = mapToLog10(static_cast<double>(i)/static_cast<double>(w), 20.0, 20000.0);
  
        ([&]
         {
            auto& f = std::get<I>(peaks);
            if(!monoChain.isBypassed<I>())
            {
                mag *= static_cast<float>(f.coefficients->getMagnitudeForFrequency(freq, sampleRate));
            }
        }(), ...);

        mags[static_cast<size_t>(i)] = static_cast<float>(Decibels::gainToDecibels(mag));
    }
    
    responseCurve.clear();
    
    const float outputMin = responseArea.getBottom();
    const float outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](float input)
    {
        return jmap<float>(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for( size_t i = 1; i < mags.size(); ++i )
    {
        responseCurve.lineTo(static_cast<size_t>(responseArea.getX()) + i, map(mags[i]));
    }
}

void audio_plugin::ResponseCurveComponent::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour(255u, 87u, 87u));

    drawBackgroundGrid(g);
    
    auto responseArea = getAnalysisArea();
    
    if( shouldShowFFTAnalysis )
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        
        g.setColour(Colour(97u, 18u, 167u)); //purple-
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
        
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        
        g.setColour(Colour(215u, 201u, 134u));
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }
    
    g.setColour(juce::Colour(140u, 82u, 255u));
    g.strokePath(responseCurve, PathStrokeType(2.f));
    
    Path border;
    
    border.setUsingNonZeroWinding(false);
    
    border.addRoundedRectangle(getRenderArea(), 4);
    border.addRectangle(getLocalBounds());
    
    g.setColour(juce::Colour(39u, 39u, 39u));
    
    g.fillPath(border);
    
    drawTextLabels(g);
    
    g.setColour(juce::Colour(140u, 82u, 255u));
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

std::vector<float> audio_plugin::ResponseCurveComponent::getFrequencies()
{
    return std::vector<float>
    {
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };
}

std::vector<float> audio_plugin::ResponseCurveComponent::getGains()
{
    return std::vector<float>
    {
        -24, -12, 0, 12, 24
    };
}

std::vector<float> audio_plugin::ResponseCurveComponent::getXs(const std::vector<float> &freqs, float left, float width)
{
    std::vector<float> xs;
    for( auto f : freqs )
    {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        xs.push_back( left + width * normX );
    }
    
    return xs;
}

void audio_plugin::ResponseCurveComponent::drawBackgroundGrid(juce::Graphics &g)
{
    using namespace juce;
    auto freqs = getFrequencies();
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto xs = getXs(freqs, left, width);
    
    g.setColour(juce::Colour(255u, 87u, 87u));
    for( auto x : xs )
    {
        g.drawVerticalLine(static_cast<int>(x), top, bottom);
    }
    
    auto gain = getGains();
    
    for( auto gDb : gain )
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        
        g.setColour(gDb == 0.f ? juce::Colour(0u, 191u, 99u) : juce::Colour(255u, 87u, 87u) );
        g.drawHorizontalLine(static_cast<int>(y), left, right);
    }
}

void audio_plugin::ResponseCurveComponent::drawTextLabels(juce::Graphics &g)
{
    using namespace juce;
    g.setColour(juce::Colour(255u, 87u, 87u));
    const int fontHeight = 8;
    g.setFont(fontHeight);
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);
    
    for( size_t i = 0; i < freqs.size(); ++i )
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if( f > 999.f )
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if( addK )
            str << "k";
        str << "Hz";
        
        auto font = g.getCurrentFont();
        juce::String text = str;
        juce::GlyphArrangement ga;
        ga.addLineOfText(font, text, 0.0f, 0.0f);
        auto bounds = ga.getBoundingBox(0, 20, true);
        auto textWidth = bounds.getWidth();

        Rectangle<int> r;

        r.setSize(static_cast<int>(textWidth), fontHeight);
        r.setCentre(static_cast<int>(x), 0);
        r.setY(1);
        
        g.drawFittedText(str, r, juce::Justification::centred, 10);
    }
    
    auto gain = getGains();

    for( auto gDb : gain )
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        
        String str;
        if( gDb > 0 )
            str << "+";
        str << gDb;
        
        auto font = g.getCurrentFont();
        juce::String text = str;
        juce::GlyphArrangement ga;
        ga.addLineOfText(font, text, 0.0f, 0.0f);
        auto bounds = ga.getBoundingBox(0, 10, true);
        auto textWidth = bounds.getWidth();
        
        Rectangle<int> r;
        r.setSize(static_cast<int>(textWidth), fontHeight);
        r.setX(getWidth() - static_cast<int>(textWidth) - 15);
        r.setCentre(r.getCentreX(), static_cast<int>(y));
        
        g.setColour(gDb == 0.f ? Colour(0u, 191u, 99u) : juce::Colour(255u, 87u, 87u));
        
        g.drawFittedText(str, r, juce::Justification::centredLeft, 10);
        
        str.clear();
        str << (gDb - 24.f);
    }
}

void audio_plugin::ResponseCurveComponent::resized()
{
    using namespace juce;
    
    responseCurve.preallocateSpace(getWidth() * 3);
    updateResponseCurve();
}

void audio_plugin::ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    juce::ignoreUnused(parameterIndex);
    juce::ignoreUnused(newValue);
    parametersChanged.set(true);
}

juce::Rectangle<int> audio_plugin::ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(15);
    bounds.removeFromBottom(35);
    bounds.removeFromLeft(35);
    bounds.removeFromRight(35);
    
    return bounds;
}


juce::Rectangle<int> audio_plugin::ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}

void audio_plugin::ResponseCurveComponent::timerCallback()
{
    if( shouldShowFFTAnalysis )
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();
        
        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if( parametersChanged.compareAndSetBool(false, true) )
    {
        updateChain();
        updateResponseCurve();
    }
    
    repaint();
}

template<std::size_t... I>
void audio_plugin::ResponseCurveComponent::updateChainImpl(std::index_sequence<I...>)
{
    std::vector<Peak> currentChainSettings = getChainSettings(audioProcessor.apvts);
    float gainOffsetDb = 0.0f;
    bool  allBypassed = false;
    auto* gainParam = audioProcessor.apvts.getRawParameterValue("Peak Master Gain");
    auto* bypassAllParam = audioProcessor.apvts.getRawParameterValue("Bypass All");
    if(gainParam)
    {
        gainOffsetDb = gainParam->load();
        std::cout<<"param=" << gainParam->load();
 
    }
    
    if(bypassAllParam)
    {
        allBypassed = static_cast<bool>(bypassAllParam ->load());
        std::cout<<"All Bypassed="<<allBypassed<<std::endl;
    }
    
    auto peakCoefficients = makePeakFilters(currentChainSettings, audioProcessor.getSampleRate(), gainOffsetDb);
    std::vector<bool> bypassedSettings;
    bypassedSettings.reserve(currentChainSettings.size());
    for (size_t i = 0; i < static_cast<size_t>(currentChainSettings.size()); ++i)
    {
        bypassedSettings.push_back(currentChainSettings[i].freq <= 0.0f ? true : currentChainSettings[i].bypass);
    }
    (monoChain.setBypassed<I>(I < currentChainSettings.size() ? bypassedSettings[I] : true), ...);
    
    ([&](){
        if (I < peakCoefficients.size())
            updateCoefficients(monoChain.get<I>().coefficients, peakCoefficients[I]);
    }(), ...);
 
}

void audio_plugin::RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(static_cast<float>(getValue()), static_cast<float>(range.getStart()), static_cast<float>(range.getEnd()), 0.0f, 1.0f),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(0u, 191u, 99u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for( int i = 0; i < numChoices; ++i )
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;


        
        auto str = labels[i].label;
        auto font = g.getCurrentFont();
        juce::String text = str;
        juce::GlyphArrangement ga;
        ga.addLineOfText(font, text, 0.0f, 0.0f);
        auto bounds = ga.getBoundingBox(0, 2, true);
        auto textWidth = bounds.getWidth();
        
        r.setSize(textWidth, getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<int> audio_plugin::RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
    
}

juce::String audio_plugin::RotarySliderWithLabels::getDisplayString() const
{
    if( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = static_cast<float>(getValue());
        juce::ignoreUnused(floatParam);
        
        if( val > 999.f )
        {
            val /= 1000.f; //1001 / 1000 = 1.001
            addK = true;
        }
        
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse; //this shouldn't happen!
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " ";
        if( addK )
            str << "k";
        
        str << suffix;
    }
    
    return str;
}


void audio_plugin::PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    
    juce::AudioBuffer<float> tempIncomingBuffer;
    while( leftChannelFifo->getNumCompleteBuffersAvailable() > 0 )
    {
        if( leftChannelFifo->getAudioBuffer(tempIncomingBuffer) )
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                              monoBuffer.getReadPointer(0, size),
                                              monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);
            
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while( leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0 )
    {
        std::vector<float> fftData;
        if( leftChannelFFTDataGenerator.getFFTData( fftData) )
        {
            pathProducer.generatePath(fftData, fftBounds, static_cast<int>(fftSize), static_cast<float>(binWidth), -48.f);
        }
    }
    
    while( pathProducer.getNumPathsAvailable() > 0 )
    {
        pathProducer.getPath( leftChannelFFTPath );
    }
}
//==


namespace audio_plugin {

EglofAudioProcessorEditor::EglofAudioProcessorEditor(
    EglofAudioProcessor& p)
    : AudioProcessorEditor(&p),
    openButton("csv", "*.csv", "Choose a CSV...", "Save File", p),
    processorRef(p),
    peakLinearGainSlider(),
    responseCurveComponent(processorRef),
    peakLinearGainSliderAttachment(processorRef.apvts, "Peak Master Gain", peakLinearGainSlider),
    bypassButtonAttachment(processorRef.apvts, "Bypass All", bypassButton)
  
{
  juce::ignoreUnused(processorRef);
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
        int textBoxSizeX = 75;
        int textBoxSizeY = 25;
        bool readOnly = false;
        juce::ignoreUnused(textBoxSizeX);
        juce::ignoreUnused(textBoxSizeY);
        juce::ignoreUnused(readOnly);
        
        setSize(1200, 800);
    
        setLookAndFeel(&uiAesthetic);
    
        addAndMakeVisible (&openButton);
        openButton.setColumnMenus(&dataColumnMenu1, &dataColumnMenu2, &dataColumnMenu3, &dataColumnMenu4);
        openButton.setButtonText("Click to choose a CSV!");
        addAndMakeVisible(&presetMenu);
        presetMenu.setText("Default Preset");
        
        addAndMakeVisible(&dataColumnMenu1);
        dataColumnMenu1.setText("Q Mapping");
        addAndMakeVisible(&dataColumnMenu2);
        dataColumnMenu2.setText("CSV Column -> Frequency Mapping");
        addAndMakeVisible(&dataColumnMenu3);
        dataColumnMenu3.setText("Cutoff Mapping");
        addAndMakeVisible(&dataColumnMenu4);
        dataColumnMenu4.setText("Resonance Mapping");
        
        addAndMakeVisible(&forwardPresetButton);
        addAndMakeVisible(&backwardPresetButton);
        addAndMakeVisible(&compareButton);
        addAndMakeVisible(&copyButton);
        addAndMakeVisible(&pasteButton);
        addAndMakeVisible(&helpButton);
        addAndMakeVisible(&chooseRandomDataButton);
        addAndMakeVisible(&downloadCSVButton);
        
        for( auto* comp : getComps() )
        {
            addAndMakeVisible(comp);
        }
    
        auto safePtr = juce::Component::SafePointer<EglofAudioProcessorEditor>(this);
        bypassButton.onClick = [safePtr]()
        {
            if( auto* comp = safePtr.getComponent() )
            {
                auto bypassed = comp->bypassButton.getToggleState();
                comp->peakLinearGainSlider.setEnabled(!bypassed);
            }
        };
    
        peakLinearGainSlider.setRange(-24, 24);
}

EglofAudioProcessorEditor::~EglofAudioProcessorEditor() {}

std::vector<juce::Component*> EglofAudioProcessorEditor::getComps()
{
    return
    {
        &responseCurveComponent,
        &bypassButton,
        &peakLinearGainSlider
    };
}

void EglofAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  
  auto bounds = getLocalBounds();
  auto responseArea = bounds.removeFromBottom(2 * bounds.getHeight()/3);
  auto responseWidth = responseArea.getWidth();
  auto centre = bounds.getCentre();
  juce::ignoreUnused(centre);

  std::vector<float> magnitudes;
    
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colour(140u, 82u, 255u));
    // Top-left title
  g.setFont (juce::FontOptions ("Arial Rounded MT Bold", 18.0f, juce::Font::bold));
  g.drawText ("Eglof FILTER", getWidth()/3, 6, 260, 28, juce::Justification::centred, true);
  g.setFont (juce::FontOptions ("Courier", 18.0f, juce::Font::bold));

    // Bottom-left tagline
  g.setFont (juce::FontOptions ("Monaco", 24.0f, juce::Font::bold));
  g.setColour(juce::Colour(255u, 154u, 1u));
  g.drawText ("From spreadsheets",
                16, 120, 200, 36,
                juce::Justification::left, true);
  g.drawText ("to soundscapes :)",
                  45, 170, 200, 36,
                  juce::Justification::left, true);
  g.setColour(juce::Colour(140u, 82u, 255u));
  g.setFont (juce::FontOptions ("Courier", 15.0f, juce::Font::bold));

    
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

  g.setColour(juce::Colour(140u, 82u, 255u));
  g.setFont(15.0f);
  g.drawText("Gain Scale", getWidth()/4, 10, 120, 65, juce::Justification::centredLeft);

    
  g.setColour(juce::Colour(255u, 154u, 1u));
  g.setColour(juce::Colours::grey);
  g.setFont(14);
}

void EglofAudioProcessorEditor::resized() {
    int marginX = getWidth()/20;
    int marginY = 90;
    int dialWidth = (getWidth() - marginX)/8;
    int dialHeight = (getWidth() - marginX)/8;
    int menuWidth = 4*getWidth()/10;
    int menuHeight = (getWidth() - marginX)/8;
    juce::ignoreUnused(marginY);
    int gapX = 150;
    int gapY = gapX/3;
    juce::ignoreUnused(dialWidth);
    juce::ignoreUnused(dialHeight);
    juce::ignoreUnused(gapY);

    auto bounds = getLocalBounds();
    bounds.removeFromTop(4);
    
    
    bounds.removeFromTop(5);
    
    float hRatio = 60.f / 100.f;

    auto responseArea = bounds.removeFromBottom(static_cast<int>((bounds.getHeight() * hRatio))); //change from 0.33 to 0.25 because I needed peak hz text to not overlap the

    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5);
    
    
    bypassButton.setBounds(-15,-15, getWidth()/10, getWidth()/10);
    openButton.setBounds (3*getWidth()/4 , 35, 270, 230);
    dataColumnMenu2.setBounds(getWidth()/4, 120, menuWidth, menuHeight);
    peakLinearGainSlider.setBounds(dataColumnMenu2.getX(), dataColumnMenu2.getY() - 60, 4*getWidth()/10, 50);

}

}  // namespace audio_plugin
