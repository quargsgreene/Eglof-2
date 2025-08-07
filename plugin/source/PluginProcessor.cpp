
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
    
    updateFilters();
    
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

    updateFilters();
    
    juce::dsp::AudioBlock<float> block(buffer);
    
//    buffer.clear();
//
//    for( int i = 0; i < buffer.getNumSamples(); ++i )
//    {
//        buffer.setSample(0, i, osc.processSample(0));
//    }
//
//    juce::dsp::ProcessContextReplacing<float> stereoContext(block);
//    osc.process(stereoContext);
    
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
        updateFilters();
    }
}

void initializeChainSettings ()
  {
    
        for (int i = 0; i < CSV_MAX_ROWS; ++i)
        {
            chainSettings.insert({std::make_pair(0.f, 0.f), std::make_pair(100.f, false)});
        }

}


std::map<std::pair<std::atomic< float >*,std::atomic< float >*>, std::pair<std::atomic< float >*, std::atomic< float >*>> getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
//    ChainSettings settings;
     // retrieve each setting from apvts and name it
    std::map<std::pair<std::atomic< float >*,std::atomic< float >*>, std::pair<std::atomic< float >*, std::atomic< float >*>> settings;
    
    for (int i = 0; i < CSV_MAX_ROWS; ++i)
    {
        settings.insert({std::make_pair(apvts.getRawParameterValue(std::format("Peak Freq {}", i)),apvts.getRawParameterValue(std::format("Peak Gain {}", i))), std::make_pair(apvts.getRawParameterValue(std::format("Peak Quality {}", i)), apvts.getRawParameterValue(std::format("Peak {} Bypassed", i)))});
    }
//
//    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
//    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
//    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
//    settings.peakFreq2 = apvts.getRawParameterValue("Peak Freq 2")->load();
//    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
//    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
//    settings.peak2Quality = apvts.getRawParameterValue("Peak 2 Quality")->load();
//    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
//    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
//    
//    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
//    settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
//    settings.peak2Bypassed = apvts.getRawParameterValue("Peak 2 Bypassed")->load() > 0.5f;
//    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;
//    
     return settings;
}

std::vector<Coefficients> makePeakFilters(const std::map<std::pair<float, float>, std::pair<float, bool>>& currentChainSettings, double sampleRate)
{
    std::vector<Coefficients> filters;
    for (const auto& [freqGainPair, qBypassedPair]: currentChainSettings)
    {
        filters.push_back((juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                               freqGainPair.first,
                                                                               qBypassedPair.first,
                                                                               juce::Decibels::decibelsToGain(freqGainPair.second))));
    }
//    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
//                                                               chainSettings.peakFreq,
//                                                               chainSettings.peakQuality,
//                                                               juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    return filters;
}

//Coefficients makePeakFilter2(const ChainSettings& chainSettings, double sampleRate)
//{
//    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
//                                                               chainSettings.peakFreq2,
//                                                               chainSettings.peak2Quality,
//                                                               juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
//}
//
//void EglofAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings)
//{
//    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
//    //make into loop
//    leftChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
//    rightChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
//
//    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//}
//
template<std::size_t... I>
void EglofAudioProcessor::updatePeakFilters(const std::map<std::pair<float, float>, std::pair<float, bool>>& currentChainSettings, std::index_sequence<I...>)
{
    auto peakCoefficients = makePeakFilters(currentChainSettings, getSampleRate());
    std::vector<bool> bypassedSettings;
    for (const auto& [gainFreqPair, qBypassPair] : currentChainSettings)
    {
        bypassedSettings.push_back(qBypassPair.second);
    }

    (leftChain.setBypassed<I>(bypassedSettings[I]),...);
    (rightChain.setBypassed<I>(bypassedSettings[I]),...);
    
    (updateCoefficients(leftChain.get<I>(), peakCoefficients),...);
    (updateCoefficients(rightChain.get<I>(), peakCoefficients),...);
}
//void EglofAudioProcessor::updatePeakFilter2(const ChainSettings &chainSettings)
//{
//    auto peakCoefficients = makePeakFilter2(chainSettings, getSampleRate());
//    
//    leftChain.setBypassed<ChainPositions::Peak2>(chainSettings.peak2Bypassed);
//    rightChain.setBypassed<ChainPositions::Peak2>(chainSettings.peak2Bypassed);
//    
//    updateCoefficients(leftChain.get<ChainPositions::Peak2>().coefficients, peakCoefficients);
//    updateCoefficients(rightChain.get<ChainPositions::Peak2>().coefficients, peakCoefficients);
//}

void updateCoefficients(std::vector<Coefficients>& old, const std::vector<Coefficients>& replacements)
{
    for (size_t i = 0; i < old.size(); ++i)
    {
        *old[i] = *replacements[i];
    }
}

//void EglofAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
//{
//    auto cutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
//    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
//    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
//    
//    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
//    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
//    
//    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
//    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
//}

//void EglofAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
//{
//    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
//    
//    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
//    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
//    
//    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
//    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
//    
//    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
//    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
//}

void EglofAudioProcessor::updateFilters()
{
    auto currentChainSettings = getChainSettings(apvts);
    
//    updateLowCutFilters(chainSettings);
    updatePeakFilters(chainSettings);
//    updateHighCutFilters(chainSettings);
}

juce::AudioProcessorValueTreeState::ParameterLayout EglofAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
//    settings.insert({apvts.getRawParameterValue(std::format("Peak Freq {}", i)), std::make_pair(apvts.getRawParameterValue(std::format("Peak Quality {}", i)), apvts.getRawParameterValue(std::format("Peak {} Bypassed", i)))});
    int testArrSize = sizeof(testFreqs)/sizeof(testFreqs[0]); // rename to generalize
    
    for(int i = 0; i < CSV_MAX_ROWS; ++i)
    {
        if(i >= 0 && i < testArrSize){
            layout.add(std::make_unique<juce::AudioParameterFloat>(std::format("Peak Freq {}", i),
                                                                  std::format("Peak Freq {}", i),
                                                                  juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), testFreqs[i]));
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
                                                              juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 100.f));
        layout.add(std::make_unique<juce::AudioParameterBool>(std::format("Peak {} Bypassed", i), std::format("Peak {} Bypassed", i), false));
        
    }
//    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
//                                                           "LowCut Freq",
//                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
//                                                           200.f));
//    
//    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
//                                                           "HighCut Freq",
//                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
//                                                           20000.f));
//    
//    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
//                                                           "Peak Freq",
//                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
//                                                           1000.f));
//    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq 2",
//                                                           "Peak Freq 2",
//                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
//                                                           100.f));
//    
//    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
//                                                           "Peak Gain",
//                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
//                                                           0.0f));
//    
//    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak 2 Gain",
//                                                           "Peak 2 Gain",
//                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
//                                                           0.0f));
//    
//    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
//                                                           "Peak Quality",
//                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
//                                                           1.f));
//    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak 2 Quality",
//                                                           "Peak 2 Quality",
//                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
//                                                           1.f));
    
    juce::StringArray stringArray;
    for( int i = 0; i < 4; ++i )
    {
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }
    // loop with std::format
    
//    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
//    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
//    
//    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));
//    layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed", "Peak Bypassed", false));
//    layout.add(std::make_unique<juce::AudioParameterBool>("Peak 2 Bypassed", "Peak Bypassed", false));
//    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));
//    layout.add(std::make_unique<juce::AudioParameterBool>("Analyzer Enabled", "Analyzer Enabled", true));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EglofAudioProcessor();
}
