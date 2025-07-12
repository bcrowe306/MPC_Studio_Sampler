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
        
        // Full Level Button
        addConnection(controlSurface->fullLevelButton->onPressed.connect([this]() {
                controlSurface->mpcSampler->fullLevel->toggleEnabled();
        }));
        
        addConnection(mpcSampler->fullLevel->enabled.onValueChanged.connect([this](bool enabled) {
            if(enabled) {
                controlSurface->fullLevelButton->sendColor(TwoColorButtonControl::Colors::COLOR1); // Set button color to yellow when enabled
            } else {
                controlSurface->fullLevelButton->sendColor(TwoColorButtonControl::Colors::OFF); // Set button color to off when disabled
            }
        }));

        // Sequence Selection Plus and Minus Buttons
        addConnection(controlSurface->plusButton->onPressed.connect([this]() {
            mpcSampler->project->sequencer->nextSequence(); // Select the next sequence
        }));

        addConnection(controlSurface->minusButton->onPressed.connect([this]() {
            mpcSampler->project->sequencer->previousSequence(); // Select the previous sequence
        }));

        // Note Repeat Button
        addConnection(controlSurface->noteRepeatButton->onPressed.connect([this]() {
            controlSurface->mpcSampler->noteRepeat->toggleEnabled();
        }));

        addConnection(controlSurface->qlinkScrollEncoder->onOffset.connect([this](int offset) {
            if(controlSurface->shiftButton->isPressed){
                if (offset > 0) {
                    mpcSampler->project->bpm.incrementValue(true); // Increment BPM by coarse value
                } else if (offset < 0) {
                    mpcSampler->project->bpm.decrementValue(true); // Decrement BPM by coarse value
                }
            }
        }));

        addConnection(controlSurface->pads->onMidiIn.connect([this](int index, choc::midi::ShortMessage &msg) {
            onPadsPlay(index, msg);
        }));
        addConnection(controlSurface->padBankAButton->onPressed.connect([this]() {
            onPadBankAButtonPressed();
        }));
        addConnection(controlSurface->padBankBButton->onPressed.connect([this]() {
            onPadBankBButtonPressed();
        }));
        addConnection(controlSurface->padBankCButton->onPressed.connect([this]() {
            onPadBankCButtonPressed();
        }));
        addConnection(controlSurface->padBankDButton->onPressed.connect([this]() {
            onPadBankDButtonPressed();
        }));
        addConnection(controlSurface->mpcSampler->project->onTrackSelected.connect([this](int trackIndex) {
            onTrackSelected(trackIndex);
        }));

        addConnection(mpcSampler->project->sequencer->onMidiOutput.connect([this](int trackIndex, choc::midi::ShortMessage &msg) {
            padFeedback(trackIndex, msg);
        }));

        setTrackBankIndex(trackBankIndex); // Set the initial track bank index
    };

    void onDeactivateComponent() override {
        
    };

    void padFeedback(int trackIndex, choc::midi::ShortMessage &msg){
        // Handle pad feedback here if needed
        int relativeIndex = trackIndex - trackBankIndex * 16;
        if (relativeIndex >= 0 && relativeIndex < 16){
            auto padControl = dynamic_cast<PadControl*>(controlSurface->pads->controls[relativeIndex].get());
            auto selectedTrackIndex = mpcSampler->project->selectedTrackIndex();
            if(msg.isNoteOn()){
                if (padControl) {
                    padControl->sendColor(PadControl::PAD_COLOR::YELLOW_FULL); // Set pad color to yellow when note is on
                }
            } else if (msg.isNoteOff()) {
                if (padControl) {
                    if (selectedTrackIndex == trackIndex) {
                        padControl->sendColor(PadControl::PAD_COLOR::RED_FULL); // Highlight the selected pad
                    } 
                    else {
                        // Check if the track is empty
                        auto track = mpcSampler->project->getTracks()[trackIndex];
                        if (track && track->isTrackEmpty()) {
                            padControl->sendColor(PadControl::PAD_COLOR::OFF); // Indicate empty track with green
                        } else {
                            padControl->sendColor(PadControl::PAD_COLOR::GREEN_HALF); // Reset color if not empty
                        }
                    }
                    
                }
            } else {
                std::cout << "Received unsupported MIDI message type for pad feedback.\n"; // Handle unsupported message types
            }
        }
    }

    void onPadsPlay(int index, choc::midi::ShortMessage &msg) {
        int trackToPlay = index + trackBankIndex * 16; // Calculate the track index based on the pad index and bank
        
        auto track = mpcSampler->project->getTracks()[trackToPlay]; // Get the track from the project
        if (track) {
            mpcSampler->project->selectTrack(index  + trackBankIndex * 16); // Select the track based on the pad index and bank
            mpcSampler->sendMidiInput(msg);
        }

    };

    void onTrackSelected(int trackIndex) {
        updatePadColors(); // Update pad colors when a track is selected
        
    };

    void updatePadColors(){
        int trackIndex = mpcSampler->project->selectedTrackIndex();
        int relativeIndex = trackIndex - trackBankIndex * 16; // Calculate the relative index within the current bank

        if (relativeIndex >= 0 && relativeIndex < 16) {
            for (int i = 0; i < 16; ++i) {
                auto track = mpcSampler->project->getTracks()[i + trackBankIndex * 16]; // Get the track for the current pad
                auto padControl = dynamic_cast<PadControl*>(controlSurface->pads->controls[i].get());
                if (padControl) {
                    if (i + trackBankIndex * 16 == trackIndex) {
                        padControl->sendColor(PadControl::PAD_COLOR::RED_FULL); // Highlight the selected pad
                    } else {

                        // Check if the track is empty
                        if (track && track->isTrackEmpty()) {
                            padControl->sendColor(PadControl::PAD_COLOR::OFF); // Indicate empty track with green
                        } else {
                            padControl->sendColor(PadControl::PAD_COLOR::GREEN_HALF); // Reset color if not empty
                        }
                    }
                }
            }
        }
        else{
            for(int i = 0; i < 16; ++i) {
                auto track = mpcSampler->project->getTracks()[i + trackBankIndex * 16]; // Get the track for the current pad
                auto padControl = dynamic_cast<PadControl*>(controlSurface->pads->controls[i].get());
                if (padControl) {
                    padControl->sendColor(PadControl::PAD_COLOR::OFF); // Reset the color if out of range
                    // Check if the track is empty
                    if (track && track->isTrackEmpty()) {
                        padControl->sendColor(PadControl::PAD_COLOR::OFF); // Indicate empty track with green
                    } else {
                        padControl->sendColor(PadControl::PAD_COLOR::GREEN_HALF); // Reset color if not empty
                    }
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