#pragma once
#include "audio/choc_MIDI.h"
#include "core/constants.h"
#include "core/property.h"
#include "project.h"
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

using std::string;
using std::shared_ptr;
using std::make_shared;


// Main class for the Sampler/Sequencer
class MPCSampler{
public:
   
    MPCSampler(){
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

        
        newProject(); // Create a new project when initializing
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
    
    void newProject(){
        project = make_shared<Project>(audioEngine->context, timer); // Create a new project
        audioEngine->context->synchronizeConnections(); // Synchronize connections after creating a new project
    }
    shared_ptr<Project> project; // Project containing tracks and devices
private:
    void _sendMidiToProject(ShortMessage &msg){
        if(project){
            auto selectedTrack = project->selectedTrack();
            if (selectedTrack) {
                selectedTrack->midiInput(msg); // Forward MIDI input to the selected track
            } else {
                std::cerr << "No track selected for MIDI input." << std::endl;
            }
        }
    }
};
