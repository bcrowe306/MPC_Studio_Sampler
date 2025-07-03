#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "core/devices/sampler_device.h"
#include "meter_node.h"
#include "core/parameter.h"
#include <memory>

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

    FloatParameter volume = FloatParameter("volume", 1.0f, 0.0f, 6.0f);
    FloatParameter pan = FloatParameter("pan", 0.0f, -1.0f, 1.0f);
    BoolParameter mute = BoolParameter("mute", false);
    StringParameter name = StringParameter("name", "Track");
    BoolParameter solo = BoolParameter("solo", false);
    
    Track(shared_ptr<AudioContext> ac) {
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
        }
    }

    void serialize() {
        // Implement serialization logic if needed
    }

    void deserialize() {
        // Implement deserialization logic if needed
    }

private:
    shared_ptr<SamplerDevice> samplerDevice; // Device associated with the track, if any

};