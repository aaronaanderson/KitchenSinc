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
  
  virtual void prepareToPlay (double, int) override;
  virtual void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
  virtual void releaseResources() override; 
  const juce::String getName() const override {return "TestTone";}
  private:
  TSine oscillator;
};
#endif