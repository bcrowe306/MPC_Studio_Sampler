#pragma once
#include "component.h"
#include <memory>


class TransportComponent : public Component {
public:

    TransportComponent(shared_ptr<class MPCStudioBlackControlSurface> controlSurface) : Component(controlSurface) {
        name = "TransportComponent"; // Set the name of the component
       
        
    }

    ~TransportComponent() = default;

    void onActivateComponent() override {
        std::cout << "Transport Component activated.\n";
        isActive = true;
    }

    void onDeactivateComponent() override {
        std::cout << "Transport Component deactivated.\n";
        isActive = false;
    }
};

