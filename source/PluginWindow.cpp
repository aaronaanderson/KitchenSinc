#include "PluginWindow.hpp"

//Since we inherit from DocumentWindow, we need to take care of its construction
//I don't want to look in to it right now so I'm copying AudioPluginHosts implem
PluginWindow::PluginWindow(juce::AudioProcessorGraph::Node::Ptr n, juce::OwnedArray<PluginWindow>& windowList) 
    : DocumentWindow (n->getProcessor()->getName(),
                      juce::LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::minimiseButton | DocumentWindow::closeButton),
      activeWindowListReference (windowList),//set reference to owner list
      node (n)//create local reference to node
{
    setSize(400, 300);

    //try to create ourself
    if(auto* ui = createProcessorEditor(*node->getProcessor())){
        setContentOwned(ui, true);
    }
    setTopLeftPosition(40, 40);
    node->properties.set(getOpenProp(), true);
    setVisible(true);
}
//We have stuff to do in the deconstructor, don't let it default
PluginWindow::~PluginWindow(){
    clearContentComponent();//Idk what this do, but juce did it so, here we are    
}

//We delete ourselves from the list of windows
void PluginWindow::closeButtonPressed(){
    //I don't know what this does yet
    node->properties.set(getOpenProp(), false);
    //remove ourselves from the owner's list
    activeWindowListReference.removeObject(this);
}

//static juce::AudioProcessorEditor* PluginWindow::createProcessorEditor(juce::AudioProcessor& processorRef)