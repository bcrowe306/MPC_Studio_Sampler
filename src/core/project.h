#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/FunctionNode.h"
#include "core/sequencer/sequence.h"
#include "sigslot/signal.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include "core/timing.h"
#include "core/track.h"
#include "metronome.h"
#include "playhead.h"
#include "command.h"
#include "value_receiver.h"
#include <array>
#include <vector>
#include "core/nodes/tap_tempo_node.h"
#include "core/constants.h"
#include "sequencer/sequencer.h"


enum class DisplayPages {
    DevicePage, // Page for device-specific controls
    PerformPage, // Page for performance controls
    SequencePage, // Page for sequence editing
    ArrangerPage, // Page for arranging sequences and clips
    MixerPage, // Page for mixing and audio routing
    BrowserPage, // Page for browsing samples and instruments
    ProjectPage, // Page for project management
    SettingsPage, // Page for settings and configurations
 };

//  String representation of display pages
inline const std::vector<std::string> kDisplayPageNames = {
    "devicePage", "performPage", "sequencePage", "arrangerPage",
    "mixerPage",  "browserPage", "projectPage",  "settingsPage"};

class Project {
public:
    Project(std::shared_ptr<AudioContext> audioContext, std::shared_ptr<Timer> timer) {
        // Initialize the project with an empty track list
        this->audioContext = audioContext;
        this->timer = timer; // Set the MIDI clock for the project

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

        
        _createTracks(); // Create the initial tracks
        _createBusses(); // Create the initial busses

        _connectParams(); // Connect parameters to their respective signals
        projectName.setValue("Untitled Project"); // Set default project name
    };



    ~Project() = default;

    //  Listeners for project events
    sigslot::signal<> onProjectLoaded; // Signal emitted when the project is loaded
    sigslot::signal<> onProjectSaved; // Signal emitted when the project is saved
    sigslot::signal<int> onTrackSelected; // Signal emitted when a track is selected
    sigslot::signal<bool> onIsPlaying; // Signal emitted when playback changes state
    sigslot::signal<bool> onIsRecording; // Signal emitted when recording starts

    // Public members
    shared_ptr<TrackNode> masterTrack;
    shared_ptr<TrackNode> cueTrack; // Cue track for the project
    shared_ptr<AudioContext> audioContext; // Audio context for the project
    shared_ptr<Timer> timer; // MIDI clock for the project
    shared_ptr<MetronomeNode> metronomeNode; // Metronome node for the project
    shared_ptr<Playhead> playhead; // Playhead for the project
    shared_ptr<UndoManager> undoManager; // Undo manager for the project
    shared_ptr<TapTempoNode> tapTempoNode; // Tap tempo node for the project
    shared_ptr<Sequencer> sequencer; // Sequencer for the project

    // Project parameters
    VRString projectName = VRString("projectName", "Untitled Project", undoManager); // Parameter for project name
    ValueOptionsReceiver<string> displayPage = ValueOptionsReceiver<string>( "displayPage", kDisplayPageNames[0], kDisplayPageNames, kDisplayPageNames); // Parameter for the current display page
    VRBool metronomeEnabled = VRBool("metronomeEnabled", true, undoManager); // Parameter to enable/disable the metronome
    VRFloat metronomeVolumeDb = VRFloat("metronomeVolumeDb", -6.0f, -60.0f, 6.0f, 1.0, 0.01, undoManager); // Metronome volume in dB
    VRBool returnToZero = VRBool("returnToZero", true, undoManager); // Parameter to return to zero position when stopping playback
    VRFloat bpm = VRFloat("bpm", 120.0f, 30.0f, 300.0f, 1.0, 0.01, undoManager); // BPM parameter with range from 30 to 300
    VRInt timeSignatureNumerator = VRInt("timeSignatureNumerator", 4, 1, 16, 1, 1, undoManager); // Time signature numerator parameter
    VRBool inputQuantize = VRBool("inputQuantize", true, undoManager); // Input quantization parameter
    // IntOptionsParameter timeSignatureDenominator = IntOptionsParameter("timeSignatureDenominator", { 1, 2, 4, 8, 16 }, 2); // Time signature denominator parameter with options


    void serialize() {
        // Implement serialization logic if needed
    }

    void deserialize() {
        // Implement deserialization logic if needed
    }

    

    const std::vector<std::shared_ptr<Track>>& getTracks() const {
        return tracks;
    }

    int getMaxTracks() const {
        return kMaxTracks; // Return the maximum number of tracks
    }

