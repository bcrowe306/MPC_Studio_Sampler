#pragma once
#include "component.h"
#include <memory>


class TransportComponent : public Component {
public:

    TransportComponent(shared_ptr<class MPCStudioBlackControlSurface> controlSurface) : Component(controlSurface) {
        
    }

    ~TransportComponent() = default;

    void onActivateComponent() override {
       auto &con = controlConnections;
       auto &cs = controlSurface;
         controlConnections.push_back(controlSurface->playButton->onPressed.connect([this]() {
              mpcSampler->project->play();
         }));

         controlConnections.push_back(controlSurface->stopButton->onPressed.connect([this]() {
              mpcSampler->project->stop();
         }));

         controlConnections.push_back(controlSurface->recordButton->onPressed.connect([this]() {
              mpcSampler->project->toggleRecord();
         }));

         controlConnections.push_back(controlSurface->numericButton->onPressed.connect([this]() {
              mpcSampler->project->metronomeEnabled.setValue(!mpcSampler->project->metronomeEnabled.getValue());
         }));

         controlConnections.push_back(controlSurface->tapTempoButton->onPressed.connect([this]() {
              mpcSampler->project->tapTempo();
         }));
         controlConnections.push_back(mpcSampler->project->onIsPlaying.connect([this](bool isPlaying) {
              updateLeds(); // Update the LEDs when playback state changes
         }));

         controlConnections.push_back(mpcSampler->project->playhead->onPlayheadStateChanged.connect([this](Playhead::PlayheadState state) {
              updateLeds(); // Update the LEDs when playhead state changes
         }));
         addConnection(controlSurface->undoButton->onPressed.connect([this]() {
               if(controlSurface->shiftButton->isPressed) {
                   mpcSampler->project->undoManager->redo();
               } else {
                   mpcSampler->project->undoManager->undo();
               }
         }));
         updateLeds(); // Update the LEDs based on the current state
    }

    void updateLeds(){
     

        auto &cs = controlSurface;
        // Play button LED
        controlSurface->tapTempoButton->sendColor(OneColorButtonControl::Colors::ON);
        if (mpcSampler->project->isPlaying()) {

            cs->playButton->blinking = true; // Enable blinking for play button
        } else {
            cs->playButton->blinking = false; // Disable blinking for play button
            cs->playButton->sendColor(OneColorButtonControl::Colors::OFF);
        }

        // Record button LED
        if(mpcSampler->project->playhead->isRecording()) {
            cs->recordButton->sendColor(OneColorButtonControl::Colors::ON); // Set record button to ON color
        } else {
            cs->recordButton->sendColor(OneColorButtonControl::Colors::OFF); // Set record button to OFF color
        }

        // Undo button LED
        controlSurface->undoButton->sendColor(TwoColorButtonControl::Colors::COLOR1);
    }

    void onDeactivateComponent() override {
       
    }
};

