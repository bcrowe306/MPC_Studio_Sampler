#include "audio/choc_MIDI.h"
#include "control.h"
#include "sigslot/signal.hpp"
#include <_types/_uint8_t.h>

// class RGBColorDef:
//     OFF = 0
//     BLACK = 0
//     class RED:
//         DIM = 8
//         HALF = 24
//         FULL = 56
//     class YELLOW:
//         DIM = 10
//         HALF = 28
//         FULL = 63
//     class ORANGE:
//         DIM = 17
//         HALF = 34
//         FULL = 59
//     class GREEN:
//         DIM = 1
//         HALF = 3
//         FULL = 7



class PadControl : public Control {
public:
    enum PAD_COLOR {
        OFF = 0,
        BLACK = 0,
        RED_DIM = 8,
        RED_HALF = 24,
        RED_FULL = 56,
        YELLOW_DIM = 10,
        YELLOW_HALF = 28,
        YELLOW_FULL = 63,
        ORANGE_DIM = 17,
        ORANGE_HALF = 34,
        ORANGE_FULL = 59,
        GREEN_DIM = 1,
        GREEN_HALF = 3,
        GREEN_FULL = 7
    };
    // Signal emitted when the pad is pressed
    sigslot::signal<> onPressed;
    // Signal emitted when the pad is released
    sigslot::signal<> onReleased;

    sigslot::signal<ShortMessage&> onPlay; // Signal for MIDI value changes
    bool isPressed = false; // State to track if the pad is currently pressed
    PadControl(uint8_t channel,int padId, int padIndex, const std::string& padName)
        : Control(Control::Type::NOTE, channel, padId, padName), padIndex(padIndex) 
    {
        
        this->onValue.connect([this](ShortMessage& msg) {
            if (msg.isNoteOn()) {
                onPressed(); // Emit signal when button is pressed
                isPressed = true; // Update pressed state
            } else if (msg.isNoteOff()) {
                onReleased(); // Emit signal when button is released
                isPressed = false; // Update pressed state
            }
            onPlay(msg); // Emit the play signal with the MIDI message
        });
    }

    void sendColor(PAD_COLOR color) {
        uint8_t value = static_cast<uint8_t>(color);
        uint8_t statusByte = 0xB0 | getChannel(); // Create status byte for Control Change message
        vector<uint8_t> msg = {statusByte, getId(), value}; // Create MIDI message
        sendMidi(&msg); // Send the MIDI message
    }

    

  private:
    int padIndex;
};