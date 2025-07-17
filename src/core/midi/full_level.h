#include "audio/choc_MIDI.h"
#include "core/constants.h"
#include "core/property.h"
#include "sigslot/signal.hpp"
#include <string>
#include <unordered_map>

class FullLevel {

public:
    enum class VelocityLevels {
        LOW,
        MEDIUM,
        HIGH,
        FULL,
    };
    sigslot::signal<int> levelChanged; // Signal for full level state changes
    Property<bool> enabled = Property<bool>(false); // Enable/disable full level functionality

    FullLevel() {
        
    }

    void processVelocity(choc::midi::ShortMessage &msg){
        auto newMsg = choc::midi::ShortMIDIMessageStorage();
        if(!enabled.get()) {
            return; // If full level is not enabled, do nothing
        }
        if(msg.isNoteOn()){
            newMsg = choc::midi::ShortMIDIMessageStorage(msg.data()[0], msg.data()[1], velocityMap[currentLevel]); // Create a new MIDI message
        }
    }

    void toggleEnabled() {
        enabled.set(!enabled.get()); // Toggle the enabled state
    }

    void setLevel(VelocityLevels level) {
        if (currentLevel != level) {
            currentLevel = level;
            levelChanged(static_cast<int>(currentLevel)); // Emit the level changed signal
        }
    }

    void setLevel(int level) {
        // Convert integer level to VelocityLevels enum
        switch (level) {
            case 0:
                setLevel(VelocityLevels::LOW);
                break;
            case 1:
                setLevel(VelocityLevels::MEDIUM);
                break;
            case 2:
                setLevel(VelocityLevels::HIGH);
                break;
            case 3:
                setLevel(VelocityLevels::FULL);
                break;
            default:
                setLevel(VelocityLevels::FULL); // Default to full level if invalid
        }
        levelChanged(static_cast<int>(currentLevel)); // Emit the level changed signal
    }

    void cycleLevel() {
        int nextLevel = static_cast<int>(currentLevel) + 1;
        if (nextLevel > static_cast<int>(VelocityLevels::FULL)) {
            nextLevel = static_cast<int>(VelocityLevels::LOW);
        }
        setLevel(static_cast<VelocityLevels>(nextLevel));
    }

    ~FullLevel() = default;


    bool isEnabled() const {
        return enabled.get();
    }

private:
    VelocityLevels currentLevel = VelocityLevels::FULL; // Default to full level
    std::unordered_map<VelocityLevels, int> velocityMap = {
        {VelocityLevels::LOW, 64},
        {VelocityLevels::MEDIUM, 96},
        {VelocityLevels::HIGH, 127},
        {VelocityLevels::FULL, 127}
    }; // Map of velocity levels to MIDI values
};