#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "audio/choc_SampleBuffers.h"
#include "core/adsr.h"
#include "audio/choc_MIDI.h"
#include "core/value_receiver.h"
#include "device_base.h"
#include "sigslot/signal.hpp"
#include "core/buffer_player.h"
#include "core/nodes/buffer_node.h"
#include "util.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
using std::string;
using std::make_shared;

using lab::AudioContext;
using lab::SampledAudioNode;
using lab::GainNode;
using lab::AudioBus;
using std::shared_ptr;




struct PolyVoice {
    uint32_t voiceId = 0;
    shared_ptr<BufferNode> samplerNode; // Sampler node for the voice
    shared_ptr<GainNode> sampleGainNode; // Gain node for the voice
    shared_ptr<GainNode> velocityAttenuationNode; // Velocity attenuation node for the voice
    shared_ptr<GainNode> amplitudeAttenuationNode; // Amplitude attenuation node for the voice

    shared_ptr<AudioContext> audioContext; // Audio context for the voice
    shared_ptr<GainNode> outputVolumeNode; // Output volume node for the voice
    shared_ptr<ADSR> amplitudeEnvelopeNode; // Amplitude envelope node for the voice
    shared_ptr<ADSR> filterEnvelopeNode; // Filter envelope node for the voice
    shared_ptr<ADSR> pitchEnvelopeNode; // Pitch envelope node for the voice
    int baseNote = 60;
    int note = -1;         // MIDI note number (-1 = inactive)
    int velocity = 0;      // Note velocity (0-127)
    float velocitySensitivity = 0.5f; // Velocity sensitivity (0.0 - 1.0)
    bool isActive = false; // Flag to indicate if the voice note is being held down
    bool isReleasing = false; // Flag to indicate if the voice note is being released
    bool isSustained = false;
    int startSample = 0; // Start sample position in the sample
    int endSample = 0; // End sample position in the sample
    double glideMs = 5.0; // Glide time in milliseconds
    bool legato = false; // Legato mode for the voice
    std::chrono::steady_clock::time_point triggerTime;  // When voice was triggered
    std::chrono::steady_clock::time_point releaseTime;  // When voice was released

    PolyVoice(shared_ptr<AudioContext> &audioContext)
        : audioContext(audioContext)
    {
        velocityAttenuationNode = std::make_shared<GainNode>(*audioContext.get());
        amplitudeAttenuationNode = std::make_shared<GainNode>(*audioContext.get());
        amplitudeAttenuationNode->gain()->setValue(0.0); // Default to 0.0 gain
        amplitudeEnvelopeNode = std::make_shared<ADSR>(audioContext);
        amplitudeEnvelopeNode->setAll(
            0.1f, // attack time 5 ms
            0.1f, // decay time 100 ms
            1.0f, // sustain level 100%
            0.01f  // release time 10 ms
        );
        sampleGainNode = std::make_shared<GainNode>(*audioContext.get());
        
        outputVolumeNode = std::make_shared<GainNode>(*audioContext.get());
        filterEnvelopeNode = std::make_shared<ADSR>(audioContext);
        pitchEnvelopeNode = std::make_shared<ADSR>(audioContext);
        
        samplerNode = std::make_shared<BufferNode>(*audioContext.get());
        samplerNode->start(0.0); // Start the sampler node immediately
        audioContext->connect(sampleGainNode, samplerNode, 0, 0); // Connect the sampler node to the sample gain node
        audioContext->connect(velocityAttenuationNode, sampleGainNode, 0, 0); // Connect the sampler node to the sample gain node
        audioContext->connectParam(amplitudeAttenuationNode->gain(), amplitudeEnvelopeNode->functionNode, 0); // Connect the amplitude envelope to the sampler node
        audioContext->connect(amplitudeAttenuationNode, velocityAttenuationNode, 0, 0); // Connect the output volume node to the sampler node
        audioContext->connect(outputVolumeNode, amplitudeAttenuationNode);


        audioContext->synchronizeConnections();
        setInitialParameters(); // Set initial parameters for the voice
        
    }

