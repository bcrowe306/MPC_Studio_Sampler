#pragma once
#include "project.h"
#include <memory>
#include <string>
#include "audio_engine.h"
#include "core/browser.h"

using std::string;
using std::shared_ptr;
using std::make_shared;

// Main class for the Sampler/Sequencer
class MPCSampler{
public:
    shared_ptr<Browser> browser; // Browser for file management and navigation
    MPCSampler(){
        initialize(); // Initialize the sampler when created
        browser = make_shared<Browser>(audioEngine->context); // Initialize the browser
        
    };
    ~MPCSampler(){
        uninitialize(); // Clean up resources when the sampler is destroyed
    };
    shared_ptr<AudioEngine> audioEngine; // Audio engine for playback and processing
    void initialize(){
        audioEngine = make_shared<AudioEngine>();
        audioEngine->activate(); // Activate the audio engine
        newProject(); // Create a new project when initializing
    };
    void uninitialize(){};
    void loadProject(std::string projectFilePath){};
    void saveProject(std::string projectFilePath){};
    void newProject(){
        project = make_shared<Project>(audioEngine->context); // Create a new project
        audioEngine->context->synchronizeConnections(); // Synchronize connections after creating a new project
    }
    shared_ptr<Project> project; // Project containing tracks and devices
};
