#include "AudioSettingsComponent.h"
//==============================================================================
audioSettingsComponent::audioSettingsComponent(juce::AudioDeviceManager& parentDeviceManager_)
  : audioSetupComp(parentDeviceManager_,
                   0,       // minimum input channels
                   8,       // maximum input channels
                   0,       // minimum output channels
                   8,       // maximum output channels
                   false,   // ability to select midi inputs
                   false,   // ability to select midi output device
                   false,   // treat channels as stereo pairs
                   false),  // hide advanced options
    parentDeviceManager(parentDeviceManager_) {
  addAndMakeVisible(audioSetupComp);
  addAndMakeVisible(diagnosticsBox);

  diagnosticsBox.setMultiLine(true);
  diagnosticsBox.setReturnKeyStartsNewLine(true);
  diagnosticsBox.setReadOnly(true);
  diagnosticsBox.setScrollbarsShown(true);
  diagnosticsBox.setCaretVisible(false);
  diagnosticsBox.setPopupMenuEnabled(true);
  diagnosticsBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
  diagnosticsBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
  diagnosticsBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

  cpuUsageLabel.setText("CPU Usage", juce::dontSendNotification);
  cpuUsageText.setJustificationType(juce::Justification::right);
  addAndMakeVisible(&cpuUsageLabel);
  addAndMakeVisible(&cpuUsageText);

  setSize(760, 360);

  parentDeviceManager.addChangeListener(this);

  startTimer(50);

  button.setButtonText("Audio Settings");
  button.onClick = [&] {
    this->window.launchAsync();
    window.content.set(this, false);
  };

  window.dialogTitle = "Audio Settings";
  window.escapeKeyTriggersCloseButton = true;
  window.resizable = true;
  window.useBottomRightCornerResizer = true;
  window.useNativeTitleBar = false;
  window.content.set(this, false);
}

audioSettingsComponent::~audioSettingsComponent() {
  parentDeviceManager.removeChangeListener(this);
}

void audioSettingsComponent::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void audioSettingsComponent::resized() {
  auto rect = getLocalBounds();

  audioSetupComp.setBounds(rect.removeFromLeft(proportionOfWidth(0.6f)));
  rect.reduce(10, 10);

  auto topLine(rect.removeFromTop(20));
  cpuUsageLabel.setBounds(topLine.removeFromLeft(topLine.getWidth() / 2));
  cpuUsageText.setBounds(topLine);
  rect.removeFromTop(20);

  diagnosticsBox.setBounds(rect);
}

void audioSettingsComponent::changeListenerCallback(juce::ChangeBroadcaster*) { dumpDeviceInfo(); }

juce::String audioSettingsComponent::getListOfActiveBits(const juce::BigInteger& b) {
  juce::StringArray bits;

  for (auto i = 0; i <= b.getHighestBit(); ++i)
    if (b[i]) bits.add(juce::String(i));

  return bits.joinIntoString(", ");
}

void audioSettingsComponent::timerCallback() {
  auto cpu = parentDeviceManager.getCpuUsage() * 100;
  cpuUsageText.setText(juce::String(cpu, 6) + " %", juce::dontSendNotification);
}

void audioSettingsComponent::dumpDeviceInfo() {
  logMessage("--------------------------------------");
  logMessage("Current audio device type: " +
             (parentDeviceManager.getCurrentDeviceTypeObject() != nullptr
                ? parentDeviceManager.getCurrentDeviceTypeObject()->getTypeName()
                : "<none>"));

  if (auto* device = parentDeviceManager.getCurrentAudioDevice()) {
    logMessage("Current audio device: " + device->getName().quoted());
    logMessage("Sample rate: " + juce::String(device->getCurrentSampleRate()) + " Hz");
    logMessage("Block size: " + juce::String(device->getCurrentBufferSizeSamples()) + " samples");
    logMessage("Bit depth: " + juce::String(device->getCurrentBitDepth()));
    logMessage("Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
    logMessage("Active input channels: " + getListOfActiveBits(device->getActiveInputChannels()));
    logMessage("Output channel names: " + device->getOutputChannelNames().joinIntoString(", "));
    logMessage("Active output channels: " + getListOfActiveBits(device->getActiveOutputChannels()));
  } else {
    logMessage("No audio device open");
  }
}

void audioSettingsComponent::logMessage(const juce::String& m) {
  diagnosticsBox.moveCaretToEnd();
  diagnosticsBox.insertTextAtCaret(m + juce::newLine);
}