    void setInitialParameters() {
        velocityAttenuationNode->gain()->setValue(1.0); // Set the velocity attenuation
        sampleGainNode->gain()->setValue(dBToLinear(0.0f)); // Set the sample gain initially to 0 dB
        outputVolumeNode->gain()->setValue(dBToLinear(-6.0f)); // Set the output volume to -6 dB
        samplerNode->setPlaybackRate(1.0); // Set the playback rate to normal speed
        
    }

    // Map MIDI velocity to a linear gain value according to the velocity sensitivity
    float calculateVelocityAttenuation(int velocity, float sensitivity) {
        float range= 24.0f;
        float mappedVelocity = mapFloat((float)velocity, 0.0f, 127.0f, -range, range); // Map velocity to a range of -12.0 to 12.0 in db
        float dbAttenuation =  mappedVelocity * sensitivity;
        return dBToLinear(dbAttenuation); // Convert dB to linear gain
    }

    void setSample(shared_ptr<choc::buffer::ChannelArrayBuffer<float>> audioBuffer) {
        // get sample length in seconds
        auto sampleLength = audioBuffer->getNumFrames() / audioContext->sampleRate();
        samplerNode->setBuffer(audioBuffer);
    }

    void slideNote(int newNote){
        auto baseFreq = choc::midi::noteNumberToFrequency(baseNote); // Middle C frequency
        auto newFrequency = choc::midi::noteNumberToFrequency(newNote) / baseFreq; // Calculate frequency based on new note
        samplerNode->setPlaybackRate(newFrequency, glideMs); // Set the playback rate to the new frequency with sliding effect
        note = newNote; // Update the note to the new note
    }

    void play(bool slide = false) {
            
        auto baseFreq = choc::midi::noteNumberToFrequency(baseNote); // Middle C frequency
        auto frequency = choc::midi::noteNumberToFrequency(note) / baseFreq; // Calculate frequency based on note
        if(legato){
            samplerNode->setPlaybackRate(frequency, glideMs); // Set the playback rate to the new frequency with sliding effect
        }
        else{
            samplerNode->setPlaybackRate(frequency, 5);
        }
        samplerNode->play(startSample, endSample); // Start the sampler node with the specified start and end samples
        velocityAttenuationNode->gain()->setValue(calculateVelocityAttenuation(velocity, velocitySensitivity));

    }

    void stop() {
        samplerNode->stop();
    }
    void gateEnvelopes(bool gate, bool retrigger = false) {
        // filterEnvelopeNode->gate(gate ? 1.0f : 0.0f);
        // pitchEnvelopeNode->gate(gate ? 1.0f : 0.0f);
        if(gate) {
            play(); // Start the sampler node
            amplitudeEnvelopeNode->gate(1.0f); // Gate the envelope to start
        } 
        else {
            amplitudeEnvelopeNode->gate(0.0f);
        }

    }

    void resetEnvelopes() {
        amplitudeEnvelopeNode->resetEnvelope();
        filterEnvelopeNode->resetEnvelope();
        pitchEnvelopeNode->resetEnvelope();
    }

    shared_ptr<GainNode> getOutputNode() {
        return outputVolumeNode; // Return the output node for the voice
    }
    bool isInUse() const { return isActive || isSustained || isReleasing; }

    void reset() {
        note = -1;
        velocity = 0;
        isActive = false;
        isSustained = false;
        isReleasing = false;
        // voiceId = 0;
        resetEnvelopes(); // Reset the envelopes
        stop(); 
    }
};

struct VoiceAllocator {
    vector<PolyVoice> &voices;
    int _maxVoices = 32;           
    bool sustainPedalPressed_ = false; 

    uint32_t nextVoiceId_ = 0;

