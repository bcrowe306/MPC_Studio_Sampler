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
         updateLeds(); // Update the LEDs based on the current state
    }

    void updateLeds(){
        auto &cs = controlSurface;
        // Play button LED
        if (mpcSampler->project->isPlaying()) {

            cs->playButton->blinking = true; // Enable blinking for play button
        } else {
            cs->playButton->blinking = false; // Disable blinking for play button
            cs->playButton->sendColor(OneColorButtonControl::Colors::OFF);
        }
    }

    void onDeactivateComponent() override {
       
    }
};

