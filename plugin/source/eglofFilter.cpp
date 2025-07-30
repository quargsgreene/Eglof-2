#include "../include/Eglof/eglofFilter.h"

namespace audio_plugin {
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
                output[sample] = 0;
                currentTime[static_cast<size_t>(channel)] = currentTime[static_cast<size_t>(channel)] + timeIncrement;
            }
        }
    }

eglofFilter::ChainSettings eglofFilter::getChainSettings(juce::AudioProcessorValueTreeState& apvts)
    {
        eglofFilter::ChainSettings settings;
        settings.lowCutFreq = apvts.getRawParameterValue("LowCut Frequency")->load();
        settings.highCutFreq = apvts.getRawParameterValue("HighCut Frequency")->load();
        settings.peakFreq = apvts.getRawParameterValue("Peak Frequency")->load();
        settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
        settings.peakQ = apvts.getRawParameterValue("Peak Q")->load();
        settings.lowCutSlope = static_cast<eglofFilter::Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
        settings.highCutSlope = static_cast<eglofFilter::Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
        settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
        settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
        settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;
    
        return settings;
    }

    void eglofFilter::updateCoefficients(Coefficients &prev, const Coefficients &current)
    {
        *prev = *current;
    }
    
    eglofFilter::Coefficients eglofFilter::makePeakFilter(const ChainSettings &chainSettings, double sampleRate)
    {
        return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq, chainSettings.peakQ, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    }

    void eglofFilter::updatePeakFilter(const eglofFilter::ChainSettings &chainSettings)
    {
        // fix
        auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
        chain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
        updateCoefficients(chain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    }

    void eglofFilter::updateLowCutFilters(const eglofFilter::ChainSettings &chainSettings)
    {
        //fix
        auto lowCutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
        auto& lowCut = chain.get<ChainPositions::LowCut>();
        chain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
        updateCutFilter(lowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    }

    void eglofFilter::updateHighCutFilters(const eglofFilter::ChainSettings &chainSettings)
    {
        //fix
        auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
        auto& highCut = chain.get<ChainPositions::HighCut>();
        chain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
        updateCutFilter(highCut, highCutCoefficients, chainSettings.highCutSlope);
        
    }

    void eglofFilter::updateFilters()
    {
        //fix
        auto chainSettings = getChainSettings(apvts);
        updateLowCutFilters(chainSettings);
        updatePeakFilter(chainSettings);
        updateHighCutFilters(chainSettings);
    }

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
