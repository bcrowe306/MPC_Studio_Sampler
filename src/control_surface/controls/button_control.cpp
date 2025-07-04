#include "button_control.h"

ButtonControl::ButtonControl(Control::Type controlType, uint8_t controlChannel, uint8_t controlId, const std::string &label, bool active)
    : Control(controlType, controlChannel, controlId, label, active) {

    // this->onValue.connect([this](ShortMessage& msg) {
    //     if (msg.isNoteOn()) {
    //         onPressed(); // Emit signal when button is pressed
    //     } else if (msg.isNoteOff()) {
    //         onReleased(); // Emit signal when button is released
    //     }
    // });
}