    // Statistics (optional, for debugging/monitoring)
    mutable std::chrono::steady_clock::time_point lastUpdate_;
    mutable int voiceStealCount_; 
    bool legato = false;
    shared_ptr<lab::FunctionNode> functionNode; 

    VoiceAllocator(shared_ptr<AudioContext> audioContext, vector<PolyVoice> &voices) : voices(voices) {
        _maxVoices = voices.size(); 
        lastUpdate_ = std::chrono::steady_clock::now();
        voiceStealCount_ = 0; 

        functionNode = make_shared<lab::FunctionNode>(*audioContext.get()); 
        functionNode->start(0.0);
        functionNode->setFunction([&voices](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
            // for(auto &voice : voices) {
                // auto currentSamplePosition = voice.samplerNode->getPosition();
                // if(voice.isReleasing && voice.amplitudeEnvelopeNode->getState() == envState::env_idle) {

                //     if(voice.amplitudeEnvelopeNode->getMode() != ADSR::ADSRMode::ONESHOT) {
                //         std::cout << "Voice is releasing, resetting voice" << std::endl;
                //         voice.reset(); 
                //     }

                // }

                // // Reset the voice if it is releasing or sustained and the cursor exceeds the sample length
                // if(currentSamplePosition >= voice.samplerNode->getLength() && voice.isInUse() && !voice.samplerNode->isLooping()) {
                //     std::cout << "Resetting voice due to cursor exceeding sample length" << std::endl;
                //     voice.reset(); // Stop the sampler node if cursor exceeds length
                // }
                // if(voice.samplerNode->hasEnded() && voice.isInUse()) {
                //     voice.reset(); // Reset the voice if the sampler node has ended
                // }
                
            // }
        });
        audioContext->connect(audioContext->destinationNode(), functionNode);
        audioContext->synchronizeConnections();
    }


    uint32_t generateVoiceId() { return ++nextVoiceId_; }

