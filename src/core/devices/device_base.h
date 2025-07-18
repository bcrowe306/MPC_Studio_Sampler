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

using std::string;
using std::shared_ptr;
using std::make_shared;
using lab::AudioContext;

class DeviceBase {
    public:
        
        DeviceBase(shared_ptr<AudioContext> audioContext)
            : _audioContext(audioContext) 
        {
               output = make_shared<lab::AnalyserNode>(*_audioContext.get()); // Initialize output with AnalyserNode
        }
        virtual ~DeviceBase() = default;
        sigslot::signal<> onDeviceChanged; // Signal emitted when the device changes
        std::shared_ptr<lab::AnalyserNode> output; // Power monitor for device
        vector<shared_ptr<ValueReceiverBase>> parameters; // List of parameters for the device
        virtual void serialize() = 0;
        virtual void deserialize() = 0;
        const std::string &getName() const {
            return _name;
        };
        virtual void midiInput(choc::midi::ShortMessage &msg) = 0;
        virtual void stopAllNotes() {
            
        }
        virtual void changeParameterValue(int index, int offset, bool isFine){

        }

    protected:
        shared_ptr<AudioContext> _audioContext; // Audio context for the device
        std::string _name;
        DeviceType _type = DeviceType::Undefined; // Default type for devices

};