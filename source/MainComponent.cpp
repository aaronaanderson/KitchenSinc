#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() :
  audioGraph(std::make_unique<juce::AudioProcessorGraph>())
{
  setSize (600, 400);
    
  processorPlayer.setProcessor(audioGraph.get());
  deviceManager.initialiseWithDefaultDevices(0, 2);
  deviceManager.addAudioCallback(&processorPlayer);
  //set up the graph
  audioGraph->clear();
  audioInputNode = audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
  audioOutputNode = audioGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
  testToneNode = audioGraph->addNode(std::make_unique<ToneGenerator>());
  testToneNode->getProcessor()->setPlayConfigDetails(0, 2, deviceManager.getAudioDeviceSetup().sampleRate, deviceManager.getAudioDeviceSetup().bufferSize);
  audioGraph->addConnection({{testToneNode->nodeID, 0}, 
                             {audioOutputNode->nodeID, 0}});
  audioGraph->addConnection({{testToneNode->nodeID, 1}, 
                             {audioOutputNode->nodeID, 1}});

}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{

}
