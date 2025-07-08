#pragma once
#include "control_surface/controls/control.h"
#include "sigslot/signal.hpp"
#include "util.h"
#include <memory>
#include <string>
#include <vector>

struct MultiControl {
    std::string name;
    sigslot::signal<int, ShortMessage &> onMidiIn; // Signal for control value changes
    sigslot::signal<int> onPressed; // Signal for control pressed events
    sigslot::signal<int> onReleased; // Signal for control released events
    sigslot::signal<int, int> onOffset; // Signal for encoder offset changes
    sigslot::signal<int, int> onOffsetUnit; // Signal for encoder offset changes
    vector<shared_ptr<Control>> controls; // Vector of controls in the multi-control

    MultiControl(std::string name, vector<shared_ptr<Control>> controlsVector, bool isEncoder = false) : name(name), isEncoder(isEncoder) {
        for(int i = 0; i < controlsVector.size(); ++i) {
            controlsVector[i]->onValue.connect([&, i](ShortMessage &msg) {
                propagateValue(i, msg);
            });
            this->controls.push_back(controlsVector[i]);
        }
    }

    bool isEncoder = false;

    void propagateValue(int index, ShortMessage &msg) {
        
        onMidiIn(index, msg);
        if(msg.isNoteOn()) {
            onPressed(index);
        }
        if(msg.isNoteOff()) {
            onReleased(index);
        }

        if(isEncoder) {
            // For encoders, we can also emit the value directly
            auto offsetAmount = getEncoderOffsetAmount(msg.getControllerValue());
            onOffset(index, offsetAmount);
            onOffsetUnit(index, offsetAmount / 64);
        }

    }
};