#pragma once
#include "audio/choc_MIDI.h"
#include "core/command.h"
#include "core/constants.h"
#include "core/sequencer/midi_event.h"
#include "midi_clip.h"
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include <atomic>

using std::atomic;
using std::shared_ptr;
using std::memory_order_relaxed;

    enum class LQ_VALUE {
      BEAT,
      BAR,
      BAR_2,
      BAR_4,
      SEQUENCE_END
    };

struct ActiveNote {
    int tick;
    choc::midi::ShortMessage noteOn;
    int trackIndex;
};

class Sequence : public IReceiver, public enable_shared_from_this<Sequence> {
    
public:

    class AddNoteCommand : public Command {
    public:
        AddNoteCommand(shared_ptr<IReceiver> receiver, any midiEvent)
            : Command(receiver, "AddNote", midiEvent, midiEvent) {}
        void execute() override {
            // Custom execution logic for adding a note
            auto midiEvent = any_cast<MidiEvent>(newValue_);
            dynamic_cast<Sequence*>(receiver_.get())->addMidiEvent(midiEvent);
        }
        void undo() override {
            // Custom undo logic for removing a note
            auto midiEvent = any_cast<MidiEvent>(currentValue_);
            dynamic_cast<Sequence*>(receiver_.get())->removeMidiEvent(midiEvent.trackIndex, midiEvent.id);
        }
    };


    enum class SequenceState {
        Stopped,
        Triggered,
        Playing,
        Recording,
        Stopping,
        Precount
    };
    sigslot::signal<> onSequenceChanged; // Signal emitted when the sequence changes
    sigslot::signal<int, ShortMessage &> midiOut; // Signal for MIDI output from the sequence
    bool inputQuantize = true; // Flag for input quantization
    shared_ptr<UndoManager> undoManager_; // Undo manager for handling commands
    QUANTIZATION_VALUE quantizationValue = QUANTIZATION_VALUE::SIXTEENTH; // Default quantization value
    
    int id; // Unique identifier for the sequence
    vector<MidiClip> clips; // List of MIDI clips in the sequence
    

    Sequence(std::shared_ptr<UndoManager> undoManager, int id) : id(id), undoManager_(undoManager) {

        // create clips for 64 tracks
        clips.reserve(64); // Reserve space for 64 clips
        for(int i = 0; i < 64; i++) {
            clips.emplace_back(_clipId++); // Initialize clips with unique IDs
        }
        setLQValue(_lqValue); // Set the launch quantization value
    }

    // TODO: Undo/Redo support for sequence changes
    void setLQValue(LQ_VALUE value) {
        _lqValue = value; // Set the launch quantization value
        switch (_lqValue) {
            case LQ_VALUE::BEAT:
                _lqTicks = kTPQN; // Set ticks for beat quantization
                break;
            case LQ_VALUE::BAR:
                _lqTicks = kTPQN * 4; // Set ticks for bar quantization
                break;
            case LQ_VALUE::BAR_2:
                _lqTicks = kTPQN * 8; // Set ticks for 2 bars quantization
                break;
            case LQ_VALUE::BAR_4:
                _lqTicks = kTPQN * 16; // Set ticks for 4 bars quantization
                break;
            case LQ_VALUE::SEQUENCE_END:
                _lqTicks = -1; // No quantization for sequence end
                break;
        
        }
    }
    // TODO: Undo/Redo support for sequence changes
    void setLaunchQuantization(bool enabled) {
        _lq = enabled; // Set the launch quantization flag
    }

    // TODO: Undo/Redo support for sequence changes
    void setEndInTicks(int newLengthInTicks) {
        newLengthInTicks = std::max(newLengthInTicks, kMinSequenceLengthInTicks); // Ensure the new length is at least 1920 ticks
        _endTick.store(newLengthInTicks, std::memory_order_relaxed);
        _songPosition.lengthInTicks = newLengthInTicks; // Update the song position length in ticks
        onSequenceChanged(); // Emit signal when the length in ticks changes
    }
    int getSongPositionTick() {
        return _songPosition.tick.load(memory_order_relaxed); // Return the current tick of the song position
    }

