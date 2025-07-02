#include "audio/choc_MIDI.h"
#include "control.h"
#include "util.h"


class EncoderControl : public Control {
    public:
    sigslot::signal<> onIncrement; // Signal emitted when encoder is incremented
    sigslot::signal<> onDecrement; // Signal emitted when encoder is decremented
    sigslot::signal<int> onOffset; // Signal emitted with offset amount
    sigslot::signal<uint8_t> onOffsetUnit; // Signal emitted with the current encoder value

    EncoderControl(uint8_t controlChannel, uint8_t controlId, const std::string &label = "", bool active = true);
    
    virtual ~EncoderControl() = default;

    int getOffsetAmount(uint8_t &value) const ;
};