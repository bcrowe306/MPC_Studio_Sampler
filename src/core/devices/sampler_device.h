#pragma once
#include "LabSound/LabSound.h"
#include "core/parameter.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <string>
#include "device_base.h"
#include "util.h"
using std::string;

class SamplerDevice : public DeviceBase {
public:
    uuids::uuid id = generateUUID(); // Unique identifier for the sampler device
    SamplerDevice(std::shared_ptr<lab::AudioContext> &audioContext, std::string file_path = "")
        : DeviceBase(audioContext)
    {
        // Bind the file path parameter to the onValueChanged signal
        _samplerNode = std::make_shared<lab::SampledAudioNode>(*audioContext.get());
        _samplerNode->schedule(0.0f); // Schedule the node to start immediately
        _audioContext->connect(output, _samplerNode, 0, 0); // Connect the output to the sampler node
        _audioContext->connect(audioContext->destinationNode(), output, 0, 0);
        _audioContext->synchronizeConnections();
        _filePath.onValueChanged.connect(std::bind(&SamplerDevice::_onFilePathChanged, this, std::placeholders::_1));

        if(!file_path.empty()) {
            _filePath.setValue(file_path);
            _displayName.setValue(file_path);
        }

    }

    void serialize() override {
        // Implement serialization logic if needed
    }
    void deserialize() override {
        // Implement deserialization logic if needed
    }

    void trigger() {
        _samplerNode->start(0.0f); // Start playback immediately
}

protected:
    void _onFilePathChanged(const std::string &filePath) {
        if(!_samplerNode) {
            _samplerNode = std::make_shared<lab::SampledAudioNode>(*_audioContext.get());
        }
        _audioBus = lab::MakeBusFromFile(filePath, false, _audioContext->sampleRate());
        if(_samplerNode && _audioBus) {
            _samplerNode->setBus(_audioBus);
        }
    }
    std::string _name = "Sampler";
    StringParameter _filePath = StringParameter("file_path", "file_path"); // File path parameter
    
    std::shared_ptr<lab::SampledAudioNode> _samplerNode; // Sampler node
    std::shared_ptr<lab::AudioBus> _audioBus; // Audio bus

};