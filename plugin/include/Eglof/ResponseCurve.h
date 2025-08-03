//#pragma once
//#include <juce_data_structures/juce_data_structures.h>
//#include <juce_events/juce_events.h>
//#include <juce_dsp/juce_dsp.h>
//#include <juce_graphics/juce_graphics.h>
//#include "Eglof/AnalyserPathGenerator.h"
//#include "FFTDataGenerator.h"
//#include "Eglof/PluginProcessor.h"
//#include "Eglof/PathProducer.h"
//
//namespace audio_plugin {
//    class ResponseCurve: public juce::Component,
//                         public juce::AudioProcessorParameter::Listener,
//                         public juce::Timer
//
//    {
//        public:
//            ResponseCurve(EglofAudioProcessor&);
//            ~ResponseCurve() override;
//            void parameterValueChanged (int parameterIndex, float newValue) override;
//            void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
//            void timerCallback() override;
//            void paint(juce::Graphics& g) override;
//            void resized() override;
//            void toggleAnalysisEnablement(bool enabled);
//        private:
//            EglofAudioProcessor& audioProcessor;
//            bool shouldShowFftAnalysis = true;
//            juce::Atomic<bool> parametersChanged {false};
//            // update stereo to mono
//            eglofFilter::StereoChain stereo;
//            juce::Path responseCurve;
//            PathProducer leftPathProducer, rightPathProducer;
//            void updateResponseCurve();
//            void updateChain();
//            void drawBackgroundGrid(juce::Graphics& g);
//            void drawTextLabels(juce::Graphics& g);
//            std::vector<float> getFrequencies();
//            std::vector<float> getGains();
//            std::vector<float> getXs(const std::vector<float>& freqs, float left, float width);
//            juce::Rectangle<int> getRenderArea();
//            juce::Rectangle<int> getAnalysisArea();
//    };
//}
//
