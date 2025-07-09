#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/GainNode.h"
#include "core/value_receiver.h"
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
    vector<float> _waveformData;
    VRFloat gain = VRFloat("Gain", .6f, 0.0f, 2.0f, 0.01f, 0.001f); // Gain parameter for the sampler device
    
    SamplerDevice(std::shared_ptr<lab::AudioContext> &audioContext)
        : DeviceBase(audioContext)
    {
        // Bind the file path parameter to the onValueChanged signal
        _samplerNode = std::make_shared<lab::SampledAudioNode>(*audioContext.get());
        _gainNode = std::make_shared<lab::GainNode>(*audioContext.get());
        _velocityNode = std::make_shared<lab::GainNode>(*audioContext.get());

        _samplerNode->schedule(0.0f); // Schedule the node to start immediately
        _audioContext->connect(_velocityNode, _samplerNode, 0, 0); // Connect the output to the sampler node
        _audioContext->connect(_gainNode, _velocityNode, 0, 0); // Connect the sampler node to the gain node
        _audioContext->connect(output, _gainNode, 0, 0); // Connect the sampler node to the gain node
        _audioContext->synchronizeConnections();

        connectParams(); // Connect the gain parameter to the gain node

        // After loading a sample, you can access the audio data via the AudioBus
        // Example: get number of channels and frames if a sample is loaded
       
    }

    void connectParams (){
        gain.onValueChanged.connect([this](float value) {
            _gainNode->gain()->setValue(value); // Update the gain node with the new value
        });
        _gainNode->gain()->setValue(gain.getValue()); // Set initial gain value
    }

    void generateWaveformData(){
        _waveformData.clear();
        if (!_audioBus) return;

        const int numChannels = _audioBus->numberOfChannels();
        const int numFrames = _audioBus->length();
        const int numPixels = 240;

        _waveformData.resize(numPixels, 0.0f);

        if (numChannels == 0 || numFrames == 0) return;

        const float* channelData = _audioBus->channel(0)->data(); // Use first channel

        int framesPerPixel = numFrames / numPixels;
        if (framesPerPixel < 1) framesPerPixel = 1;

        for (int i = 0; i < numPixels; ++i) {
            int start = i * framesPerPixel;
            int end = std::min(start + framesPerPixel, numFrames);
            float maxSample = 0.0f;
            for (int j = start; j < end; ++j) {
                float sample = std::abs(channelData[j]);
                if (sample > maxSample) maxSample = sample;
            }
            _waveformData[i] = maxSample;
        }
    }
    void serialize() override {
        // Implement serialization logic if needed
    }
    void deserialize() override {
        // Implement deserialization logic if needed
    }

    void midiInput(choc::midi::ShortMessage &msg)  {
        if(msg.isNoteOn()) {
            float velocity = msg.getVelocity() / 127.0f; // Normalize velocity to 0.0 - 1.0
            _velocityNode->gain()->setValue(velocity); // Set the gain based on velocity
            if (_samplerNode) {
                _samplerNode->start(0.0f);
            }
        } else if (msg.isNoteOff()) {
            if (_samplerNode) {
                // _samplerNode->stop(0.0f); // Stop playback on note off
            }
        }
    }


    void playSample(float startTime = 0.0f) {
        if (_samplerNode) {
            _samplerNode->start(startTime); // Start playback at the specified time
        } else {
            std::cerr << "Sampler node is not initialized." << std::endl;
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
    void loadSample(const std::string &filePath) {
        
        clearSample(); // Clear any existing sample
        setFilePath(filePath); // Set the file path for the sampler node
        if (_audioBus) {
            _samplerNode->setBus(_audioBus); // Load the audio bus into the sampler node
            generateWaveformData(); // Generate waveform data after loading the sample
        } else {
            std::cerr << "Failed to load sample: " << filePath << std::endl;
        }
    }

    void clearSample() {
        // _samplerNode->setBus(nullptr); // Clear the audio bus from the sampler node
        if(_audioBus) {
            // _samplerNode->setBus(nullptr); // Clear the audio bus from the sampler node
            _audioBus->reset();
        }
        _waveformData.clear(); // Clear the waveform data
    }

protected:
    
    std::string _name = "Sampler";
    std::shared_ptr<lab::SampledAudioNode> _samplerNode; // Sampler node
    std::shared_ptr<lab::AudioBus> _audioBus; // Audio bus
    std::shared_ptr<lab::GainNode> _gainNode;
    std::shared_ptr<lab::GainNode> _velocityNode; // Velocity node for dynamic playback

};