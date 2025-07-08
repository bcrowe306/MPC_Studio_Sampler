
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
        * Have the basics
        * Implement zooming and windowing
    * File Loading
        * Think about loading files on another thread to not block ui thread
    * Threading / thread pools
        * need to think about thread pools to accomplish tasks that can't block.

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
    


* State object requirements
    * Serializable
    * Undo / Redo
    * Listenable / Observable
    * Hierarchical 
* State Object Implementations
    * The state will have child values of various types, int, bool, float, string, vector
    * Each type must implement undo and redo
    * Each value type must emit signal when the state changes
    * Each value type must be serializable
* Example Implementation sudo code
```python
class UndoManager:
    pass

class StringValue:
    
    def __init__(self, undoManager: UndoManager, value:str = ""):
        self._undoManager:UndoManager = undoManager
        self._listeners = []
        self._value:str = ""
        self.setValue(value)

    def setValue(self, value):
        self._value = value
        self.onValueChanged(_value)

    def onValueChanged(self, value):
        for listener in self._listeners:
            listener(value)

    def undo(self, previosValue, newValue):
        self.setValue
    

class StateObject:
    name: StringValue = StringValue("")     
```

