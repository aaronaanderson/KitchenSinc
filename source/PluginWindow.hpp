/*
This document is mostly taken from AudioPluginHost PluginWindow.h
This offers fewer features, and takes more code.
Aaron 
*/
#ifndef PluginWindow_hpp
#define PluginWindow_hpp

//I'm not sure what the most reasonable thing to include here would be
//this works though
#include <juce_audio_utils/juce_audio_utils.h>

//It seems that Juce's feature-rich window is a DocumentWindow.
//Our window should inherit from that
class PluginWindow : public juce::DocumentWindow 
{
public:
    //No default constructor
    PluginWindow() = delete;//Not necessary, but shows intent
    //We need acces to a graph node, and an owned array to add ourselves to
    PluginWindow(juce::AudioProcessorGraph::Node::Ptr, juce::OwnedArray<PluginWindow>&);
    //We have stuff to do in the deconstructor, don't let it default
    ~PluginWindow() override;

    //We delte ourselves from the list of windows
    void closeButtonPressed() override;
    //reference to the list to which we belong
    juce::OwnedArray<PluginWindow>& activeWindowListReference;
    const juce::AudioProcessorGraph::Node::Ptr node;
private:
    
    //I don't know why this is here yet, but JUCE used it so we will too
    float getDesktopScaleFactor() const override     { return 1.0f; }
    static juce::String getOpenProp() { return "uiopoen_" ;} //Not really sure what this is for yet
    //has to be inline definition since static I believe
    static juce::AudioProcessorEditor* createProcessorEditor(juce::AudioProcessor& processorRef){
        if(auto* ui = processorRef.createEditorIfNeeded()){
            return ui;//if this is called, none of the rest of this function will be executed
        }
        //if that failed, make a generic one
        return new juce::GenericAudioProcessorEditor(processorRef);
        //if that failed, just give up and pour some whisky
        return {};
    }
};

#endif