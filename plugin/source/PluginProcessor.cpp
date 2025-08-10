
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    
    spec.numChannels = 1;
    
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters(std::make_index_sequence<CSV_MAX_ROWS>{});
    
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    
    osc.initialise([](float x) { return std::sin(x); });
    
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    osc.prepare(spec);
    osc.setFrequency(440);
}

void EglofAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EglofAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (//layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void EglofAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    if( tree.isValid() )
    {
        apvts.replaceState(tree);

        updateFilters(std::make_index_sequence<CSV_MAX_ROWS>{});
    }
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
    
    std::cout<<"Band0 freq="<<settings[0].freq<<std::endl;
    std::cout<<"Q="<<settings[0].Q<<std::endl;
    std::cout<<"gain="<<settings[0].gain<<std::endl;
    std::cout<<"bypass="<<settings[0].bypass<<std::endl;

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
    bool  allBypassed = false;
    auto* masterGainParam = apvts.getRawParameterValue("Peak Master Gain");
    auto* bypassAllParam = apvts.getRawParameterValue("Bypass All");
    
    if (masterGainParam)
    {
        gainOffsetDb = masterGainParam ->load();
        std::cout<<"Master=" <<gainOffsetDb<<std::endl;
    }
    
    if(bypassAllParam)
    {
        allBypassed = static_cast<bool>(bypassAllParam ->load());
        std::cout<<"All Bypassed="<<allBypassed<<std::endl;
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

//TODO: implement updateChainSettings method that gets called when a new csv column is selected
void updateChainSettings()
{
    size_t csvFreqVecSize = csvFreqs.size();
    // reset all set frequencies to 0
    // for each csv frequency, set chain setting frequency
    size_t index = 0;
    
    for(size_t i = 0; i < chainSettings.size(); ++i)
    {
        std::cout<<"Before update:"<<std::endl;
        std::cout<<chainSettings[i].freq<<std::endl;
    }
    
    for(auto& filter : chainSettings)
    {
        filter.setFreq(0);
        if(index < csvFreqVecSize)
        {
            filter.setFreq(csvFreqs[index]);
        }
        
        ++index;
    }
    
    for(size_t i = 0; i < chainSettings.size(); ++i)
    {
        std::cout<<"After update:"<<std::endl;
        std::cout<<chainSettings[i].freq<<std::endl;
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
