#pragma once
#include "sigslot/signal.hpp"
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "control_surface/mpc_studio_black_surface.h"
#include "core/mpc_sampler.h"

using std::string;
using std::shared_ptr;
using std::unordered_map;

typedef sigslot::signal<> Signal; // Define a signal type for convenience


// Instructions for how to use components:
// 1. Create a component by inheriting from this class.
// 2. Implement the onActivateComponent methods to define behavior when the component is activated.
// 3. The activate() and deactivate() control the component's state and these
// methods are called by the ModesComponent, or can be called directly.
// 4. Use sigslot::signals to connect to events in the controlSurface->[controls], such as button presses or encoder movements.
// 5. Keep track of the connection by passing them into vector<sigslot::connection>
// 6. When the component is deactivated, all connections will be disconnected automatically and the vector will be cleared.
// 7. Implement the onDeactivateComponent method to define behavior when the component is deactivated.

class Component {
public:
    string name = "Component"; // Name of the component
    Signal onActivate; // Signal emitted when the component is activated
    Signal onDeactivate; // Signal emitted when the component is deactivated
    vector<sigslot::connection> controlConnections; // Store connections for controls
    shared_ptr<MPCStudioBlackControlSurface> controlSurface; // Reference to the MPC Studio Black control surface
    shared_ptr<MPCSampler> mpcSampler; // Reference to the MPC Sampler

    
    Component(shared_ptr<MPCStudioBlackControlSurface> controlSurface)  : controlSurface(controlSurface) {
        mpcSampler = controlSurface->mpcSampler; // Initialize the MPC Sampler reference
    };

    virtual ~Component() = default;
    void activate() {
       
        std::cout << "Component activated.\n";
        _isActive = true;

        onActivateComponent();
        onActivate(); // Emit the activation signal
    };
    void deactivate() {
        std::cout << "Component deactivated.\n";
        for(auto &conn : controlConnections) {
            conn.disconnect(); // Disconnect all control connections
        }
        controlConnections.clear(); // Clear the connections vector
        _isActive = false;
        onDeactivateComponent();
        onDeactivate(); // Emit the deactivation signal
    };

    virtual void onActivateComponent() {
        
    };

    virtual void onDeactivateComponent() {
    };
    bool isActive() const {
        return _isActive;
    }
protected:
    bool _isActive = false; // Flag to indicate if the component is active
};
