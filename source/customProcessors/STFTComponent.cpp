#include "STFTComponent.hpp"

STFTComponent::STFTComponent() {
  // Intialize both our fft objects. juce fft objects can perform both forward and inverse fft, but
  // having one for each is more efficient.
  forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);
  inverseFFT = std::make_unique<juce::dsp::FFT>(fftOrder);
  // resize window array to fft size
  window.resize(fftSize);
  // fill window array with values
  juce::dsp::WindowingFunction<float>::fillWindowingTables(
    window.data(), fftSize, juce::dsp::WindowingFunction<float>::WindowingMethod(windowIndex));
  // we need as many input buffers as overlaps
  inputBuffer.resize(overlap);
  inputBufferIndex.resize(overlap);
  for (int i = 0; i < overlap; i++) {
    // resize each input bufffer to fftsize
    inputBuffer[i].resize(fftSize, 0.0f);
    // stagger input buffers for overlapping
    inputBufferIndex[i] = 0 - hopSize * i;
  }
  frequencyDomainData.resize(fftSize, 0.0f);
  outputQueue.resize(fftSize * 2, 0.0f);

  // add parameters
  addParameter(fftOrderMenu = new juce::AudioParameterChoice(
                 "fftOrderMenu", "FFT Order", {"64", "128", "256", "512", "1024", "2048", "4096"},
                 fftOrder - 6, "FFT Order"));
  // get list of window shapes included in juce
  juce::StringArray juceWindows;
  for (int i = 0; i < juce::dsp::WindowingFunction<float>::numWindowingMethods; i++)
    juceWindows.add(juce::dsp::WindowingFunction<float>::getWindowingMethodName(
      juce::dsp::WindowingFunction<float>::WindowingMethod(i)));

  addParameter(fftWindowMenu = new juce::AudioParameterChoice(
                 "fftWindowMenu", "FFT Window", juceWindows, windowIndex, "FFT Window"));
  addParameter(overlapSlider = new juce::AudioParameterInt("overlap", "Overlap", 1, 12, 2));
  addParameter(bypass = new juce::AudioParameterBool("bypass", "Bypass", 0));
}

STFTComponent::~STFTComponent() {}

//==============================================================================
void STFTComponent::prepareToPlay(double, int) {}
void STFTComponent::releaseResources() {}

void STFTComponent::processBlock(juce::AudioBuffer<float>& audioBuffer,
                                 juce::MidiBuffer& midiBuffer) {
  // Check if our stft paramters have changed and call the appropriate functions before processing
  // the incoming buffer. Adding a listener/broadcaster is probably the more jucy way of doing this.
  if (*fftWindowMenu != windowIndex) changeWindowType(*fftWindowMenu);
  if (*fftOrderMenu != fftOrder - 6) changeOrder(*fftOrderMenu + 6);
  if (*overlapSlider != overlap) changeOverlap(*overlapSlider);
  if (!*bypass) {
    if (audioBuffer.getNumChannels() > 0) {
      float* channelData = audioBuffer.getWritePointer(0, 0);
      for (int i = 0; i < audioBuffer.getNumSamples(); ++i) {
        // push sample into input buffers
        pushNextSampleIntoBuffers(channelData[i]);
        // overwrite buffer with next sample in output queue
        channelData[i] = outputQueue[outputQueueIndex];
        // zero the output queue sample we wrote and advance the index
        outputQueue[outputQueueIndex] = 0;
        outputQueueIndex = (outputQueueIndex + 1) % outputQueue.size();
      }
    }
  }
}
//==============================================================================

void STFTComponent::pushNextSampleIntoBuffers(float sample) noexcept {
  // if the inputBuffer contains enough data, copy it to fftData.
  for (int i = 0; i < overlap; i++) {
    if (inputBufferIndex[i] == fftSize) {
      fftAndBack(inputBuffer[i]);
      inputBufferIndex[i] = 0;
    }
    // write sample to next index of inputBuffer
    if (inputBufferIndex[i] >= 0) inputBuffer[i][(size_t)inputBufferIndex[i]] = sample;
    inputBufferIndex[i]++;
  }
}
//==============================================================================

void STFTComponent::fftAndBack(std::vector<std::complex<float>> buffer) {
  // window the samples
  for (int i = 0; i < fftSize; i++) buffer[i] *= window[i];
  // perform FFT
  forwardFFT->perform(buffer.data(), frequencyDomainData.data(), false);

  /*
  INSERT FREQUENCY DOMAIN PROCESSING HERE
  */

  // Inverse FFT
  inverseFFT->perform(frequencyDomainData.data(), buffer.data(), true);
  // write to output queue
  for (int i = 0; i < fftSize; i++)
    outputQueue[(outputQueueIndex + i) % outputQueue.size()] += buffer[i].real() / overlap;
}
//==============================================================================

void STFTComponent::changeOrder(int order) {
  fftOrder = order;
  fftSize = 1 << fftOrder;
  forwardFFT.reset(new juce::dsp::FFT(fftOrder));
  inverseFFT.reset(new juce::dsp::FFT(fftOrder));

  window.resize(fftSize);
  juce::dsp::WindowingFunction<float>::fillWindowingTables(
    window.data(), fftSize, juce::dsp::WindowingFunction<float>::WindowingMethod(windowIndex),
    true);
  hopSize = fftSize / overlap;
  for (int i = 0; i < overlap; i++) {
    inputBuffer[i].resize(fftSize, 0.0f);
    inputBufferIndex[i] = 0 - hopSize * i;
  }
  frequencyDomainData.resize(fftSize, 0.0f);
  outputQueue.resize(fftSize * 2, 0.0f);
  outputQueueIndex = 0;
}
//==============================================================================

void STFTComponent::changeWindowType(int type) {
  windowIndex = type;
  juce::dsp::WindowingFunction<float>::fillWindowingTables(
    window.data(), fftSize, juce::dsp::WindowingFunction<float>::WindowingMethod(windowIndex),
    true);
}
//==============================================================================

void STFTComponent::changeOverlap(int newOverlap) {
  overlap = newOverlap;
  hopSize = fftSize / overlap;
  inputBuffer.resize(overlap);
  inputBufferIndex.resize(overlap);
  for (int i = 0; i < overlap; i++) {
    inputBuffer[i].resize(fftSize * 2, 0.0f);
    inputBufferIndex[i] = 0 - hopSize * i;
  }
}