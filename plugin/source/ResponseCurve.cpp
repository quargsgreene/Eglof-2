//#include "Eglof/ResponseCurve.h"
//#include "Eglof/eglofFilter.h"
//#include <juce_gui_basics/juce_gui_basics.h>
//
//namespace audio_plugin {
//    ResponseCurve::ResponseCurve(EglofAudioProcessor&)
//    {
//        
//    }
//
//    ResponseCurve::~ResponseCurve()
//    {
//        
//    }
//
//    void ResponseCurve::parameterValueChanged(int parameterIndex, float newValue)
//    {
//        juce::ignoreUnused(parameterIndex);
//        juce::ignoreUnused(newValue);
//        parametersChanged.set(true);
//    }
//
//    void ResponseCurve::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
//    {
//        juce::ignoreUnused(parameterIndex);
//        juce::ignoreUnused(gestureIsStarting);
//    }
//
//    void ResponseCurve::timerCallback()
//    {
//        if( shouldShowFftAnalysis )
//        {
//            auto fftBounds = getAnalysisArea().toFloat();
//            auto sampleRate = audioProcessor.getSampleRate();
//            
//            leftPathProducer.process(fftBounds, sampleRate);
//            rightPathProducer.process(fftBounds, sampleRate);
//        }
//
//        if( parametersChanged.compareAndSetBool(false, true) )
//        {
//            updateChain();
//            updateResponseCurve();
//        }
//        
//        //fix
//        repaint();
//    }
//
//    void ResponseCurve::updateChain()
//    {
//        auto chainSettings = getChainSettings(audioProcessor.apvts);
//        
//        monoChain.setBypassed<eglofFilter::ChainPositions::LowCut>(chainSettings.lowCutBypassed);
//        monoChain.setBypassed<eglofFilter::ChainPositions::Peak>(chainSettings.peakBypassed);
//        monoChain.setBypassed<eglofFilter::ChainPositions::HighCut>(chainSettings.highCutBypassed);
//        
//        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
//        updateCoefficients(monoChain.get<eglofFilter::ChainPositions::Peak>().coefficients, peakCoefficients);
//        
//        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
//        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
//        
//        updateCutFilter(monoChain.get<eglofFilter::ChainPositions::LowCut>(),
//                        lowCutCoefficients,
//                        chainSettings.lowCutSlope);
//        
//        updateCutFilter(monoChain.get<eglofFilter::ChainPositions::HighCut>(),
//                        highCutCoefficients,
//                        chainSettings.highCutSlope);
//    }
//
//    juce::Rectangle<int> ResponseCurve::getRenderArea()
//    {
//        auto bounds = getLocalBounds();
//        
//        bounds.removeFromTop(12);
//        bounds.removeFromBottom(2);
//        bounds.removeFromLeft(20);
//        bounds.removeFromRight(20);
//        
//        return bounds;
//    }
//
//
//    juce::Rectangle<int> ResponseCurve::getAnalysisArea()
//    {
//        auto bounds = getRenderArea();
//        bounds.removeFromTop(4);
//        bounds.removeFromBottom(4);
//        return bounds;
//    }
//
//
//    std::vector<float> ResponseCurve::getFrequencies()
//    {
//        return std::vector<float>
//        {
//            20, /*30, 40,*/ 50, 100,
//            200, /*300, 400,*/ 500, 1000,
//            2000, /*3000, 4000,*/ 5000, 10000,
//            20000
//        };
//    }
//
//    std::vector<float> ResponseCurve::getGains()
//    {
//        return std::vector<float>
//        {
//            -24, -12, 0, 12, 24
//        };
//    }
//
//    std::vector<float> ResponseCurve::getXs(const std::vector<float> &freqs, float left, float width)
//    {
//        std::vector<float> xs;
//        for( auto f : freqs )
//        {
//            auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
//            xs.push_back( left + width * normX );
//        }
//        
//        return xs;
//    }
//
//    void ResponseCurve::drawBackgroundGrid(juce::Graphics &g)
//    {
//        using namespace juce;
//        auto freqs = getFrequencies();
//        
//        auto renderArea = getAnalysisArea();
//        auto left = renderArea.getX();
//        auto right = renderArea.getRight();
//        auto top = renderArea.getY();
//        auto bottom = renderArea.getBottom();
//        auto width = renderArea.getWidth();
//        
//        auto xs = getXs(freqs, left, width);
//        
//        g.setColour(Colours::dimgrey);
//        for( auto x : xs )
//        {
//            g.drawVerticalLine(x, top, bottom);
//        }
//        
//        auto gain = getGains();
//        
//        for( auto gDb : gain )
//        {
//            auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
//            
//            g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey );
//            g.drawHorizontalLine(y, left, right);
//        }
//    }
//
//    void ResponseCurve::drawTextLabels(juce::Graphics &g)
//    {
//        using namespace juce;
//        g.setColour(Colours::lightgrey);
//        const int fontHeight = 10;
//        g.setFont(fontHeight);
//        
//        auto renderArea = getAnalysisArea();
//        auto left = renderArea.getX();
//        
//        auto top = renderArea.getY();
//        auto bottom = renderArea.getBottom();
//        auto width = renderArea.getWidth();
//        
//        auto freqs = getFrequencies();
//        auto xs = getXs(freqs, left, width);
//        
//        for( int i = 0; i < static_cast<int>(freqs.size()); ++i )
//        {
//            auto f = freqs[static_cast<size_t>(i)];
//            auto x = xs[static_cast<size_t>(i)];
//
//            bool addK = false;
//            String str;
//            if( f > 999.f )
//            {
//                addK = true;
//                f /= 1000.f;
//            }
//
//            str << f;
//            if( addK )
//                str << "k";
//            str << "Hz";
//            
//            auto textWidth = g.getCurrentFont().getStringWidth(str);
//
//            Rectangle<int> r;
//
//            r.setSize(textWidth, fontHeight);
//            r.setCentre(static_cast<int>(x), 0);
//            r.setY(1);
//            
//            g.drawFittedText(str, r, juce::Justification::centred, 1);
//        }
//        
//        auto gain = getGains();
//
//        for( auto gDb : gain )
//        {
//            auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
//            
//            String str;
//            if( gDb > 0 )
//                str << "+";
//            str << gDb;
//            
//            auto textWidth = g.getCurrentFont().getStringWidth(str);
//            
//            Rectangle<int> r;
//            r.setSize(textWidth, fontHeight);
//            r.setX(getWidth() - textWidth);
//            r.setCentre(r.getCentreX(), y);
//            
//            g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey );
//            
//            g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
//            
//            str.clear();
//            str << (gDb - 24.f);
//
//            r.setX(1);
//            textWidth = g.getCurrentFont().getStringWidth(str);
//            r.setSize(textWidth, fontHeight);
//            g.setColour(Colours::lightgrey);
//            g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
//        }
//    }
//
//    void ResponseCurve::resized()
//    {
//        using namespace juce;
//        // fix
//        responseCurve.preallocateSpace(getWidth() * 3);
//        updateResponseCurve();
//    }
//}
