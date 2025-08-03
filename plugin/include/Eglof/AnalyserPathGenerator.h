//#pragma once
//#include <juce_data_structures/juce_data_structures.h>
//#include <juce_events/juce_events.h>
//#include <juce_dsp/juce_dsp.h>
//#include <juce_graphics/juce_graphics.h>
//#include "Eglof/Fifo.h"
//
//
//namespace audio_plugin{
//    template<typename PathType>
//
//    class AnalyserPathGenerator
//    {
//
//        public:
//            AnalyserPathGenerator();
//            ~AnalyserPathGenerator();
//            void generatePath(const std::vector<float>& renderData, juce::Rectangle<float> fftBounds, int fftSize, float binWidth, float negativeInfinity)
//            {
//                auto top = fftBounds.getY();
//                auto bottom = fftBounds.getHeight();
//                auto width = fftBounds.getWidth();
//
//                int numBins = static_cast<int>(fftSize / 2);
//
//                PathType p;
//                p.preallocateSpace(3 * static_cast<int>(fftBounds.getWidth()));
//
//                auto map = [bottom, top, negativeInfinity](float v)
//                {
//                    return juce::jmap(v,
//                                      negativeInfinity, 0.f,
//                                      float(bottom+10),   top);
//                };
//
//                auto y = map(renderData[0]);
//
//        //        jassert( !std::isnan(y) && !std::isinf(y) );
//                if( std::isnan(y) || std::isinf(y) )
//                    y = bottom;
//                
//                p.startNewSubPath(0, y);
//
//                const int pathResolution = 2; //you can draw line-to's every 'pathResolution' pixels.
//
//                for( int binNum = 1; binNum < numBins; binNum += pathResolution )
//                {
//                    y = map(renderData[binNum]);
//
//        //            jassert( !std::isnan(y) && !std::isinf(y) );
//
//                    if( !std::isnan(y) && !std::isinf(y) )
//                    {
//                        auto binFreq = binNum * binWidth;
//                        auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
//                        int binX = static_cast<int>(std::floor(normalizedBinX * width));
//                        p.lineTo(binX, y);
//                    }
//                }
//                //fix fifo
//                pathFifo.push(p);
//            }
//            int getNumPathsAvailable() const
//            {
//                // fix fifo
//                return pathFifo.getNumAvailableForReading();
//            }
//            bool getPath(PathType& path)
//            {
//                // fix fifo
//                return pathFifo.pull(path);
//            }
//        private:
//            // fix fifo
//            Fifo<PathType> pathFifo;
//    };
//}
