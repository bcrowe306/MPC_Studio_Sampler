#pragma once
#include "midi_event.h"
#include <iostream>
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
    MidiClip(int id) : id(id) {
        // Constructor initializes the clip with a unique ID
        events.reserve(50);
    }
    ~MidiClip() = default;

    void addEvent(MidiEvent event) {
        // Add event to the clip. If an event with the same pitch and startTick already exists, replace it
        auto it = std::find_if(events.begin(), events.end(),
                               [&event](const MidiEvent &e) {
                                   return e.pitch == event.pitch && e.startTick == event.startTick;
                               });
        if (it != events.end()) {
            // If an existing event is found, update it with new duration, velocity, and ID
            std::cout << "Updating existing event with ID: " << it->id << std::endl;
            it->duration = event.duration;
            it->velocity = event.velocity;
            it->id = event.id;
            onClipChanged(); // Emit signal when the clip changes
            return;
        }
        // If no existing event is found, add the new event
        if (event.startTick < 0) {
            event.startTick = 0; // Ensure startTick is not negative
        }

        // TODO: Need a better method for thread safety here. possible queue on an event thread and check atomic flag for changes
        std::lock_guard<std::mutex> lock(_eventsMutex);
        events.push_back(std::move(event)); // Add a new MIDI event to the clip
        onClipChanged(); // Emit signal when the clip changes
    }

    void removeEvent(int eventId) {
        // Remove an event by its unique ID
        // TODO: may produce segment fault is modifying while reading. May need to implement a removal queue to process removals after iterating in between ticks
        // For now we will just lock the mutex..
        auto it = std::remove_if(events.begin(), events.end(),
                                 [eventId](const MidiEvent &e) { return e.id == eventId; });
        if (it != events.end()) {
            std::lock_guard<std::mutex> lock(_eventsMutex);
            events.erase(it, events.end()); // Erase the removed events
            onClipChanged(); // Emit signal when the clip changes
        }
    }

    void setLoopPoint(int newLoopPoint) {
        // Set the loop point for the clip
        loopPoint = newLoopPoint;
        onClipChanged(); // Emit signal when the loop point changes
    }

    int getNewEventId() {
        // Generate a new unique ID for a MIDI event
        return _midiEventId++;
    }

protected:
    std::mutex _eventsMutex; // Mutex to protect access to events
    int _midiEventId = 0; // Unique ID for MIDI events in this clip

};