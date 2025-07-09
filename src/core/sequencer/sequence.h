#pragma once
#include "audio/choc_MIDI.h"
#include "core/constants.h"
#include "midi_clip.h"
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include <iostream>


class Sequence {
    
public:
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
    int id; // Unique identifier for the sequence
    vector<MidiClip> clips; // List of MIDI clips in the sequence
    

    Sequence(int id) : id(id) {

        // create clips for 64 tracks
        clips.reserve(64); // Reserve space for 64 clips
        for(int i = 0; i < 64; i++) {
            clips.emplace_back(_clipId++); // Initialize clips with unique IDs
        }
    }
   

    void setLengthInTicks(int newLengthInTicks) {
        newLengthInTicks = std::max(newLengthInTicks, 480); // Ensure the new length is at least 480 ticks
        _length = newLengthInTicks;
        onSequenceChanged(); // Emit signal when the length in ticks changes
    }

    void setLengthInBeatTime(float beatTime){
        setLengthInTicks(beatTimeToTicks(beatTime));
    }

    void setLengthInBars(int bars) {
        setLengthInTicks(barsToTicks(bars)); // Convert bars to ticks and set the length
    }

    float getLengthInBeatTime() const {
        return ticksToBeatTime(_length); // Convert the length in ticks to beat time
    }

    int getLengthInTicks() const {
        return _length; // Return the length in ticks
    }

    int getLengthInBars() const {
        return ticksToBars(_length); // Convert the length in ticks to bars
    }

    void resetToStart(){
        _tickCounter = 0; // Reset the tick counter to start
        _songPosition.tick = 0; // Reset song position tick
        _songPosition.updateFromTick(_tickCounter, 4, 4); // Update song position
        _songPosition.lengthInTicks = _length; // Set song position length in ticks
        onSongPositionDisplayChanged(_songPosition); // Emit signal for song position display changes
        onPlayheadPositionChanged(_songPosition); // Emit signal for playhead position changes
        onSequenceChanged(); // Emit signal when the sequence is reset
    }

    void clear() {
        clips.clear(); // Clear all clips in the sequence
        onSequenceChanged(); // Emit signal when the sequence is cleared
    }
    void setNextState(SequenceState newState) {
        _state = newState; // Set the new state of the sequence
        
    }

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

    void onTick(){
        switch(_state) {
            case SequenceState::Stopped:
                // Handle stopped state
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
    

    void fireEvent(int trackIndex, ShortMessage &msg) {
        midiOut(trackIndex, msg); // Emit MIDI output signal
    }
    
    void recordingState(){

        playbackSequence(); // Play the sequence on each tick

        _songPosition.updateFromTick(_tickCounter, 4,4);
        _songPosition.lengthInTicks = _length;

        if(isTickSixteenth(_tickCounter)) {
                onSongPositionDisplayChanged(_songPosition);
                
        }
        
        // Emit signal for playhead position changes
        onPlayheadPositionChanged(_songPosition); 
        _incrementTick();
    }

    void playingState(){
        playbackSequence(); // Play the sequence on each tick
        _songPosition.tick = _tickCounter;
        _songPosition.updateFromTick(_tickCounter, 4, 4);
        _songPosition.lengthInTicks = _length;

        if (isTickSixteenth(_tickCounter)) {
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
                if (event.startTick == _tickCounter) {
                    fireEvent(trackIndex, event.startEvent); // Fire the start event
                }
                if (event.type == MidiEvent::EventType::Note && event.startTick + event.duration == _tickCounter) {
                    fireEvent(trackIndex, event.endEvent); // Fire the end event
                }
            }
        }
    }

    

protected:

    SongPosition _songPosition;
    int _length = kDefaultSequenceLengthInTicks; // Length of the sequence in ticks
    int _tickCounter = 0; // Counter for ticks in the sequence
    int _clipId = 0; // Unique ID for clips in this sequence
    SequenceState _state = SequenceState::Stopped; // Current state of the sequence
    std::unordered_map<int, std::pair<int, ShortMessage>> activeNotes;


    void _incrementTick() {
            if (_tickCounter == _length) {
                _tickCounter = 0;
            } else {
                _tickCounter++; // Increment the tick counter
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

                activeNotes[msg.getNoteNumber()] = std::make_pair(
                    _tickCounter, msg); // Store active note with its track index

            } else if (msg.isNoteOff()) {
                auto it = activeNotes.find(msg.getNoteNumber());
                if (it != activeNotes.end()) {
                    int startTick = it->second.first;
                    ShortMessage startMsg = it->second.second;
                    int endTick = _tickCounter;
                    MidiEvent noteEvent(startTick, endTick, startMsg, msg);
                    clip.addEvent(noteEvent);

                    // Emit signal when the sequence changes
                    onSequenceChanged();
                    activeNotes.erase(it);
                }
            } else {
                auto event = MidiEvent(_tickCounter, msg);
                clip.addEvent(event);
            }
    }
    
};