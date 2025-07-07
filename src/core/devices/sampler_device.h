#pragma once
#include "LabSound/LabSound.h"
#include "sigslot/signal.hpp"
#include "audio/choc_MIDI.h"
#include <iostream>
#include <string>
#include "device_base.h"
#include "util.h"
using std::string;

class SamplerDevice : public DeviceBase {
public:
    uuids::uuid id = generateUUID(); // Unique identifier for the sampler device
    SamplerDevice(std::shared_ptr<lab::AudioContext> &audioContext)
        : DeviceBase(audioContext)
    {
        // Bind the file path parameter to the onValueChanged signal
        _samplerNode = std::make_shared<lab::SampledAudioNode>(*audioContext.get());
        _samplerNode->schedule(0.0f); // Schedule the node to start immediately
        _audioContext->connect(output, _samplerNode, 0, 0); // Connect the output to the sampler node
        _audioContext->connect(audioContext->destinationNode(), output, 0, 0);
        _audioContext->synchronizeConnections();

    }

    void serialize() override {
        // Implement serialization logic if needed
    }
    void deserialize() override {
        // Implement deserialization logic if needed
    }

    void midiInput(choc::midi::ShortMessage &msg)  {
        if(msg.isNoteOn()) {
            if (_samplerNode) {
                _samplerNode->start(0.0f);
            }
        } else if (msg.isNoteOff()) {
            if (_samplerNode) {
                _samplerNode->stop(0.0f); // Stop playback on note off
            }
        }
    }

    void setFilePath(const std::string &filePath) {
        if (!_samplerNode) {
          _samplerNode =
              std::make_shared<lab::SampledAudioNode>(*_audioContext.get());
        }
        _audioBus =
            lab::MakeBusFromFile(filePath, false, _audioContext->sampleRate());
        if (_samplerNode && _audioBus) {
          _samplerNode->setBus(_audioBus);
        }
    }

protected:
    
    std::string _name = "Sampler";
    
    std::shared_ptr<lab::SampledAudioNode> _samplerNode; // Sampler node
    std::shared_ptr<lab::AudioBus> _audioBus; // Audio bus

};