#pragma once
#include "sigslot/signal.hpp"
#include <_types/_uint8_t.h>
#include <_types/_uint8_t.h>
#include <iostream>
#include <rtmidi/RtMidi.h>
#include <sys/types.h>
#include "audio/choc_MIDI.h"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::vector;


using choc::midi::ShortMessage;

class Control {
    
public:
    enum class Type { 
        NOTE,
        CC,
        SYSEX,
        PROGRAM_CHANGE,
        PITCH_BEND,
        AFTERTOUCH
    };
    sigslot::signal<uint8_t> onValue; // Signal for control value changes
    sigslot::signal<float> onUnitValue; // Signal for control value changes in float format
    sigslot::signal<bool> onActiveChanged; // Signal for active state changes


    Control(Type controlType, uint8_t controlChannel, uint8_t controlId, const std::string &label = "", bool active = true);
    virtual ~Control() = default;
    void midiInput(ShortMessage &msg);

    uint8_t getValue() const ;
    Type getType() const ;
    uint8_t getChannel() const ;
    uint8_t getId() const ;
    std::string getLabel() const ;
    bool isActive() const ;
    void setActive(bool active);
    void sendMidi(vector<uint8_t> * value);
    void sendMidi(ShortMessage &msg); 
    void setMidiOutPort(shared_ptr<RtMidiOut> midiOut);

protected:
    bool _active = true; // Control is active by default
    Type _controlType; // Type of control (e.g., NOTE, CC, etc.)
    uint8_t _controlChannel; // MIDI channel for the control
    uint8_t _controlId; // Unique identifier for the control
    uint8_t _value; // Current value of the control
    std::string _label; // Label for the control
    shared_ptr<RtMidiOut> _midiOut; // MIDI output object for sending messages
    bool _matchType(ShortMessage &msg);
    bool _matchId(ShortMessage &msg) ;
    uint8_t _getValue(ShortMessage &msg);
};