    int processMidiMessage(const choc::midi::ShortMessage& message) {
    if (message.isNoteOn()) {
        return noteOn(message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff()) {
        return noteOff(message.getNoteNumber());
    }
    else if (message.isController()) {
        int controller = message.getControllerNumber();
        int value = message.getControllerValue();
        int channel = message.getChannel0to15();
        
        switch (controller) {
            case 64: // Sustain pedal
                setSustainPedal(value);
                break;
            case 120: // All sound off
                allSoundOff();
                break;
            case 123: // All notes off
                allNotesOff();
                break;
        }
    }
    
    return -1; // No voice affected
}
    int findAvailableVoice() {
        for (int i = 0; i < _maxVoices; ++i) {
            if (!voices[i].isInUse()) {
                return i;
            }
        }
        return -1;
    }
    
    int findOldestVoice() const {
        int oldestVoice = -1;
        auto oldestTime = std::chrono::steady_clock::now();

        for (int i = 0; i < _maxVoices; ++i) {
            const PolyVoice &voice = voices[i];
            if (voice.isInUse() && !voice.isSustained) {
                if (oldestVoice == -1 || voice.triggerTime < oldestTime) {
                  oldestVoice = i;
                  oldestTime = voice.triggerTime;
                }
            }
        }

        return oldestVoice;
    }

    void setSustainPedal(int value) {
        bool newState = value >= 64;
        bool oldState = sustainPedalPressed_;

        sustainPedalPressed_ = newState;

        // If sustain pedal was released, release all sustained voices
        if (oldState && !newState) {
            releaseSustainedVoices();
        }
    }

    void allNotesOff() {
        for (auto &voice : voices) {
            if (voice.isActive) {
                voice.isReleasing = true;
                voice.releaseTime = std::chrono::steady_clock::now();
            }
        }
    }

    void allSoundOff() {
        for (int i = 0; i < voices.size(); ++i) {
            PolyVoice &voice = voices[i];
            if (voice.isActive) {
                voice.reset();
            }
        }
    }

    void releaseSustainedVoices() {
        for (auto &voice : voices) {
            if (voice.isSustained) {
                voice.isSustained = false;
                voice.isReleasing = true;
                voice.releaseTime = std::chrono::steady_clock::now();
                // Release ADSR envelopes
                voice.gateEnvelopes(false);
            }
        }
    }

    // Find a voice for a specific note. If not found, return -1.
    int findVoiceForNote(int note) const {
        for (int i = 0; i < _maxVoices; ++i) {
            const PolyVoice &voice = voices[i];
            if (voice.isInUse() && voice.note == note) {
                return i;
            }
        }
        return -1;
    }

    int noteOn(int note, int velocity) {
        if (velocity == 0) {
            // Velocity 0 note-on is treated as note-off
            return noteOff(note);
        }

        // Check if this note is already playing (for retriggering)
        int existingVoiceIndex = findVoiceForNote(note);
        if (existingVoiceIndex != -1) {
            // Retrigger existing voice

            auto &voice = voices[existingVoiceIndex];
            voice.note = note;
            voice.velocity = velocity;
            voice.triggerTime = std::chrono::steady_clock::now();
            voice.isReleasing = false;
            voice.isSustained = false;
            voice.isActive = true; // Key is pressed
            voice.voiceId = generateVoiceId();

            // Trigger ADSR envelopes and start on the sampler node
            // voice.resetEnvelopes(); // Reset envelopes to start fresh
            voice.gateEnvelopes(true);

            return existingVoiceIndex;
        }

        // Find available voice
        int voiceIndex = findAvailableVoice();
        bool stolenVoice = false;
        if (voiceIndex == -1) {
            // No available voice, steal one
            stolenVoice = true;
            voiceIndex = findOldestVoice();
            if (voiceIndex == -1) {
                return -1;
            }
            voiceStealCount_++;
            
        }

        PolyVoice &voice = voices[voiceIndex];
        if(stolenVoice) {
            if(legato && voice.isActive)
                voice.slideNote(note); // Slide to the new note if voice was stolen
            else
            {
                voice.note = note;
                voice.velocity = velocity;
                voice.isActive = true;
                voice.isSustained = false;
                voice.isReleasing = false;
                voice.triggerTime = std::chrono::steady_clock::now();
                voice.voiceId = generateVoiceId();

                // Trigger ADSR envelopes
                voice.gateEnvelopes(true);
            }
        }
        else {
            // Allocate voice
            // std::cout << "Allocating Voice Index: " << voiceIndex  << " for note: " << note << std::endl;
            voice.note = note;
            voice.velocity = velocity;
            voice.isActive = true;
            voice.isSustained = false;
            voice.isReleasing = false;
            voice.triggerTime = std::chrono::steady_clock::now();
            voice.voiceId = generateVoiceId();

            // Trigger ADSR envelopes
            voice.gateEnvelopes(true);
        }
        
        return voiceIndex;
    }
    int noteOff(int note) {
        int voiceIndex = findVoiceForNote(note);
        if (voiceIndex == -1) {
            return -1;
        }
        PolyVoice &voice = voices[voiceIndex];

        // Check if sustain pedal is pressed
        if (sustainPedalPressed_) {
            // Mark as sustained instead of releasing
            // std::cout << "Sustain pedal pressed, sustaining voice: " << voiceIndex << std::endl;
            voice.isActive = false;    // Key is no longer pressed
            voice.isSustained = true;  // But held by sustain pedal
            voice.isReleasing = false; // Not releasing, sustained
        } else {
            // Release the voice
            voice.isActive = false;   // Key is no longer pressed
            voice.isReleasing = true; // Voice is releasing
            voice.releaseTime = std::chrono::steady_clock::now();

            // Release ADSR envelopes
            voice.gateEnvelopes(false);
            
        }

        return voiceIndex;
    }
};


class PolySampler : public DeviceBase {

public:
    enum class SamplerMode {
        ONESHOT,
        ADSR
    };
    
