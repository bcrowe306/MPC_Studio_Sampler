#pragma once
#include "control_surface/controls/control.h"
#include "sigslot/signal.hpp"
#include "util.h"
#include <memory>
#include <string>
#include <vector>

struct MultiControl {
    std::string name;
    uint8_t incCount = 0;
    uint8_t _decCount = 0;
    uint8_t _slowOffsetAmount = 4;
    vector<shared_ptr<Control>> controls; // Vector of controls in the multi-control

    sigslot::signal<int, ShortMessage &> onMidiIn; // Signal for control value changes
    sigslot::signal<int> onPressed; // Signal for control pressed events
    sigslot::signal<int> onReleased; // Signal for control released events

    // Signals for encoder-specific events
    sigslot::signal<int, int> onOffset; // Signal for encoder offset changes
    sigslot::signal<int, int> onOffsetUnit; // Signal for encoder offset changes
    sigslot::signal<int, int> onOffsetSlow;
    sigslot::signal<int, int> onIncrementSlow;
    sigslot::signal<int, int> onDecrementSlow;
    sigslot::signal<int, double> onOffsetSlowUnit; // Signal emitted with double offset amount

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
            genSlowOffset(index, offsetAmount);
            onOffset(index, offsetAmount);
            onOffsetUnit(index, offsetAmount / 64);
        }
    }

    void genSlowOffset(int controlIndex, int offsetAmount) {
        // Generate an offset events based on 2 or more increments/decrements, defined by _slowOffsetAmount
        if (offsetAmount > 0) {
            incCount++; // Increment the count
            if(incCount > _slowOffsetAmount) {
                
                incCount = 0; // Reset the increment count

                // Emit signals for slow increment
                onIncrementSlow(controlIndex, offsetAmount);
                onOffsetSlow(controlIndex, offsetAmount);
                onOffsetSlowUnit(controlIndex, offsetAmount / 64.0);
            }
        } else if (offsetAmount < 0) {
            _decCount++;
            if(_decCount > _slowOffsetAmount) {
                _decCount = 0; // reset the decrement count

                // Emit signals for slow decrement
                onDecrementSlow(controlIndex, offsetAmount); 
                onOffsetSlow(controlIndex, offsetAmount); 
                onOffsetSlowUnit(controlIndex, offsetAmount / 64.0);
            }
        }
    }

};