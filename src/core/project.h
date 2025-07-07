#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/FunctionNode.h"
#include "sigslot/signal.hpp"
#include <algorithm>
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

inline const int kMaxTracks = 64; // Maximum number of tracks in a project
inline const int kMaxBusses = 8; // Maximum number of busses in a project

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
    Project(std::shared_ptr<AudioContext> audioContext) {
        // Initialize the project with an empty track list
        this->audioContext = audioContext;
        masterTrack = std::make_shared<TrackNode>(audioContext);
        cueTrack = std::make_shared<TrackNode>(audioContext);
        metronomeNode = std::make_shared<MetronomeNode>(audioContext);
        playhead = std::make_shared<Playhead>(audioContext);
        undoManager = std::make_shared<UndoManager>(audioContext); // Initialize the undo manager

        playhead->midiClock->onMetronomeTick.connect(std::bind(&MetronomeNode::onMetronomeTick, metronomeNode.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        audioContext->connect(audioContext->destinationNode(), masterTrack->output, 0, 0); 
        audioContext->connect(audioContext->destinationNode(), cueTrack->output, 0, 0); 
        audioContext->connect(audioContext->destinationNode(), playhead->playheadNode, 0, 0);
        audioContext->connect(cueTrack->input, metronomeNode->clickGainNode, 0, 0); // Connect metronome output to audio context destination
        audioContext->synchronizeConnections(); // Synchronize connections after setup

        tracks = std::vector<std::shared_ptr<Track>>();
        tracks.reserve(kMaxTracks); // Reserve space for 64 tracks
        _createTracks(); // Create the initial tracks

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
    shared_ptr<MetronomeNode> metronomeNode; // Metronome node for the project
    shared_ptr<Playhead> playhead; // Playhead for the project
    shared_ptr<UndoManager> undoManager; // Undo manager for the project

    // Project parameters
    VRString projectName = VRString("projectName", "Untitled Project", undoManager); // Parameter for project name
    ValueOptionsReceiver<string> displayPage = ValueOptionsReceiver<string>(
        "displayPage", kDisplayPageNames[0], kDisplayPageNames, kDisplayPageNames); // Parameter for the current display page
    VRBool metronomeEnabled = VRBool("metronomeEnabled", true, undoManager); // Parameter to enable/disable the metronome
    VRBool returnToZero = VRBool("returnToZero", true, undoManager); // Parameter to return to zero position when stopping playback
    VRFloat bpm = VRFloat("bpm", 120.0f, 30.0f, 300.0f, 1.0, 0.01, undoManager); // BPM parameter with range from 30 to 300
    VRInt timeSignatureNumerator = VRInt("timeSignatureNumerator", 4, 1, 16, 1, 1, undoManager); // Time signature numerator parameter
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
        onIsPlaying(true); // Emit signal that playback has started
    }

    void stop() {
        playhead->stop(); // Stop the playhead
        onIsPlaying(false); // Emit signal that playback has stopped
    }

    void togglePlay() {
        playhead->togglePlaying(); // Toggle play/pause state
        if (playhead->midiClock->isEnabled()) {
            onIsPlaying(true); // Emit signal that playback has started
        } else {
            onIsPlaying(false); // Emit signal that playback has stopped
        }
    }
    

private:
    void _createTracks() {
            for (int i = 0; i < kMaxTracks; ++i) {
                auto track = std::make_shared<Track>(audioContext, undoManager); // Create a new track
                track->createSamplerDevice("/Users/brandoncrowe/Documents/Audio Samples/DECAP - Drums That Knock X/Hihats/DECAP hihat 219 crich.wav");
                (*track->name.get()) = _createNewTrackName();

               
                audioContext->connect(
                    masterTrack->input, track->getOutput(), 0,
                    0); // Connect track output to audio context destination
                tracks.push_back(track); // Add the track to the list
            }
            audioContext->synchronizeConnections(); // Synchronize connections
            selectTrack(0); // Select the first track by default
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

        returnToZero.onValueChanged.connect([this](bool value) {
            playhead->setReturnToZero(value);
        });

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