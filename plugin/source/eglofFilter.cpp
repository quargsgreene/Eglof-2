#include "../include/Eglof/eglofFilter.h"
#include "Eglof/PluginProcessor.h"

namespace audio_plugin {

    juce::Array<float> eglofFilter::getNormalisedCsvData() const
    {
        return normalisedCsvData;
    }

    juce::Array<float> eglofFilter::getMinQValuesForNotches() const
    {
        return minQs;
    }

    juce::Array<float> eglofFilter::getResonanceMaxIntervalSizesForNotches() const
    {
        return resonanceIntervals;
    }

    juce::Array<float> eglofFilter::getCuts() const
    {
        return cuts;
    }

    float eglofFilter::getGain() const
    {
        return gain;
    }

    void eglofFilter::normaliseCsvData(const juce::Array<float> &csvDataColumn)
    {
        (void) csvDataColumn;
    }

    void eglofFilter::setMinQValuesForNotches(const juce::Array<float> &csvDataColumn)
    {
        (void) csvDataColumn;
    }

    void eglofFilter::setResonanceMaxIntervalSizesForNotches(const juce::Array<float> &csvDataColumn)
    {
        (void) csvDataColumn;
    }

    void eglofFilter::cleanCsvData(const juce::Array<float> &csvDataColumn)
    {
        (void) csvDataColumn;
    }

    juce::Array<float> eglofFilter::getCleanedCsvData()
    {
        return cleanedCsvData;
    }
}
