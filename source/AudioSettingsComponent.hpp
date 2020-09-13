#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module
// headers directly. If you need to remain compatible with Projucer-generated
// builds, and have called `juce_generate_juce_header(<thisTarget>)` in your
// CMakeLists.txt, you could `#include <JuceHeader.h>` here instead, to make all
// your module headers visible.
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <juce_dsp/juce_dsp.h>
//==============================================================================
class audioSettingsComponent : public juce::Component,
                               public juce::ChangeListener,
                               private juce::Timer {
 public:
  audioSettingsComponent(juce::AudioDeviceManager& ParentDeviceManager_);

  ~audioSettingsComponent() override;

  void paint(juce::Graphics& g) override;

  void resized() override;

  juce::TextButton button;                   // button to open Audio Settings window
  juce::DialogWindow::LaunchOptions window;  // dialog window containing Audio Settings

 private:
  void changeListenerCallback(juce::ChangeBroadcaster*) override;

  static juce::String getListOfActiveBits(const juce::BigInteger& b);

  void timerCallback() override;

  void dumpDeviceInfo();

  void logMessage(const juce::String& m);

  //==========================================================================
  juce::Random random;
  juce::AudioDeviceSelectorComponent audioSetupComp;
  juce::Label cpuUsageLabel;
  juce::Label cpuUsageText;
  juce::TextEditor diagnosticsBox;
  juce::AudioDeviceManager& parentDeviceManager;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(audioSettingsComponent)
};