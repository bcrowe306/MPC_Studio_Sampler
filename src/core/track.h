#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "core/devices/device_base.h"
#include "core/devices/device_types.h"
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
#include "core/serializable.h"
#include "yaml-cpp/emittermanip.h"
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
    shared_ptr<PolySampler> device;

    sigslot::signal<bool> onIsEmpty; // Signal emitted when the track is empty or not

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

    void setSamplerDevice(shared_ptr<PolySampler> newDevice) {
        device = newDevice;
        if (device) {
            context->connect(input, device->output, 0, 0);
            context->synchronizeConnections();
            _isEmpty = false; // Track is no longer empty after creating a sampler device
            onIsEmpty(false); 
        }
    }
    
    void createSamplerDeviceFromSample(const std::string& filePath) {

        device = make_shared<PolySampler>(context);
        device->filePath->setValue(filePath); // Set the file path for the sampler device
        context->connect(input, device->output, 0, 0);
        context->synchronizeConnections();
        _isEmpty = false; // Track is no longer empty after creating a sampler device
        onIsEmpty(false); // Emit signal that the track is not empty
    }
    void createSamplerDevice() {
        device = make_shared<PolySampler>(context);
        context->connect(input, device->output, 0, 0);
        context->synchronizeConnections();
        _isEmpty = false; // Track is no longer empty after creating a sampler device
        onIsEmpty(false); // Emit signal that the track is not empty
    }


    void midiInput(choc::midi::ShortMessage &msg) {
        
        if (device) {
            device->midiInput(msg); // Forward MIDI input to the sampler device
        }
    }
private:
    bool _isEmpty = true; // Track if the track is empty
     // Device associated with the track, if any

};

class Track : public Serializable {
  public:
    shared_ptr<VRString> name;  // Track name
    shared_ptr<VRFloat> volumeDb; // Track volume
    shared_ptr<VRFloat> pan;    // Track pan
    shared_ptr<VRBool> mute;    // Track mute state
    shared_ptr<VRBool> solo;    // Track solo state

    sigslot::signal<float, float> onLevelMetersChanged;
    sigslot::signal<> onTrackDeviceUpdated; // Signal emitted when the sampler device is changed
    sigslot::signal<int, choc::midi::ShortMessage&> midiOutput;

    // Constructor for Track
    Track(shared_ptr<AudioContext> ac, shared_ptr<UndoManager> undoManager = nullptr) : 
        name(make_shared<VRString>("Name", "New Track", undoManager)),
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

    void serialize(YAML::Emitter &out) override {
        name->serialize(out);
        volumeDb->serialize(out);
        pan->serialize(out);
        mute->serialize(out);
        solo->serialize(out);
        out << YAML::Key << "Device";
        out << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "Type";
            out << YAML::Value << deviceTypeToStringMap[_deviceType]; // Serialize the device type as a string
            if (trackNode->device) {
                trackNode->device->serialize(out); // Serialize the device if it exists
            }
            out << YAML::EndMap;
    }

    void deserialize(const YAML::Node &node) override {
        auto rootNode = node;
        name->deserialize(rootNode);
        volumeDb->deserialize(rootNode);
        pan->deserialize(rootNode);
        mute->deserialize(rootNode);
        solo->deserialize(rootNode);
        if (rootNode["Device"]) {
            auto deviceNode = rootNode["Device"];
            if (deviceNode["Type"]) {
                string deviceTypeStr = deviceNode["Type"].as<string>();
                _deviceType = deviceTypeMap[deviceTypeStr];
                std::string empty = "";
                createDevice(_deviceType, empty);
                if(trackNode->device) {
                    trackNode->device->deserialize(deviceNode);
                     // Emit signal that the device has been updated
                    
                }
            }
        }
        else {
            std::string empty = "";
            createDevice(DeviceType::Undefined, empty); // Create an undefined device if no device node is present
        }
        onTrackDeviceUpdated();
    }

    bool isTrackEmpty() const {
        return trackNode->device == nullptr; // Check if the track has a sampler device
    }

    //  Args ...
    template <typename... Args>
    void createDevice(DeviceType type, Args... args) {
        if(trackNode->device) {
            trackNode->device->onDeviceChanged.disconnect_all(); // Disconnect any existing device change signals
        } 
        switch(type) {
            case DeviceType::Sampler: {
                    auto filePath = std::get<std::string>(std::make_tuple(args...));
                    if(filePath.empty()) {
                        trackNode->createSamplerDevice(); // Create a sampler device without a file path
                    }else{
                        trackNode->createSamplerDeviceFromSample(filePath); // Create a sampler device with the provided file path
                    }
                break;
            }
            case DeviceType::Synthesizer:
                // Implement synthesizer creation logic here
                break;
            case DeviceType::Undefined:
                trackNode->device.reset(); // Reset the device if undefined
                break;
            default:
                throw std::runtime_error("Unsupported device type");
        }
        _deviceType = type; // Set the device type
    }
    

    bool loadSample(const std::string& filePath) {
        if (!trackNode->device) {
            createDevice(DeviceType::Sampler, filePath);
            onTrackDeviceUpdated();
            return true;
        } 
        else {
            if(trackNode->device->getType() == DeviceType::Sampler) {
                trackNode->device->filePath->setValue(filePath); // Load the sample into the existing sampler device
                onTrackDeviceUpdated(); // Emit signal that the device has been updated
                return true;
            }
        }
        return false;
    }

    void midiPlayback(choc::midi::ShortMessage &msg) {
        trackNode->midiInput(msg);
    }

    void midiInput(choc::midi::ShortMessage &msg) {
        trackNode->midiInput(msg); // Forward MIDI input to the track node
        midiOutput(_trackIndex, msg); // Emit MIDI output signal for the track to whoever is listening, ie sequencer
    }

    vector<float> * getWaveformData() {
        if (trackNode->device) {
            trackNode->device->generateWaveformData(); // Generate waveform data from the sampler device
            return &trackNode->device->_waveformData; // Return the waveform data
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
        if (trackNode->device) {
            return "Sampler";
        }
        return "Empty";
    }

    shared_ptr<DeviceBase> getDevice() {
        return trackNode->device; // Return the device associated with the track
    }
    
    void setTrackIndex(int index) {
        _trackIndex = index; // Set the track index
    }

    int getTrackIndex() const {
        return _trackIndex; // Get the track index
    }

protected:
    int _trackIndex;
    DeviceType _deviceType = DeviceType::Undefined;
    shared_ptr<TrackNode> trackNode; // Node representing the track in the audio context
};