//#pragma once
//#include <juce_data_structures/juce_data_structures.h>
//#include <juce_events/juce_events.h>
//#include <juce_dsp/juce_dsp.h>
//#include <juce_graphics/juce_graphics.h>
//#include "Eglof/AnalyserPathGenerator.h"
//#include "FFTDataGenerator.h"
//
//namespace audio_plugin
//{
//    class PathProducer
//    {
//        public:
//            // fifo
//            EglofAudioProcessor& processor;
//            explicit PathProducer(EglofAudioProcessor& p);
//            ~PathProducer();
//            void process(juce::Rectangle<float>fftBounds, double sampleRate);
//            juce::Path getPath();
//         private:
//            //fifo
//            
//            juce::AudioBuffer<float> monoBuffer;
//            FFTDataGenerator<std::vector<float>> leftChannelFftDataGenerator;
//            AnalyserPathGenerator<juce::Path> pathProducer;
//            juce::Path leftChannelFftPath;
//    };
//
//}
