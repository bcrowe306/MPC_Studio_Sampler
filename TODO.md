* TODO
    * Must improve loading of samples and files in general
        * Aux thread to load samples from disk... Midi thread cannot freeze waiting on this.
        * Maybe come up with a system of pushing commands unto a queue to be picked up by main thread.
        * Need to tweak the loading of samples into a AudioBus. Right now weird artifacts are happening when I change samples.
    * SamplerDevice.
        * First iteration of polyphonic sampler is complete.
        * Tweak voice allocator to steal old samples even when sustain pedal is active.
        * Features still needed:
            * filter and filter env
            * detune and pitch env
            * oneshot sample mode
            * mono mode and legato
            * start and end samples
            * looping
    * Application
        * Time to make some progress on serialization
            * Yaml has been chosen
            * Each object needs to have a serialize and deserialize method
        * Need to auto save on time interval and on certain events like sample load, and sequence record.
    * Device:
        * Implement Choke Groups
    * Sequencing improvements
        * Implement Swing
        * Have sequence auto batch commands on each loop cycle
        * Implement Record/Overdub record mode. Currently overdub is the default, but needs to be toggled on and off.
        * Sequence Screen has 3 modes of editing
            * 1. Sequence Mode: Adjust start end of entire sequence - default
            * 2. Line: Make adjustments to each note in a line
            * 3. Note: Make adjustments to individual notes
        * Sequence More Actions
            * Transpose
            * 2x speed up
            * /2x slow down
            * Quantize
    * UI and page Routing
        * By default, the Data wheel, QLink, qlink scroll, directional pad, plus and minus buttons follow the screens page.
        * Finally implemented an actual router with push and pop functionality
        * Now we can have dialog screens as well as setting pages/ sub menus
        * Implement dialog screens
            * Noterepeat settings
            * Input quantize settings
            * LaunchQuantize settings
            * Metronome Settings

    * Try out the ModesComponent so pads can have different modes
    * Waveform generation: 
        * Implement zooming and windowing and possible different waveform view, currently a symetrical view is implemented
    * File Loading: 
        * Currently Midi System hangs on large file loads. this is because the file load is launched from the Midi thread... Not ideal
        * Think about loading files on another thread to not block ui thread.
    * Threading:  
        * Need to think about thread pools to accomplish tasks that can't block and need threads. Don't want to spin up threads at random
    * Finish building pages
        * Sequence Page
        * Mixer Page
        * Browser Page
        * Settings Page
        * Perform Page
    * Finish Building basic widgets 
        * Grid Widget
        * Radiobox Widget
        * VSlider Widget
        * HSlider Widget
    * Mixer
        * Implement the use of the 8 busses. Test connecting and reconnecting tracks to busses.
        * Implement a solo controller for the tracks

    * Audio:
        * Audio Recording? Can we do it with Labsound? If not, a new engine needs to be programmed... SMH !!
        * Will need device selection if we want to record. You will want to select the device you are recording from.
    * ControlSurface
        * Implement more control events, like click, double click, long press, hold, etc
        * Build Components
            * Notes/Performance Component: Pads are split across keyboard, or sliced sample. This is where chords, scaled, and progressions are playable
            * Mixer Component: Pads help to perform mixing
            * Transport Component
            * ScreenControls Components: These are the components that are synced to the screen page, qlinks, function buttons, scroll qlink, data and navigation all follow the screen
                * devicePage
                * sequencerPage
                * ...
    * Build modes to bring in and out active components

* Future:
    * Timing - Midi tempo BPM, make sure timing is correct --- Getting close here
    * Arranger - Future feature
    * Devices - Figure out how I want to program devices. Leaning toward cmajor if possible
        * Sampler Device
        * Synth Device
        * AudioClip Device
    * Effects - Implement opensource GUI-less effects 
        * Reverb
        * Delay
        * Compressor
        * Gate
        * EQ
        * Filter
        * Saturation
        * Limiter0