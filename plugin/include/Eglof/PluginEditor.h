#pragma once

#include "PluginProcessor.h"
#include "Knob.h"
#include "Menu.h"
#include "AddCsv.h"
#include "CsvColumnSelectionDropdown.h"
#include "LookAndFeel.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

namespace audio_plugin {

template<typename BlockType>
struct FFTDataGenerator
{

    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        const auto fftSize = getFFTSize();
        
        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());
        
        // first apply a windowing function to our data
        window->multiplyWithWindowingTable (fftData.data(), static_cast<size_t>(fftSize));       // [1]
        
        // then render our FFT data..
        forwardFFT->performFrequencyOnlyForwardTransform (fftData.data());  // [2]
        
        int numBins = static_cast<int>(fftSize / 2);
        
        //normalize the fft values.
        for( int i = 0; i < numBins; ++i )
        {
            auto v = fftData[static_cast<size_t>(i)];
//            fftData[i] /= (float) numBins;
            if( !std::isinf(v) && !std::isnan(v) )
            {
                v /= static_cast<size_t>(float(numBins));
            }
            else
            {
                v = 0.f;
            }
            fftData[static_cast<size_t>(i)] = v;
        }
        
        //convert them to decibels
        for( int i = 0; i < numBins; ++i )
        {
            fftData[static_cast<size_t>(i)] = juce::Decibels::gainToDecibels(fftData[static_cast<size_t>(i)], negativeInfinity);
        }
        
        fftDataFifo.push(fftData);
    }
    
    void changeOrder(FFTOrder newOrder)
    {
                
        order = newOrder;
        auto fftSize = getFFTSize();
        
        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
        
        fftData.clear();
        fftData.resize(static_cast<size_t>(fftSize * 2), 0);

        fftDataFifo.prepare(fftData.size());
    }
    //==============================================================================
    int getFFTSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
    //==============================================================================
    bool getFFTData(BlockType& currentFftData) { return fftDataFifo.pull(currentFftData); }
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<BlockType> fftDataFifo;
};

template<typename PathType>
struct AnalyzerPathGenerator
{
    /*
     converts 'renderData[]' into a juce::Path
     */
    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      int fftSize,
                      float binWidth,
                      float negativeInfinity)
    {
        auto top = fftBounds.getY();
        auto bottom = fftBounds.getHeight();
        auto width = fftBounds.getWidth();

        int numBins = static_cast<int>(fftSize / 2);

        PathType p;
        p.preallocateSpace(3 * static_cast<int>(fftBounds.getWidth()));

        auto map = [bottom, top, negativeInfinity](float v)
        {
            return juce::jmap(v,
                              negativeInfinity, 0.f,
                              float(bottom+10),   top);
        };

        auto y = map(renderData[0]);

        if( std::isnan(y) || std::isinf(y) )
            y = bottom;
        
        p.startNewSubPath(0, y);

        const int pathResolution = 2; //you can draw line-to's every 'pathResolution' pixels.

        for( size_t binNum = 1; binNum < static_cast<size_t>(numBins); binNum += static_cast<size_t>(pathResolution) )
        {
            y = map(renderData[binNum]);


            if( !std::isnan(y) && !std::isinf(y) )
            {
                auto binFreq = binNum * binWidth;
                auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = static_cast<int>(std::floor(normalizedBinX * width));
                p.lineTo(binX, y);
            }
        }

        pathFifo.push(p);
    }

    int getNumPathsAvailable() const
    {
        return pathFifo.getNumAvailableForReading();
    }

    bool getPath(PathType& path)
    {
        return pathFifo.pull(path);
    }
private:
    Fifo<PathType> pathFifo;
};


struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels() override
    {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
private:
    LookAndFeel lnf;
    
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<EglofAudioProcessor::BlockType>& scsf) :
    leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return leftChannelFFTPath; }
private:
    SingleChannelSampleFifo<EglofAudioProcessor::BlockType>* leftChannelFifo;
    
    juce::AudioBuffer<float> monoBuffer;
    
    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;
    
    AnalyzerPathGenerator<juce::Path> pathProducer;
    
    juce::Path leftChannelFFTPath;
};

struct ResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer
{
    ResponseCurveComponent(EglofAudioProcessor&);
    ~ResponseCurveComponent() override;
    
    void parameterValueChanged (int parameterIndex, float newValue) override;

    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {
        juce::ignoreUnused(gestureIsStarting, parameterIndex);
        
    }
    
    void timerCallback() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void toggleAnalysisEnablement(bool enabled)
    {
        shouldShowFFTAnalysis = enabled;
    }
    void updateChain() {updateChainImpl(std::make_index_sequence<CSV_MAX_ROWS>{});}
    void updateResponseCurve() {updateResponseCurveImpl(std::make_index_sequence<CSV_MAX_ROWS>{});}
private:
    EglofAudioProcessor& audioProcessor;

    bool shouldShowFFTAnalysis = true;

    juce::Atomic<bool> parametersChanged { false };
    
    MonoChain monoChain;
    
    template<std::size_t... I>
    void updateResponseCurveImpl(std::index_sequence<I...>);
    
    juce::Path responseCurve;
    
    template<std::size_t... I>
    void updateChainImpl(std::index_sequence<I...>);
    
    void drawBackgroundGrid(juce::Graphics& g);
    void drawTextLabels(juce::Graphics& g);
    
    std::vector<float> getFrequencies();
    std::vector<float> getGains();
    std::vector<float> getXs(const std::vector<float>& freqs, float left, float width);

    juce::Rectangle<int> getRenderArea();
    
    juce::Rectangle<int> getAnalysisArea();
    
    PathProducer leftPathProducer, rightPathProducer;
};
//==============================================================================


class EglofAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit EglofAudioProcessorEditor(EglofAudioProcessor&);
    ~EglofAudioProcessorEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    void operator[](juce::Rectangle<float>){}
    operator int () const;
    
    CsvColumnSelectionDropdown dataColumnMenu1;
    CsvColumnSelectionDropdown dataColumnMenu2;
    CsvColumnSelectionDropdown dataColumnMenu3;
    CsvColumnSelectionDropdown dataColumnMenu4;
    AddCsv openButton;
    LookAndFeel uiAesthetic;
private:
    EglofAudioProcessor& processorRef;
    juce::Slider peakLinearGainSlider;
    ResponseCurveComponent responseCurveComponent;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment  peakLinearGainSliderAttachment;


    std::vector<juce::Component*> getComps();
    
    LookAndFeel::PowerButton bypassButton;
    
    using ButtonAttachment = APVTS::ButtonAttachment;
    
    ButtonAttachment bypassButtonAttachment;
    
    CsvColumnSelectionDropdown presetMenu;

    
    juce::ShapeButton powerButton{"Power", juce::Colours::red, juce::Colours::green, juce::Colours::blue};
    juce::TextButton helpButton{"Help"};
    juce::TextButton forwardPresetButton{">>"};
    juce::TextButton backwardPresetButton{"<<"};
    juce::TextButton compareButton{"Compare"};
    juce::TextButton copyButton{"Copy"};
    juce::TextButton pasteButton{"Paste"};
    juce::TextButton chooseRandomDataButton{"Choose data for me!"};
    juce::TextButton downloadCSVButton{"Download CSV"};
//    AddCsv openButton;
    juce::Image background;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EglofAudioProcessorEditor)
};
}  // namespace audio_plugin
