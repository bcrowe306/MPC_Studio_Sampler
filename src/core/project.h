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
#include <vector>
#include "core/constants.h"
#include "core/mpc_sampler.h"

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


     //  Listeners for project events
    sigslot::signal<> onProjectLoaded; // Signal emitted when the project is loaded
    sigslot::signal<> onProjectSaved; // Signal emitted when the project is saved

    shared_ptr<MPCSampler> mpcSampler; // Sampler instance for the project
    // Project parameters
    VRString projectName;                     // Parameter for project name
    ValueOptionsReceiver<string> displayPage; // Parameter for the current display page
    VRBool metronomeEnabled;                  // Parameter to enable/disable the metronome
    VRFloat metronomeVolumeDb;                // Metronome volume in dB
    VRBool returnToZero;                      // Parameter to return to zero position when stopping playback
    VRFloat bpm;                              // BPM parameter with range from 30 to 300
    VRInt timeSignatureNumerator;             // Time signature numerator parameter
    VRBool inputQuantize;                     // Input quantization parameter

    Project(shared_ptr<MPCSampler> mpcSampler):
     projectName("projectName", "Untitled Project", mpcSampler->undoManager),
        displayPage("displayPage", kDisplayPageNames[0], kDisplayPageNames, kDisplayPageNames),
        metronomeEnabled("metronomeEnabled", true, mpcSampler->undoManager),
        metronomeVolumeDb("metronomeVolumeDb", -6.0f, -60.0f, 6.0f, 1.0, 0.01, mpcSampler->undoManager),
        returnToZero("returnToZero", true, mpcSampler->undoManager),
        bpm("bpm", 120.0f, 30.0f, 300.0f, 1.0, 0.01, mpcSampler->undoManager),
        timeSignatureNumerator("timeSignatureNumerator", 4, 1, 16, 1, 1, mpcSampler->undoManager),
        inputQuantize("inputQuantize", true, mpcSampler->undoManager),
        mpcSampler(mpcSampler)
    {
        

        

        

        _connectParams(); // Connect parameters to their respective signals
        projectName.setValue("Untitled Project"); // Set default project name
    };



    ~Project() = default;

    void serialize() {
        // Implement serialization logic if needed
    }

    void deserialize() {
        // Implement deserialization logic if needed
    }

private:

    void _connectParams (){
        bpm.onValueChanged.connect([this](float value) {
            mpcSampler->playhead->setBPM(value);
        });
        bpm.updateObservers(); // Ensure initial value is set

        metronomeEnabled.onValueChanged.connect([this](bool value) {
            mpcSampler->metronomeNode->setEnabled(value);
        });
        metronomeEnabled.updateObservers(); // Ensure initial value is set

        metronomeVolumeDb.onValueChanged.connect([this](float value) {
            mpcSampler->metronomeNode->setVolumeDb(value); // Set the metronome volume in dB
        });
        metronomeVolumeDb.updateObservers(); // Ensure initial value is set

        returnToZero.onValueChanged.connect([this](bool value) {
            mpcSampler->playhead->setReturnToZero(value);
        });

        inputQuantize.onValueChanged.connect([this](bool value) {
            mpcSampler->sequencer->setInputQuantize(value); // Set input quantization for the sequencer
        });
        inputQuantize.updateObservers(); // Ensure initial value is set

        displayPage.updateObservers();

    }

};