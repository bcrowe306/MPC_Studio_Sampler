#pragma once
#include "LabSound/LabSound.h"
#include "sigslot/signal.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include "core/timing.h"
#include "core/track.h"
#include "command.h"
#include "value_receiver.h"
#include <array>
#include <string>
#include <vector>
#include "core/constants.h"
#include "core/mpc_sampler.h"
#include "core/serializable.h"
#include "yaml-cpp/emittermanip.h"

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

class Project : public Serializable {
public:


     //  Listeners for project events
    sigslot::signal<> onProjectLoaded; // Signal emitted when the project is loaded
    sigslot::signal<> onProjectSaved; // Signal emitted when the project is saved
    sigslot::signal<std::string> saveProject; // Signal emitted when the project is saved
    vector<sigslot::connection> projectConnections;
    shared_ptr<MPCSampler> mpcSampler; // Sampler instance for the project
    // Project parameters
    shared_ptr<VRString> projectName;                     // Parameter for project name
    shared_ptr<VRBool> metronomeEnabled;                  // Parameter to enable/disable the metronome
    shared_ptr<VRFloat> metronomeVolumeDb;                // Metronome volume in dB
    shared_ptr<VRBool> returnToZero;                      // Parameter to return to zero position when stopping playback
    shared_ptr<VRFloat> bpm;                              // BPM parameter with range from 30 to 300
    shared_ptr<VRInt> timeSignatureNumerator;             // Time signature numerator parameter
    shared_ptr<VRBool> inputQuantize;                     // Input quantization parameter


    Project(shared_ptr<MPCSampler> mpcSampler):
        projectName(make_shared<VRString>("projectName", "Untitled Project", mpcSampler->undoManager)),
        metronomeEnabled(make_shared<VRBool>("metronomeEnabled", true, mpcSampler->undoManager)),
        metronomeVolumeDb(make_shared<VRFloat>("metronomeVolumeDb", -6.0f, -60.0f, 6.0f, 1.0, 0.01, mpcSampler->undoManager)),
        returnToZero(make_shared<VRBool>("returnToZero", true, mpcSampler->undoManager)),
        bpm(make_shared<VRFloat>("bpm", 120.0f, 30.0f, 300.0f, 1.0, 0.01, mpcSampler->undoManager)),
        timeSignatureNumerator(make_shared<VRInt>("timeSignatureNumerator", 4, 1, 16, 1, 1, mpcSampler->undoManager)),
        inputQuantize(make_shared<VRBool>("inputQuantize", true, mpcSampler->undoManager)),
        mpcSampler(mpcSampler)
    {  
        _connectParams(); // Connect parameters to their respective signals
        projectName->setValue("Untitled Project"); // Set default project name
        mpcSampler->sequencer->resetSequencer();
    };



    ~Project(){
        // Disconnect all project connections
        for (auto &connection : projectConnections) {
            connection.disconnect();
        }
        std::cout << "Project destroyed" << std::endl;
    };

    void addConnection(const sigslot::connection &connection){
        // Add a signal connection to the project
        projectConnections.push_back(connection);
    }

    void serialize(YAML::Emitter &out) override {
        out << YAML::BeginDoc;
        out << YAML::BeginMap;
          out << YAML::Key << "Project";
          out << YAML::Value;
          out << YAML::BeginMap;
            projectName->serialize(out);
            metronomeEnabled->serialize(out);
            metronomeVolumeDb->serialize(out);
            returnToZero->serialize(out);
            bpm->serialize(out);
            timeSignatureNumerator->serialize(out);
            inputQuantize->serialize(out);
            out << YAML::Key << "selectedTrackIndex";
            out << YAML::Value << mpcSampler->selectedTrackIndex(); // Serialize the selected track index

            // Serialize the tracks
            out << YAML::Key << "Tracks";
            out << YAML::Value << YAML::BeginSeq;
            int trackIndex = 0;
            for (const auto &track : mpcSampler->getTracks()) {
                out << YAML::BeginMap;
                out << YAML::Key << "Index";
                out << YAML::Value << trackIndex;
                track->serialize(out);
                out << YAML::EndMap;
                trackIndex++;
            }
            out << YAML::EndSeq;
            // End of tracks serialization

            // Serialize the Sequencer
            out << YAML::Key << "Sequencer";
            out << YAML::Value;
            mpcSampler->sequencer->serialize(out);

        out << YAML::EndMap;
        out << YAML::EndDoc;
       
    }

    void deserialize(const YAML::Node &node) override{
        auto rootNode = node["Project"];
        if (!rootNode) {
            std::cerr << "Invalid YAML node for Project" << std::endl;
            return;
        }
        projectName->deserialize(rootNode);
        metronomeEnabled->deserialize(rootNode);
        metronomeVolumeDb->deserialize(rootNode);
        returnToZero->deserialize(rootNode);
        bpm->deserialize(rootNode);
        timeSignatureNumerator->deserialize(rootNode);
        inputQuantize->deserialize(rootNode);

        if(rootNode["selectedTrackIndex"]) {
            mpcSampler->selectTrack(rootNode["selectedTrackIndex"].as<int>());
        } else {
            mpcSampler->selectTrack(0); // Default to no track selected
        }

        // Deserialize the tracks
        auto tracksNode = rootNode["Tracks"];
        if (tracksNode && tracksNode.IsSequence()) {
            int trackIndex = 0;
            auto tracks = mpcSampler->getTracks();
            for (const auto &trackNode : tracksNode) {
                tracks[trackIndex]->deserialize(trackNode);
                trackIndex++;
            }
        }

        // Deserialize the Sequencer
        auto sequencerNode = rootNode["Sequencer"];
        if (sequencerNode) {
            mpcSampler->sequencer->deserialize(sequencerNode);
        } else {
            std::cerr << "Invalid YAML node for Sequencer" << std::endl;
        }

        onProjectLoaded();
    }

    void save() {
        saveProject(projectName->getValue());
        onProjectSaved();
    }

private:

    void _connectParams (){
        addConnection(bpm->onValueChanged.connect([this](float value) {
            mpcSampler->playhead->setBPM(value);
        }));
        bpm->updateObservers(); // Ensure initial value is set

        addConnection(metronomeEnabled->onValueChanged.connect([this](bool value) {
            mpcSampler->metronomeNode->setEnabled(value);
        }));
        metronomeEnabled->updateObservers(); // Ensure initial value is set

        addConnection(metronomeVolumeDb->onValueChanged.connect([this](float value) {
            mpcSampler->metronomeNode->setVolumeDb(value); // Set the metronome volume in dB
        }));
        metronomeVolumeDb->updateObservers(); // Ensure initial value is set

        addConnection(returnToZero->onValueChanged.connect([this](bool value) {
            mpcSampler->playhead->setReturnToZero(value);
        }));

        addConnection(inputQuantize->onValueChanged.connect([this](bool value) {
            mpcSampler->sequencer->setInputQuantize(value); // Set input quantization for the sequencer
        }));
        inputQuantize->updateObservers(); // Ensure initial value is set

    }

};