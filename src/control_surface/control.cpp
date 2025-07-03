#include "control.h"
#include "util.h"

Control::Control(Type controlType, uint8_t controlChannel, uint8_t controlId, const std::string &label, bool active)
    : _controlType(controlType), _controlChannel(controlChannel), _controlId(controlId), _label(label), _value(0), _active(active) {
}

void Control::midiInput(ShortMessage &msg) {
    if (_matchType(msg) && msg.getChannel0to15() == _controlChannel && _matchId(msg)) {
        _value = _getValue(msg); // Get the value from the message
        if(_active){
            onValue(_value); // Emit signal with the new value
            onUnitValue(midiValueToFloat(_value)); // Emit signal with the new value as float
        }
    }
}

uint8_t Control::getValue() const {
    return _value; // Return the current value of the control
}

Control::Type Control::getType() const {
    return _controlType; // Return the type of control
}

uint8_t Control::getChannel() const {
    return _controlChannel; // Return the MIDI channel for the control
}

uint8_t Control::getId() const {
    return _controlId; // Return the unique identifier for the control
}

std::string Control::getLabel() const {
    return _label; // Return the label for the control
}

bool Control::isActive() const {
    return _active; // Return whether the control is active
}   

void Control::setActive(bool active) {
    _active = active; // Set the active state of the control
    onActiveChanged(_active); // Emit signal for active state changes
}

void Control::sendMidi(vector<uint8_t> *msg) {
    if (_midiOut) {
        try {
            _midiOut->sendMessage(msg);
        } catch (RtMidiError &error) {
            std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        }
    } else {
        std::cerr << "MIDI output not initialized.\n";
    }
}

void Control::sendMidi(ShortMessage &msg) {
    if (_midiOut) {
        auto raw = msg.data;
        vector<uint8_t> msg = {raw[0], raw[1], raw[2]};
        try {
            _midiOut->sendMessage(&msg);
        } catch (RtMidiError &error) {
            std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        }
    } else {
        std::cerr << "MIDI output not initialized.\n";
    }
}

void Control::setMidiOutPort(shared_ptr<RtMidiOut> midiOut) {
    _midiOut = midiOut; // Set the MIDI output port for sending messages
}

bool Control::_matchType(ShortMessage &msg) 
{
    // Check if the message type matches the control type
    switch (_controlType) {
    case Type::NOTE:
        return msg.isNoteOn() || msg.isNoteOff();
    case Type::CC:
        return msg.isController();
    case Type::SYSEX:
        return false; // SYSEX messages are handled separately
    case Type::PROGRAM_CHANGE:
        return msg.isProgramChange();
    case Type::PITCH_BEND:
        return msg.isPitchWheel();
    case Type::AFTERTOUCH:
        return msg.isAftertouch();
    default:
        return false;
    }
}

bool Control::_matchId(ShortMessage &msg) {
    // Check if the message ID matches the control ID
    switch (_controlType) {
    case Type::NOTE:
            return msg.getNoteNumber() == _controlId;
    case Type::CC:
            return msg.getControllerNumber() == _controlId;
    case Type::PROGRAM_CHANGE:
            return msg.getProgramChangeNumber() == _controlId;
    case Type::PITCH_BEND:
            return true; // Pitch bend does not have a specific ID
    case Type::AFTERTOUCH:
            return true; // Aftertouch does not have a specific ID
    default:
            return false;
    }
}

uint8_t Control::_getValue(ShortMessage &msg) {
    // Get the value from the message based on the control type
    switch (_controlType) {
    case Type::NOTE:
            return msg.getVelocity();
    case Type::CC:
            return msg.getControllerValue();
    case Type::PROGRAM_CHANGE:
            return msg.getProgramChangeNumber();
    case Type::PITCH_BEND:
            return msg.getPitchWheelValue();
    case Type::AFTERTOUCH:
            return msg.getAfterTouchValue();
    default:
            return 0;
    }
}