    uuids::uuid id = generateUUID(); // Unique identifier for the poly sampler device
    vector<float> _waveformData; // Waveform data for the poly sampler
    const int kMaxVoices = 16; // Maximum number of voices for polyphony
    vector<PolyVoice> _voices; // Voices for the poly sampler
    
    // Parameters for the poly sampler
    shared_ptr<VRFloatDB> volumeDb = make_shared<VRFloatDB>("volumeDB", -6.0f); // Gain parameter for the poly sampler
    shared_ptr<VRInt> startPosition = make_shared<VRInt>("startPosition", 0, 0, 0, 1000, 100); // Start position for the sample
    shared_ptr<VRInt> endPosition = make_shared<VRInt>("endPosition", 0, 0, 0, 1000, 100); // End position for the sample
    shared_ptr<VRBool> loop = make_shared<VRBool>("loop", false); // Looping mode for the poly sampler
    shared_ptr<VRIntOptions> samplerMode = make_shared<VRIntOptions>("samplerMode", 0, std::vector<int>{0, 1}, std::vector<string>{"One Shot", "ADSR"}); // Sampler mode (One Shot or ADSR)
    shared_ptr<VRInt> maxVoices = make_shared<VRInt>("maxVoices", 16, 1, kMaxVoices, 1, 1); // Number of voices to use in polyphony
    shared_ptr<VRBool> legato = make_shared<VRBool>("legato", true); // Legato mode for the poly sampler
    shared_ptr<VRDouble> glideMs = make_shared<VRDouble>("glide", 5.0, 1.0, 1000.0, 1.0, 1.0); // Glide time for the poly sampler
    shared_ptr<VRString> filePath = make_shared<VRString>("filePath", ""); // File path for the sample
    shared_ptr<VRFloatTime> attack = make_shared<VRFloatTime>("attack"); // Attack time for the amplitude envelope
    shared_ptr<VRFloatTime> decay = make_shared<VRFloatTime>("decay"); // Decay time for the amplitude envelope
    shared_ptr<VRFloat> sustain = make_shared<VRFloat>("sustain", 1.0f, 0.0f, 1.0f, 0.01f, 0.01f); // Sustain level for the amplitude envelope
    shared_ptr<VRFloatTime> release = make_shared<VRFloatTime>("release"); // Release time for the amplitude envelope
    shared_ptr<VRFloat> velocitySensitivity = make_shared<VRFloat>("velocitySensitivity", .5f, 0.0f, 1.0f, 0.01f, 0.001f); // Velocity sensitivity for the amplitude envelope


    shared_ptr<choc::buffer::ChannelArrayBuffer<float>> _sampleBus;
    shared_ptr<VoiceAllocator> voiceAllocator; // Voice allocator for managing polyphony

    PolySampler(std::shared_ptr<lab::AudioContext> audioContext)
        : DeviceBase(audioContext)
    {
        _type = DeviceType::Sampler; // Set the device type to Sampler
        _name = "Sampler"; // Set the device name
        for (int i = 0; i < kMaxVoices; ++i) {
            _voices.emplace_back(audioContext);
        }
        for(auto &voice : _voices) {
            audioContext->connect(output, voice.getOutputNode(), 0, 0); // Connect each voice's output to the device output
        }
        voiceAllocator = std::make_shared<VoiceAllocator>(audioContext, _voices); // Initialize the voice allocator
        connectParameters(); // Connect parameters for the poly sampler
        
    }

    void midiInput(choc::midi::ShortMessage &msg) override {
        voiceAllocator->processMidiMessage(msg); // Process MIDI messages through the voice allocator
    }

    void clearSample() {
        if (_sampleBus) {
            
        }
        _waveformData.clear();
    }

