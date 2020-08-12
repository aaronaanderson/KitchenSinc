# KitchenSinc

## Everything *including* the kitch sink

This project will serve as a container full of random things made with JUCE. Anything can be added to the project. Nothing should be removed. It should become a monstrosity.

## Building

I have set this to work with cmake/make on Ubuntu, both cmake and make *should* work on mac too. It uses Juce 6 and pedal, both are submodules.

To clone the kitchen sinc repository, open terminal:

```terminal
git clone https://github.com/aaronaanderson/KitchenSinc.git
```

Now you should have a folder called KitchenSinc; change directories to it

```terminal
cd KitchenSinc
```

Now grab juce and pedal by running this line (Only links are stored in this repo, these point to the original repo for pedal and JUCE)

```terminal
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
