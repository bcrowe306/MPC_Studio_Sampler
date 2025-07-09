#pragma once
#include "midi_event.h"
#include <vector>

using std::vector;

class MidiClip {
public:
    sigslot::signal<> onClipChanged; // Signal emitted when the clip changes
    int id;
    vector<MidiEvent> events; // List of MIDI events in the clip
    int loopPoint = 0;
    bool enabled = true; // Whether the clip is enabled
    
    // Copy constructor
    MidiClip(const MidiClip& other)
        : id(other.id),
          events(other.events),
          loopPoint(other.loopPoint),
          enabled(other.enabled),
          _midiEventId(other._midiEventId)
    {
        // Note: onClipChanged signal is not copied
    }

    // Move constructor
    MidiClip(MidiClip&& other) noexcept
        : id(other.id),
          events(std::move(other.events)),
          loopPoint(other.loopPoint),
          enabled(other.enabled),
          _midiEventId(other._midiEventId)
    {
        // Note: onClipChanged signal is not moved
    }
    MidiClip() = default; // Default constructor
    MidiClip(int id) : id(id) {
        // Constructor initializes the clip with a unique ID
        events.reserve(50);
    }
    ~MidiClip() = default;

    void addEvent(MidiEvent event) {
        event.id = _midiEventId++; // Assign a unique ID to the event
        events.push_back(std::move(event)); // Add a new MIDI event to the clip
        onClipChanged(); // Emit signal when the clip changes
    }

    void removeEvent(int eventId) {
        // Remove an event by its unique ID
        // TODO: may produce segment fault is modifying while reading. May need to implement a removal queue to process removals after iterating in between ticks
        auto it = std::remove_if(events.begin(), events.end(),
                                 [eventId](const MidiEvent &e) { return e.id == eventId; });
        if (it != events.end()) {
            events.erase(it, events.end()); // Erase the removed events
            onClipChanged(); // Emit signal when the clip changes
        }
    }

    void setLoopPoint(int newLoopPoint) {
        // Set the loop point for the clip
        loopPoint = newLoopPoint;
        onClipChanged(); // Emit signal when the loop point changes
    }


protected:
    int _midiEventId = 0; // Unique ID for MIDI events in this clip

};