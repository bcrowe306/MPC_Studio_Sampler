#pragma once
#include "sigslot/signal.hpp"
#include <string>
#include <iostream>
#include "control_surface/mpc_studio_black_surface.h"
#include <memory>

using std::string;
using std::shared_ptr;

typedef sigslot::signal<> Signal; // Define a signal type for convenience

class Component {
public:
    string name = "Component"; // Name of the component
    Signal onActivate;
    Signal onDeactivate;
    shared_ptr<MPCStudioBlackControlSurface> controlSurface; // Reference to the MPC Studio Black control surface
    Component(shared_ptr<MPCStudioBlackControlSurface> controlSurface)
        : controlSurface(controlSurface) {
        onActivate.connect(std::bind(&Component::onActivateComponent, this));
        onDeactivate.connect(std::bind(&Component::onDeactivateComponent, this));
    };

    ~Component() = default;
    void activate() {
        std::cout << "Component activated.\n";
        isActive = true;
        onActivate();
    };
    void deactivate() {
        std::cout << "Component deactivated.\n";
        isActive = false;
        onDeactivate();
    };

    virtual void onActivateComponent() {
        std::cout << "Component activated.\n";
        isActive = true;
        onActivate();
    };

    virtual void onDeactivateComponent() {
        std::cout << "Component deactivated.\n";
        isActive = false;
        onDeactivate();
    };
protected:
    bool isActive = false; // Flag to indicate if the component is active
};