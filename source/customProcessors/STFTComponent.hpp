/*
This is a template component for performing an STFT on an incoming audio stream and then inverse
STFT to output. As it is, it converts the signal to the frequency domain and back to time domain, so
in its current form, the output should sound the same as the input. The usefulness of this template
is that you can simply insert whatever frequency domain manipulations you want in between the fft
and ifft.

Nice things like window size, window shape, overlap, are already implemented with sliders and menus
to change them at runtime.

I don't claim that this is the most efficient STFT, but it seems fairly robust based on my tests. I
plan on making several spectral processing effects, so this provides a base processor that can
easily be put into any processorGraph.

- Rodney DuPlessis
*/

#ifndef STFTComponent_hpp
#define STFTComponent_hpp

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "BaseProcessor.hpp"

class STFTComponent : public BaseProcessor {
 public:
  STFTComponent();

  juce::AudioProcessorEditor* createEditor() override {
    return new juce::GenericAudioProcessorEditor(*this);
  }

  bool hasEditor() const override { return true; }

  ~STFTComponent();

  //==============================================================================
  void prepareToPlay(double, int) override;
  void releaseResources() override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
  // Adding samples from the input stream into buffers
  void pushNextSampleIntoBuffers(float sample) noexcept;

  // FFT, transform in frequency domain, IFFT, and add to output queue
  void fftAndBack(std::vector<std::complex<float>> buffer);

  // Change the fft Order/window size at runtime. Note that it
  // takes an ORDER, i.e. x where 2^x is the new window size.
  void changeOrder(int order);

  // Changing the window shape.
  void changeWindowType(int type);

  // Change number of overlapped stft windows. 1 is no overlap (discrete windows).
  void changeOverlap(int newOverlap);

  int fftOrder = 10;
  int fftSize = 1 << fftOrder;
  int windowIndex = 2;

 private:
  std::unique_ptr<juce::dsp::FFT> forwardFFT;
  std::unique_ptr<juce::dsp::FFT> inverseFFT;
  std::vector<float> window;
  std::vector<std::vector<std::complex<float>>> inputBuffer;
  std::vector<std::complex<float>> frequencyDomainData;
  std::vector<float> outputQueue;
  int outputQueueIndex = 0;
  int overlap = 2;
  int hopSize = fftSize / overlap;
  std::vector<int> inputBufferIndex;

  juce::AudioParameterChoice* fftOrderMenu;
  juce::AudioParameterChoice* fftWindowMenu;
  juce::AudioParameterInt* overlapSlider;
  juce::AudioParameterBool* bypass;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(STFTComponent)
};
#endif