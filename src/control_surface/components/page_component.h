#pragma once
#include "component.h"
#include "control_surface/controls/control.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include <vector>


class PageComponent : public Component {
    // Base class for all page components
public:
    
    PageComponent(shared_ptr<MPCStudioBlackControlSurface> controlSurface)  : Component(controlSurface) 
    {
        name = "PageComponent"; // Set the name for this component
    }

    ~PageComponent() override = default;

    void onActivateComponent() override {
        controlConnections.push_back(
            mpcSampler->project->displayPage.onValueChanged.connect(
                [this](std::string pageName) { onPageChanged(pageName); }));

        controlConnections.push_back(
            controlSurface->progEditButton->onPressed.connect(
                [this]() { mpcSampler->project->displayPage = "devicePage"; }));

        controlConnections.push_back(
            controlSurface->progMixButton->onPressed.connect(
                [this]() { mpcSampler->project->displayPage = "mixerPage"; }));

        controlConnections.push_back(
            controlSurface->seqEditButton->onPressed.connect([this]() {
              mpcSampler->project->displayPage = "sequencePage";
            }));

        controlConnections.push_back(
            controlSurface->sampleEditButton->onPressed.connect([this]() {
              mpcSampler->project->displayPage = "performPage";
            }));

        controlConnections.push_back(
            controlSurface->songButton->onPressed.connect([this]() {
              mpcSampler->project->displayPage = "arrangerPage";
            }));

        controlConnections.push_back(
            controlSurface->browserButton->onPressed.connect([this]() {
              mpcSampler->project->displayPage = "browserPage";
            }));

        controlConnections.push_back(controlSurface->mainButton->onPressed.connect([this]() {
              mpcSampler->project->displayPage = "projectPage";
        }));

        // Run the listener functions once to setup the initial state
        onPageChanged("");
    };

    void onDeactivateComponent() override {
        std::cout << "PageComponent deactivated.\n";
        // Additional deactivation logic can be added here
    };

    void onPageChanged(const std::string &pName) {

        std::string pageName = pName.empty() ? mpcSampler->project->displayPage.getValue() : pName;

        std::cout << "Page changed to: " << pageName << "\n";

        if (pageName == "devicePage") {
            controlSurface->progEditButton->sendColor(TwoColorButtonControl::Colors::COLOR1);
        } else {
            controlSurface->progEditButton->sendColor(TwoColorButtonControl::Colors::OFF);
        }

        if (pageName == "mixerPage") {
            controlSurface->progMixButton->sendColor(TwoColorButtonControl::Colors::COLOR1);
        } else {
            controlSurface->progMixButton->sendColor(TwoColorButtonControl::Colors::OFF);
        }

        if (pageName == "sequencePage") {
            controlSurface->seqEditButton->sendColor(TwoColorButtonControl::Colors::COLOR1);
        } else {
            controlSurface->seqEditButton->sendColor(TwoColorButtonControl::Colors::OFF);
        }

        if (pageName == "performPage") {
            controlSurface->sampleEditButton->sendColor(TwoColorButtonControl::Colors::COLOR1);
        } else {
            controlSurface->sampleEditButton->sendColor(TwoColorButtonControl::Colors::OFF);
        }

        if (pageName == "arrangerPage") {
            controlSurface->songButton->sendColor(TwoColorButtonControl::Colors::COLOR1);
        } else {
            controlSurface->songButton->sendColor(TwoColorButtonControl::Colors::OFF);
        }

        if (pageName == "browserPage") {
            controlSurface->browserButton->sendColor(OneColorButtonControl::Colors::ON);
        } else {
            controlSurface->browserButton->sendColor(OneColorButtonControl::Colors::OFF);
        }

        if (pageName == "projectPage") {
            controlSurface->mainButton->sendColor(OneColorButtonControl::Colors::ON);
        } else {
            controlSurface->mainButton->sendColor(OneColorButtonControl::Colors::OFF);
        }
    };

};