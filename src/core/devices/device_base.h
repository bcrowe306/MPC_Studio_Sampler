#pragma once
#include "LabSound/LabSound.h"
#include <future>
#include <string>
#include "LabSound/core/AnalyserNode.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/PowerMonitorNode.h"
#include "sigslot/signal.hpp"
#include <memory>
#include "core/parameter.h"

using std::string;
using std::shared_ptr;
using std::make_shared;
using lab::PowerMonitorNode;
using lab::AudioContext;

class DeviceBase {
    public:
        
        DeviceBase(shared_ptr<AudioContext> audioContext)
            : _audioContext(audioContext) 
        {
               output = make_shared<lab::AnalyserNode>(*_audioContext.get()); // Initialize output with AnalyserNode
        }
        virtual ~DeviceBase() = default;
        std::shared_ptr<lab::AnalyserNode> output; // Power monitor for device
        virtual void serialize() = 0;
        virtual void deserialize() = 0;
        const std::string &getName() const {
            return _name;
        };
    
    
    protected:
        shared_ptr<AudioContext> _audioContext; // Audio context for the device
        std::string _name;
        std::string _type = "device"; // Default type for devices
        StringParameter _displayName = StringParameter("display_name", ""); // Display name parameter

};