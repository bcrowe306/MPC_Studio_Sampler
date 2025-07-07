#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "core/devices/sampler_device.h"
#include "meter_node.h"
#include "core/value_receiver.h"
#include <memory>
#include "util.h"
#include "audio/choc_MIDI.h"
using std::shared_ptr;
using std::make_shared;

struct LevelMeters {
    float left = 0.0f; // Left channel level
    float right = 0.0f; // Right channel level
};

class TrackNode {
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
    
    TrackNode(shared_ptr<AudioContext> ac) {
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


    void setVolume(float volume) {
        volumeNode->gain()->setValue(static_cast<float>(volume));
    }

    void setPan(float pan) {
        panNode->pan()->setValue(static_cast<float>(pan));
    }

    void setMute(bool mute) {
        muteNode->gain()->setValue(mute ? 0.0f : 1.0f);
    }

    void setSolo(bool solo) {
        // Solo logic: if solo is true, mute all other tracks
        if (solo) {
            // Mute all other tracks (not implemented here, would require access to all tracks)
            // This is a placeholder for the actual implementation
            setMute(false); // Unmute this track
        } else {
            setMute(true); // Mute this track if solo is turned off
        }
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
        samplerDevice = make_shared<SamplerDevice>(context);
        samplerDevice->setFilePath(filePath); // Set the file path for the sampler device
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

    void midiInput(choc::midi::ShortMessage &msg) {
        
        if (samplerDevice) {
            samplerDevice->midiInput(msg); // Forward MIDI input to the sampler device
        }
    }
private:
    bool _isEmpty = true; // Track if the track is empty
    shared_ptr<SamplerDevice> samplerDevice; // Device associated with the track, if any

};

class Track {
  public:
    shared_ptr<VRString> name;  // Track name
    shared_ptr<VRFloat> volume; // Track volume
    shared_ptr<VRFloat> pan;    // Track pan
    shared_ptr<VRBool> mute;    // Track mute state
    shared_ptr<VRBool> solo;    // Track solo state

    sigslot::signal<float, float> onLevelMetersChanged;
    Track(shared_ptr<AudioContext> ac, shared_ptr<UndoManager> undoManager = nullptr) : name(make_shared<VRString>("Track Name", "New Track", undoManager)),
          volume(make_shared<VRFloat>("Volume", 1.0f, 0.0, 1.0, 0.01, 0.001, undoManager)),
          pan(make_shared<VRFloat>("Pan", 0.0f, -1.0f, 1.0f, 0.01f, 0.001f, undoManager)),
          mute(make_shared<VRBool>("Mute", false, undoManager)),
          solo(make_shared<VRBool>("Solo", false, undoManager))
    {
        trackNode = make_shared<TrackNode>(ac); // Create a new TrackNode with the audio context
       

        // Connect property changes to the TrackNode
        volume->onValueChanged.connect([this](float value) {
            trackNode->setVolume(value);
        });
        pan->onValueChanged.connect([this](float value) {
            trackNode->setPan(value);
        });
        mute->onValueChanged.connect([this](bool value) {
            trackNode->setMute(value);
        });
        solo->onValueChanged.connect([this](bool value) {
            trackNode->setSolo(value);
        });

        trackNode->setVolume(volume->getValue()); // Set initial volume
        trackNode->setPan(pan->getValue());       // Set initial pan
        trackNode->setMute(mute->getValue());     // Set initial mute state
        trackNode->setSolo(solo->getValue());     // Set initial solo state
    };
    ~Track() = default;


    void createSamplerDevice(const std::string& filePath) {
        trackNode->createSamplerDevice(filePath); // Create a sampler device for the track
    }

    void midiInput(choc::midi::ShortMessage &msg) {
        trackNode->midiInput(msg); // Forward MIDI input to the track node
    }

    LevelMeters getLevelMeters() {
        LevelMeters meters;
        if (trackNode->meterNode) {
            auto res = trackNode->meterNode->rmsDbLinear();
            meters.left = res[0];  // Get left channel level
            meters.right = res[1]; // Get right channel level
        }
        return meters; // Return the current level meters
    }

    shared_ptr<AudioNode> getOutput() {
        return trackNode->output; // Return the output node of the track
    }

protected:
    shared_ptr<TrackNode> trackNode; // Node representing the track in the audio context
    shared_ptr<SamplerDevice> samplerDevice; // Device associated with the track, if any
};