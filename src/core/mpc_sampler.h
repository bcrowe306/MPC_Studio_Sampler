#pragma once
#include "audio/choc_MIDI.h"
#include "core/constants.h"
#include "core/property.h"
#include <Security/Security.h>
#include <iostream>
#include <memory>
#include <string>
#include "audio_engine.h"
#include "core/browser.h"
#include "core/midi/midi_engine.h"
#include "timing.h"
#include "core/midi//note_repeat.h"
#include "core/midi/full_level.h"
#include <functional>
#include "core/metronome.h"
#include "core/sequencer/sequencer.h"
#include "core/playhead.h"
#include "core/track.h"
#include "core/command.h"
#include "core/nodes/tap_tempo_node.h"

using std::string;
using std::shared_ptr;
using std::make_shared;

// Forward declaration of Project class


// Main class for the Sampler/Sequencer
class MPCSampler : public enable_shared_from_this<MPCSampler> {
public:

    // Signals
    sigslot::signal<int> onTrackSelected;    // Signal emitted when a track is selected
    sigslot::signal<bool> onIsPlaying;       // Signal emitted when playback changes state
    sigslot::signal<bool> onIsRecording;     // Signal emitted when recording starts

    // Public members
    shared_ptr<TrackNode> masterTrack;       // Master track for the project
    shared_ptr<TrackNode> cueTrack;          // Cue track for the project
    shared_ptr<AudioContext> audioContext;   // Audio context for the project
    shared_ptr<MetronomeNode> metronomeNode; // Metronome node for the project
    shared_ptr<Playhead> playhead;           // Playhead for the project
    shared_ptr<UndoManager> undoManager;     // Undo manager for the project
    shared_ptr<TapTempoNode> tapTempoNode;   // Tap tempo node for the project
    shared_ptr<Sequencer> sequencer;         // Sequencer for the project
    MPCSampler() {
        initialize(); // Initialize the sampler when created
        
        
    };
    ~MPCSampler(){
        uninitialize(); // Clean up resources when the sampler is destroyed
    };
    shared_ptr<Browser> browser; // Browser for file management and navigation
    shared_ptr<MPCSBC::MidiEngine> midiEngine; // MIDI engine for handling MIDI input/output
    shared_ptr<Timer> timer; // MIDI clock for timing
    shared_ptr<AudioEngine> audioEngine; // Audio engine for playback and processing
    shared_ptr<NoteRepeat> noteRepeat; // Note repeat functionality
    shared_ptr<FullLevel> fullLevel; // Full level functionality for MIDI velocity

