#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "core/devices/device_base.h"
#include "core/devices/sampler_device.h"
#include "core/devices/poly_sampler.h"
#include "meter_node.h"
#include "core/value_receiver.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include "sigslot/signal.hpp"
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
    shared_ptr<PolySampler> trackDevice;

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
        this->context->connect(muteNode, volumeNode, 0, 0);
        this->context->connect(meterNode, muteNode, 0, 0);
        this->context->connect(output, meterNode, 0, 0);

    }


    void setVolumeDb(float volumeDb) {
        volumeDb = std::clamp(volumeDb, -60.0f, 6.0f); // Clamp volume to a range of -60 dB to 6 dB
        volumeNode->gain()->setValue(dBToLinear(volumeDb));
    }

    void setPan(float pan) {
        panNode->pan()->setValue(pan);
    }

    void setMute(bool mute) {
        auto gainValue = mute ? 0.0f : 1.0f; // Set gain to 0 if muted, otherwise 1
        muteNode->gain()->setValue(gainValue);
    }

    void setSolo(bool solo) {
        
    }

    void setSamplerDevice(shared_ptr<PolySampler> device) {
        trackDevice = device;
        if (trackDevice) {
            context->connect(input, trackDevice->output, 0, 0);
            context->synchronizeConnections();
            _isEmpty = false; // Track is no longer empty after creating a sampler device
            onIsEmpty(false); 
        }
    }
    
    void createSamplerDevice(const std::string& filePath) {
        trackDevice = make_shared<PolySampler>(context);
        trackDevice->filePath.setValue(filePath); // Set the file path for the sampler device
        context->connect(input, trackDevice->output, 0, 0);
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
        
        if (trackDevice) {
            trackDevice->midiInput(msg); // Forward MIDI input to the sampler device
        }
    }
private:
    bool _isEmpty = true; // Track if the track is empty
     // Device associated with the track, if any

};

class Track {
  public:
    shared_ptr<VRString> name;  // Track name
    shared_ptr<VRFloat> volumeDb; // Track volume
    shared_ptr<VRFloat> pan;    // Track pan
    shared_ptr<VRBool> mute;    // Track mute state
    shared_ptr<VRBool> solo;    // Track solo state

    sigslot::signal<float, float> onLevelMetersChanged;
    sigslot::signal<> onDeviceUpdate; // Signal emitted when the sampler device is changed
    sigslot::signal<int, choc::midi::ShortMessage&> midiOutput;

    // Constructor for Track
    Track(shared_ptr<AudioContext> ac, shared_ptr<UndoManager> undoManager = nullptr) : name(make_shared<VRString>("Track Name", "New Track", undoManager)),
          volumeDb(make_shared<VRFloat>("Volume", 0.0f, -60.0f, 6.0f, 1, 0.1, undoManager)),
          pan(make_shared<VRFloat>("Pan", 0.0f, -1.0f, 1.0f, 0.01f, 0.001f, undoManager)),
          mute(make_shared<VRBool>("Mute", false, undoManager)),
          solo(make_shared<VRBool>("Solo", false, undoManager))
    {
        trackNode = make_shared<TrackNode>(ac); // Create a new TrackNode with the audio context
       

        // Connect property changes to the TrackNode
        volumeDb->onValueChanged.connect([this](float value) {
            trackNode->setVolumeDb(value);
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

        trackNode->setVolumeDb(volumeDb->getValue()); // Set initial volume
        trackNode->setPan(pan->getValue());       // Set initial pan
        trackNode->setMute(mute->getValue());     // Set initial mute state
        trackNode->setSolo(solo->getValue());     // Set initial solo state
    };
    ~Track() = default;

    bool isTrackEmpty() {
        return trackNode->trackDevice == nullptr; // Check if the track has a sampler device
    }
    void createSamplerDevice(const std::string& filePath) {
        trackNode->createSamplerDevice(filePath); // Create a sampler device for the track
        onDeviceUpdate(); // Emit signal that the device has been updated
    }

    void loadSample(const std::string& filePath) {
        if (!trackNode->trackDevice) {
            trackNode->createSamplerDevice(filePath); // Create a new sampler device if it doesn't exist
        } else {
            trackNode->trackDevice->filePath.setValue(filePath); // Load the sample into the existing sampler device
        }
        onDeviceUpdate(); // Emit signal that the device has been updated
    }
    void midiPlayback(choc::midi::ShortMessage &msg) {
        trackNode->midiInput(msg);
    }
    void midiInput(choc::midi::ShortMessage &msg) {
        trackNode->midiInput(msg); // Forward MIDI input to the track node
        midiOutput(_trackIndex, msg); // Emit MIDI output signal for the track to whoever is listening, ie sequencer
    }

    vector<float> * getWaveformData() {
        if (trackNode->trackDevice) {
            trackNode->trackDevice->generateWaveformData(); // Generate waveform data from the sampler device
            return &trackNode->trackDevice->_waveformData; // Return the waveform data
        }
        return nullptr; // Return nullptr if no sampler device is present
    }

    LevelMeters getLevelRMSdB() {
        LevelMeters meters;
        if (trackNode->meterNode) {
            auto res = trackNode->meterNode->rmsDb();
            meters.left = res[0];  // Get left channel level
            meters.right = res[1]; // Get right channel level
        }
        return meters; // Return the current level meters
    }

    LevelMeters getLevelPeakdB() {
        LevelMeters meters;
        if (trackNode->meterNode) {
            auto res = trackNode->meterNode->db();
            meters.left = res[0];  // Get left channel peak
            meters.right = res[1]; // Get right channel peak
        }
        return meters; // Return the current level meters
    }

    LevelMeters getLevelRMSLinear() {
        LevelMeters meters;
        if (trackNode->meterNode) {
            auto res = trackNode->meterNode->rmsDbLinear();
            meters.left = res[0];  // Get left channel level
            meters.right = res[1]; // Get right channel level
            onLevelMetersChanged(meters.left, meters.right); // Emit signal with the current level meters
        }
        return meters; // Return the current level meters
    }

    LevelMeters getLevelPeakLinear() {
        LevelMeters meters;
        if (trackNode->meterNode) {
            auto res = trackNode->meterNode->dbLinear();
            meters.left = res[0];  // Get left channel peak
            meters.right = res[1]; // Get right channel peak
            onLevelMetersChanged(meters.left, meters.right); // Emit signal with the current level meters
        }
        return meters; // Return the current level meters
    }

    shared_ptr<AudioNode> getOutput() {
        return trackNode->output; // Return the output node of the track
    }

    string getDeviceTypeName(){
        if (trackNode->trackDevice) {
            return "Sampler";
        }
        return "Empty";
    }

    void setTrackIndex(int index) {
        _trackIndex = index; // Set the track index
    }

    int getTrackIndex() const {
        return _trackIndex; // Get the track index
    }

protected:
    int _trackIndex;
    shared_ptr<TrackNode> trackNode; // Node representing the track in the audio context
    shared_ptr<SamplerDevice> samplerDevice; // Device associated with the track, if any
};