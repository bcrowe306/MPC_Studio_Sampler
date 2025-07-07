#pragma once
#include "audio/choc_MIDI.h"
#include "component.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include <vector>

using sigslot::connection;


class SessionComponent : public Component {
    // Session-specific functionality goes here
public:
    int trackBankIndex = 0;
    SessionComponent(shared_ptr<MPCStudioBlackControlSurface> controlSurface)
        : Component(controlSurface) {
        name = "SessionComponent"; // Set the name for this component
        std::cout << "SessionComponent initialized.\n";
    };
    ~SessionComponent() override = default;

    void onActivateComponent() override {
        std::cout << "SessionComponent activated.\n";
        
        controlConnections.emplace_back(controlSurface->pads->onMidiIn.connect([this](int index, choc::midi::ShortMessage &msg) {
            onPadsPlay(index, msg);
        }));
        controlConnections.emplace_back(controlSurface->padBankAButton->onPressed.connect([this]() {
            onPadBankAButtonPressed();
        }));
        controlConnections.emplace_back(controlSurface->padBankBButton->onPressed.connect([this]() {
            onPadBankBButtonPressed();
        }));
        controlConnections.emplace_back(controlSurface->padBankCButton->onPressed.connect([this]() {
            onPadBankCButtonPressed();
        }));
        controlConnections.emplace_back(controlSurface->padBankDButton->onPressed.connect([this]() {
            onPadBankDButtonPressed();
        }));
        controlConnections.emplace_back(controlSurface->mpcSampler->project->onTrackSelected.connect([this](int trackIndex) {
            onTrackSelected(trackIndex);
        }));

        setTrackBankIndex(trackBankIndex); // Set the initial track bank index
    };

    void onDeactivateComponent() override {
        
    };

    void onPadsPlay(int index, choc::midi::ShortMessage &msg) {
        int trackToPlay = index + trackBankIndex * 16; // Calculate the track index based on the pad index and bank
        
        auto track = mpcSampler->project->getTracks()[trackToPlay]; // Get the track from the project
        if (track) {
            track->midiInput(msg); // Forward the MIDI message to the track
        }
        mpcSampler->project->selectTrack(index  + trackBankIndex * 16); // Select the track based on the pad index and bank

    };

    void onTrackSelected(int trackIndex) {
        updatePadColors(); // Update pad colors when a track is selected
        
    };

    void updatePadColors(){
        int trackIndex = mpcSampler->project->selectedTrackIndex();
        int relativeIndex = trackIndex - trackBankIndex * 16; // Calculate the relative index within the current bank
        if (relativeIndex >= 0 && relativeIndex < 16) {
            for (int i = 0; i < 16; ++i) {
                auto padControl = dynamic_cast<PadControl*>(controlSurface->pads->controls[i].get());
                if (padControl) {
                    if (i + trackBankIndex * 16 == trackIndex) {
                        padControl->sendColor(PadControl::PAD_COLOR::RED_FULL); // Highlight the selected pad
                    } else {
                        padControl->sendColor(PadControl::PAD_COLOR::OFF); // Reset the color
                    }
                }
            }
        }
        else{
            for(int i = 0; i < 16; ++i) {
                auto padControl = dynamic_cast<PadControl*>(controlSurface->pads->controls[i].get());
                if (padControl) {
                    padControl->sendColor(PadControl::PAD_COLOR::OFF); // Reset the color if out of range
                }
            }
        }
    }

    void setTrackBankIndex(int index) {
        // Set the track bank index and update the session component accordingly
        trackBankIndex = index;
        std::cout << "Track Bank Index set to: " << trackBankIndex << "\n";
        // index to padbank mapping
        std::vector<shared_ptr<TwoColorButtonControl>> padBankButtons = {
            controlSurface->padBankAButton,
            controlSurface->padBankBButton,
            controlSurface->padBankCButton,
            controlSurface->padBankDButton
        };
        for(int i = 0; i < padBankButtons.size(); ++i) {
            if (i == index) {
                padBankButtons[i]->sendColor(TwoColorButtonControl::Colors::COLOR1); // Set active bank color
            } else {
                padBankButtons[i]->sendColor(TwoColorButtonControl::Colors::OFF); // Set inactive bank color
            }
        }
        updatePadColors();
    }

    void onPadBankAButtonPressed() {
        setTrackBankIndex(0); // Set to bank A
    }

    void onPadBankBButtonPressed() {
        setTrackBankIndex(1); // Set to bank B
    }

    void onPadBankCButtonPressed() {
        setTrackBankIndex(2); // Set to bank C
    }

    void onPadBankDButtonPressed() {
        setTrackBankIndex(3); // Set to bank D
    }
    
};