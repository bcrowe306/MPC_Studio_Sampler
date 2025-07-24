#pragma once
#include "audio/choc_MIDI.h"
#include "sigslot/signal.hpp"
#include <Security/Security.h>
#include <_types/_uint8_t.h>

using choc::midi::ShortMessage;


class MidiEvent {
public:
    enum class EventType {
        Note,
        ControlChange,
        ProgramChange,
        ChannelAftertouch,
        PitchBend,
    };
    int id;
    int trackIndex = -1; // Track index for the event, -1 if not applicable
    bool enabled = true; // Whether the event is enabled
    int startTick;
    int duration = 0; // Duration in ticks for NoteOn events
    int channel = 0; // MIDI channel for the event
    EventType type;
    int pitch;
    int velocity = 0; // Velocity for NoteOn events
    int value = 0; // Value for ControlChange events

    MidiEvent(int id, int startTick, ShortMessage startEvent)
        : id(id), startTick(startTick) {
        _determineEventType(startEvent);
        
        if (startEvent.isController()) {
            pitch = startEvent.getControllerNumber();
            value = startEvent.getControllerValue();
            duration = 1;
        } else if (startEvent.isProgramChange()) {
            pitch = startEvent.getProgramChangeNumber();
        } else if (startEvent.getAfterTouchValue()) {
            velocity = startEvent.getAfterTouchValue();
        } else if (startEvent.isPitchWheel()) {
            velocity = startEvent.getPitchWheelValue();
        }
    }

    MidiEvent(int id, int startTick, int endTick, ShortMessage startEvent, ShortMessage endEvent)
        : id(id), startTick(startTick) {
        _determineEventType(startEvent);
        if (startEvent.isNoteOn()) {
            duration = endTick - startTick; // Calculate duration for NoteOn events
            pitch = startEvent.getNoteNumber();
            velocity = startEvent.getVelocity();
        }
        if (startEvent.isController()) {
            pitch = startEvent.getControllerNumber();
            value = startEvent.getControllerValue();
        }
    }

    void setPitch(int newPitch) {
        pitch = std::clamp(newPitch, 0, 127); // Ensure pitch is within MIDI range
    }

    ShortMessage generateMidiData(bool isNoteOn = true) const {
        uint8_t data[3];
        switch(type) {
            case EventType::Note:
                if(isNoteOn) {
                    data[0] = 0x90 | (channel & 0x0F); // Note On message
                } else {
                    data[0] = 0x80 | (channel & 0x0F); // Note Off message
                }
                data[1] = pitch;
                data[2] = velocity;
                break;
            case EventType::ControlChange:
                data[0] = 0xB0 | (channel & 0x0F); // Control Change message
                data[1] = pitch; // Controller number
                data[2] = value; // Controller value
                break;
            case EventType::ProgramChange:
                data[0] = 0xC0 | (channel & 0x0F); // Program Change message
                data[1] = pitch; // Program number
                data[2] = 0; // No third byte for Program Change
                break;
            case EventType::ChannelAftertouch:
                data[0] = 0xD0 | (channel & 0x0F); // Channel Aftertouch message
                data[1] = velocity; // Aftertouch value
                data[2] = 0; // No third byte for Channel Aftertouch
                break;
            case EventType::PitchBend:
                data[0] = 0xE0 | (channel & 0x0F); // Pitch Bend message
                data[1] = velocity & 0x7F; // LSB of Pitch Bend value
                data[2] = (velocity >> 7) & 0x7F; // MSB of Pitch Bend value
                break;
        }
        return ShortMessage(data, 3);
    }

    void setStart(int newStartTick) {
        // Moves the note on the timeline without changing its duration
        int delta = newStartTick - startTick;
        startTick += delta; // Update start tick
        duration += delta; // Adjust duration accordingly
    }

    void setEnd(int newEndTick) {
        // Moves the note on the timeline and adjusts its duration
        int delta = newEndTick - (startTick + duration);
        duration += delta; // Adjust duration accordingly
    }

    void setDuractionTicks(int newDuration) {
        // Sets the duration of the event in ticks
        if (newDuration < 0) {
            newDuration = 0; // Ensure duration is non-negative
        }
        duration = newDuration;
    }

    void setEnabled(bool isEnabled) {
        enabled = isEnabled; // Set the enabled state
    }

    void toggleEnabled() {
        enabled = !enabled; // Toggle the enabled state
    }

    void setVelocity(int newVelocity) {
        velocity = std::clamp(newVelocity, 0, 127); // Ensure velocity is within MIDI range
    }

    

protected:
  void _determineEventType(ShortMessage &msg) {
        // Determine the type of MIDI event based on the message
        if (msg.isNoteOn()) {
            type = EventType::Note;
        } else if (msg.isNoteOff()) {
            type = EventType::Note;
        } else if (msg.isController()) {
            type = EventType::ControlChange;
        } else if (msg.isProgramChange()) {
            type = EventType::ProgramChange;
        } else if (msg.isAftertouch()) {
            type = EventType::ChannelAftertouch;
        } else if (msg.isPitchWheel()) {
            type = EventType::PitchBend;
        }
        
    }

};