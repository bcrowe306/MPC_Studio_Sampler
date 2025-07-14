#include "encoder_control.h"
#include "audio/choc_MIDI.h"
#include <_types/_uint8_t.h>

EncoderControl::EncoderControl(uint8_t controlChannel, uint8_t controlId, const std::string &label, bool active)
    : Control(Type::CC, controlChannel, controlId, label, active) {
    this->onValue.connect([this](ShortMessage & msg) {
        // Emit signal when encoder value changes
        int offsetAmount = getOffsetAmount(msg.getControllerValue());

        genSlowOffset(offsetAmount);
        onOffset(offsetAmount);
        onOffsetUnit(offsetAmount / 64);

        if (offsetAmount > 0) {
            onIncrement();
        } else if (offsetAmount < 0) {
            onDecrement();
        }
    });
}


int EncoderControl::getOffsetAmount(uint8_t value) const {
    int seventhBit = (value & 0b1000000) >> 6;
    int offsetAmount = (value & 0b0111111);
    if (bool(seventhBit)) { // Convert to signed value
        offsetAmount -= 64;
        offsetAmount = offsetAmount; // Make it negative
    } else {
        offsetAmount = offsetAmount; // No change needed
    }
    return offsetAmount;
}

void EncoderControl::genSlowOffset(int offsetAmount) {
    // Generate an offset event based on 2 or more increments/decrements
    if (offsetAmount > 0) {
        incCount++; // Increment the count
        if(incCount > _slowOffsetAmount) {
            
            incCount = 0; // Reset the increment count
            onIncrementSlow(offsetAmount); // Emit signal for slow increment
            onOffsetSlow(offsetAmount); // Emit signal with slow offset
            onOffsetSlowUnit(offsetAmount / 64.0); // Emit signal with slow offset as a unit
        }
    } else if (offsetAmount < 0) {
        _decCount++; // Increment the decrement count
        if(_decCount > _slowOffsetAmount) {
            _decCount = 0; // reset the decrement count
            onDecrementSlow(offsetAmount); // Emit signal for slow decrement
            onOffsetSlow(offsetAmount); // Emit signal with slow offset
        }
    }
}
