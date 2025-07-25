#pragma once
#include "midi_event.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include "core/serializable.h"

using std::vector;

class MidiClip : public Serializable {
public:
    sigslot::signal<> onClipChanged; // Signal emitted when the clip changes
    int id;
    vector<MidiEvent> events; // List of MIDI events in the clip
    int loopPoint = 0;

    // Serialize the object to a YAML emitter
    void serialize(YAML::Emitter &out) override {
        out << YAML::Key << "id" << YAML::Value << id;
        out << YAML::Key << "loopPoint" << YAML::Value << loopPoint;
        out << YAML::Key << "enabled" << YAML::Value << enabled;
        out << YAML::Key << "events" << YAML::Value << YAML::BeginSeq;
        for (auto &event : events) {
            out << YAML::BeginMap;
            event.serialize(out); // Serialize each MIDI event
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    // Deserialize the object from a YAML node
    void deserialize(const YAML::Node &yaml) override {
        auto node = yaml;
        
        id = node["id"].as<int>();
        loopPoint = node["loopPoint"].as<int>();
        enabled = node["enabled"].as<bool>();
        if(node["events"] && node["events"].IsSequence()) {
            events.clear(); // Clear existing events
            for (auto eventNode : node["events"]) {
              MidiEvent event;
              event.deserialize(eventNode);
              events.push_back(std::move(event));
            }
        }
        
        if(!events.empty()) {
            std::cout << "MidiClip deserialized with " << events.size() << " events." << std::endl;
        }
        
    }

    bool isEmpty() const {
        // Check if the clip has no events
        return events.empty();
    }

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

    void doubleClipLengthAndEvents(int lengthInTicks) {
        // Double the length of the clip by adjusting the loop point
        vector<MidiEvent> newEvents;
        newEvents.reserve(events.size() ); // Reserve space for doubled events
        for(auto &event : events) {
            // Create a new event with the same properties but adjusted start and end ticks
            newEvents.emplace_back(
                getNewEventId(), 
                event.startTick + lengthInTicks, 
                event.startTick + event.duration + lengthInTicks,
                event.generateMidiData(),
                event.generateMidiData(false) // Generate end event data
            );
        }

        for(auto &event : newEvents) {
            addEvent(event); // Add the new event to the clip
        }
    }

    void addEvent(MidiEvent event) {
        // Add event to the clip. If an event with the same pitch and startTick already exists, replace it
        auto it = std::find_if(events.begin(), events.end(),
                               [&event](const MidiEvent &e) {
                                   return e.pitch == event.pitch && e.startTick == event.startTick && e.type == event.type && e.type == MidiEvent::EventType::Note;
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
        // TODO: may produce segment fault is modifying while reading. Currently a mutex and lock is used... BAD because read is happening in the audio thread!
        // Implement a better thread-safe method, maybe CAS or spinlock to check if another thread is reading/modifying
        // For now we will just lock the mutex..
        auto it = std::remove_if(events.begin(), events.end(),
                                 [eventId](const MidiEvent &e) { return e.id == eventId; });
        if (it != events.end()) {
            std::lock_guard<std::mutex> lock(_eventsMutex);
            events.erase(it, events.end()); // Erase the removed events
            onClipChanged(); // Emit signal when the clip changes
        }
    }

    void clear() {
        // Clear all events in the clip
        std::lock_guard<std::mutex> lock(_eventsMutex);
        events.clear(); 
        onClipChanged(); 
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
    void resetClip(){
        std::lock_guard<std::mutex> lock(_eventsMutex);
        events.clear();
        loopPoint = 0;
        _midiEventId = 0;
        onClipChanged();
    }


protected:
    std::mutex _eventsMutex; // Mutex to protect access to events
    int _midiEventId = 0; // Unique ID for MIDI events in this clip

};