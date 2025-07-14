#include "audio/choc_MIDI.h"
#include "control.h"
#include "sigslot/signal.hpp"
#include "util.h"
#include <_types/_uint8_t.h>


class EncoderControl : public Control {
    public:
    sigslot::signal<> onIncrement; // Signal emitted when encoder is incremented
    sigslot::signal<> onDecrement; // Signal emitted when encoder is decremented
    sigslot::signal<int> onOffset; // Signal emitted with offset amount
    sigslot::signal<uint8_t> onOffsetUnit; // Signal emitted with the current encoder value

    sigslot::signal<int> onOffsetSlow;
    sigslot::signal<int> onIncrementSlow;
    sigslot::signal<int> onDecrementSlow;
    sigslot::signal<double> onOffsetSlowUnit; // Signal emitted with double offset amount
    

    EncoderControl(uint8_t controlChannel, uint8_t controlId, const std::string &label = "", bool active = true);
    
    virtual ~EncoderControl() = default;

    int getOffsetAmount(uint8_t value) const ;

    void genSlowOffset(int offsetAmount);
private:
    uint8_t incCount = 0;
    uint8_t _decCount = 0;
    uint8_t _slowOffsetAmount = 3;
};