#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "core/devices/sampler_device.h"
#include "meter_node.h"
#include "core/parameter.h"
#include <memory>
#include "util.h"

using std::shared_ptr;
using std::make_shared;

class Track {
public:

    shared_ptr<MeterNode> meterNode;
    shared_ptr<GainNode> volumeNode;
    shared_ptr<GainNode> muteNode;
    shared_ptr<StereoPannerNode> panNode;
    shared_ptr<AnalyserNode> input;
    shared_ptr<AnalyserNode> output;
    shared_ptr<AudioContext> context;

    sigslot::signal<bool> onIsEmpty; // Signal emitted when the track is empty or not
    sigslot::signal<> onSamplerDeviceChanged; // Signal emitted when the sampler device is changed

    // Parameters for the track
    uuids::uuid id;
    FloatParameter volume = FloatParameter("volume", 1.0f, 0.0f, 6.0f);
    FloatParameter pan = FloatParameter("pan", 0.0f, -1.0f, 1.0f);
    BoolParameter mute = BoolParameter("mute", false);
    StringParameter name = StringParameter("name", "Track");
    BoolParameter solo = BoolParameter("solo", false);
    
    Track(shared_ptr<AudioContext> ac) {
        id = generateUUID(); // Generate a unique ID for the track
        context = ac;
        meterNode = make_shared<MeterNode>(*context);
        volumeNode = make_shared<GainNode>(*context);
        muteNode = make_shared<GainNode>(*context);
        panNode = make_shared<StereoPannerNode>(*context);
        input = make_shared<AnalyserNode>(*context);
        output = make_shared<AnalyserNode>(*context);

        this->context->connect(panNode, input, 0, 0);
        this->context->connect(volumeNode, panNode, 0, 0);
        this->context->connect(meterNode, volumeNode, 0, 0);
        this->context->connect(output, meterNode, 0, 0);

    }

    void setSamplerDevice(shared_ptr<SamplerDevice> device) {
        samplerDevice = device;
        if (samplerDevice) {
            context->connect(input, samplerDevice->output, 0, 0);
            context->synchronizeConnections();
            _isEmpty = false; // Track is no longer empty after creating a sampler device
            onIsEmpty(false); 
        }
    }
    
    void createSamplerDevice(const std::string& filePath) {
        samplerDevice = make_shared<SamplerDevice>(context, filePath);
        context->connect(input, samplerDevice->output, 0, 0);
        context->synchronizeConnections();
        onSamplerDeviceChanged();
        _isEmpty = false; // Track is no longer empty after creating a sampler device
        onIsEmpty(false); // Emit signal that the track is not empty
    }

    void serialize() {
        // Implement serialization logic if needed
    }

    void deserialize() {
        // Implement deserialization logic if needed
    }

private:
    bool _isEmpty = true; // Track if the track is empty
    shared_ptr<SamplerDevice> samplerDevice; // Device associated with the track, if any

};