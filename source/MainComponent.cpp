#include "MainComponent.hpp"

MainComponent::AudioMidiSettingsWindow::AudioMidiSettingsWindow(MainComponent& self, juce::AudioDeviceManager& deviceManagerRef)
  : juce::DocumentWindow("Audio/MIDI Settings", 
                         juce::LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), 
                         DocumentWindow::minimiseButton | DocumentWindow::closeButton),
    owner(self)
{
  //auto crashedFile = getAppProperties().getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");
  setContentOwned(new juce::AudioDeviceSelectorComponent(deviceManagerRef, 
                                                         0,
                                                         16,
                                                         0,
                                                         16,
                                                         true, true, false, false), true);  
  //auto* plC = dynamic_cast<juce::AudioDeviceSelectorComponent*>(getContentComponent());
  //setResizable(true, false);
  setResizable(false, false);
  setResizeLimits(300, 400, 1920, 1080);
  setTopLeftPosition(60, 60);
  setVisible(true);
  //for keyboard listener
}
MainComponent::AudioMidiSettingsWindow::~AudioMidiSettingsWindow(){
  //We should set this here so it can be recovered when opening
  //getAppProperties().getUserSettings()->setValue("listWindowPos", getWindowStateAsString());
  clearContentComponent();
}
void MainComponent::AudioMidiSettingsWindow::closeButtonPressed(){
  //erase ourself from the owner's reference
  owner.settingsWindow = nullptr;
}
//==============================================================================
MainComponent::MainComponent() : 
  audioGraph(std::make_unique<juce::AudioProcessorGraph>()),
  audioSettings(deviceManager)
{
  setSize(600, 400);
  startTimerHz(30);
  addKeyListener(this);
  // tell the ProcessorPlayer what audio callback function to play (.get() needed since audioGraph
  // is a unique_ptr)

  processorPlayer.setProcessor(audioGraph.get());
  // simplest way to start audio device. Uses whichever device the current system (mac/pc/linux
  // machine) uses
  deviceManager.initialiseWithDefaultDevices(2, 2);
  // Tell the processor player to keep moving every time the device requests more data
  deviceManager.addAudioCallback(&processorPlayer);
  deviceManager.setMidiInputDeviceEnabled(juce::MidiInput::getDefaultDevice().identifier, true);
  deviceManager.addMidiInputDeviceCallback(juce::MidiInput::getDefaultDevice().identifier, &processorPlayer);
  // set up the graph
  audioGraph->clear();  // likely not needed but won't hurt

  // a node that passes audio out to your device
  audioOutputNode = audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
  // a node that passes audio from your device to the graph
  audioInputNode = audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
  // a node that takes midi input from hardware interface, or from another software interface
  midiInputNode = audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode));

  
  // a simple sine tone generator node
  testToneNode = audioGraph->addNode(std::make_unique<ToneGenerator>());
  // set the details of the processor (io and samplerate/buffersize
  testToneNode->getProcessor()->setPlayConfigDetails(
    0, 2, deviceManager.getAudioDeviceSetup().sampleRate,
    deviceManager.getAudioDeviceSetup().bufferSize);
  // connect the 'left' channel
  
  // audioGraph->addConnection({{testToneNode->nodeID, 0}, {audioOutputNode->nodeID, 0}});
  // // connect the 'right' channel
  // audioGraph->addConnection({{testToneNode->nodeID, 1}, {audioOutputNode->nodeID, 1}});
  
  sineSynthNode = audioGraph->addNode(std::make_unique<MidiSynthProcessor>());
  sineSynthNode->getProcessor()->setPlayConfigDetails(0, 2, deviceManager.getAudioDeviceSetup().sampleRate, 
                                                            deviceManager.getAudioDeviceSetup().bufferSize);
  //give the sine synth some MIDI so it knows what to do
  audioGraph->addConnection({{midiInputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex}, 
                             {sineSynthNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex}});
  //connect the audio output of sinesynth to hardware output
  audioGraph->addConnection({{sineSynthNode->nodeID, 0}, {audioOutputNode->nodeID, 0}});
  audioGraph->addConnection({{sineSynthNode->nodeID, 1}, {audioOutputNode->nodeID, 1}});

  // Spectrogram
  spectrogramNode = audioGraph->addNode(std::make_unique<SpectrogramComponent>());

  spectrogramNode->getProcessor()->setPlayConfigDetails(
    1, 0, deviceManager.getAudioDeviceSetup().sampleRate,
    deviceManager.getAudioDeviceSetup().bufferSize);
  audioGraph->addConnection({{audioInputNode->nodeID, 0}, {spectrogramNode->nodeID, 0}});
  // This is the best way I've found to get the editor and be able to display it. Just have your
  // mainComponent own a pointer to an editor, then point it to the editor when it's created.
  spectrogramEditor = spectrogramNode->getProcessor()->createEditor();

  audioSettings.button.setBounds(getLocalBounds().removeFromTop(50));
  addAndMakeVisible(audioSettings.button);
  addAndMakeVisible(spectrogramEditor);
}
// TrackGroup* trackGroupPtr = dynamic_cast<TrackGroup*>(graphElementList.data()[groupID].get());
//==============================================================================
void MainComponent::timerCallback() {
  // Should be able to set the timer and call repaint from within the processorEditor itself, but it
  // gives segfaults when I try, so I'm setting up the timer and repaint call from here in the
  // mainComponent because it seems to work.
  spectrogramEditor->repaint();
}

bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent){
    if(key.isKeyCode('s')){


        settingsWindow = std::make_unique<AudioMidiSettingsWindow>(*this, deviceManager);
      
    }
}
void MainComponent::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  // g.setFont(juce::Font(16.0f));
  // g.setColour(juce::Colours::white);
  // g.drawText("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized() {
  // This is called when the MainComponent is resized.
  // If you add any child components, this is where you should
  // update their positions.
  audioSettings.button.setBounds(getLocalBounds().removeFromTop(50));
}

MainComponent::~MainComponent() {
  deviceManager.closeAudioDevice();
  // unfortunately the naming convention to de-allocate a unique pointer is .reset()
  audioGraph.reset();
  deleteAllChildren();
}
