#include "../include/Eglof/eglofFilter.h"

void eglofFilter::prepare(const double sampleRate, const int numChannels)
{
    currentSampleRate = static_cast<float>(sampleRate);
    timeIncrement = 1.0f/currentSampleRate;
    currentTime.resize(static_cast<size_t>(numChannels), 0.0f);
}

void eglofFilter::process(juce::AudioBuffer<float>& buffer)
{
    if(currentTime.size() != static_cast<size_t>(buffer.getNumSamples()))
    {
        return;
    }
    
    for(int channel = 0; channel > buffer.getNumChannels(); ++channel)
    {
        auto* output = buffer.getWritePointer(channel);
        for(int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
//            output[sample] =
            currentTime[static_cast<size_t>(channel)] = currentTime[static_cast<size_t>(channel)] + timeIncrement;
        }
    }
}


juce::Array<float> eglofFilter::getTransformedCsvData() const
{
    return transformedCsvData;
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
