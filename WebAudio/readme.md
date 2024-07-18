# Web Audio using the 21M.080 Web Audio Framework

The 21M.080 Web Audio Framework is a live coding environment for working with audio in web browsers. This folder contains four files:
* SimpleSynth.txt: Gives a simple example of how to create a synthesizer
* Rumble.txt: A more complex synthesizer, which also includes a basic sequencer
* drum_sampler_midi.txt: An example of a drum sampler controller by our MIDI controller
* polyphonic_synth_midi.txt: An example using our MIDI controller to trigger four different pitch sequences

## Working with the framework

The website includes a codebox on the left hand side. You can execute code in three ways:
* Hitting the 'run' button will execute the whole codebox, from top to bottom.
* Putting your mouse cursor on a line and hitting option-return will execute a single line of code
* Putting your mouse cursor on a line and hitting shift-option-return will execute a block of code
  * Code blocks are separated by blank lines.
 
## Debugging
It can be really helpful to use the javascript console to see key information and any errors as you run code. 
* The javascript console can be found by going to the view menu, and then developer->javascript console. Or you can just type option-command-j

# Using a midi controller 
To use your midi controller with the framework: 
* copy and paste one of the files that say midi in the name into the codebox
* Hit 'run'
* Open the javascript console. If there are any errors try refreshing and running again
* With your MIDI controller plugged in, look for the midi inputs in the console. Your controller will show up as Wifiduino or something similar.
* Note the number associated with your controller in the list of inputs.
* In the codebox, there is a line `setMidiInput(2)`. Change the `2` to whatever number is associated with your controller and execute that line (option-return)
* Your controller should start triggering sounds!
