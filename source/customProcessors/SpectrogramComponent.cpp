#include "SpectrogramComponent.hpp"

SpectrogramComponent::SpectrogramComponent() : spectrogramImage(juce::Image::RGB, 512, 512, true) {
  forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);
  window = std::make_unique<juce::dsp::WindowingFunction<float>>(
    fftSize, juce::dsp::WindowingFunction<float>::WindowingMethod(windowType - 1));
  fifo.resize(fftSize);
  fftData.resize(fftSize * 2);
}

SpectrogramComponent::~SpectrogramComponent() {}

//==============================================================================
void SpectrogramComponent::prepareToPlay(double, int) {}
void SpectrogramComponent::releaseResources() {}

void SpectrogramComponent::processBlock(juce::AudioBuffer<float>& audioBuffer,
                                        juce::MidiBuffer& midiBuffer) {
  if (!bypass) {  // bypass variable stops from writing to fft arrays while they are being resized
    if (audioBuffer.getNumChannels() > 0) {
      float* channelData = audioBuffer.getWritePointer(0, 0);
      for (auto i = 0; i < audioBuffer.getNumSamples(); ++i) pushNextSampleIntoFifo(channelData[i]);
    }
  }
}

//==============================================================================

void SpectrogramComponent::pushNextSampleIntoFifo(float sample) noexcept {
  // if the fifo contains enough data, copy it to fftData. IS THIS A BAD IDEA?? This function gets
  // called in the audio thread... Though this copy only happens every "fftSize" samples.
  if (fifoIndex == fftSize) {
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    std::copy(fifo.begin(), fifo.end(), fftData.begin());
    drawNextLineOfSpectrogram();

    fifoIndex = 0;
  }
  // write sample to next index of fifo
  fifo[(size_t)fifoIndex++] = sample;
}

void SpectrogramComponent::drawNextLineOfSpectrogram() {
  auto rightHandEdge = spectrogramImage.getWidth() - 1;
  auto imageHeight = spectrogramImage.getHeight();

  // first, shuffle our image leftwards by 1 pixel..
  spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

  window->multiplyWithWindowingTable(fftData.data(), fftSize);

  // then render our FFT data..
  forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());

  // find the range of values produced, so we can scale our rendering to
  // show up the detail clearly
  auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2);

  for (auto y = 1; y < imageHeight; ++y) {
    auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
    auto fftDataIndex =
      (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2));
    auto level =
      juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

    spectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
  }
}

void SpectrogramComponent::changeOrder(int order) {
  // bypass to make sure we don't write new samples while the arrays are being resized.
  bypass = true;
  int fftOrder = order;
  int fftSize = 1 << fftOrder;
  forwardFFT.reset(new juce::dsp::FFT(fftOrder));
  fifo.resize(fftSize);
  fftData.resize(fftSize * 2);
  window.reset(new juce::dsp::WindowingFunction<float>(
    fftSize, juce::dsp::WindowingFunction<float>::WindowingMethod(windowType - 1)));
  bypass = false;
}

void SpectrogramComponent::changeWindowType(int type) {
  bypass = true;
  windowType = type;
  window.reset(new juce::dsp::WindowingFunction<float>(
    fftSize, juce::dsp::WindowingFunction<float>::WindowingMethod(windowType - 1)));
  bypass = false;
}

//==================================================================================================

SpectrogramComponent::Editor::Editor(SpectrogramComponent& owner)
  : ownerProcessor(owner), AudioProcessorEditor(owner) {
  setOpaque(true);
  setSize(700, 500);
  setResizable(true, false);
  // Adding items to the window size menu. This looks messy but basically I'm adding options from
  // 2^6 (64) to 2^12 (4096), and displaying those window sizes (first arg), while the "id" that
  // represents those options is just the order (6 to 13; second arg).
  for (int i = 6; i < 13; i++) fftOrderMenu.addItem(std::to_string(int(pow(2, i))), i);
  // Set default menu item selected.
  fftOrderMenu.setSelectedId(10);
  // attach label to the menu; second arg "true" means label will be on the left, instead of above
  fftOrderMenuLabel.attachToComponent(&fftOrderMenu, true);
  addAndMakeVisible(fftOrderMenu);
  // LAMBDA! Calling the owner processor's changeOrder function with the menu's reported order
  // whenever the menu selection is changed.
  fftOrderMenu.onChange = [&] { owner.changeOrder(fftOrderMenu.getSelectedId()); };

  for (int i = 0; i < juce::dsp::WindowingFunction<float>::numWindowingMethods; i++)
    windowTypeMenu.addItem(juce::dsp::WindowingFunction<float>::getWindowingMethodName(
                             juce::dsp::WindowingFunction<float>::WindowingMethod(i)),
                           i + 1);
  windowTypeMenu.setSelectedId(owner.windowType);
  windowTypeMenu.onChange = [&] { owner.changeWindowType(windowTypeMenu.getSelectedId()); };
  windowTypeLabel.attachToComponent(&windowTypeMenu, true);
  addAndMakeVisible(windowTypeMenu);
}

SpectrogramComponent::Editor::~Editor() {}

void SpectrogramComponent::Editor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
  g.setOpacity(1.0f);
  // Just getting a slice of the parent window's canvas. Note, removeFromBottom probably isn't what
  // you think: it actually takes the given rectangle (in this case the parent window's
  // localBounds), and slices a bit (250 pixels in this case) off the bottom, returning that
  // rectangle. This is what we store in "drawingCanvas" to put our spectrogram in. Also, behind the
  // scenes it removes this bit from the original rectangle. So, after doing this, the parent's
  // localBounds are smaller by 250 pixels from the bottom.
  drawingCanvas = getParentComponent()->getLocalBounds().removeFromBottom(250);
  setBounds(drawingCanvas);
  // More removeFromBottom to slice a chunk in which to draw our parameter menus. Here I also use
  // "withLeft" to move the left edge of the menu over to make room for the label.
  paramSection = getLocalBounds().removeFromTop(25).withLeft(100);
  fftOrderMenu.setBounds(paramSection.removeFromLeft(100));
  windowTypeMenu.setBounds(paramSection.removeFromLeft(200).withLeft(300));
  // Draw
  g.drawImage(ownerProcessor.spectrogramImage, getLocalBounds().toFloat());
}