    void connectParameters(){

        // Volume parameter
        volumeDb->onValueChanged.connect([this](float value) {
            for (auto &voice : _voices) {
                voice.outputVolumeNode->gain()->setValue(dBToLinear(value)); // Set the output volume for each voice
            }
        });
        volumeDb->updateObservers();
        parameters.push_back(volumeDb); // Add the volume parameter to the list of parameters


        // File path parameter
        filePath->onValueChanged.connect([this](const string &value) {
          loadSample(value);
        });
        filePath->updateObservers(); 

        // Start Position parameter
        startPosition->onValueChanged.connect([this](int value) {
            for (auto &voice : _voices) {
                voice.startSample = value; // Update the start sample position for each voice
                voice.samplerNode->setStart(value);
            }
        });
        startPosition->updateObservers();
        startPosition->setDisplayName("start");
        parameters.push_back(startPosition); 

        // End Position parameter
        endPosition->onValueChanged.connect([this](int value) {
            for (auto &voice : _voices) {
                voice.endSample = value; 
                voice.samplerNode->setEnd(value);
            }
        });
        endPosition->updateObservers();
        endPosition->setDisplayName("end");
        parameters.push_back(endPosition); 


        // Loop parameter
        loop->onValueChanged.connect([this](bool value) {
            for (auto &voice : _voices) {
                voice.samplerNode->setLooping(value); 
            }
        });
        loop->updateObservers();
        loop->setDisplayName("loop");
        parameters.push_back(loop); 


        // Sampler Mode parameter
        samplerMode->onValueChanged.connect([this](int value) {
            for(auto &voice : _voices) {
                voice.amplitudeEnvelopeNode->setMode(static_cast<ADSR::ADSRMode>(value));
            }
        });
        samplerMode->updateObservers(); 
        samplerMode->setDisplayName("mode");
        parameters.push_back(samplerMode);

        // maxVoices
        maxVoices->onValueChanged.connect([this](int value) {
            if (value < 1 || value > kMaxVoices) {
            }
            voiceAllocator->_maxVoices = value; 
        });
        maxVoices->setDisplayName("voices");
        maxVoices->updateObservers();
        parameters.push_back(maxVoices);

        // Legato
        legato->onValueChanged.connect([this](bool value) {
            voiceAllocator->legato = value; // Update the legato mode in the voice allocator
            for (auto &voice : _voices) {
                voice.legato = value; // Set the legato mode for each voice
            }
        });
        legato->updateObservers();
        legato->setDisplayName("legato");
        parameters.push_back(legato);

        // Glide
        glideMs->onValueChanged.connect([this](double value) {
            for (auto &voice : _voices) {
                voice.glideMs = value; // Set the glide time for each voice
            }
        });
        glideMs->updateObservers();
        glideMs->setDisplayName("glide");
        parameters.push_back(glideMs);

        // Attack
        attack->onValueChanged.connect([this](float value) {
            for (auto &voice : _voices) {
                voice.amplitudeEnvelopeNode->setAttackTimeSeconds(value); // Set the attack time for each voice
            }
        });
        attack->updateObservers();
        attack->setDisplayName("attack");
        parameters.push_back(attack);

        // Decay
        decay->onValueChanged.connect([this](float value) {
            for (auto &voice : _voices) {
                voice.amplitudeEnvelopeNode->setDecayTimeSeconds(value); // Set the decay time for each voice
            }
        });
        decay->updateObservers();
        decay->setDisplayName("decay");
        parameters.push_back(decay);

        // Sustain
        sustain->onValueChanged.connect([this](float value) {
            for (auto &voice : _voices) {
                voice.amplitudeEnvelopeNode->setSustainLevel(value); // Set the sustain level for each voice
            }
        });
        sustain->updateObservers();
        sustain->setDisplayName("sustain");
        parameters.push_back(sustain);

        // Release
        release->onValueChanged.connect([this](float value) {
            for (auto &voice : _voices) {
                voice.amplitudeEnvelopeNode->setReleaseTimeSeconds(value); // Set the release time for each voice
            }
        });
        release->updateObservers();
        release->setDisplayName("release");
        parameters.push_back(release);

        // Velocity Sensitivity
        velocitySensitivity->onValueChanged.connect([this](float value) {
            for (auto &voice : _voices) {
                voice.velocitySensitivity = value; // Set the velocity sensitivity for each voice
            }
        });
        velocitySensitivity->updateObservers();
        velocitySensitivity->setDisplayName("velocity");
        parameters.push_back(velocitySensitivity);

    } // End of connectParameters

