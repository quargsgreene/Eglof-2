#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace audio_plugin{
    class eglofFilter
    {
    public:

        void normaliseCsvData(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getNormalisedCsvData() const;
        void setMinQValuesForNotches(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getMinQValuesForNotches() const;

        void setResonanceMaxIntervalSizesForNotches(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getResonanceMaxIntervalSizesForNotches() const;
        void cleanCsvData(const juce::Array<float>& csvDataColumn);
        [[nodiscard]] juce::Array<float> getCleanedCsvData();

        [[nodiscard]] juce::Array<float> getCuts() const;
        float getGain() const;
        
        
    private:
        juce::Array<float> cuts;
        juce::Array<float> resonanceIntervals;
        juce::Array<float> minQs;
        juce::Array<float> normalisedCsvData;
        juce::Array<float> cleanedCsvData;
        float gain;
        std::vector<float> currentTime;

    };
}

