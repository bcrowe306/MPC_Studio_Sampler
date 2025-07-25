#pragma once
#include "LabSound/LabSound.h"
#include <future>
#include <string>
#include "LabSound/core/AnalyserNode.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/PowerMonitorNode.h"
#include "core/value_receiver.h"
#include "sigslot/signal.hpp"
#include <memory>
#include "audio/choc_MIDI.h"
#include "device_types.h"
#include "core/serializable.h"

using std::string;
using std::shared_ptr;
using std::make_shared;
using lab::AudioContext;

class DeviceBase : public Serializable {
    public:
        
        DeviceBase(shared_ptr<AudioContext> audioContext) : _audioContext(audioContext) 
        { output = make_shared<lab::AnalyserNode>(*_audioContext.get());  }
        virtual ~DeviceBase() = default;
        
        // Signal emitted when the device changes
        sigslot::signal<> onDeviceChanged; 
        
        // Power monitor for device
        std::shared_ptr<lab::AnalyserNode> output; 
        
        // List of parameters for the device
        vector<shared_ptr<ValueReceiverBase>> parameters; 

        const std::string &getName() const { return _name; }
        DeviceType getType() const { return _type; }
        
       
        virtual void midiInput(choc::midi::ShortMessage &msg) = 0;
        virtual void stopAllNotes() {}
        

    protected:
        shared_ptr<AudioContext> _audioContext; // Audio context for the device
        std::string _name;
        DeviceType _type = DeviceType::Undefined; // Default type for devices

};