    int getMaxBusses() const {
        return kMaxBusses; // Return the maximum number of busses
    }

    shared_ptr<Track> selectTrack(int index) {
        if (index < 0 || index >= static_cast<int>(tracks.size())) {
            std::cerr << "Invalid track index: " << index << std::endl;
            return nullptr;
        }
        if (_selectedTrackIndex == index) {
            return tracks[_selectedTrackIndex]; // Return the already selected track
        }   
        _selectedTrackIndex = index;
        onTrackSelected(index); // Emit signal that a track has been selected
        return tracks[_selectedTrackIndex];
    }

    shared_ptr<Track> selectedTrack() const {
        if (_selectedTrackIndex < 0 || _selectedTrackIndex >= static_cast<int>(tracks.size())) {
            return nullptr; // No track selected
        }
        return tracks[_selectedTrackIndex];
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
            onIsPlaying(false);
        }
    }

    void tapTempo() {
        tapTempoNode->tap(); // Call the tap method on the tap tempo node
    }
    

private:

    void _connectSequencer(){
        // Connect sequencer
        playhead->onTick.connect(std::bind(&Sequencer::onTick, sequencer.get(), std::placeholders::_1)); // Connect playhead ticks to sequencer ticks
        playhead->onPlayheadStateChanged.connect(std::bind(&Sequencer::onPlayheadStateChanged, sequencer.get(), std::placeholders::_1)); // Connect playhead state changes to sequencer
        sequencer->onMidiOutput.connect(std::bind(&Project::_onSequencerMidiOutput, this, std::placeholders::_1, std::placeholders::_2)); // Connect sequencer MIDI output to project

    }

    void _onSequencerMidiOutput(int trackIndex, ShortMessage &msg) {
        // Forward MIDI output from the sequencer to the project
        if (trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) {
            std::cerr << "Invalid track index: " << trackIndex << std::endl;
            return;
        }
        tracks[trackIndex]->midiPlayback(msg); // Emit MIDI output signal for the selected track
    }
    
    void _createTracks() {
        
        // Reserve space for 64 tracks
        tracks = std::vector<std::shared_ptr<Track>>();
        tracks.reserve(kMaxTracks); 

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
            tracks.push_back(track); // Add the track to the list
            }
            audioContext->synchronizeConnections(); // Synchronize connections
            selectTrack(0); // Select the first track by default
    }

    void _createBusses() {
        for(int i = 0; i < kMaxBusses; ++i) {
            auto bus = std::make_shared<Track>(audioContext, undoManager); // Create a new bus track
            (*bus->name.get()) = "Bus " + std::to_string(i + 1); // Set bus name
            audioContext->connect(
                masterTrack->input, bus->getOutput(), 0,
                0); // Connect bus output to audio context destination
            tracks.push_back(bus); // Add the bus to the list
        }
    }
    void _connectParams (){
        bpm.onValueChanged.connect([this](float value) {
            playhead->setBPM(value);
        });
        bpm.updateObservers(); // Ensure initial value is set

        metronomeEnabled.onValueChanged.connect([this](bool value) {
            metronomeNode->setEnabled(value);
        });
        metronomeEnabled.updateObservers(); // Ensure initial value is set

        metronomeVolumeDb.onValueChanged.connect([this](float value) {
            metronomeNode->setVolumeDb(value); // Set the metronome volume in dB
        });
        metronomeVolumeDb.updateObservers(); // Ensure initial value is set

        returnToZero.onValueChanged.connect([this](bool value) {
            playhead->setReturnToZero(value);
        });

        tapTempoNode->onTempoCalculated.connect([this](double bpm) {
            this->bpm.setValue(static_cast<float>(bpm)); // Update BPM when a new tempo is calculated
            this->play();
        });

        inputQuantize.onValueChanged.connect([this](bool value) {
            sequencer->setInputQuantize(value); // Set input quantization for the sequencer
        });
        inputQuantize.updateObservers(); // Ensure initial value is set

        displayPage.updateObservers();

    }

    int _getNextTrackIndex() {
        // Find the next available track index
        return static_cast<int>(tracks.size());
    }

    int _selectedTrackIndex = -1; // Index of the currently selected track, -1 if none selected
    
    string _createNewTrackName() {
        // Generate a new track name based on the next available index
        int nextIndex = _getNextTrackIndex();
        return "Track " + std::to_string(nextIndex + 1); // Track names start from 1
    }
    std::vector<std::shared_ptr<Track>> tracks; // List of tracks in the project
};