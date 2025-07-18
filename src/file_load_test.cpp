#include "audio/choc_AudioFileFormat.h"
#include "audio/io/choc_RtAudioPlayer.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include "audio/choc_SampleBuffers.h"
#include <Security/Security.h>
#include <_types/_uint32_t.h>
#include <iostream>
#include <atomic>
#include <string>
#include <sys/types.h>
#include <thread>

using choc::buffer::ChannelArrayBuffer;
using choc::audio::WAVAudioFileFormat;


struct CallBack : public choc::audio::io::AudioMIDICallback {
    double sampleRate = 48000.0; // Default sample rate
    ChannelArrayBuffer<float> sampleBuffer; // Buffer to hold audio samples
    uint32 counter = 0;
    bool playing = false;
    std::atomic<bool> isReady = false;
    std::string filePath =
        "/Users/brandoncrowe/Documents/Audio Samples/BVKER - Elevate Beamaker "
        "Kit/Tonal Shots/BVKER - Artifacts Keys 09 - C.wav"; // Path to the WAV
                                                             // file

    void loadSample(std::string filePath) {
        isReady.store(false, std::memory_order_relaxed); // Reset isReady before loading
        WAVAudioFileFormat<false> wavFormat;
        auto reader = wavFormat.createReader(filePath);
    
        sampleBuffer = reader->readEntireStream<float>();
        if (sampleBuffer.getNumFrames() > 0) {
            isReady.store(true, std::memory_order_relaxed);
            std::cout << "Sample loaded successfully with " << sampleBuffer.getNumFrames() << " frames." << std::endl;
        } else {
            std::cerr << "Failed to load sample or sample is empty." << std::endl;
            isReady.store(false, std::memory_order_relaxed);
        }
    }

    void startBlock() override {
    }

    void processSubBlock(const choc::audio::AudioMIDIBlockDispatcher::Block &block, bool replaceOutput) override {
        auto midiMessage = block.midiMessages;
        for(auto &msg : midiMessage) {
            uint8_t msgArray[3] = {0xb0, 0x77, 0x7f};
            if(msg.message.isController() && msg.message.getNoteNumber() == 0x77 && msg.message.getVelocity() == 0x7f) { // Example MIDI message
                std::cout << "Received MIDI message: " << msg.message.toHexString() << std::endl;
                std::thread fileLoadThread([this]() {
                    loadSample(filePath);
                });
                fileLoadThread.detach();
            }
            if(msg.message.isNoteOn()) {
                playing = true; // Set playing to true when a note on message is received
                counter = 0; // Reset counter to start playback from the beginning
            } else if(msg.message.isNoteOff()) {
                playing = false; // Set playing to false when a note off message is received
            }
        }

        for(int frame = 0; frame < block.audioOutput.getNumFrames(); ++frame) {
            
            for(int channel = 0; channel < block.audioOutput.getNumChannels(); ++channel) {
                if(playing && isReady.load(std::memory_order_relaxed)) {
                    block.audioOutput.getSample(channel, frame) = sampleBuffer.getSample(channel, counter);
                } else {
                    block.audioOutput.getSample(channel, frame) = 0.0f; // Silence if not playing
                }
            }
            if(playing && isReady.load(std::memory_order_relaxed)) {
                counter++; // Increment counter to play the next sample
                if(counter >= sampleBuffer.getNumFrames()) {
                    counter = 0; // Loop back to the start if we reach the end of the sample
                }
            }
            else{
                counter = 0; // Reset counter if not playing
            }
        }
    }

    void endBlock() override {
    }
    void sampleRateChanged(double newRate) override {
        sampleRate = static_cast<uint32_t>(newRate);
    }
};

int main() {
    // Create an instance of RtAudioMIDIPlayer with default options
    choc::audio::io::AudioDeviceOptions options;
    options.blockSize = 128;
    options.sampleRate = 48000;
    options.inputChannelCount = 2;
    options.outputChannelCount = 2;
    options.audioAPI = "CoreAudio"; // Use the default audio API
    auto player = std::make_shared<choc::audio::io::RtAudioMIDIPlayer>(options);
    if (!player->getLastError().empty()) {
        std::cerr << "Error creating RtAudioMIDIPlayer: " << player->getLastError() << std::endl;
        return -1; // Exit if there was an error
    }
    CallBack callback;
    player->addCallback(callback);

    // Check if the player was created successfully
    if (player->getLastError().empty()) {
        std::cout << "RtAudioMIDIPlayer created successfully!" << std::endl;
    } else {
        std::cerr << "Error creating RtAudioMIDIPlayer: " << player->getLastError() << std::endl;
    }

    std::cin.get(); // Wait for user input before exiting
    return 0;

};