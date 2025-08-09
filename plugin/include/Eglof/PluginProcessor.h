#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <array>
#include <map>
#include <tuple>
#include <utility>

//int CSV_MAX_ROWS = 1000;
inline constexpr size_t CSV_MAX_ROWS = 1000;
inline float testFreqs[10] = {25.f, 100.2f, 230.f, 997.f, 1234.5f, 3456.7f, 7890.f, 12345.6f, 13579.f, 19990.9f}; // rename

struct Peak { float freq, gain, Q; bool bypass; };

template<typename T>
struct Fifo
{
    void prepare(int numChannels, int numSamples)
    {
        static_assert( std::is_same_v<T, juce::AudioBuffer<float>>,
                      "prepare(numChannels, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>");
        for( auto& buffer : buffers)
        {
            buffer.setSize(numChannels,
                           numSamples,
                           false,   //clear everything?
                           true,    //including the extra space?
                           true);   //avoid reallocating if you can?
            buffer.clear();
        }
    }
    
    void prepare(size_t numElements)
    {
        static_assert( std::is_same_v<T, std::vector<float>>,
                      "prepare(numElements) should only be used when the Fifo is holding std::vector<float>");
        for( auto& buffer : buffers )
        {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }
    
    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if( write.blockSize1 > 0 )
        {
            buffers[static_cast<size_t>(write.startIndex1)] = t;
            return true;
        }
        
        return false;
    }
    
    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if( read.blockSize1 > 0 )
        {
            t = buffers[static_cast<size_t>(read.startIndex1)];
            return true;
        }
        
        return false;
    }
    
    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }
private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo {Capacity};
};

enum Channel
{
        Right, //effectively 0
        Left //effectively 1
};

template<typename BlockType>
struct SingleChannelSampleFifo
{
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch)
    {
        prepared.set(false);
    }
    
    void update(const BlockType& buffer)
    {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse );
        auto* channelPtr = buffer.getReadPointer(channelToUse);
        
        for( int i = 0; i < buffer.getNumSamples(); ++i )
        {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);
        
        bufferToFill.setSize(1,             //channel
                             bufferSize,    //num samples
                             false,         //keepExistingContent
                             true,          //clear extra space
                             true);         //avoid reallocating
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    //==============================================================================
    int getNumCompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
    //==============================================================================
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;
    
    void pushNextSampleIntoFifo(float sample)
    {
        if (fifoIndex == bufferToFill.getNumSamples())
        {
            auto ok = audioBufferFifo.push(bufferToFill);

            juce::ignoreUnused(ok);
            
            fifoIndex = 0;
        }
        
        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// add dynamic number of chain settings
// turn into a map of freq and Q values

inline std::vector<Peak> chainSettings;

// set freq to 0.0f, Q to 100, gain to 0.
void initializeChainSettings();

//std::map<std::pair<float, float>, std::pair<float, bool>> getChainSettings(juce::AudioProcessorValueTreeState& apvts);
std::vector<Peak> getChainSettings(juce::AudioProcessorValueTreeState& apvts);
using Filter = juce::dsp::IIR::Filter<float>;
//using Filter = juce::dsp::ProcessorDuplicator<
//juce::dsp::IIR::Filter<float>,
//juce::dsp::IIR::Coefficients<float>>;
//// make tuple size of ProcessorChain dynamic
//// tuple_cat loop
////using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
//
using MonoChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter>;
//template <typename T, std::size_t... I>
//auto make_chain(std::index_sequence<I...>) -> juce::dsp::ProcessorChain<std::conditional_t<true, T, std::integral_constant<std::size_t, I>> ...>;

//template <typename T, std::size_t NUM_FILTERS>
//using RepeatChain = decltype(make_chain<T>(std::make_index_sequence<NUM_FILTERS>{}));
//using Coeffs = juce::dsp::IIR::Coefficients<float>;
//using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, Coeffs>;
//using MonoChain = RepeatChain<Filter, 1000>;

//class chainPositions
//{
//    public:
//        static const int NUM_FILTERS = 1000;
//        int chainPositionNames[NUM_FILTERS];
//    
//        chainPositions()
//        {
//            for(int i = 0; i < NUM_FILTERS; ++i)
//            {
//                chainPositionNames[i] = i;
//            }
//        }
//};

//std::string ChainPos[4] = {"LowCut", "Peak", "Peak2", "HighCut" };

//using Coefficients = juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>;
using Coefficients = Filter::CoefficientsPtr;

//void updateCoefficients(std::vector<Coefficients>& old, const std::vector<Coefficients>& replacements);
void updateCoefficients(Coefficients& old, const Coefficients& replacements);


std::vector<Coefficients> makePeakFilters(const std::vector<Peak>& currentChainSettings, double sampleRate, float masterGainDb);
//std::vector<Coefficients> makePeakFilter2(const std::map<std::pair<float, float>, std::pair<float, float>>& chainSettings, double sampleRate);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const std::vector<CoefficientType>& coefficients)
{
    for(Coefficients coefficientSet : coefficients)
    {
        updateCoefficients(chain.template get<Index>().coefficientSet, coefficientSet);
        chain.template setBypassed<Index>(false);
    }
}
////==============================================================================
/**
*/
class EglofAudioProcessor  : public juce::AudioProcessor
{
public:
    
    //==============================================================================
    EglofAudioProcessor();
    ~EglofAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo { Channel::Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo { Channel::Right };
private:
    MonoChain leftChain, rightChain;

    
    template<std::size_t... I>
    void updatePeakFilters(const std::vector<Peak>& currentChainSettings, std::index_sequence<I...>);
    template<std::size_t... I>
    void updateFilters(std::index_sequence<I...> seq);
    
    
    juce::dsp::Oscillator<float> osc;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EglofAudioProcessor)
};
