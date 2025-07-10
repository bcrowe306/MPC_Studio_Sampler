#include <iostream>
#include <memory>
#include <functional>
#include "core/constants.h"
#include "core/property.h"
#include "core/timing.h"
#include "audio/choc_MIDI.h"
#include "sigslot/signal.hpp"
#include <string>
#include <unordered_map>
#include <array>
using std::shared_ptr;
using choc::midi::ShortMessage;

inline static unordered_map<int, string> kNoteRepeatRateNames = {
    {1, "1/4"},
    {2, "1/2"},
    {3, "1/2t"},
    {4, "1/4"},
    {6, "1/4t"},
    {8, "1/8"},
    {12, "1/8t"},
    {16, "1/16"},
    {24, "1/16t"},
    {32, "1/32"},
    {48, "1/32t"},
    {64, "1/64"},
};

inline static std::array<int, 12> repeatRates = {
    1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64
}; // List of available note repeat rates

class NoteRepeat {
public:
    sigslot::signal<int> repeatRate; // Signal for repeat rate changes
    Property<bool> enabled = Property<bool>(false); // Enable/disable note repeat functionality
    shared_ptr<Timer> timer;  // Timer for note repeat functionality
    std::function<void(ShortMessage &)> sendMidi;

    // Constructor
    NoteRepeat(shared_ptr<Timer> timer) : timer(timer) {
        repeatRate(_repeatRate); 
        setRepeatRate(_repeatRate);
    } 


    // Default destructor
    ~NoteRepeat() = default;

    void setRepeatRate(int rate) 
    { 
        // Make sure the rate is in the repeatRates array
        if (std::find(repeatRates.begin(), repeatRates.end(), rate) == repeatRates.end()) {
            std::cerr << "Invalid repeat rate: " << rate << std::endl;
            return; // Exit if the rate is not valid
        }
        _repeatRate = rate; // Set the repeat rate
        _ticksPerNote = kTPQN / (rate / 4); 
        repeatRate(_repeatRate); // Emit the repeat rate signal
    }

    void toggleEnabled() {
        enabled.set(!enabled.get()); // Toggle the enabled state
    }

    std::array<string, 12> getRepeatRateNames() {
        std::array<string, 12> names;
        for (size_t i = 0; i < repeatRates.size(); ++i) {
            names[i] = kNoteRepeatRateNames[repeatRates[i]];
        }
        return names;
    }

    std::array<int, 12> getRepeatRates() {
        return repeatRates;
    }

    std::unordered_map<int, std::string> getRepeatRateMap() {
        std::unordered_map<int, std::string> rateMap;
        for (size_t i = 0; i < repeatRates.size(); ++i) {
            rateMap[repeatRates[i]] = kNoteRepeatRateNames[repeatRates[i]];
        }
        return rateMap;
    }

    void onTick(int tick) {

        
        if (_tickCounter % _ticksPerNote / 2 == 0) {
            
            if(_tickCounter % _ticksPerNote == 0) {
                
                if( _isNoteOn && enabled.get()) {
                    std::cout << "Note Repeat Tick: " << _tickCounter << std::endl;
                    if(sendMidi){
                        sendMidi(_lastNoteOnMsg); 
                    }
                }
            }else {
                if (_isNoteOn && enabled.get()) {
                    if (sendMidi) {
                        sendMidi(_lastNoteOffMsg);
                    }
                }
            }

            
        }
        _tickCounter++; // Increment the tick counter
    }
    void midiInput(choc::midi::ShortMessage &msg) {
        if (msg.isNoteOn()) {
            
            // Handle note on messages for note repeat
            _isNoteOn = true;
            setLastNoteMsg(msg);
            
        } else if (msg.isNoteOff()) {
        
            // Handle note off messages for note repeat
            _isNoteOn = false;
        }
    }

    void setLastNoteMsg(ShortMessage msg){
        // Set the last note message for note repeat
        _lastNoteOnMsg = msg; // Store the last note on message
        auto offStatusByte = 0x80 | (msg.getChannel0to15() & 0x0F); // Create note off status byte
        _lastNoteOffMsg = ShortMessage(offStatusByte, msg.getNoteNumber(), 0); // Create the last note off message
    }
    
    private:
      int _repeatRate = 16; // Rate of note repeat
      int _ticksPerNote;
      
      bool _isNoteOn = false;
      ShortMessage _lastNoteOnMsg;  // Last note on message
      ShortMessage _lastNoteOffMsg; // Last note off message
      uint64_t _tickCounter = 0;    // Timestamp of the last note on event
};