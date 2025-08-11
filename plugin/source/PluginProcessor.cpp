
#include "Eglof/PluginProcessor.h"
#include "Eglof/PluginEditor.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <map>
#include <format>
#include <utility>


//==============================================================================
EglofAudioProcessor::EglofAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

EglofAudioProcessor::~EglofAudioProcessor()
{
    
}


//==============================================================================
const juce::String EglofAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EglofAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EglofAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EglofAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EglofAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EglofAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EglofAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EglofAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String EglofAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void EglofAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================================
void EglofAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    
    spec.numChannels = 1;
    
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters(std::make_index_sequence<CSV_MAX_ROWS>{});
    
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    
}

void EglofAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EglofAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (//layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    auto in = layouts.getMainInputChannelSet();
    if(in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
    {
        return false;
    }
   #endif
    auto out = layouts.getMainInputChannelSet();
    if(out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    return true;
  #endif
}
#endif

void EglofAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateFilters(std::make_index_sequence<CSV_MAX_ROWS>{});
    
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);
    
}

//==============================================================================
bool EglofAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EglofAudioProcessor::createEditor()
{
    return new audio_plugin::EglofAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EglofAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
//    juce::MemoryOutputStream mos(destData, true);
//    apvts.state.writeToStream(mos);
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EglofAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    if( tree.isValid() )
    {
        apvts.replaceState(tree);

        updateFilters(std::make_index_sequence<CSV_MAX_ROWS>{});
    }
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

std::vector<Peak> getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    auto loadFloat = [](std::atomic<float>* param) -> float {return param ? param->load() : 0.f;};
    auto loadBool = [](std::atomic<float>* param) -> bool {return param && param->load() >= 0.5f;};
    std::vector<Peak>settings;
    
    for (size_t i = 0; i < CSV_MAX_ROWS; ++i)
    {
        settings.push_back({loadFloat(apvts.getRawParameterValue(std::format("Peak Freq {}", i))),loadFloat(apvts.getRawParameterValue(std::format("Peak Gain {}", i))),
            loadFloat(apvts.getRawParameterValue(std::format("Peak Quality {}", i))),
            loadBool (apvts.getRawParameterValue(std::format("Peak Bypassed {}", i)))});
    }
    
     return settings;
}


// update filter frequencies here
std::vector<Coefficients> makePeakFilters(const std::vector<Peak>& currentChainSettings, double sampleRate, float masterGainDb)
{
    std::vector<Coefficients> filters;
    
    for(size_t i = 0; i < static_cast<size_t>(currentChainSettings.size()); ++i){
        if (currentChainSettings[i].freq <=0)
        {
            continue;
        }
        const float totalGain = juce::jlimit(-24.f, 24.f, currentChainSettings[i].gain + masterGainDb);
        filters.push_back(juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, currentChainSettings[i].freq, currentChainSettings[i].Q, juce::Decibels::decibelsToGain(totalGain)));
    }
    return filters;

}

template<std::size_t... I>
void EglofAudioProcessor::updatePeakFilters(const std::vector<Peak>& currentChainSettings, std::index_sequence<I...>)
{
    float gainOffsetDb = 0.f;
    auto* masterGainParam = apvts.getRawParameterValue("Peak Master Gain");
    
    if (masterGainParam)
    {
        gainOffsetDb = masterGainParam ->load();
    }

    std::vector<Coefficients> peakCoefficients = makePeakFilters(currentChainSettings, getSampleRate(), gainOffsetDb);
    std::vector<bool> bypassedSettings;
    bypassedSettings.reserve(currentChainSettings.size());
    std::vector<float> frequencySettings;
    frequencySettings.reserve(currentChainSettings.size());
    std::vector<float> qSettings;
    qSettings.reserve(currentChainSettings.size());
    std::vector<float> gainSettings;
    gainSettings.reserve(currentChainSettings.size());
    
    for (size_t i = 0; i < static_cast<size_t>(currentChainSettings.size()); ++i)
    {
        bypassedSettings.push_back(currentChainSettings[i].freq <=0.f ? true : currentChainSettings[i].bypass);
        frequencySettings.push_back(currentChainSettings[i].freq);
        gainSettings.push_back(currentChainSettings[i].gain);
        qSettings.push_back(currentChainSettings[i].Q);
    }
    

    (leftChain.setBypassed<I>(bypassedSettings[I]),...);
    (rightChain.setBypassed<I>(bypassedSettings[I]),...);
    
    ([&](){
        if(I < peakCoefficients.size() && peakCoefficients[I] != nullptr)
        {
                    (updateCoefficients(leftChain.get<I>().coefficients, peakCoefficients[I]),...);
                    (updateCoefficients(rightChain.get<I>().coefficients, peakCoefficients[I]),...);
        }
    }(),...);

}


void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
        *old = *replacements;
}

void updateChainSettings()
{
    size_t csvFreqVecSize = csvFreqs.size();
    size_t index = 0;
    
    for(auto& filter : chainSettings)
    {
        filter.setFreq(0);
        if(index < csvFreqVecSize)
        {
            filter.setFreq(csvFreqs[index]);
        }
        
        ++index;
    }
    
}

template<std::size_t... I>
void EglofAudioProcessor::updateFilters(std::index_sequence<I...> seq)
{
    auto currentChainSettings = getChainSettings(apvts);
    updatePeakFilters(currentChainSettings, seq);
}


juce::AudioProcessorValueTreeState::ParameterLayout EglofAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    size_t csvFreqVecSize = csvFreqs.size();
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(std::format("Peak Master Gain"), std::format("Peak Master Gain"), juce::NormalisableRange<float>(-24.f, 24.f, 0.1f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(std::format("Bypass All"), std::format("Bypass All"), false));
    
    
    for(size_t i = 0; i < CSV_MAX_ROWS; ++i)
    {
        const bool defaultBypassed = i >= static_cast<size_t>(csvFreqVecSize);
        if(i >= 0 && i < static_cast<size_t>(csvFreqVecSize)){
            layout.add(std::make_unique<juce::AudioParameterFloat>(std::format("Peak Freq {}", i),
                                                                  std::format("Peak Freq {}", i),
                                                                  juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), csvFreqs[i]));
        } else {
            layout.add(std::make_unique<juce::AudioParameterFloat>(std::format("Peak Freq {}", i),
                                                                  std::format("Peak Freq {}", i),
                                                                  juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 0.f));
        }

        layout.add(std::make_unique<juce::AudioParameterFloat>(std::format("Peak Gain {}", i),
                                                              std::format("Peak Gain {}", i),
                                                              juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(std::format("Peak Quality {}", i),
                                                              std::format("Peak Quality {}", i),
                                                              juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));
        layout.add(std::make_unique<juce::AudioParameterBool>(std::format("Peak Bypassed {}", i), std::format("Peak Bypassed {}", i), defaultBypassed));
    }

    juce::StringArray stringArray;
    for( int i = 0; i < 4; ++i )
    {
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EglofAudioProcessor();
}
