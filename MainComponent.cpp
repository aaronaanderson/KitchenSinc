#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    setAudioChannels(0, 2);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate){
  pdlSettings::bufferSize = samplesPerBlockExpected;
  pdlSettings::sampleRate = sampleRate;
  sineOsc.setFrequency(440.0f);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::dsp::AudioBlock<float> block(*bufferToFill.buffer);
	juce::dsp::ProcessContextReplacing<float> context(block);
    auto&& outputBlock = context.getOutputBlock();
    
    float* outputPointer;
    for(int i = 0; i < bufferToFill.buffer->getNumSamples(); i++){

        for(int j = 0; j < bufferToFill.buffer->getNumChannels(); j++){
            //get the pointer for the correct channel
            outputPointer = outputBlock.getChannelPointer(j);
            //address that pointer at the ith index
            outputPointer[i] = sineOsc.generateSample() * 0.05f;
        }
    }

}

void MainComponent::releaseResources(){
    juce::Logger::getCurrentLogger()->writeToLog ("Releasing audio resources");
    shutdownAudio();
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
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