    // TODO: Undo/Redo support for sequence changes
    void setEndInBeatTime(float beatTime){
        setEndInTicks(beatTimeToTicks(beatTime));
    }

    // TODO: Undo/Redo support for sequence changes
    void setEndInBars(int bars) {
        setEndInTicks(barsToTicks(bars)); // Convert bars to ticks and set the length
    }

    float getEndInBeatTime() const {
        return ticksToBeatTime(_endTick.load(memory_order_relaxed)); // Convert the length in ticks to beat time
    }

    int getEndInTicks() const {
        return _endTick.load(memory_order_relaxed); // Return the length in ticks
    }

    int getEndInBars() const {
        return ticksToBars(_endTick.load(memory_order_relaxed)); // Convert the length in ticks to bars
    }

    int getLengthInTicks() const {
        return _endTick.load(memory_order_relaxed) - _startTick.load(memory_order_relaxed); // Calculate the length in ticks from start to end
    }

    int getLengthInBars() const {
        return ticksToBars(getLengthInTicks()); // Convert the length in ticks to bars
    }

    int getStartInBars() const {
        return ticksToBars(_startTick.load(memory_order_relaxed)); // Convert the start tick to bars
    }

    // TODO: Undo/Redo support for sequence changes
    void clearSequenceEvents(){
        for (auto &clip : clips) {
            clip.clear(); // Clear all clips in the sequence
        }
        onSequenceChanged(); // Emit signal when the sequence is cleared
    }

