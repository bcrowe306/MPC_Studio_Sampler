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
        Stopping
    };
    sigslot::signal<> onSequenceChanged; // Signal emitted when the sequence changes
    sigslot::signal<int, ShortMessage &> midiOut; // Signal for MIDI output from the sequence
    sigslot::signal<SongPosition> onPlayheadPositionChanged; // Signal for playhead position changes emitted every tick
    sigslot::signal<SongPosition> onSongPositionDisplayChanged; // Signal for song position display changes, emitted every sixteenth note
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

    void setLaunchQuantization(bool enabled) {
        _lq = enabled; // Set the launch quantization flag
    }
    void setLengthInTicks(int newLengthInTicks) {
        newLengthInTicks = std::max(newLengthInTicks, 480); // Ensure the new length is at least 480 ticks
        _length.store(newLengthInTicks, std::memory_order_relaxed);
        _songPosition.lengthInTicks = newLengthInTicks; // Update the song position length in ticks
        onSequenceChanged(); // Emit signal when the length in ticks changes
    }

    void setLengthInBeatTime(float beatTime){
        setLengthInTicks(beatTimeToTicks(beatTime));
    }

    void setLengthInBars(int bars) {
        setLengthInTicks(barsToTicks(bars)); // Convert bars to ticks and set the length
    }

    float getLengthInBeatTime() const {
        return ticksToBeatTime(_length.load(memory_order_relaxed)); // Convert the length in ticks to beat time
    }

    int getLengthInTicks() const {
        return _length.load(memory_order_relaxed); // Return the length in ticks
    }

    int getLengthInBars() const {
        return ticksToBars(_length.load(memory_order_relaxed)); // Convert the length in ticks to bars
    }

    void doAction(any value, bool undo = false) override {

    }

    void resetToStart() {
        _songPosition.tick = 0; // Reset the tick counter to start
        _songPosition.updateFromTick(4, 4); // Update song position
        _songPosition.lengthInTicks = _length.load(memory_order_relaxed); // Set song position length in ticks
        onSongPositionDisplayChanged(_songPosition); // Emit signal for song position display changes
        onPlayheadPositionChanged(_songPosition); // Emit signal for playhead position changes
        onSequenceChanged(); // Emit signal when the sequence is reset
    }

    void clear() {
        clips.clear(); // Clear all clips in the sequence
        onSequenceChanged(); // Emit signal when the sequence is cleared
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
    
    void setNextState(SequenceState newState) {
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
                if (_songPosition.tick == _length.load(memory_order_relaxed) - 1) {
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
        }
    }

    void stoppedState() {
        // Handle stopped state
        _songPosition.tick = 0; // Reset the song position tick
        _songPosition.updateFromTick(4, 4); // Update the song position
        _songPosition.lengthInTicks = _length.load(memory_order_relaxed); // Set the length in ticks for the song position
        onSongPositionDisplayChanged(_songPosition); // Emit signal for song position display changes
        onPlayheadPositionChanged(_songPosition); // Emit signal for playhead position changes
    }

    string getSongPositionDisplay() const {
        return _songPosition.getSongPositionDisplay(); // Get the song position display string
    }

    float getSongPositionProgress() const {
        if (_length.load(memory_order_relaxed) <= 0) {
            return 0.0f; // Avoid division by zero
        }
        return static_cast<float>(_songPosition.tick) / _length.load(memory_order_relaxed); // Calculate the song position progress
    }

    void gotoNextState() {
        _state = _pendingState; // Switch to the pending state
    }

    void onPrecountTick(int precountTick, int precountTickLength) {
        // Handle precount tick
        if (_state == SequenceState::Recording) {
            _precountTicks = precountTick; // Update the song position tick
            _precountLengthInTicks = precountTickLength; // Set the length in ticks for the song position
            
        }
    }
    

    void fireEvent(int trackIndex, ShortMessage &msg) {
        midiOut(trackIndex, msg); // Emit MIDI output signal
    }
    
    void recordingState(){

        playbackSequence(); // Play the sequence on each tick

        _songPosition.updateFromTick(4, 4);
        _songPosition.lengthInTicks = _length.load(memory_order_relaxed);

        if(isTickSixteenth(_songPosition.tick)) {
                onSongPositionDisplayChanged(_songPosition);
                
        }
        
        // Emit signal for playhead position changes
        onPlayheadPositionChanged(_songPosition); 
        _incrementTick();
    }

    void playingState(){
        playbackSequence(); // Play the sequence on each tick
        _songPosition.updateFromTick( 4, 4);
        _songPosition.lengthInTicks = _length.load(memory_order_relaxed);

        if (isTickSixteenth(_songPosition.tick)) {
                onSongPositionDisplayChanged(_songPosition);
        }

        // Emit signal for playhead position changes
        onPlayheadPositionChanged(_songPosition);
        _incrementTick();
    }

    void playbackSequence() {
        // Process the sequence on each tick
        for(int trackIndex = 0; trackIndex < static_cast<int>(clips.size()); ++trackIndex) {
            auto &clip = clips[trackIndex];
            if (!clip.enabled) {
                continue; // Skip disabled clips
            }
            for (auto &event : clip.events) {
                if (event.startTick == _songPosition.tick) {
                    fireEvent(trackIndex, event.startEvent); // Fire the start event
                }
                if (event.type == MidiEvent::EventType::Note && event.startTick + event.duration == _songPosition.tick) {
                    fireEvent(trackIndex, event.endEvent); // Fire the end event
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
    int _precountTicks = 0; // Ticks during precount
    int _precountLengthInTicks = 0; // Length of the precount in ticks
    atomic<int> _length = kDefaultSequenceLengthInTicks; // Length of the sequence in ticks
    int _clipId = 0; // Unique ID for clips in this sequence
    SequenceState _state = SequenceState::Stopped; // Current state of the sequence
    std::unordered_map<int, ActiveNote> activeNotes;
    SequenceState _pendingState;
    LQ_VALUE _lqValue = LQ_VALUE::BAR; // Default launch quantization value
    int _lqTicks = 0; // Ticks for launch quantization
    bool _lq = true; // Flag for launch quantization


    void _incrementTick() {
            if (_songPosition.tick == _length.load(memory_order_relaxed) - 1) {
                _songPosition.tick = 0;
            } else {
                _songPosition++; // Increment the tick counter
            }
    }

    void _recordEvent(int trackIndex, ShortMessage &msg) {
            if (trackIndex < 0 || trackIndex >= static_cast<int>(clips.size())) {
                return; // Invalid track index
            }
            auto &clip = clips[trackIndex];
            if (!clip.enabled) {
                return; // Clip is disabled
            }
            if (msg.isNoteOn()) {

                activeNotes[msg.getNoteNumber()] = ActiveNote{_calcEventTick(), msg, trackIndex}; // Store active note with its track index

            } 
            else if (msg.isNoteOff()) {
                auto it = activeNotes.find(msg.getNoteNumber());
                if (it != activeNotes.end() && it->second.trackIndex == trackIndex) {
                    MidiEvent noteEvent(clip.getNewEventId(), it->second.tick, _songPosition.tick, it->second.noteOn, msg);
                    noteEvent.trackIndex = trackIndex; // Set the track index for the note event

                    undoManager_->executeCommand(
                        make_shared<AddNoteCommand>(shared_from_this(), noteEvent), false); // Add command to undo manager

                    // Emit signal when the sequence changes
                    onSequenceChanged();
                    activeNotes.erase(it);
                }
            } 
            else {
                // addMidiEvent( MidiEvent(clip.getNewEventId(), _songPosition.tick, msg));
                undoManager_->executeCommand(
                    make_shared<AddNoteCommand>(shared_from_this(), MidiEvent(clip.getNewEventId(), _songPosition.tick, msg)), false);
            }
    }

    int _calcEventTick(bool isEndTick = false){
        // Function encapsulates the logic to determine the tick of an event
        auto currentTick = _songPosition.tick; // Get the current tick from the song position

       
        if (currentTick < 0) {
            currentTick = 0; // Ensure the tick is not negative
        }
        if (inputQuantize && !isEndTick) {
            currentTick = quantizeTick(currentTick, quantizationValue); // Quantize the tick if input quantization is enabled
        }
        // Wrap the tick if it exceeds the sequence length
        if (currentTick >= _length.load(memory_order_relaxed)) {
            currentTick = 0; // Reset to the start of the sequence
        }
        return currentTick; // Return the calculated tick
    }
    
};