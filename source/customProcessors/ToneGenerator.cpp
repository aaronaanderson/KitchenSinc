#include "ToneGenerator.hpp"

ToneGenerator::ToneGenerator() : BaseProcessor()
{

}
void ToneGenerator::prepareToPlay (double, int){
    oscillator.setFrequency(439.0f);
}
void ToneGenerator::processBlock (juce::AudioSampleBuffer& audioBuffer, juce::MidiBuffer& midiBuffer){
  //wrap that buffer in an AudioBlock class
  juce::dsp::AudioBlock<float> audioBlock (audioBuffer);
  //now we can get a pointer for each channel of audio data
  //buffers are not interleaved; samples are in order of channel
  //however, we can make it 'look' normal by swapping the channel pointer around
  
  //for every sample
  for(int sampleIndex = 0; sampleIndex < audioBlock.getNumSamples(); sampleIndex++){
    //progress the oscillator by one sample
    oscillator.generateSample();
    //for every channel this sample is applied to
    for(int channelIndex = 0; channelIndex < audioBlock.getNumChannels(); channelIndex++){
      //get the address of the 0th sample of the channel sample array
      float* channelPointer = audioBlock.getChannelPointer(channelIndex);
      channelPointer[sampleIndex] = oscillator.getSample();
    }
  }

}
void ToneGenerator::releaseResources(){

}