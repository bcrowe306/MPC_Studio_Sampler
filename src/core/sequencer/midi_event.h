#pragma once
#include "audio/choc_MIDI.h"
#include "sigslot/signal.hpp"

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
    bool enabled = true; // Whether the event is enabled
    EventType type;
    int startTick;
    int duration = 0; // Duration in ticks for NoteOn events
    int pitch;
    int velocity = 0; // Velocity for NoteOn events
    int value = 0; // Value for ControlChange events
    ShortMessage startEvent;
    ShortMessage endEvent; // For NoteOff events  

    MidiEvent(int startTick, ShortMessage startEvent)
        : startTick(startTick), startEvent(startEvent), endEvent(startEvent) {
        _determineEventType();
        
        if (startEvent.isController()) {
            value = startEvent.getControllerValue();
            duration = 1;
        } else if (startEvent.isNoteOn()) {
            duration = 0; // Duration is not applicable for NoteOn events
            pitch = startEvent.getNoteNumber();
            velocity = startEvent.getVelocity();
        } else if (startEvent.isNoteOff()) {
            pitch = startEvent.getNoteNumber();
            duration = 0; // Duration is not applicable for NoteOff events
        } else if (startEvent.isProgramChange()) {
            pitch = startEvent.getProgramChangeNumber();
        } else if (startEvent.getAfterTouchValue()) {
            velocity = startEvent.getAfterTouchValue();
        } else if (startEvent.isPitchWheel()) {
            velocity = startEvent.getPitchWheelValue();
        }
    }

    MidiEvent(int startTick, int endTick, ShortMessage startEvent, ShortMessage endEvent)
        : startTick(startTick), startEvent(startEvent), endEvent(endEvent) {
        _determineEventType();
        if (startEvent.isNoteOn()) {
            duration = endTick - startTick; // Calculate duration for NoteOn events
            pitch = startEvent.getNoteNumber();
            velocity = startEvent.getVelocity();
        }
        if (startEvent.isController()) {
            value = startEvent.getControllerValue();
        }
    }

    void setPitch(int newPitch) {
        pitch = std::clamp(newPitch, 0, 127); // Ensure pitch is within MIDI range
        if (!startEvent.isNoteOn() && !startEvent.isNoteOff()) {
            return; // Only update pitch for NoteOn and NoteOff events
        }
        startEvent.data[1] = pitch; // Update pitch in the start event
        endEvent.data[1] = pitch; // Update pitch in the end event
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
        if (startEvent.isNoteOn()) {
            startEvent.data[2] = velocity; // Update velocity in the start event
        }
    }

    

protected:
  void _determineEventType() {
        if (startEvent.isNoteOn()) {
          type = EventType::Note;
          pitch = startEvent.getNoteNumber();
          velocity = startEvent.getVelocity();
        } else if (startEvent.isNoteOff()) {
          type = EventType::Note;
          pitch = startEvent.getNoteNumber();
        } else if (startEvent.isController()) {
          type = EventType::ControlChange;
          pitch = startEvent.getControllerNumber();
          velocity = startEvent.getControllerValue();
        } else if (startEvent.isProgramChange()) {
          type = EventType::ProgramChange;
          pitch = startEvent.getProgramChangeNumber();
        } else if (startEvent.getAfterTouchValue()) {
          type = EventType::ChannelAftertouch;
          velocity = startEvent.getAfterTouchValue();
        } else if (startEvent.isPitchWheel()) {
          type = EventType::PitchBend;
          velocity = startEvent.getPitchWheelValue();
        }
    }

};