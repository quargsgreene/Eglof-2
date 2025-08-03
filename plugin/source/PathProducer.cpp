//#include "Eglof/PluginProcessor.h"
//#include "Eglof/PathProducer.h"
//#include "Eglof/SingleChannelSampleFifo.h"
//
//namespace audio_plugin {
//    PathProducer::PathProducer(EglofAudioProcessor&p):processor(p)
//    {}
//
//    PathProducer::~PathProducer()
//    {}
//
//    juce::Path PathProducer::getPath()
//    {
//        return leftChannelFftPath;
//    }
//
//    void PathProducer::process(juce::Rectangle<float>fftBounds, double sampleRate)
//    {
//        juce::AudioBuffer<float> tempIncomingBuffer;
//        //fifo
//        while(processor.leftChannelFifo.getNumCompleteBuffersAvailable() > 0)
//        {
//            if (processor.leftChannelFifo.getAudioBuffer(tempIncomingBuffer))
//            {
//                auto size = tempIncomingBuffer.getNumSamples();
//                // mono
//                juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0), monoBuffer.getReadPointer(0, size), monoBuffer.getNumSamples() - size);
//                juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size), tempIncomingBuffer.getReadPointer(0, 0), size);
//                leftChannelFftDataGenerator.produceFftDataForRendering(monoBuffer, -48.f);
//                
//            }
//        }
//        const auto fftSize = static_cast<int>(leftChannelFftDataGenerator.getFFTOrder());
//        const auto binWidth = static_cast<float>(sampleRate) / static_cast<float>(fftSize);
//        
//        while(leftChannelFftDataGenerator.getNumAvailableFftDataBlocks() > 0)
//        {
//            std::vector<float> fftData;
//            if(leftChannelFftDataGenerator.getFftData(fftData)) // might pass fft data
//            {
//                pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
//            }
//        }
//        while(pathProducer.getNumPathsAvailable() > 0)
//        {
//            pathProducer.getPath(leftChannelFftPath);
//        }
//    }
//}
