* TODO
    * Implement Input Quantize
    * Figure out undo functionality with recording
    * Try out the ModesComponent so pads can have different modes
    * display needs feedback about precount, song position selected sequence, etc..
    * fix volume meter to display dB correctly
    * Waveform generation: Implement zooming and windowing
    * File Loading: Think about loading files on another thread to not block ui thread
    * Threading:  need to think about thread pools to accomplish tasks that can't block.
    * Look into routing system for more complex UI options
    * Finish building pages
        * Device Page
        * Sequence Page
        * Mixer Page
        * Browser Page
        * Settings Page
        * Perform Page
    * Finish Building basic widgets 
        * HScrollbar Widget
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
        * Build Components
            * Session Component: Each pad selects and plays a track
            * Notes/Performance Component: Pads are split across keyboard, or sliced sample. This is where chords, scaled, and progressions are playable
            * Mixer Component: Pads help to perform mixing
            * Transport Component
            * ScreenControls Components: These are the components that are synced to the screen page, qlinks, function buttons, scroll qlink, data and navigation all follow the screen
                * devicePage
                * sequencerPage
                * ...
    * Build modes to bring in and out active components

* Future:
    * Timing - Midi tempo BPM, make sure timing is correct
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
        * Limiter