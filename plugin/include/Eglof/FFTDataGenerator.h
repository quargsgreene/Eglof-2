//#pragma once
//#include <juce_dsp/juce_dsp.h>
//#include <juce_graphics/juce_graphics.h>
//#include <juce_gui_basics/juce_gui_basics.h>
//#include <juce_data_structures/juce_data_structures.h>
//#include <juce_events/juce_events.h>
//#include "Eglof/Fifo.h"
//
//namespace audio_plugin{
//    enum FFTOrder
//    {
//        size2048,
//        size4096,
//        size8192
//    };
//
//    template<typename BlockType>
//    class FFTDataGenerator
//    {
//        public:
//            void produceFftDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
//            {
//                const auto FFTSize = getFFTOrder();
//                
//                fftData.assign(fftData.size(), 0);
//                auto* readIndex = audioData.getReadPointer(0);
//                std::copy(readIndex, readIndex + order, fftData.begin());
//                
//                // first apply a windowing function to our data
//                window->multiplyWithWindowingTable (fftData.data(), order);       // [1]
//                
//                // then render our FFT data..
//                forwardFFT->performFrequencyOnlyForwardTransform (fftData.data());  // [2]
//                
//                int numBins = static_cast<int>(order / 2);
//                
//                //normalize the fft values.
//                for( int i = 0; i < numBins; ++i )
//                {
//                    auto v = fftData[i];
//            //            FftData[i] /= (float) numBins;
//                    if( !std::isinf(v) && !std::isnan(v) )
//                    {
//                        v /= float(numBins);
//                    }
//                    else
//                    {
//                        v = 0.f;
//                    }
//                    fftData[i] = v;
//                }
//                
//                //convert them to decibels
//                for( int i = 0; i < numBins; ++i )
//                {
//                    fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
//                }
//                // fifo
//                fftDataFifo.push(fftData);
//            }
//
//            void changeOrder(FFTOrder newOrder)
//            {
//                //when you change order, recreate the window, forwardFFT, fifo, FftData
//                //also reset the fifoIndex
//                //things that need recreating should be created on the heap via std::make_unique<>
//                
//                //find where this is declared
//                order = newOrder;
//                auto LocalFFTOrder = getFFTOrder();
//                
//                forwardFFT = std::make_unique<juce::dsp::FFT>(order);
//                window = std::make_unique<juce::dsp::WindowingFunction<float>>(LocalFFTOrder, juce::dsp::WindowingFunction<float>::blackmanHarris);
//                
//                fftData.clear();
//                fftData.resize(LocalFFTOrder * 2, 0);
//
//                fftDataFifo.prepare(fftData.size());
//            }
//            //==============================================================================
//            int getFFTOrder() const { return 1 << order; }
//            int getNumAvailableFftDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
//            //==============================================================================
//            bool getFftData(BlockType& FftData) { return fftDataFifo.pull(FftData); }
//
//        private:
//            FFTOrder order;
//            BlockType fftData;
//            std::unique_ptr<juce::dsp::FFT> forwardFFT;
//            std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
//            Fifo<BlockType> fftDataFifo;
//    };
//
//}
