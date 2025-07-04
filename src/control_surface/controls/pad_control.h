#include "audio/choc_MIDI.h"
#include "control.h"
#include "sigslot/signal.hpp"
#include <_types/_uint8_t.h>

class PadControl : public Control {
public:

    // Signal emitted when the pad is pressed
    sigslot::signal<> onPressed;
    // Signal emitted when the pad is released
    sigslot::signal<> onReleased;

    sigslot::signal<ShortMessage&> onPlay; // Signal for MIDI value changes

    PadControl(uint8_t channel,int padId, int padIndex, const std::string& padName)
        : Control(Control::Type::NOTE, channel, padId, padName), padIndex(padIndex) 
    {
        
        this->onValue.connect([this](ShortMessage& msg) {
            if (msg.isNoteOn()) {
                onPressed(); // Emit signal when button is pressed
            } else if (msg.isNoteOff()) {
                onReleased(); // Emit signal when button is released
            }
        });
        sigslot::connect(onValue, onPlay); // Connect the onValue signal to the onPlay signal
    }

    

  private:
    int padIndex;
};