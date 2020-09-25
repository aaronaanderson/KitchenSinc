#ifndef MainComponent_hpp
#define MainComponent_hpp

// CMake builds don't use an AppConfig.h, so it's safe to include juce module
// headers directly. If you need to remain compatible with Projucer-generated
// builds, and have called `juce_generate_juce_header(<thisTarget>)` in your
// CMakeLists.txt, you could `#include <JuceHeader.h>` here instead, to make all
// your module headers visible.
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

// grab a trival sine wave
#include "customProcessors/ToneGenerator.hpp"
#include "PluginWindow.hpp"
// our components
#include "AudioSettingsComponent.hpp"
#include "customProcessors/SpectrogramComponent.hpp"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component, 
                      private juce::Timer, 
                      private juce::KeyListener{
 public:
  //==============================================================================
  MainComponent();
  ~MainComponent();

  // Graphics
  // stuff==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;
  
  //from juce::Timer
  void timerCallback() override;
  
  //from juce::KeyListener
  bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
  TSine sineOsc;  // simple sine oscillator

 private:
  float* floatptr;
  // Instead of inheriting from juce::AudioAppComponent, we make our own deviceManager
  juce::AudioDeviceManager deviceManager;
  // The audio graph will store and set the full DSP chain.
  std::unique_ptr<juce::AudioProcessorGraph> audioGraph;

  juce::AudioProcessorGraph::Node::Ptr audioInputNode;   // access to hardware input
  juce::AudioProcessorGraph::Node::Ptr audioOutputNode;  // access to hardware output
  juce::AudioProcessorGraph::Node::Ptr testToneNode;
  juce::AudioProcessorGraph::Node::Ptr spectrogramNode;           // Spectrogram
  std::unique_ptr<juce::AudioProcessorEditor> spectrogramEditor;  // Spectrogram editor
  // this DSP chain will be executed by the processorPlayer
  juce::AudioProcessorPlayer processorPlayer;
  // Audio Settings window for changing IO settings at runtime
  audioSettingsComponent audioSettings;

  //trying the default for MIDI input as well
  juce::AudioDeviceSelectorComponent audioMidiSelectorComponent;
  // this is JUCE's convenience MACRO to make sure we don't make terrible, terrible mistakes with
  // our pointers
  
  //juce::ApplicationProperties& getAppProperties() {return *getApp().appProperties;}
  //========PlugIn Hosting 
  //this is a replacement for AudioProcessorGraph's .addNode() function, for plugins
  juce::AudioProcessorGraph::Node::Ptr addPlugin(const juce::PluginDescription&);
  //Most plugins have interfaces to be interfaced. This checks for that, or creates a generic interface if none found
  PluginWindow* getOrCreateWindowFor (juce::AudioProcessorGraph::Node::Ptr);
  //List of all created windows (they remove themselves when closed)
  juce::OwnedArray<PluginWindow> activeWindows;
  //Magic class that takes care of format stuff
  juce::AudioPluginFormatManager formatManager;
  //list of known plugins (takes a while to fill, worth storing)
  juce::KnownPluginList knownPluginList;
  //making this as a subclass of MainComponent since it's small and only used here
  class PluginListWindow;//defined in CPP
  //only ever need one of these windows
  std::unique_ptr<PluginListWindow> pluginListWindow;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class MainComponent::PluginListWindow : public juce::DocumentWindow {
  public:
  PluginListWindow(MainComponent& self, juce::AudioPluginFormatManager& managerReference);
  ~PluginListWindow() override;
  void closeButtonPressed() override;
  private:
  MainComponent& owner;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow);
};
#endif