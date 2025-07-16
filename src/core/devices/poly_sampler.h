#pragma once
#include "LabSound/LabSound.h"
#include "core/adsr.h"
#include "audio/choc_MIDI.h"
#include "core/value_receiver.h"
#include "device_base.h"
#include "sigslot/signal.hpp"
#include "util.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
using std::string;

using lab::AudioContext;
using lab::SampledAudioNode;
using lab::GainNode;
using lab::MakeBusFromFile;
using lab::AudioBus;
using std::shared_ptr;



struct PolyVoice {
    uint32_t voiceId = 0;
    shared_ptr<SampledAudioNode> samplerNode; // Sampler node for the voice
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
    std::chrono::steady_clock::time_point triggerTime;  // When voice was triggered
    std::chrono::steady_clock::time_point releaseTime;  // When voice was released

    PolyVoice(shared_ptr<AudioContext> &audioContext)
        : audioContext(audioContext)
    {
        samplerNode = std::make_shared<SampledAudioNode>(*audioContext.get());
        samplerNode->schedule(0.0);

        sampleGainNode = std::make_shared<GainNode>(*audioContext.get());
        velocityAttenuationNode = std::make_shared<GainNode>(*audioContext.get());
        amplitudeAttenuationNode = std::make_shared<GainNode>(*audioContext.get());
        amplitudeAttenuationNode->gain()->setValue(0.0); // Default to 0.0 gain
        outputVolumeNode = std::make_shared<GainNode>(*audioContext.get());
        amplitudeEnvelopeNode = std::make_shared<ADSR>(audioContext);
        amplitudeEnvelopeNode->setAll(
            0.01f, // attack time 10 ms
            0.1f, // decay time 100 ms
            1.0f, // sustain level 100%
            0.01f  // release time 10 ms
        );
        filterEnvelopeNode = std::make_shared<ADSR>(audioContext);
        pitchEnvelopeNode = std::make_shared<ADSR>(audioContext);
        
        
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
        samplerNode->playbackRate()->setValue(1.0f); // Set the playback rate to normal speed
        
    }

    // Map MIDI velocity to a linear gain value according to the velocity sensitivity
    float calculateVelocityAttenuation(int velocity, float sensitivity) {
        float range= 24.0f;
        float mappedVelocity = mapFloat((float)velocity, 0.0f, 127.0f, -range, range); // Map velocity to a range of -12.0 to 12.0 in db
        float dbAttenuation =  mappedVelocity * sensitivity;
        return dBToLinear(dbAttenuation); // Convert dB to linear gain
    }
    void setSample(shared_ptr<AudioBus> audioBus) {
        // get sample length in seconds
        auto sampleLength = audioBus->length() / audioBus->sampleRate();
        samplerNode->setBus(audioBus);
    }

    void play() {
            
        auto baseFreq = choc::midi::noteNumberToFrequency(baseNote); // Middle C frequency
        auto frequency = choc::midi::noteNumberToFrequency(note) / baseFreq; // Calculate frequency based on note
        samplerNode->playbackRate()->setValue(frequency);
        
        velocityAttenuationNode->gain()->setValue(calculateVelocityAttenuation(velocity, velocitySensitivity));
        samplerNode->start(0.0);
    }

    void stop() {
        samplerNode->clearSchedules();
    }
    void gateEnvelopes(bool gate) {
        amplitudeEnvelopeNode->gate(gate ? 1.0f : 0.0f);
        filterEnvelopeNode->gate(gate ? 1.0f : 0.0f);
        pitchEnvelopeNode->gate(gate ? 1.0f : 0.0f);
        if(gate) {
            play(); // Start the sampler node
        } 
        else {
        }

    }