    void initialize(){

        // Initialize the MIDI engine
        midiEngine = make_shared<MPCSBC::MidiEngine>(true); 
       
        audioEngine = make_shared<AudioEngine>();
        audioEngine->activate(); 
        audioContext = audioEngine->context; // Get the audio context from the audio engine
        timer = make_shared<Timer>(audioEngine->context, audioEngine->context->sampleRate(), 120.0);
        audioEngine->context->connect(audioEngine->context->destinationNode(), timer->timerNode, 0, 0); 
        audioEngine->context->synchronizeConnections(); // Synchronize connections in the audio context
        browser = make_shared<Browser>(audioEngine->context); 
        
        // Initialize the note repeat functionality
        noteRepeat = make_shared<NoteRepeat>(timer); 
        fullLevel = make_shared<FullLevel>(); // Initialize the full level functionality
        midiEngine->onMidiInput.connect([this](const std::string &deviceName, choc::midi::ShortMessage msg, double timeStamp) {
                sendMidiInput(msg); // Forward MIDI input to the sampler
        });
        noteRepeat->sendMidi = std::bind(&MPCSampler::_sendMidiToProject, this, std::placeholders::_1);
        timer->onTick.connect(std::bind(&NoteRepeat::onTick, noteRepeat.get(), std::placeholders::_1)); // Connect the timer's tick signal to the note repeat functionality

        masterTrack = std::make_shared<TrackNode>(audioContext);
        cueTrack = std::make_shared<TrackNode>(audioContext);
        playhead = std::make_shared<Playhead>(timer); // Initialize the playhead with the audio context and MIDI clock
        metronomeNode = std::make_shared<MetronomeNode>(audioContext, playhead);
        undoManager = std::make_shared<UndoManager>(audioContext); // Initialize the undo manager
        tapTempoNode = std::make_shared<TapTempoNode>(audioContext); // Initialize the tap tempo node
        sequencer = std::make_shared<Sequencer>(undoManager); // Initialize the sequencer

        playhead->onMetronomeTick.connect(std::bind(&MetronomeNode::onMetronomeTick, metronomeNode.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _connectSequencer(); // Connect the sequencer to the playhead

       
        audioContext->connect(audioContext->destinationNode(), masterTrack->output, 0, 0); 
        audioContext->connect(audioContext->destinationNode(), cueTrack->output, 0, 0); 
        audioContext->connect(cueTrack->input, metronomeNode->clickGainNode, 0, 0); // Connect metronome output to audio context destination

        audioContext->synchronizeConnections(); // Synchronize connections after setup
        _createBusses(); // Create audio busses for the project
        _createTracks(); // Create tracks for the project
    };


    void uninitialize(){};
    void loadProject(std::string projectFilePath){};
    void saveProject(std::string projectFilePath){};
    void sendMidiInput(ShortMessage &msg){
        // Forward MIDI input to the project
        fullLevel->processVelocity(msg); // Process MIDI velocity with full level functionality
        if (noteRepeat->enabled.get()) {
            noteRepeat->midiInput(msg); // Handle MIDI input for note repeat
        }
        else {
            _sendMidiToProject(msg);
        }
    };
    // Forwards MIDI input to the project's selected track. Could be better. Could use the midi engine's callback system and have the tracks armed for input.
    // This is a temporary solution to get MIDI input working from all devices.
    
     int getMaxTracks() const {
        return kMaxTracks; // Return the maximum number of tracks
    }

    int getMaxBusses() const {
        return kMaxBusses; // Return the maximum number of busses
    }

    const std::array<std::shared_ptr<Track>, 64> &getTracks() const {
        return _tracks;
    }

    shared_ptr<Track> selectTrack(int index) {
        if (index < 0 || index >= static_cast<int>(_tracks.size())) {
            std::cerr << "Invalid track index: " << index << std::endl;
            return nullptr;
        }
        if (_selectedTrackIndex == index) {
            return _tracks[_selectedTrackIndex]; // Return the already selected track
        }   
        _selectedTrackIndex = index;
        onTrackSelected(index); // Emit signal that a track has been selected
        return _tracks[_selectedTrackIndex];
    }

    shared_ptr<Track> selectedTrack() const {
        if (_selectedTrackIndex < 0 || _selectedTrackIndex >= static_cast<int>(_tracks.size())) {
            return nullptr; // No track selected
        }
        return _tracks[_selectedTrackIndex];
    }

    int selectedTrackIndex() const {
        return _selectedTrackIndex;
    }

    void play() {
        
        playhead->start(); // Start the playhead
        // End the current batch if recording is stopped
        if (undoManager->isBatching()) {
            undoManager->endBatch();
        }
        onIsPlaying(isPlaying()); // Emit signal that playback has started
    }

    bool isPlaying() const {
        return playhead->isPlaying(); // Return whether the playhead is currently playing
    }

    void stop() {
        if(playhead->isRecording()) {
            
            // End the current batch if recording is stopped
            if(undoManager->isBatching()){
                undoManager->endBatch(); 
            }
        }
        playhead->stop(); // Stop the playhead
        sendStopSignalToAllTracks();
        onIsPlaying(false); // Emit signal that playback has stopped
    }
    void sendStopSignalToAllTracks() {
        for (auto &track : _tracks) {
            if (track->getDevice()) {
                track->getDevice()->stopAllNotes(); // Stop all notes in the track
            }
        }
        playhead->stop(); // Stop the playhead
        onIsPlaying(false); // Emit signal that playback has stopped
    }

    void toggleRecord() {

        playhead->toggleRecording(); // Toggle recording state
        if (playhead->isRecording()) {
            undoManager->startBatch(); 
            onIsRecording(true); 
        } 
        else {
            // End the current batch if recording is stopped
            if (undoManager->isBatching()) {
                undoManager->endBatch();
            }
            onIsRecording(false);
        }
    }

    void togglePlay() {

        playhead->togglePlaying(); 
        if (playhead->isPlaying()) {
            onIsPlaying(true); 
        } 
        else {
            // End the current batch if recording is stopped
            if (undoManager->isBatching()) {
                undoManager->endBatch();
            }
            sendStopSignalToAllTracks(); // Stop all tracks
            onIsPlaying(false);
        }
    }

    void tapTempo() {
        tapTempoNode->tap(); // Call the tap method on the tap tempo node
    }
private:
    int _selectedTrackIndex = -1; // Index of the currently selected track, -1 if none selected
    std::array<std::shared_ptr<Track>, kMaxTracks> _tracks; // List of tracks in the project
    std::array<std::shared_ptr<Track>, kMaxBusses> _busses; // List of busses in the project

    void _sendMidiToProject(ShortMessage &msg){
        auto selectedTrack = this->selectedTrack();
        if (selectedTrack) {
            selectedTrack->midiInput(msg); // Forward MIDI input to the selected track
        } else {
            std::cerr << "No track selected for MIDI input." << std::endl;
        }
    }
    void _onSequencerMidiOutput(int trackIndex, ShortMessage &msg) {
        // Forward MIDI output from the sequencer to the project
        if (trackIndex < 0 || trackIndex >= static_cast<int>(_tracks.size())) {
            std::cerr << "Invalid track index: " << trackIndex << std::endl;
            return;
        }
        _tracks[trackIndex]->midiPlayback(msg); // Emit MIDI output signal for the selected track
    }
    void _connectSequencer(){
        // Connect sequencer
        playhead->onTick.connect(std::bind(&Sequencer::onTick, sequencer.get(), std::placeholders::_1)); // Connect playhead ticks to sequencer ticks
        playhead->onPrecountTick.connect(std::bind(&Sequencer::onPrecountTick, sequencer.get(), std::placeholders::_1, std::placeholders::_2)); // Connect precount ticks to sequencer
        playhead->onPlayheadStateChanged.connect(std::bind(&Sequencer::onPlayheadStateChanged, sequencer.get(), std::placeholders::_1)); // Connect playhead state changes to sequencer
        sequencer->onMidiOutput.connect(std::bind(&MPCSampler::_onSequencerMidiOutput, this, std::placeholders::_1, std::placeholders::_2)); // Connect sequencer MIDI output to project

    }

    void _createTracks() {

        // create all 64 tracks
        for (int i = 0; i < kMaxTracks; ++i) {
            // Create a new track
            auto track = std::make_shared<Track>(audioContext, undoManager);

            // Set the track index
            track->setTrackIndex(i);
            (*track->name.get()) = _createNewTrackName();

            // Forward MIDI input to the sequencer
            track->midiOutput.connect(
                [this](int trackIndex, choc::midi::ShortMessage &msg) {
                  sequencer->onMidiInput(trackIndex, msg);
                });

            // Connect the track output to the master track input
            audioContext->connect(
                masterTrack->input, track->getOutput(), 0,
                0); // Connect track output to audio context destination
            _tracks[i] = track; // Add the track to the list
        }
        audioContext->synchronizeConnections(); // Synchronize connections
        selectTrack(0); // Select the first track by default
    }

    void _createBusses() {
        for (int i = 0; i < kMaxBusses; ++i) {
            auto bus = std::make_shared<Track>(
                audioContext, undoManager); // Create a new bus track
            (*bus->name.get()) = "Bus " + std::to_string(i + 1); // Set bus name
            audioContext->connect(
                masterTrack->input, bus->getOutput(), 0,
                0);           // Connect bus output to audio context destination
            _busses[i] = bus; // Add the bus to the list
        }
    }
    int _getNextTrackIndex() {
        // Find the next available track index
        return static_cast<int>(_tracks.size());
    }

    string _createNewTrackName() {
        // Generate a new track name based on the next available index
        int nextIndex = _getNextTrackIndex();
        return "Track " +
               std::to_string(nextIndex + 1); // Track names start from 1
    }
};
