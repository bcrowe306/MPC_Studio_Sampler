#pragma once
#include "component.h"
#include "control_surface/controls/control.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include <vector>


class BrowserComponent : public Component {
    // Base class for all page components
public:
    
    BrowserComponent(shared_ptr<MPCStudioBlackControlSurface> controlSurface)  : Component(controlSurface) 
    {
        name = "BrowserComponent"; // Set the name for this component

    }

    ~BrowserComponent() override = default;

    void onActivateComponent() override {
        controlConnections.push_back(controlSurface->jogWheel->onOffset.connect([this](int offset) {
            if(controlSurface->shiftButton->isPressed) {
                mpcSampler->browser->page(offset);
            } else {
                mpcSampler->browser->scroll(offset);
            }
        }));
        
        controlConnections.push_back(controlSurface->leftButton->onPressed.connect([this]() {
            mpcSampler->browser->goBack();
        }));

        controlConnections.push_back(controlSurface->rightButton->onPressed.connect([this]() {
            mpcSampler->browser->navigateToSelectedItem();
        }));

        controlConnections.push_back(controlSurface->plusButton->onPressed.connect([this]() {
            if(controlSurface->shiftButton->isPressed) {
                mpcSampler->browser->page(1);
            } else {
                mpcSampler->browser->scroll(1);
            }
        }));

        controlConnections.push_back(controlSurface->minusButton->onPressed.connect([this]() {
            if(controlSurface->shiftButton->isPressed) {
                mpcSampler->browser->page(-1);
            } else {
                mpcSampler->browser->scroll(-1);
            }   
        }));

        controlConnections.push_back(controlSurface->f4Button->onPressed.connect([this]() {
            mpcSampler->browser->autoPreview = !mpcSampler->browser->autoPreview.get(); // Toggle auto-preview
        }));

        controlConnections.push_back(controlSurface->f5Button->onPressed.connect([this]() {
            mpcSampler->browser->previewItem();
        }));

        controlConnections.push_back(controlSurface->f6Button->onPressed.connect([this]() {
            auto selectedItem = mpcSampler->browser->getSelectedItem();
            auto track = mpcSampler->project->selectedTrack();
            if (selectedItem.is_regular_file() && selectedItem.path().extension() == ".wav") {
                track->loadSample(selectedItem.path().string());
            }
        }));
    };

    void onDeactivateComponent() override {
        std::cout << "BrowserComponent deactivated.\n";
        // Additional deactivation logic can be added here
    };

    

};