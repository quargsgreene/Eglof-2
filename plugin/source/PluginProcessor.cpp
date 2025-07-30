#include "Eglof/PluginProcessor.h"
#include "Eglof/PluginEditor.h"
#include "Eglof/eglofFilter.h"


namespace audio_plugin {
EglofAudioProcessor::EglofAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
{
          
}

EglofAudioProcessor::~EglofAudioProcessor() {}

const juce::String EglofAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool EglofAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool EglofAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool EglofAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double EglofAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int EglofAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int EglofAudioProcessor::getCurrentProgram() {
  return 0;
}

void EglofAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String EglofAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void EglofAudioProcessor::changeProgramName(int index,
                                                  const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

void EglofAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  juce::dsp::ProcessSpec spec;
  spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
  spec.numChannels = 1;
  spec.sampleRate = sampleRate;
  chain.prepare(spec);
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
//  filter.prepare(sampleRate, getTotalNumOutputChannels());
  spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
  osc.prepare(spec);
  osc.setFrequency(220);
  juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void EglofAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool EglofAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void EglofAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  filter.updateFilters();
  juce::dsp::AudioBlock<float>block(buffer);
  auto stereoBlock = block.getSubsetChannelBlock(0, 2);
  juce::dsp::ProcessContextReplacing<float> stereoContext(stereoBlock);
//    stereoBlock.process(stereoContext);

  // This is the place where you'd normally do the guts of your plugin's
  // audio processing...
  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.
  for (int channel = 0; channel < totalNumInputChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    juce::ignoreUnused(channelData);
    // ..do something to the data...
  }
    // fix
    filter.process(buffer);
    filter.update(buffer);
}

bool EglofAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EglofAudioProcessor::createEditor() {
  return new EglofAudioProcessorEditor(*this);
}

void EglofAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::MemoryOutputStream mos(destData, true);
  juce::ignoreUnused(destData);
  apvts.state.writeToStream(mos);
}

void EglofAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  auto tree = juce::ValueTree::readFromData(data,static_cast<juce::uint32>(sizeInBytes));
  if(tree.isValid())
  {
      apvts.replaceState(tree);
      filter.updateFilters();
  }
  juce::ignoreUnused(data, sizeInBytes);

}

juce::AudioProcessorValueTreeState::ParameterLayout EglofAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Frequency", "LowCut Frequency", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Frequency", "HighCut Frequency", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Frequency", "Peak Frequency", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Q", "Peak Q", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));
    
    juce::StringArray suffixArray;
    for(int i = 0; i < 4; ++i)
    {
        juce::String suffix;
        suffix << (12 + i*12);
        suffix << "db/Oct";
        suffixArray.add(suffix);
        
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut slope", "LowCut Slope", suffixArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut slope", "HighCut Slope", suffixArray, 0));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed", "Peak Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Analyser Enabled", "Analyser Enabled", true));
    
    return layout;
}

}  // namespace audio_plugin

// This creates new instances of the plugin.
// This function definition must be in the global namespace.

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new audio_plugin::EglofAudioProcessor();
}