    // TODO: Undo/Redo support for sequence changes
    void clearTrackEvents(int trackIndex) {
        if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
            return; // Return if the track index is out of bounds
        }
        clips[trackIndex].clear(); // Clear events for the specified track
        onSequenceChanged(); // Emit signal when the sequence is changed
    }

    // TODO: Undo/Redo support for sequence changes
    void setStartTick(int startTick) {
        // Cannot set start tick to a value greater than the end of the sequence
        if (startTick < 0 || startTick >= _endTick.load(memory_order_relaxed)) {
            return;
        }
        _startTick.store(startTick, memory_order_relaxed); // Set the start tick for the sequence
        onSequenceChanged(); // Emit signal when the start tick changes
    }

    
    // TODO: Need access to time signature numerator to properly calculate start tick in bars
    void doubleSequence() {
        for(auto &clip : clips) {
            clip.doubleClipLengthAndEvents(getLengthInTicks());
        }
        setEndInTicks(_endTick.load(memory_order_relaxed) + getLengthInTicks());
    }

    // TODO: Undo/Redo support for sequence changes
    void incrementStartTickBeat() {
        setStartTick(_startTick.load(memory_order_relaxed) + kTPQN); // Increment the start tick by one beat
    }

    // TODO: Undo/Redo support for sequence changes
    void decrementStartTickBeat() {
        setStartTick(_startTick.load(memory_order_relaxed) - kTPQN); // Decrement the start tick by one beat
    }

    // TODO: Undo/Redo support for sequence changes
    void incrementStartTickBar() {
        setStartTick(_startTick.load(memory_order_relaxed) + kTPQN * 4); // Increment the start tick by one bar
    }

    // TODO: Undo/Redo support for sequence changes
    void decrementStartTickBar() {
        setStartTick(_startTick.load(memory_order_relaxed) - kTPQN * 4); // Decrement the start tick by one bar
    }

    // TODO: Undo/Redo support for sequence changes
    void incrementEndTickBeat() {
        setEndInTicks(_endTick.load(memory_order_relaxed) + kTPQN); // Increment the end tick by one beat
    }

    // TODO: Undo/Redo support for sequence changes
    void decrementEndTickBeat() {
        setEndInTicks(_endTick.load(memory_order_relaxed) - kTPQN); // Decrement the end tick by one beat
    }

    // TODO: Undo/Redo support for sequence changes
    void incrementEndTickBar() {
        std::cout << "Incrementing end tick by one bar" << std::endl;
        setEndInTicks(_endTick.load(memory_order_relaxed) + kTPQN * 4); // Increment the end tick by one bar
    }

    // TODO: Undo/Redo support for sequence changes
    void decrementEndTickBar() {
        setEndInTicks(_endTick.load(memory_order_relaxed) - kTPQN * 4); // Decrement the end tick by one bar
    }

    void doAction(any value, bool undo = false) override {

    }

    void resetToStart() {
        _songPosition.tick.store(0, memory_order_relaxed); // Reset the tick counter to start
        _songPosition.updateFromTick(4, 4); // Update song position
        _songPosition.lengthInTicks = _endTick.load(memory_order_relaxed); // Set song position length in ticks
        onSequenceChanged(); // Emit signal when the sequence is reset
    }


    bool isEmpty () const {
        bool empty = true; // Initialize empty flag
        for (const auto &clip : clips) {
            if (!clip.events.empty()) {
                empty = false; // Found a non-empty clip
                break;
            }
        }
        return empty; // Return whether the sequence is empty
    }
    MidiClip* getClip(int trackIndex) {
        if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
            return nullptr; // Return nullptr if the track index is out of bounds
        }
        return &clips[trackIndex]; // Return the clip for the specified track index
    }

    void setNextState(SequenceState newState) {
        if(newState == SequenceState::Precount) {
            _state = newState; 
            return; // Set the state to precount immediately
        } 

        if(_lq){
            _pendingState = newState; // Store the pending state for launch quantization
        }
        else {
            _state = newState; // Set the state immediately if launch quantization is not enabled
        }
    }

    // Called from midi thread to handle MIDI input
    void onMidiInput(int trackIndex, ShortMessage msg) {
        switch(_state) {
            case SequenceState::Stopped:
                // Do nothing
                break;
            case SequenceState::Triggered:
                // Handle triggered state
                
                break;
            case SequenceState::Playing:
                break;

            case SequenceState::Recording:
                _recordEvent(trackIndex, msg); // Record MIDI input in recording state
                break;
            case SequenceState::Stopping:
                // Handle stopping state
                break;
            case SequenceState::Precount:
                _recordPrecountEvent(trackIndex, msg); // Record MIDI input in precount state
        }
    }

    // Called from the playhead to process the sequence on each tick which is on the audio thread
    void onTick(int tick){
        // Determine launch event
        if (_lqTicks > 0) {

                // Trigger launch event
                if (tick % _lqTicks == 0) {
                  gotoNextState(); // Switch to the pending state if launch
                                   // quantization is enabled
                }
        }
        // Sequence End launch event
        else if (_lqTicks >= 0) {
                    // TODO: Testing tick length switch stat
                if (_songPosition.tick.load(memory_order_relaxed) == _endTick.load(memory_order_relaxed) ) {
                    gotoNextState(); // Switch to the pending state if sequence end is reached
                }
        }   
        switch(_state) {
            case SequenceState::Stopped:
                stoppedState();
                break;
            case SequenceState::Triggered:
                // Handle triggered state
                break;
            case SequenceState::Playing:
                playingState(); // Process the sequence on each tick
                break;
            case SequenceState::Recording:
                // Handle recording state
                recordingState(); // Record the current state
                break;
            case SequenceState::Stopping:
                // Handle stopping state
                break;

            case SequenceState::Precount:
                // Handle precount state
                break;
        }
    }

    void stoppedState() {
        // Handle stopped state
        _songPosition.tick.store(_startTick.load(memory_order_relaxed), memory_order_relaxed); // Reset the song position tick
        _songPosition.updateFromTick(4, 4); // Update the song position
        _songPosition.lengthInTicks = _endTick.load(memory_order_relaxed); // Set the length in ticks for the song position
    }

    string getSongPositionDisplay() const {
        return _songPosition.getSongPositionDisplay(); // Get the song position display string
    }

    float getSongPositionProgress() const {
        if (_endTick.load(memory_order_relaxed) <= 0) {
            return 0.0f; // Avoid division by zero
        }
        float progress = static_cast<float>(_songPosition.tick.load(memory_order_relaxed)) / _endTick.load(memory_order_relaxed);
        return progress; // Calculate the song position progress
    }

    void gotoNextState() {
        _state = _pendingState; // Switch to the pending state
    }

    void onPrecountTick(int precountTick, int precountTickLength) {
        // Handle precount tick
        _precountTicks.store(precountTick, memory_order_relaxed); // Update the song position tick
        _precountLengthInTicks.store(precountTickLength, memory_order_relaxed); // Set the length in ticks for the song position

    }
    

    void fireEvent(int trackIndex, ShortMessage &msg) {
        midiOut(trackIndex, msg); // Emit MIDI output signal
    }
    
    void recordingState(){

        playbackSequence(); // Play the sequence on each tick

        _songPosition.updateFromTick(4, 4);
        _songPosition.lengthInTicks = _endTick.load(memory_order_relaxed);

        if(isTickSixteenth(_songPosition.tick.load(memory_order_relaxed))) {
                
        }
        
        // Emit signal for playhead position changes
        _incrementTick();
    }

    void playingState(){
        playbackSequence(); // Play the sequence on each tick
        _songPosition.updateFromTick( 4, 4);
        _songPosition.lengthInTicks = _endTick.load(memory_order_relaxed);

        if (isTickSixteenth(_songPosition.tick.load(memory_order_relaxed))) {
        }

        // Emit signal for playhead position changes
        _incrementTick();
    }

    void playbackSequence() {
        // Process the sequence on each tick
        for(int trackIndex = 0; trackIndex < static_cast<int>(clips.size()); ++trackIndex) {
            auto &clip = clips[trackIndex];
            if (!clip.enabled) {
                continue; // Skip disabled clips
            }

            // TODO: Use iterator to avoid constantly looping through events on each tick
            for (auto &event : clip.events) {
                if (event.startTick == _songPosition.tick.load(memory_order_relaxed)) {
                    auto msg = event.generateMidiData(true); // Generate MIDI data for the start event
                    fireEvent(trackIndex, msg); // Fire the start event
                }
                if (event.type == MidiEvent::EventType::Note && event.startTick + event.duration == _songPosition.tick.load(memory_order_relaxed)) {
                    auto msg = event.generateMidiData(false);
                    fireEvent(trackIndex, msg); // Fire the end event
                }
            }
        }
    }

    void addMidiEvent(MidiEvent midiEvent) {
        auto trackIndex = midiEvent.trackIndex; // Get the track index from the MIDI event

        if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
            return; // Invalid track index
        }
        auto &clip = clips[trackIndex];
        if (!clip.enabled) {
            return; // Clip is disabled
        }
        clip.addEvent(midiEvent); // Add the MIDI event to the clip
        onSequenceChanged(); // Emit signal when the sequence changes
    }

    void removeMidiEvent(int trackIndex, int eventId) {
        if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
            return; // Invalid track index
        }
        auto &clip = clips[trackIndex];
        if (!clip.enabled) {
            return; // Clip is disabled
        }
        clip.removeEvent(eventId); // Remove the MIDI event from the clip
        onSequenceChanged(); // Emit signal when the sequence changes
    }


