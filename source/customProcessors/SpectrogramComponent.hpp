/*
This is how we make a spectrogram display as an audio graph processor.
Most of the STFT stuff and Image updating stuff (i.e. the drawNextLineOfSpectrogram() function)
is pulled from the official Juce fft tutorial and adapted to work in a processor graph.
This spectrogram defaults to taking up the bottom 250 pixels of the parent window. To change this,
see the paint method in the SpectrogramComponent::Editor subclass below.
*/

#ifndef SpectrogramComponent_hpp
#define SpectrogramComponent_hpp

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "BaseProcessor.hpp"

class SpectrogramComponent : public BaseProcessor {
 public:
  SpectrogramComponent();

  // Any processor that needs to have controls or display anything at all needs this function
  // overridden to return a pointer to its editor. Note: If I wanted to not make my own editor and
  // allow Juce to give me a sensible control (slider, menu, etc.) for each parameter in my
  // processor, I could have returned a new juce::GenericAudioProcessorEditor(*this).
  juce::AudioProcessorEditor* createEditor() override { return new Editor(*this); }

  // override this and set to true if there's an editor for this processor
  bool hasEditor() const override { return true; }

  ~SpectrogramComponent();

  //==============================================================================
  void prepareToPlay(double, int) override;
  void releaseResources() override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
  // this is modified from the juce fft tutorial
  void pushNextSampleIntoFifo(float sample) noexcept;

  // this is modified from the juce fft tutorial
  void drawNextLineOfSpectrogram();

  // I added this function to allow changing the fft Order/window size at runtime. Note that it
  // takes an ORDER, i.e. x where 2^x is the new window size.
  void changeOrder(int order);

  // I added this function to allow changing the window shape at runtime.
  void changeWindowType(int type);

  int fftOrder = 10;
  int fftSize = 1 << fftOrder;
  int windowType = 2;

  juce::Image spectrogramImage;

 private:
  std::unique_ptr<juce::dsp::FFT> forwardFFT;
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
  std::vector<float> fifo;
  std::vector<float> fftData;
  int fifoIndex = 0;
  bool bypass = false;

  // We have to make the processorEditor a subclass of the processor itself.
  class Editor : public juce::AudioProcessorEditor {
   public:
    // Passing a reference to the parent processor isn't required but helps if we want to get into
    // the owner's stuff from the editor
    Editor(SpectrogramComponent& owner);
    ~Editor();

    void paint(juce::Graphics& g) override;

    juce::ComboBox fftOrderMenu;
    juce::Label fftOrderMenuLabel{"fftOrderMenuLabel", "Window Size"};

    juce::ComboBox windowTypeMenu;
    juce::Label windowTypeLabel{"windowTypeMenuLabel", "Window Type"};

   private:
    juce::Rectangle<int> drawingCanvas;
    juce::Rectangle<int> paramSection;
    SpectrogramComponent& ownerProcessor;
  };
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramComponent)
};
#endif