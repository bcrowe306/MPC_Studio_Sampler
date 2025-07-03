
* Application Needs
    * Event System
    * Serialization
        * Every Parameter and setting must be serializable
        * Use Yaml for this. Works well enough
    * Command Pattern w Undo/Redo
    * Build out Project structure 

* MPC Sampler Core
    * Timing - Midi tempo BPM
    * Clips - Used in sequencing
    * Sequencer
    * Arranger
    * Devices
        * Sampler Device
        * Synth Device
        * AudioClip Device
    * Effects
        * Reverb
        * Delay
        * Compressor
        * Gate
        * EQ
        * Filter
        * Saturation
        * Limiter
    * Track
    * Busses
    * Metering
    * Waveform generation

* Display
    * Build basic widgets in Cairo
        * Selectable Textbox
        * Parameter Widget
        * Meter Widget
        * Slider Widget
        * Letterbox Widget
        * Button Widget
        * Parambox Widget
        * HScrollbar Widget
        * VScrollbar Widget
        * Radiobox Widget
        * VSlider Widget
        * HSlider Widget
    * Build Main Pages
        * Device Page
        * Sequence Page
        * Mixer Page
        * Browser Page
        * Settings Page
        * Perform Page
    * Build Routing system

* ControlSurface
    * Build out Component Class with Disconnectable slots
    * Build out controls
        * Button control
        * Slider control
        * Encoder conrtol
        * Pad Control
    * Build Components
        * Session Component
        * Notes/Performance Component
        * Mixer Component
        * Transport Component


