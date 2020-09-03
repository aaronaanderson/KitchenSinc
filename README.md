# KitchenSinc

## Everything *including* the kitch sink

This project will serve as a container full of random things made with JUCE. Anything can be added to the project. Nothing should be removed. It should become a monstrosity.

## Building

I have set this to work with cmake/make on Ubuntu, both cmake and make *should* work on mac too. It uses Juce 6 and pedal, both are submodules.

To clone the kitchen sinc repository, open terminal:

```terminal
git clone --recursive -j4 https://github.com/aaronaanderson/KitchenSinc.git
```

The --recursive flag tells git to grab pedal and juce from their respective repositories at the same time. If you cloned and forgot the --recursive flag, you can grab the repositories updating the submodules. 

```terminal
cd KitchenSinc
git submodule update --init --recursive
```

Now you should have all the pieces to the puzzle. Make a build folder to store all the garbage cmake spews out 

```terminal
mkdir build
cd build
```

Now you can run cmake. Cmake should default to make as a generator....I think this applies to mac as well...

```terminal
cmake ..
```

If this suceeds, you should have a make file. Run it like this

```terminal
make
```

Pedal and JUCE should build. If you want this to happen faster, you should have typed

```terminal
make -j4
```

replacing 4 with how ever many threads your CPU can execute simultaneously (if you don't know guess low)

If by some mircale you have made it this far you should be able to execute the program.

```terminal
./KitchSinc_artefacts/Kitchen-Sinc
```

This should provide you a window with a sine wave. A humble beginning.

## Contributing

### Steps to add a new AudioProcessor

#### Create the documents
Create two new documents in the project called MyNewProcessor.hpp, and MyNewProcessor.cpp. (I personally insist on '.hpp' instead of '.h' b/c it's more specific, but it's worth noting that it doesn't matter as long as you match the name wherever referenced.) These documents should be placed in the 'source/customProcessors' folder.

Then add the new .cpp file to the CMakeLists.txt file as a compile target.

```cmake
target_sources(KitchSinc PRIVATE
    source/Main.cpp
    source/MainComponent.cpp
    source/customProcessors/AudioSettingsComponent.cpp
    source/customProcessors/ToneGenerator.cpp
    source/customProcessors/MyNewProcessor.cpp #<-------------------------
)
```
Note that the .hpp doesn't need to be there; it is #included (or, should be) at the top of your new .cpp

You should be able to write any C++ you'd like in the file, and have access to JUCE and Pedal includes within the new files. 

#### Adding New Processor To Graph

The project uses juce::AudioProcessorGraph for input/output connections. To test your audio processor, add it as a 'node' to AudioProcessorGraph:

```cpp
auto myNewProcessorNodeReference = audioGraph->addNode(std::unique_ptr<MyNewProcessor>());
```
The graph is aware of the sampling rate, buffersize, and number input and output channels. You can use this to set the settings of your processor;

```cpp
myNewProcessorNodeReference->getProcessor()->setPlayConfigDetails(audioGraph->getMainBusNumInputChannels(),//0 if no input
                                                                  audioGraph->getMainBusNumOutputChannels(), 
                                                                  audioGraph->getSamplingRate(), 
                                                                  audioGraph->getBufferSize())
```
The processor is ready to be connected. AudioProcessorGraph uses classes called 'Connection' to do this. 
When you try to connect, the function will report back with true if it worked and false if it did not. 
Use this information as a sanity check.

```cpp
bool successfulConnection = false;
//if adding audio input from device
successfulConnection = audioGraph->addConnection({{audioInputNode->nodeID, 0}, 
                                                  {myNewProcessorNodeReference->nodeID, 0}});
if(!successfulConnection) std::cout << "connection failed" << std::endl;//if adding the connection returned false
//if adding a second channel of input
successfulConnection = audioGraph->addConnection({{audioInputNode->nodeID, 1}, 
                                                  {myNewProcessorNodeReference->nodeID, 1});
if(!successfulConnection) std::cout << "connection failed" << std::endl;//if adding the connection returned false

//connect outputs of new processor to DAC
//if adding audio input from device
successfulConnection = audioGraph->addConnection({{myNewProcessorNodeReference->nodeID, 0}, 
                                                  {audioOutputNode->nodeID, 0}});
if(!successfulConnection) std::cout << "connection failed" << std::endl;//if adding the connection returned false
//if adding a second channel of input
successfulConnection = audioGraph->addConnection({{myNewProcessorNodeReference->nodeID, 1}, 
                                                  {audioOutputNode->nodeID, 1});
if(!successfulConnection) std::cout << "connection failed" << std::endl;//if adding the connection returned false
```

If these connections are successful, you should be getting a buffer full of input audio when your processBlock() is 
called, and you should hear whatever you put/leave inside that buffer by the end of your processBlock() override.

## Pushing new code to repository

I have blocked direct pushes to the master branch on the repository (I think). This will prevent any one of us from pushing
and breaking the repo by accident. 

To add new code to the repository, you must add a new branch. Fortunately this isn't too much to remember, and is worth knowing
how to do. 

Before making a new branch, commit everything you have so far (I'm not picky about these things)

```terminal
git commit -a -m "Commit for New Branch"
```
You can select, or 'checkout', a branch in the repository by using git checkout. If you want to checkout a branch that doesn't 
exist yet, you can create the branch by adding a -b flag to the call like so

```
git checkout -b "MyNewBranch"
```
This will create a branch called 'MyNewBranch', and switch your repo to this branch. I have found that it is worth getting 
comfortable with this as it wil encourage you to make new branches when trying new things.

To make your nice new code available to the rest of us, push it to the repository
Having checked out your new branch:

```terminal
git push origin MyNewBranch
```

This should put your code online, and your branch in the 'branch' dropdown menu. The next step is to merge the branch with 
the master branch after verifying that doing so will not break anything. Any of us can check and merge the branch except for
the person who pushed it. This garuntees that at least two eyes have seen the code that is pushed to main.

This may seem like a lot of unnecessary steps, but seems to be the way larger teams handle this sort of thing. The nice thing about
the current structure is that most new code will be in new files, so there will be minimal headache dealing with merge conflicts.