#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace audio_plugin{
    class eglofFilter
    {
    public:
//        enum Slope
//        {
//            Slope_1,
//            Slope_2,
//            Slope_3,
//            Slope_4
//        };
//        
//        enum ChainPositions
//        {
//          LowCut,
//          Peak,
//          HighCut
//        };
//        
//        struct ChainSettings
//        {
//            float peakFreq {0}, peakGainInDecibels{0}, peakQ{1.f};
//            float lowCutFreq{0}, highCutFreq{0};
//            
//            Slope lowCutSlope {Slope::Slope_1}, highCutSlope {Slope::Slope_1};
//            bool lowCutBypassed {false}, peakBypassed {false}, highCutBypassed {false};
//        };
//        
//        
//        using Filter = juce::dsp::IIR::Filter<float>;
//        using CutFilter = juce::dsp::ProcessorChain<Filter>;
//        using StereoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
//        StereoChain chain;
//        
//        using Coefficients = Filter::CoefficientsPtr;
//        void updateCoefficients(Coefficients& prev, const Coefficients& current);
//        
//        ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
//        
//        Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);
//        
//        template<int Index, typename ChainType, typename CoefficientType>
//        void updateChainCoefficients(ChainType& chain, const CoefficientType& coefficients);
//        
//        template <typename ChainType, typename CoefficientType>
//        void updateCutFilter(ChainType& chain, const CoefficientType& coeffficients, const Slope& slope);
//        
//        inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate);
//        
//        inline auto makeHighCutFilter(const ChainSettings&, double sampleRate);
//        
//        void updatePeakFilter(const ChainSettings& chainSettings);
//        void updateLowCutFilters(const ChainSettings& chainSettings);
//        void updateHighCutFilters(const ChainSettings& chainSettings);
//        
//        void updateFilters();
//        
//       
//        
//        void prepare (const double sampleRate, const int numChannels);
//        void process (juce::AudioBuffer<float>& buffer);
        // transormDataRange
        void normaliseCsvData(const juce::Array<float>& csvDataColumn);
        // get transformed data
        [[nodiscard]] juce::Array<float> getNormalisedCsvData() const;
        // getMinQValuesForNotches
        // set Q
        void setMinQValuesForNotches(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getMinQValuesForNotches() const;
        // getResonanceMaxIntervalSizes
        // set resonance
        void setResonanceMaxIntervalSizesForNotches(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getResonanceMaxIntervalSizesForNotches() const;
        void cleanCsvData(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getCleanedCsvData();
        // createAndApplyFilterFromCsvData
        // castCsvData
        [[nodiscard]] juce::Array<float> getCuts() const;
        float getGain() const;
        
        
    private:
        // cuts
        juce::Array<float> cuts;
        juce::Array<float> resonanceIntervals;
        juce::Array<float> minQs;
        juce::Array<float> normalisedCsvData;
        juce::Array<float> cleanedCsvData;
        // gain
        float gain;
//        float timeIncrement = 0.0f;
        std::vector<float> currentTime;
//        float currentSampleRate = 0.0f;
        // gains
        // Qs
        // resonances
        // cutoffRange
    };
}