    void resetEnvelopes() {
        amplitudeEnvelopeNode->reset();
        filterEnvelopeNode->reset();
        pitchEnvelopeNode->reset();
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
    vector<PolyVoice> &voices; // List of polyphonic voices
    int maxVoices_ = 32;                  // Maximum number of voices

    // Sustain pedal state per channel (0-15)
    bool sustainPedalPressed_ = false; // Sustain pedal state per channel
    bool sustainEnabled_ = false;                   // Global sustain enable/disable

    // Voice ID tracking
    uint32_t nextVoiceId_ = 0; // Next voice ID to assign

    // Statistics (optional, for debugging/monitoring)
    mutable std::chrono::steady_clock::time_point lastUpdate_;
    mutable int voiceStealCount_; // Number of times voices were stolen
    shared_ptr<lab::FunctionNode> functionNode; 

    VoiceAllocator(shared_ptr<AudioContext> audioContext, vector<PolyVoice> &voices) : voices(voices) {
        maxVoices_ = voices.size(); 
        lastUpdate_ = std::chrono::steady_clock::now();
        voiceStealCount_ = 0; 

        functionNode = make_shared<lab::FunctionNode>(*audioContext.get()); 
        functionNode->start(0.0);
        functionNode->setFunction([&voices](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
            for(auto &voice : voices) {
                auto cursor = voice.samplerNode->getCursor();
                
                // Reset the voice if it is releasing and the envelope is idle
                if(voice.isReleasing && voice.amplitudeEnvelopeNode->getState() == envState::env_idle) {
                    voice.reset(); 
                }

                // Reset the voice is the sample has played through the entire buffer
                if(voice.samplerNode->getBus() && voice.samplerNode->getBus()->length() > 0 &&
                   voice.samplerNode->getBus()->length() <= cursor) {
                    voice.reset(); // Stop the sampler node if cursor exceeds length
                }
                
            }
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
        for (int i = 0; i < voices.size(); ++i) {
            if (!voices[i].isInUse()) {
                return i;
            }
        }
        return -1;
    }
    
    int findOldestVoice() const {
        int oldestVoice = -1;
        auto oldestTime = std::chrono::steady_clock::now();

        for (int i = 0; i < voices.size(); ++i) {
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
        std::cout << "Sustain Pedal Value: " << value << std::endl; // Debug output --- IGNORE ---
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
        for (int i = 0; i < maxVoices_; ++i) {
            const PolyVoice &voice = voices[i];
            if (voice.isActive && voice.note == note) {
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
        int existingVoice = findVoiceForNote(note);
        if (existingVoice != -1) {
            // Retrigger existing voice
            PolyVoice &voice = voices[existingVoice];
            voice.velocity = velocity;
            voice.triggerTime = std::chrono::steady_clock::now();
            voice.isReleasing = false;
            voice.isSustained = false;
            voice.voiceId = generateVoiceId();

            // Trigger ADSR envelopes and start on the sampler node
            voice.stop();
            voice.gateEnvelopes(true);

            
            return existingVoice;
        }

        // Find available voice
        int voiceIndex = findAvailableVoice();
        if (voiceIndex == -1) {
            // No available voice, steal one
            voiceIndex = findOldestVoice();
            if (voiceIndex == -1) {
                return -1;
            }
            voiceStealCount_++;
            
        }

        // Allocate voice
        std::cout << "Allocating voice: " << voiceIndex << " for note: " << note << std::endl; // Debug output --- IGNORE ---
        PolyVoice &voice = voices[voiceIndex];
        voice.note = note;
        voice.velocity = velocity;
        voice.isActive = true;
        voice.isSustained = false;
        voice.isReleasing = false;
        voice.triggerTime = std::chrono::steady_clock::now();
        voice.voiceId = generateVoiceId();

        // Trigger ADSR envelopes
        voice.gateEnvelopes(true);
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
            std::cout << "Sustain pedal pressed, sustaining voice: " << voiceIndex << std::endl;
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
    uuids::uuid id = generateUUID(); // Unique identifier for the poly sampler device
    vector<float> _waveformData; // Waveform data for the poly sampler
    const int kMaxVoices = 32; // Maximum number of voices for polyphony
    vector<PolyVoice> _voices; // Voices for the poly sampler
    VRFloat gainDB = VRFloat("Gain", 0.0f, -60.0f, 12.0f, 1.0f, 0.1f); // Gain parameter for the poly sampler
    VRString filePath = VRString("filePath", ""); // File path for the sample
    shared_ptr<AudioBus> _sampleBus;
    shared_ptr<VoiceAllocator> voiceAllocator; // Voice allocator for managing polyphony


    PolySampler(std::shared_ptr<lab::AudioContext> audioContext)
        : DeviceBase(audioContext)
    {
        for (int i = 0; i < kMaxVoices; ++i) {
            _voices.emplace_back(audioContext);
        }
        for(auto &voice : _voices) {
            audioContext->connect(output, voice.getOutputNode(), 0, 0); // Connect each voice's output to the device output
        }
        voiceAllocator = std::make_shared<VoiceAllocator>(audioContext, _voices); // Initialize the voice allocator
        filePath.onValueChanged.connect([this](const string &value) {
            _loadSample(value); // Load the sample when the file path changes
        });
        filePath.updateObservers(); // Update observers with the initial file path
    }

    void midiInput(choc::midi::ShortMessage &msg) override {
        voiceAllocator->processMidiMessage(msg); // Process MIDI messages through the voice allocator
    }

    void clearSample() {
        if (_sampleBus) {
            _sampleBus->reset();
        }
        _waveformData.clear();
    }

    void generateWaveformData(){
        _waveformData.clear();
        if (!_sampleBus) return;

        const int numChannels = _sampleBus->numberOfChannels();
        const int numFrames = _sampleBus->length();
        const int numPixels = 240;

        _waveformData.resize(numPixels, 0.0f);

        if (numChannels == 0 || numFrames == 0) return;

        const float* channelData = _sampleBus->channel(0)->data(); // Use first channel

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
    void serialize()  override {}
    void deserialize() override {}

  private:
    void _loadSample(const std::string &filePath) {

        clearSample(); // Clear any existing sample
        if (filePath.empty()) {
            onDeviceChanged();
            return;
        }
        
        _sampleBus = MakeBusFromFile(filePath, false, _audioContext->sampleRate());
        if (_sampleBus) {
            for (auto &voice : _voices) {
                voice.setSample(_sampleBus); // Set the sample for each voice
            }
            generateWaveformData(); // Generate waveform data after loading the
                                    // sample
        } else {
            std::cerr << "Failed to load sample: " << filePath << std::endl;
        }
        onDeviceChanged();
  }
};