protected:

    SongPosition _songPosition;
    atomic<int> _precountTicks = 0; // Ticks during precount
    atomic<int> _precountLengthInTicks = 0; // Length of the precount in ticks
    atomic<int> _startTick = 0; // Start tick for the sequence
    atomic<int> _endTick = kDefaultSequenceLengthInTicks; // Length of the sequence in ticks
    int _clipId = 0; // Unique ID for clips in this sequence
    SequenceState _state = SequenceState::Stopped; // Current state of the sequence
    std::unordered_map<int, ActiveNote> activeNotes;
    SequenceState _pendingState;
    LQ_VALUE _lqValue = LQ_VALUE::BAR; // Default launch quantization value
    int _lqTicks = 0; // Ticks for launch quantization
    bool _lq = true; // Flag for launch quantization


    void _incrementTick() {
        if (_songPosition.tick.load(memory_order_relaxed) == _endTick.load(memory_order_relaxed) - 1) {
            _songPosition.tick.store(_startTick.load(memory_order_relaxed), memory_order_relaxed);
        }
        else if (_songPosition.tick.load(memory_order_relaxed) > _endTick.load(memory_order_relaxed) - 1) {
            _songPosition.tick.store(_songPosition.tick.load(memory_order_relaxed) % _endTick.load(memory_order_relaxed), memory_order_relaxed);
        } 
        else {
            _songPosition++; // Increment the tick counter
        }
    }

    // Function to handle recording events
    void _doRecordEvent(int tick, int trackIndex, MidiClip &clip, ShortMessage &msg) {

        if (msg.isNoteOn()) {
            activeNotes[msg.getNoteNumber()] = ActiveNote{tick, msg, trackIndex}; // Store active note with its track index
        } 

        else if (msg.isNoteOff()) {
            auto it = activeNotes.find(msg.getNoteNumber());
            if (it != activeNotes.end() && it->second.trackIndex == trackIndex) {
                MidiEvent noteEvent(clip.getNewEventId(), it->second.tick, _songPosition.tick.load(memory_order_relaxed), it->second.noteOn, msg);
                noteEvent.trackIndex = trackIndex; // Set the track index for the note event

                undoManager_->executeCommand(
                    make_shared<AddNoteCommand>(shared_from_this(), noteEvent), false); // Add command to undo manager

                onSequenceChanged();
                activeNotes.erase(it);
            }
        } 

        else {
            auto newMidiEvent = MidiEvent(
                clip.getNewEventId(),
                _songPosition.tick.load(memory_order_relaxed), msg);
            newMidiEvent.trackIndex = trackIndex;
            undoManager_->executeCommand( make_shared<AddNoteCommand>(shared_from_this(), newMidiEvent), false);
            onSequenceChanged(); 
        }
    }

    // Function to handle recording events during regular recording
    void _recordEvent(int trackIndex, ShortMessage &msg) {
            if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
                return; // Invalid track index
            }
            auto &clip = clips[trackIndex];
            if (!clip.enabled) {
                return; // Clip is disabled
            }
            _doRecordEvent(_calcEventTick(), trackIndex, clip, msg);
    }


    // Function to handle recording events during precount
    void _recordPrecountEvent(int trackIndex, ShortMessage &msg) {
        
        // Do not record if the tick is less than the precount length - 1/16th
        if(_precountTicks.load(memory_order_relaxed) < _precountLengthInTicks.load(memory_order_relaxed) - kTPQN / 4){
            return; 
        }
        std::cout << "Recording precount event at tick: " << _precountTicks.load(memory_order_relaxed) << std::endl; // Debug output --- IGNORE ---
        
        // Invalid track index
        if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
            return; 
        }
        
        // Clip is disabled
        auto &clip = clips[trackIndex];
        if (!clip.enabled) {
            return; 
        }
        _doRecordEvent(0, trackIndex, clip, msg);
           
    }

    int _calcEventTick(bool isEndTick = false){
        // Function encapsulates the logic to determine the tick of an event
        auto currentTick = _songPosition.tick.load(memory_order_relaxed); // Get the current tick from the song position

       
        if (currentTick < 0) {
            currentTick = 0; // Ensure the tick is not negative
        }
        if (inputQuantize && !isEndTick) {
            currentTick = quantizeTick(currentTick, quantizationValue); // Quantize the tick if input quantization is enabled
        }
        // Wrap the tick if it exceeds the sequence length
        if (currentTick >= _endTick.load(memory_order_relaxed)) {
            currentTick = 0; // Reset to the start of the sequence
        }
        return currentTick; // Return the calculated tick
    }
    
};