    void stopAllNotes() override{
        for (auto &voice : _voices) {
            voice.reset(); // Reset all voices
        }
    }

    void generateWaveformData(){
        _waveformData.clear();
        if (!_sampleBus) return;

        const int numChannels = _sampleBus->getNumChannels();
        const int numFrames = _sampleBus->getNumFrames();
        const int numPixels = 240;

        _waveformData.resize(numPixels, 0.0f);

        if (numChannels == 0 || numFrames == 0) return;

        const float* channelData = _sampleBus->getChannel(0).data.data; // Use first channel

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
    // Serialize the object to a YAML emitter
    void serialize(YAML::Emitter &out) override{
        filePath->serialize(out);
        volumeDb->serialize(out);
        startPosition->serialize(out);
        endPosition->serialize(out);
        loop->serialize(out);
        samplerMode->serialize(out);
        maxVoices->serialize(out);
        legato->serialize(out);
        glideMs->serialize(out);
        attack->serialize(out);
        decay->serialize(out);
        sustain->serialize(out);
        release->serialize(out);
        velocitySensitivity->serialize(out);
    };

    // Deserialize the object from a YAML node
    void deserialize(const YAML::Node &yaml) override{
        auto node = yaml;
        filePath->deserialize(node);
        volumeDb->deserialize(node);
        startPosition->deserialize(node);
        endPosition->deserialize(node);
        loop->deserialize(node);
        samplerMode->deserialize(node);
        maxVoices->deserialize(node);
        legato->deserialize(node);
        glideMs->deserialize(node);
        attack->deserialize(node);
        decay->deserialize(node);
        sustain->deserialize(node);
        release->deserialize(node);
        velocitySensitivity->deserialize(node);
    };

    void loadSample(const string &filePath) {
        if(filePath.empty()) {
            return;
        }
        _loadThread = std::thread(&PolySampler::_doLoadSample, this, filePath);
        _loadThread.detach(); // Detach the thread to allow it to run independently
    }

    bool isLoading() const {
        return _isLoading.load(std::memory_order_relaxed); // Check if the sample is currently loading
    }

private:
    std::thread _loadThread; // Thread for loading samples
    std::atomic<bool> _isLoading = false;

    void _doLoadSample(const std::string &filePath) {
        if(filePath.empty()) {
            return;
        }
        _isLoading.store(true, std::memory_order_relaxed); // Set loading state to true
        _waveformData.clear();


        auto reader = WAVAudioFileFormat<false>().createReader(filePath);
        try {
            auto fileData = reader->loadFileContent(
                _audioContext->sampleRate()
            );

            _sampleBus = make_shared<choc::buffer::ChannelArrayBuffer<float>>(fileData.frames);

            for (auto &voice : _voices) {
                voice.setSample(_sampleBus);
            }
            startPosition->setValue(0);
            startPosition->setMin(0);
            startPosition->setMax(_sampleBus->getNumFrames() - 100); // Set start position to the beginning of the sample
            endPosition->setValue(_sampleBus->getNumFrames() - 1); // Set end position to the length of the sample
            
            generateWaveformData();
            std::cout << "Sample loaded successfully: " << filePath << std::endl; // Debug output --- IGNORE ---
        }
        catch (const std::exception &e) {
            std::cerr << "Error loading sample: " << e.what() << std::endl;
            _isLoading.store(false, std::memory_order_relaxed); // Reset loading state
            return;
        }

        _isLoading.store(false, std::memory_order_relaxed); // Reset loading state
        onDeviceChanged();
    }
};