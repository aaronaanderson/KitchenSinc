#include "MainComponent.hpp"

//Should define this first before defining its container

MainComponent::PluginListWindow::PluginListWindow(MainComponent& self, juce::AudioPluginFormatManager& managerReference)
  : juce::DocumentWindow("Available Plugins", 
                         juce::LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), 
                         DocumentWindow::minimiseButton | DocumentWindow::closeButton),
    owner(self)
{
  //auto crashedFile = getAppProperties().getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");
  setContentOwned(new juce::PluginListComponent(managerReference, 
                                          owner.knownPluginList,
                                          juce::File(), //dummy input so I can use this component
                                          nullptr), true);  
  auto* plC = dynamic_cast<juce::PluginListComponent*>(getContentComponent());
  setResizable(true, false);
  setResizeLimits(300, 400, 1920, 1080);
  setTopLeftPosition(60, 60);
  setVisible(true);

  //for keyboard listener
}
MainComponent::PluginListWindow::~PluginListWindow(){
  //We should set this here so it can be recovered when opening
  //getAppProperties().getUserSettings()->setValue("listWindowPos", getWindowStateAsString());
  clearContentComponent();
}
void MainComponent::PluginListWindow::closeButtonPressed(){
  //erase ourself from the owner's reference
  owner.pluginListWindow = nullptr;
}
//==============================================================================
MainComponent::MainComponent()
  : audioGraph(std::make_unique<juce::AudioProcessorGraph>()), audioSettings(deviceManager) {
  setSize(600, 400);
  startTimerHz(30);
  //#define JUCE_PLUGINHOST_LADSPA 1
  formatManager.addDefaultFormats();
  addKeyListener(this);
  std::cout << formatManager.getNumFormats() << std::endl;
  // tell the ProcessorPlayer what audio callback function to play (.get() needed since audioGraph
  // is a unique_ptr)
    pluginListWindow = std::make_unique<PluginListWindow>(*this, formatManager);

  processorPlayer.setProcessor(audioGraph.get());
  // simplest way to start audio device. Uses whichever device the current system (mac/pc/linux
  // machine) uses
  deviceManager.initialiseWithDefaultDevices(2, 2);
  // Tell the processor player to keep moving every time the device requests more data
  deviceManager.addAudioCallback(&processorPlayer);
  // set up the graph
  audioGraph->clear();  // likely not needed but won't hurt
  // a node that passes in input from your device (not currently used)
  audioInputNode =
    audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
      juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
  // a node that passes audio out to your device
  audioOutputNode =
    audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
      juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
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

  // Spectrogram
  spectrogramNode = audioGraph->addNode(std::make_unique<SpectrogramComponent>());

  spectrogramNode->getProcessor()->setPlayConfigDetails(
    1, 0, deviceManager.getAudioDeviceSetup().sampleRate,
    deviceManager.getAudioDeviceSetup().bufferSize);
  audioGraph->addConnection({{audioInputNode->nodeID, 0}, {spectrogramNode->nodeID, 0}});
  // This is the best way I've found to get the editor and be able to display it. Just have your
  // mainComponent own a pointer to an editor, then point it to the editor when it's created.
  spectrogramEditor =
    std::unique_ptr<juce::AudioProcessorEditor>(spectrogramNode->getProcessor()->createEditor());

  audioSettings.button->setBounds(getLocalBounds().removeFromTop(50));
  addAndMakeVisible(audioSettings.button.get());
  addAndMakeVisible(spectrogramEditor.get());
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
    if(key.isKeyCode('a')){
      std::cout << "it worked i guess" << std::endl;
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
  audioSettings.button->setBounds(getLocalBounds().removeFromTop(50));
}

MainComponent::~MainComponent() {
  deviceManager.closeAudioDevice();
  // unfortunately the naming convention to de-allocate a unique pointer is .reset()
  audioGraph.reset();
  deleteAllChildren();
}

PluginWindow* MainComponent::getOrCreateWindowFor(juce::AudioProcessorGraph::Node::Ptr node){
  if(node != nullptr){
    //first check to make sure the window isn't already made
    for(auto* windowPtr : activeWindows){//for each window in activeWindows
      if(windowPtr->node.get() == node){//if there is already a window for this node
        return windowPtr;//return it
      }
    }
    //if the window hasn't been created and added to the active list yet, do that
    //check it the node's processor actually exists (isn't a nullptr)
    if(auto* processor = node->getProcessor()){
      //here is how you check if its a plugin
      if(auto* plugin = dynamic_cast<juce::AudioPluginInstance*>(processor)){
        //if it is a plugin, you can grab things from it here
        //presently I don't have anything to do wtih this, but leaving it here
        //because it will be important
      }
      // create the new window, and return a pointer to it
      auto* windowPtr = activeWindows.add(new PluginWindow(node, activeWindows));
      return windowPtr;
    }
    //if we've gotten this far, we have failed
    return nullptr;
  }
}