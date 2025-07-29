#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class eglofFilter
{
    public:
        void prepare (const double sampleRate, const int numChannels);
        void process (juce::AudioBuffer<float>& buffer);
        // transormDataRange
        void setCsvDataTransformation(const juce::Array<float>& csvDataColumn);
        // get transformed data
        [[nodiscard]] juce::Array<float> getTransformedCsvData() const;
        // getMinQValuesForNotches
        // set Q
        void setMinQValuesForNotches(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getMinQValuesForNotches() const;
        // getResonanceMaxIntervalSizes
        // set resonance
        void setResonanceMaxIntervalSizesForNotches(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getResonanceMaxIntervalSizesForNotches() const;
        // createAndApplyFilterFromCsvData
        // castCsvData
        [[nodiscard]] juce::Array<float> getCuts() const;
        float getGain() const;
        
    
    private:
        // cuts
    juce::Array<float> cuts;
    juce::Array<float> resonanceIntervals;
    juce::Array<float> minQs;
    juce::Array<float> transformedCsvData;
        // gain
    float gain;
    float timeIncrement = 0.0f;
    std::vector<float> currentTime;
    float currentSampleRate = 0.0f;
        
        
    
        
};


