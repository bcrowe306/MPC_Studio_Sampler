#include "encoder_control.h"

EncoderControl::EncoderControl(uint8_t controlChannel, uint8_t controlId, const std::string &label, bool active)
    : Control(Type::CC, controlChannel, controlId, label, active) {
    this->onValue.connect([this](uint8_t value) {
        // Emit signal when encoder value changes
        auto offsetAmount = getOffsetAmount(value);
        onOffset(offsetAmount);
        onOffsetUnit(offsetAmount / 64);

        if (offsetAmount > 0) {
            onIncrement();
        } else if (offsetAmount < 0) {
            onDecrement();
        }
    });
}


int EncoderControl::getOffsetAmount(uint8_t &value) const {
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
