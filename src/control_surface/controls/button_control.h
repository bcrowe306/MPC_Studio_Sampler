#include "audio/choc_MIDI.h"
#include "control.h"
#include "sigslot/signal.hpp"


class ButtonControl : public Control {
public:
    ButtonControl(Control::Type controlType, uint8_t controlChannel, uint8_t controlId, const std::string &label = "", bool active = true);
    virtual ~ButtonControl() = default;
    sigslot::signal<> onPressed; // Signal for button press events
    sigslot::signal<> onReleased; // Signal for button release events
};

class PlainButtonControl : public ButtonControl {
    public:
        enum class Colors { 
            OFF = 0, // OFF state
            ON = 1,
        };
        PlainButtonControl(uint8_t controlId, const std::string &label = "", bool active = true)
            : ButtonControl(Control::Type::NOTE, 0x00, controlId, label, active) {
            this->onValue.connect([this](ShortMessage& msg) {
                if(msg.isNoteOn()) {
                    onPressed(); // Emit signal when button is pressed
                } else if(msg.isNoteOff()) {
                    onReleased(); // Emit signal when button is released
                }
            });
        }
    };


class OneColorButtonControl : public ButtonControl {
public:
    enum class Colors { 
        OFF = 0, // OFF state
        ON = 1,
    };
    OneColorButtonControl(uint8_t controlId, const std::string &label = "", bool active = true)
        : ButtonControl(Control::Type::NOTE, 0x00, controlId, label, active) {
        this->onValue.connect([this](ShortMessage& msg) {
            if (msg.isNoteOn()) {
                onPressed(); // Emit signal when button is pressed
            } else if (msg.isNoteOff()) {
                onReleased(); // Emit signal when button is released
            }
        });
    }

    void sendColor(OneColorButtonControl::Colors color) {
        uint8_t value = static_cast<uint8_t>(color);
        uint8_t statusByte = 0xB0 | getChannel(); // Create status byte for Control Change message
        vector<uint8_t> msg = {statusByte, getId(), value}; // Create MIDI message
        sendMidi(&msg); // Send the MIDI message
    }
};

class TwoColorButtonControl : public ButtonControl {
public:
    enum class Colors {
        OFF = 0, // OFF state
        COLOR1 = 1, // ON state
        COLOR2 = 2, // FLASH state

    };
    TwoColorButtonControl(uint8_t controlId, const std::string &label = "", bool active = true)
        : ButtonControl(Control::Type::NOTE, 0x00, controlId, label, active) {
        this->onValue.connect([this](ShortMessage& msg) {
            if (msg.isNoteOn()) {
                onPressed(); // Emit signal when button is pressed
            } else if (msg.isNoteOff()) {
                onReleased(); // Emit signal when button is released
            }
        });
    }
    void sendColor(TwoColorButtonControl::Colors color) {
        uint8_t value = static_cast<uint8_t>(color);
        uint8_t statusByte = 0xB0 | getChannel(); // Create status byte for Control Change message
        vector<uint8_t> msg = {statusByte, getId(), value}; // Create MIDI message
        sendMidi(&msg); // Send the MIDI message
    }
};