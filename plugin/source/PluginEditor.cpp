#include "Eglof/PluginEditor.h"
#include "Eglof/PluginProcessor.h"
#include "../include/Eglof/AddCsv.h"
#include <sstream>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
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

void audio_plugin::ResponseCurveComponent::updateResponseCurve()
{
    using namespace juce;
    auto responseArea = getAnalysisArea();
    
    auto w = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& peak2 = monoChain.get<ChainPositions::Peak2>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    juce::ignoreUnused(lowcut, highcut);
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<float> mags;
    
    mags.resize(static_cast<size_t>(w));
    
    for( int i = 0; i < w; ++i )
    {
        float mag = 1.f;
        auto freq = mapToLog10(static_cast<double>(i)/static_cast<double>(w), 20.0, 20000.0);
        
        if(! monoChain.isBypassed<ChainPositions::Peak>() ){
            mag *= static_cast<float>(peak.coefficients->getMagnitudeForFrequency(freq, sampleRate));
        }
        if(! monoChain.isBypassed<ChainPositions::Peak2>() ){
            mag *= static_cast<float>(peak2.coefficients->getMagnitudeForFrequency(freq, sampleRate));
        }
//        if( !monoChain.isBypassed<ChainPositions::LowCut>() )
//        {
//            if( !lowcut.isBypassed<0>() )
//                mag *= static_cast<float>(lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//            if( !lowcut.isBypassed<1>() )
//                mag *= static_cast<float>(lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//            if( !lowcut.isBypassed<2>() )
//                mag *= static_cast<float>(lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//            if( !lowcut.isBypassed<3>() )
//                mag *= static_cast<float>(lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//        }
//
//        if( !monoChain.isBypassed<ChainPositions::HighCut>() )
//        {
//            if( !highcut.isBypassed<0>() )
//                mag *= static_cast<float>(highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//            if( !highcut.isBypassed<1>() )
//                mag *= static_cast<float>(highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//            if( !highcut.isBypassed<2>() )
//                mag *= static_cast<float>(highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//            if( !highcut.isBypassed<3>() )
//                mag *= static_cast<float>(highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate));
//        }
//
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
    const int fontHeight = 10;
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
        r.setX(getWidth() - static_cast<int>(textWidth));
        r.setCentre(r.getCentreX(), static_cast<int>(y));
        
        g.setColour(gDb == 0.f ? Colour(0u, 191u, 99u) : juce::Colour(255u, 87u, 87u));
        
        g.drawFittedText(str, r, juce::Justification::centredLeft, 10);
        
        str.clear();
        str << (gDb - 24.f);

//        r.setX(1);
//        textWidth = bounds.getWidth();
//        r.setSize(static_cast<int>(textWidth), fontHeight);
//        g.setColour(Colours::lightgrey);
//        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
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
    
    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);
    
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
void audio_plugin::ResponseCurveComponent::updateChain()
{
    auto currentChainSettings = getChainSettings(audioProcessor.apvts);
    auto peakCoefficients = makePeakFilters(currentChainSettings, audioProcessor.getSampleRate());
    std::vector<std::atomic< float >*> bypassedSettings;
    for (auto& [gainFreqPair, qBypassedPair] : currentChainSettings)
    {
        bypassedSettings.push_back(qBypassedPair.second);
    }
    (monoChain.setBypassed<I>(bypassedSettings[I]), ...);
    (updateCoefficients(monoChain.get<I>().coefficients, peakCoefficients),...);
    
//    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
//    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
//    monoChain.setBypassed<ChainPositions::Peak2>(chainSettings.peak2Bypassed);
//    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
//    
//    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
//    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//    
//    auto peak2Coefficients = makePeakFilter2(chainSettings, audioProcessor.getSampleRate());
//    updateCoefficients(monoChain.get<ChainPositions::Peak2>().coefficients, peak2Coefficients);
//    
//    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
//    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
//    
//    updateCutFilter(monoChain.get<ChainPositions::LowCut>(),
//                    lowCutCoefficients,
//                    chainSettings.lowCutSlope);
//    
//    updateCutFilter(monoChain.get<ChainPositions::HighCut>(),
//                    highCutCoefficients,
//                    chainSettings.highCutSlope);
   
}

void audio_plugin::RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
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
    openButton("csv", "*.csv", "Choose a CSV...", "Save File"),
    processorRef(p),
    peakFreqSlider(*processorRef.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*processorRef.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*processorRef.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider(*processorRef.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*processorRef.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*processorRef.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider(*processorRef.apvts.getParameter("HighCut Slope"), "db/Oct"),
    peakLinearGainSlider(),

    responseCurveComponent(processorRef),

    peakFreqSliderAttachment(processorRef.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(processorRef.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(processorRef.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(processorRef.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(processorRef.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(processorRef.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(processorRef.apvts, "HighCut Slope", highCutSlopeSlider),
    peakLinearGainSliderAttachment(processorRef.apvts, "Peak Gain", peakLinearGainSlider),

    lowcutBypassButtonAttachment(processorRef.apvts, "LowCut Bypassed", lowcutBypassButton),
    peakBypassButtonAttachment(processorRef.apvts, "Peak Bypassed", peakBypassButton),
    highcutBypassButtonAttachment(processorRef.apvts, "HighCut Bypassed", highcutBypassButton),
    analyzerEnabledButtonAttachment(processorRef.apvts, "Analyzer Enabled", analyzerEnabledButton)
   
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
//        addAndMakeVisible(&qRangeSlider);
//
//        qRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
//        qRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
//        qRangeSlider.setRange(0.1, 10, 0.1);
//        qRangeSlider.setValue(1);
//
//        addAndMakeVisible(&gainRangeSlider);
//        gainRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
//        gainRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
//        gainRangeSlider.setRange(-6, 6, 0.1);
//        gainRangeSlider.setValue(0);
//        gainRangeSlider.setTextValueSuffix("Db");
//
//        addAndMakeVisible(&cutoffRangeSlider);
//        cutoffRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
//        cutoffRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
//        cutoffRangeSlider.setRange(20, 20000, 1);
//        cutoffRangeSlider.setValue(20);
//        cutoffRangeSlider.setTextValueSuffix("Hz");
//
//        addAndMakeVisible(&resonanceRangeSlider);
//        resonanceRangeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
//        resonanceRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, readOnly, textBoxSizeX, textBoxSizeY);
//        resonanceRangeSlider.setRange(0, 1, 0.01);
//        resonanceRangeSlider.setValue(0);
        
        addAndMakeVisible(&presetMenu);
        presetMenu.setText("Default Preset");
        
        addAndMakeVisible(&dataColumnMenu1);
        dataColumnMenu1.setText("Q Mapping");
        addAndMakeVisible(&dataColumnMenu2);
        dataColumnMenu2.setText("Gain Mapping");
        addAndMakeVisible(&dataColumnMenu3);
        dataColumnMenu3.setText("Cutoff Mapping");
        addAndMakeVisible(&dataColumnMenu4);
        dataColumnMenu4.setText("Resonance Mapping");
        
//        float powerButtonSize = 50.f;
//        juce::Path powerButtonShape;
//        powerButtonShape.addRectangle(0, 0, powerButtonSize, powerButtonSize);
//        powerButton.setShape(powerButtonShape, true, true, false);
//        addAndMakeVisible(&powerButton);
        
        addAndMakeVisible(&forwardPresetButton);
        addAndMakeVisible(&backwardPresetButton);
        addAndMakeVisible(&compareButton);
        addAndMakeVisible(&copyButton);
        addAndMakeVisible(&pasteButton);
        addAndMakeVisible(&helpButton);
        addAndMakeVisible(&chooseRandomDataButton);
        addAndMakeVisible(&downloadCSVButton);
    
//        peakFreqSlider.labels.add({0.f, "20Hz"});
//        peakFreqSlider.labels.add({1.f, "20kHz"});
        
        peakGainSlider.labels.add({0.f, "-24dB"});
        peakGainSlider.labels.add({1.f, "+24dB"});
        
//        peakQualitySlider.labels.add({0.f, "0.1"});
//        peakQualitySlider.labels.add({1.f, "10.0"});
//
//        lowCutFreqSlider.labels.add({0.f, "20Hz"});
//        lowCutFreqSlider.labels.add({1.f, "20kHz"});
//
//        highCutFreqSlider.labels.add({0.f, "20Hz"});
//        highCutFreqSlider.labels.add({1.f, "20kHz"});
//
//        lowCutSlopeSlider.labels.add({0.0f, "12"});
//        lowCutSlopeSlider.labels.add({1.f, "48"});
//
//        highCutSlopeSlider.labels.add({0.0f, "12"});
//        highCutSlopeSlider.labels.add({1.f, "48"});
        
        for( auto* comp : getComps() )
        {
            addAndMakeVisible(comp);
        }
    
        auto safePtr = juce::Component::SafePointer<EglofAudioProcessorEditor>(this);
        peakBypassButton.onClick = [safePtr]()
        {
            if( auto* comp = safePtr.getComponent() )
            {
                auto bypassed = comp->peakBypassButton.getToggleState();
                
                comp->peakFreqSlider.setEnabled( !bypassed );
                comp->peakGainSlider.setEnabled( !bypassed );
                comp->peakQualitySlider.setEnabled( !bypassed );
                comp->peakLinearGainSlider.setEnabled( !bypassed );
            }
        };
    
        peakLinearGainSlider.setRange(-24, 24);
        
//
//        lowcutBypassButton.onClick = [safePtr]()
//        {
//            if( auto* comp = safePtr.getComponent() )
//            {
//                auto bypassed = comp->lowcutBypassButton.getToggleState();
//
//                comp->lowCutFreqSlider.setEnabled( !bypassed );
//                comp->lowCutSlopeSlider.setEnabled( !bypassed );
//            }
//        };
//
//        highcutBypassButton.onClick = [safePtr]()
//        {
//            if( auto* comp = safePtr.getComponent() )
//            {
//                auto bypassed = comp->highcutBypassButton.getToggleState();
//
//                comp->highCutFreqSlider.setEnabled( !bypassed );
//                comp->highCutSlopeSlider.setEnabled( !bypassed );
//            }
//        };
//
//        analyzerEnabledButton.onClick = [safePtr]()
//        {
//            if( auto* comp = safePtr.getComponent() )
//            {
//                auto enabled = comp->analyzerEnabledButton.getToggleState();
//                comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
//            }
//        };
//
}

EglofAudioProcessorEditor::~EglofAudioProcessorEditor() {}

std::vector<juce::Component*> EglofAudioProcessorEditor::getComps()
{
    return
    {
//        &peakFreqSlider,
//        &peakGainSlider,
//        &peakQualitySlider,
//        &lowCutFreqSlider,
//        &highCutFreqSlider,
//        &lowCutSlopeSlider,
//        &highCutSlopeSlider,
        &responseCurveComponent,
        
//        &lowcutBypassButton,
        &peakBypassButton,
//        &highcutBypassButton,
//        &analyzerEnabledButton,
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
  g.setFont(juce::FontOptions("Courier", 15.0f, juce::Font::bold));
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

//  g.setColour(juce::Colours::black);
//  g.fillRoundedRectangle(responseArea.toFloat(),4);
//  g.setColour(juce::Colours::orange);
//  g.drawRoundedRectangle(responseArea.toFloat(), 4, 1);
//  g.setColour(juce::Colours::white);
//  g.strokePath(responseCurve, juce::PathStrokeType(2));
    
//  g.drawImage(background, getLocalBounds().toFloat());
    
  g.setColour(juce::Colour(140u, 82u, 255u));
  g.setFont(15.0f);
//  g.drawText("Q", getWidth()/40, 15, 60, 175, juce::Justification::left);
  g.drawText("Cut Gain", getWidth()/40 + 150, 15, 120, 200, juce::Justification::centredLeft);
//  g.drawText("Cutoff", getWidth()/40 + 300, 15, 60, 175, juce::Justification::left);
//  g.drawText("Resonance", getWidth()/40 + 440, 15, 100, 175, juce::Justification::left);
    
    
  g.setColour(juce::Colour(255u, 154u, 1u));
  g.setColour(juce::Colours::grey);
  g.setFont(14);
//  g.drawFittedText("LowCut", lowCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
//  g.drawFittedText("Peak", peakQualitySlider.getBounds(), juce::Justification::centredBottom, 1);
//  g.drawFittedText("HighCut", highCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
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
    juce::ignoreUnused(dialWidth);
    juce::ignoreUnused(dialHeight);
    juce::ignoreUnused(gapY);
//    background = juce::Image(juce::Image::PixelFormat::RGB, getWidth(), getHeight(), true);
//    juce::Graphics g(background);
//    juce::Array<float> freqs
//    {
//        20, 30, 40, 50, 100,
//        200, 300, 400, 500, 1000,
//        2000, 3000, 4000, 5000, 10000,
//        20000
//    };
//    juce::Array<float> gain
//    {
//        -240, -120, 0, 120, 240
//    };
//
//    g.setColour(juce::Colours::greenyellow);
//    const int fontHeight = 10;
//    g.setFont(fontHeight);
//
//    for(auto f : freqs)
//    {
//        bool addK = false;
//        std::string suff;
//        auto normX = f;
//        g.drawVerticalLine(static_cast<int>(normX), getHeight()/3, getHeight());
//        if (f > 999)
//        {
//            addK = true;
//            f /= 1000;
//        }
//        std::stringstream freqLabelSS;
//        freqLabelSS << suff << f;
//        if (addK)
//        {
//            freqLabelSS << suff << "k";
//        }
//        freqLabelSS << suff << "Hz";
//
//        std::string freqLabel = freqLabelSS.str();
//
//        g.drawText(freqLabel, static_cast<int>(normX), getHeight()/3, 100, 100, juce::Justification::centred, false);
//    }
//
//    for(auto gainDb : gain)
//    {
//        auto normY = gainDb + getWidth()/2;
//        g.setColour(std::abs(gainDb) <= 0 ? juce::Colours::pink : juce::Colour(0u, 172u, 1u));
//        g.drawHorizontalLine(static_cast<int>(normY), 0, getWidth());
//        std::string pref;
//        std::string suff;
//        int addNeg = false;
//        if (gainDb > 0)
//        {
//            addNeg = true;
//        }
//        std::stringstream gainLabelSS;
//        if (addNeg)
//        {
//            gainLabelSS << "-";
//            gainLabelSS << gainDb/10;
//        } else {
//            gainLabelSS << gainDb/10;
//        }
//        gainLabelSS << suff << "Db";
//        std::string gainLabel = gainLabelSS.str();
//
//        g.drawText(gainLabel, 0, static_cast<int>(normY), 100, 100, juce::Justification::centred, false);
//    }
   /*qRangeSlider.setBounds(marginX, marginY, dialWidth, dialHeight);
    gainRangeSlider.setBounds(marginX + gapX, marginY, dialWidth, dialHeight);
    cutoffRangeSlider.setBounds(marginX + 2 * gapX, marginY, dialWidth, dialHeight);
    resonanceRangeSlider.setBounds(marginX + 3 * gapX, marginY, dialWidth, dialHeight);*/
    auto bounds = getLocalBounds();
    bounds.removeFromTop(4);
    
//    auto analyzerEnabledArea = bounds.removeFromTop(25);
//
//
//    analyzerEnabledArea.setWidth(50);
//    analyzerEnabledArea.setX(5);
//    analyzerEnabledArea.removeFromBottom(2);
//
//    analyzerEnabledButton.setBounds(analyzerEnabledArea);
    
    bounds.removeFromTop(5);
    
    float hRatio = 70.f / 100.f; //JUCE_LIVE_CONSTANT(25) / 100.f;

    auto responseArea = bounds.removeFromBottom(static_cast<int>((bounds.getHeight() * hRatio))); //change from 0.33 to 0.25 because I needed peak hz text to not overlap the slider thumb

    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5);
    
//    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth()/3);
//    auto highCutArea = bounds.removeFromRight(bounds.getWidth()/2);
//
//    lowcutBypassButton.setBounds(lowCutArea.removeFromTop(25));
//    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()/2));
//    lowCutSlopeSlider.setBounds(lowCutArea);
//
//    highcutBypassButton.setBounds(highCutArea.removeFromTop(25));
//    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()/2));
//    highCutSlopeSlider.setBounds(highCutArea);
    peakLinearGainSlider.setBounds(backwardPresetButton.getX() + 100, backwardPresetButton.getY() + 125, 4*getWidth()/10, 50);
    peakBypassButton.setBounds(-30,-30, getWidth()/10, getWidth()/10);
//    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()/3));
//    peakGainSlider.setBounds(getWidth()/4, getHeight()/8, getWidth()/8, getHeight()/8);
//    peakQualitySlider.setBounds(bounds);
    openButton.setBounds (1000, 35, 190, 200);
    
    presetMenu.setBounds(marginX + gapX/3, marginY - 80, menuWidth, menuHeight);
//    dataColumnMenu1.setBounds(marginX + 4 * gapX + 15, marginY, menuWidth/2, menuHeight);
    dataColumnMenu2.setBounds((peakFreqSlider.getX() + openButton.getX())/2 + getWidth()/10, peakLinearGainSlider.getY() + 5, menuWidth + gapX/2, menuHeight);
//    dataColumnMenu3.setBounds(marginX + 4 * gapX + 15, marginY + gapY, menuWidth/2, menuHeight);
//    dataColumnMenu4.setBounds(marginX + 5 * gapX + 30, marginY + gapY, menuWidth/2, menuHeight);
//
//    powerButton.setBounds(0, 0, getWidth()/10, getWidth()/10);
    backwardPresetButton.setBounds(presetMenu.getX(), 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    forwardPresetButton.setBounds(presetMenu.getX() + gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    compareButton.setBounds(presetMenu.getX() + 2 * gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/3, presetMenu.getHeight());
    copyButton.setBounds(presetMenu.getX() + 4 * gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
    pasteButton.setBounds(presetMenu.getX() + 5 * gapX/3, 11 * presetMenu.getY()/2, presetMenu.getWidth()/6, presetMenu.getHeight());
/*    helpButton.setBounds(dataColumnMenu1.getX(), dataColumnMenu1.getY() - gapY, dataColumnMenu1.getWidth(), dataColumnMenu1.getHeight())*/;
//    downloadCSVButton.setBounds(dataColumnMenu2.getX(), dataColumnMenu2.getY() - gapY, dataColumnMenu2.getWidth(), dataColumnMenu2.getHeight());
//    chooseRandomDataButton.setBounds(dataColumnMenu1.getX(), dataColumnMenu1.getY() + 2 * gapY, 2 * dataColumnMenu1.getWidth() + 25, dataColumnMenu1.getHeight() + 10);
}

}  // namespace audio_plugin
