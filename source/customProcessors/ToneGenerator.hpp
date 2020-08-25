/*
A very basic audio processor that shows how to build and use them in this context.
BaseProcessor.hpp exists to overwrite default values inherited from AudioProcessor. 
*/
#ifndef ToneGenerator_hpp
#define ToneGenerator_hpp

//for access to juce::dsp namespace
#include <juce_dsp/juce_dsp.h>

#include "BaseProcessor.hpp"
#include "pedal/TSine.hpp"
class ToneGenerator : public BaseProcessor{
  public: 
  ToneGenerator();
  
  void prepareToPlay (double, int) override;
  void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
  void releaseResources() override;
  private:
  TSine oscillator;
};
#endif