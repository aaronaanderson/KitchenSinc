#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module
// headers directly. If you need to remain compatible with Projucer-generated
// builds, and have called `juce_generate_juce_header(<thisTarget>)` in your
// CMakeLists.txt, you could `#include <JuceHeader.h>` here instead, to make all
// your module headers visible.
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "customProcessors/ToneGenerator.hpp"
#include "customProcessors/MidiSynthProcessor.hpp"
// our components
#include "AudioSettingsComponent.hpp"
#include "customProcessors/SpectrogramComponent.hpp"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component, private juce::Timer {
 public:
  //==============================================================================
  MainComponent();
  ~MainComponent();

  // Graphics
  // stuff==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  void timerCallback() override;

  TSine sineOsc;  // simple sine oscillator

 private:
  // Instead of inheriting from juce::AudioAppComponent, we make our own deviceManager
  juce::AudioDeviceManager deviceManager;
  // The audio graph will store and set the full DSP chain.
  std::unique_ptr<juce::AudioProcessorGraph> audioGraph;

  juce::AudioProcessorGraph::Node::Ptr audioInputNode;   // access to hardware input
  juce::AudioProcessorGraph::Node::Ptr audioOutputNode;  // access to hardware output
  juce::AudioProcessorGraph::Node::Ptr midiInputNode;    // access to hardware/software MIDI input
  juce::AudioProcessorGraph::Node::Ptr testToneNode;
  juce::AudioProcessorGraph::Node::Ptr sineSynthNode;
  juce::AudioProcessorGraph::Node::Ptr spectrogramNode;  // Spectrogram
  juce::AudioProcessorEditor* spectrogramEditor;         // Spectrogram editor
  // this DSP chain will be executed by the processorPlayer
  juce::AudioProcessorPlayer processorPlayer;
  // Audio Settings window for changing IO settings at runtime
  audioSettingsComponent audioSettings;
  // this is JUCE's convenience MACRO to make sure we don't make terrible, terrible mistakes with
  // our pointers
  juce::AudioDeviceSelectorComponent audioMidiSettingComponent;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
