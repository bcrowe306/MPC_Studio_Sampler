#pragma once
#include "LabSound/LabSound.h"
#include "audio/choc_AudioFileFormat.h"
#include "audio/choc_SampleBuffers.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include <Security/Security.h>
#include <_types/_uint64_t.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <string>
#include <sys/types.h>
#include "sigslot/signal.hpp"


using std::atomic;
using std::shared_ptr;
using std::make_shared;
using lab::AudioContext;
using lab::FunctionNode;
using choc::buffer::ChannelArrayView;
using choc::buffer::ChannelArrayBuffer;
using choc::audio::WAVAudioFileFormat;

class LinearRamp{
public:
    sigslot::signal<> onEnded; // Signal emitted when the ramp ends

    LinearRamp(double start, double end, double durationMs, double sampleRate)
        : _start(start), _end(end), _durationMs(durationMs), _currentValue(start), _sampleRate(sampleRate) {
        _calcCoefficients(); 
    }

    ~LinearRamp() {
        std::cout << "LinearRamp destroyed" << std::endl; // Debug output
    }
    // Process the ramp and return the current value. Call this function regularly to update the ramp.
    double process() {
        if(!_processing) {
            return _currentValue; 
        }

        if(_start == _end) {
            _processing = false; // If start and end are the same, stop processing
            _currentValue = _start; // Reset current value to the start
            return _currentValue; 
        }

        if(direction()){
            if (_currentValue < _end) {
              _currentValue += _step; // Increment the current value
              return _currentValue;
            }
        }
        else{
            if (_currentValue > _end) {
              _currentValue += _step; // Increment the current value
              return _currentValue;
            }
        }
    
        onEnded(); // Emit the signal when the ramp ends
        _currentValue = _end;
        _processing = false;
        return _end;
    }  

    bool direction() const {
        return _start < _end; // Return true if the ramp is increasing, false if decreasing
    }

    // Begin the ramp processing. Call this to start the ramp.
    // This resets the current value to the start and sets processing to true.
    // This allows the ramp to be processed in a loop until it reaches the end.
    void begin() {
        _processing = true; // Start processing the ramp
        _currentValue = _start; // Reset current value to the start
    }

    void reset() {
        _processing = false; // Stop processing the ramp
        _currentValue = _start; // Reset current value to the start
    }
    // Set the new start value
    void setStart(double newStart) {
        _start = newStart;
        _currentValue = _start;
        _calcCoefficients();
    }

    // Return the current start value
    double getStart() const {
        return _start; 
    }

    // Set the new end value
    void setEnd(double newEnd) {
        _end = newEnd; 
        _calcCoefficients(); 
    }

    // Return the current end value
    double getEnd() const {
        return _end; 
    }

     // Return the current duration in milliseconds
    double getDurationMs() const {
        return _durationMs; 
    }

    // Set the new duration in milliseconds
    void setDurationMs(double newDurationMs) {
        _durationMs = newDurationMs;
        _calcCoefficients(); 
    }

    // Set the sample rate for the ramp. Need to calculate time interval in samples.
    void setSampleRate(double newSampleRate) {
        _sampleRate = newSampleRate;
        _calcCoefficients(); 
    }

    // Get the current sample rate
    double getSampleRate() const {
        return _sampleRate;
    }

    // Return the current processing state
    bool getState() const {
        return _processing; 
    }

protected:
  void _calcCoefficients() {
        _step = (_end - _start) / ((_durationMs / 1000.0) *
                                   _sampleRate); // Recalculate the step size
  }
    double _start = 0.0; // Start value of the ramp
    double _end = 1.0; // End value of the ramp
    double _durationMs = 5.0; // Duration of the ramp in milliseconds
    double _currentValue; // Current value of the ramp
    double _step; // Step size for the ramp
    double _sampleRate = 48000.0; // Sample rate in Hz
    bool _processing = false; // Flag to indicate if the ramp is currently processing
};

class BufferPlayer {
public:
    shared_ptr<AudioContext> audioContext; // Audio context for the player
    LinearRamp stopRampL; // Ramp for stopping the audio
    LinearRamp stopRampR; // Ramp for stopping the audio
    LinearRamp playbackRateRamp; // Ramp for changing the playback rate
    sigslot::signal<> onFinished; // Signal emitted when playback finishes
    shared_ptr<FunctionNode> outputNode; // Function node for custom audio processing
    shared_ptr<ChannelArrayBuffer<float>> audioBufferView; // Audio buffer to play
    BufferPlayer(shared_ptr<AudioContext> context)
        : audioContext(context), 
        stopRampL(1.0, 0.0, 25.0, context->sampleRate()), 
        stopRampR(1.0, 0.0, 25.0, context->sampleRate()),
        playbackRateRamp(1.0, 1.0, 50.0, context->sampleRate())
    {
        _sampleRate = audioContext->sampleRate(); // Set the sample rate from the audio context
        outputNode = make_shared<FunctionNode>(*audioContext.get(), _channels);
        outputNode->setFunction(std::bind(&BufferPlayer::processCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        outputNode->start(0.0); // Start the function node immediately
        
    }

    ~BufferPlayer() {
        std::cout << "BufferPlayer destroyed" << std::endl; // Debug output
    }

    void processCallback(lab::ContextRenderLock & r, FunctionNode * me, int channelIndex, float * values, int bufferSize) {
         // Store the last counter value for tracking changes
        if (audioBufferView != nullptr && audioBufferView->getNumFrames() > 0 && _isPlaying.load(std::memory_order_relaxed)){
            if (channelIndex == 0) {
              _lastCounter = _counter.load(std::memory_order_relaxed);
            } else {
              _counter.store(_lastCounter, std::memory_order_relaxed); // Reset counter for the second channel
            }
        }
        

        for (int i = 0; i < bufferSize; ++i) {

            if (audioBufferView != nullptr && audioBufferView->getNumFrames() > 0 && _isPlaying.load(std::memory_order_relaxed)) {
                    
                values[i] = interpolateSample(_counter.load(std::memory_order_relaxed), channelIndex);
                _incrementCounter();

                // Apply stop ramp if active
                if (channelIndex == 0 && stopRampL.getState()) {
                    values[i] *= stopRampL.process();
                } else if (channelIndex == 1 && stopRampR.getState()) {
                    values[i] *= stopRampR.process();
                }
                if(ampRampState && !stopRampL.getState()) {
                    onRampEnded(); // Call onRampEnded if the stop ramp is not active
                }
                ampRampState = stopRampL.getState(); // Update amp ramp state
                
                // Apply the playback rate ramp
                _playbackRate.store(playbackRateRamp.process(), std::memory_order_relaxed); 
            }

            else {
                values[i] = 0.0f; // If no buffer is set or not playing, output silence
            }
        }
    }

    float interpolateSample(double currentCounter, int channelIndex) {
        uint64_t currentFrame = static_cast<uint64_t>(currentCounter);
        uint64_t nextFrame = currentFrame + 1;
        float fraction = static_cast<float>(currentCounter - static_cast<double>(currentFrame));

        // Ensure we don't read past the buffer bounds
        if (nextFrame >= audioBufferView->getNumFrames() || nextFrame >= _end) {
            nextFrame = currentFrame; // Reset next frame to current if out of bounds
        }

        // Get samples for interpolation
        float currentSample = getAudioBufferSample(channelIndex, currentFrame);
        float nextSample = getAudioBufferSample(channelIndex, nextFrame);

        // Linear interpolation (when fraction is 0, this equals currentSample)
        return currentSample + fraction * (nextSample - currentSample);
    }

    void onRampEnded() {
        _isPlaying.store(false, std::memory_order_relaxed); // Stop playback
        _ended.store(true, std::memory_order_relaxed); // Set ended state to true
        _counter.store(static_cast<double>(_start), std::memory_order_relaxed); // Reset counter to start position

    }

    // Start playback of the audio buffer with optional start and end positions
    void play(uint64_t start, uint64_t end) {

        setStart(start); // Set the start position
        setEnd(end); // Set the end position
        play(); // Start playback
    }

    void play() {
        _retrigger.store(false, std::memory_order_relaxed); // Reset retrigger flag if not already playing
        stopRampL.reset(); // Reset the stop ramp
        stopRampR.reset(); // Reset the stop ramp
        _ended.store(false, std::memory_order_relaxed); // Reset ended state
        playbackRateRamp.begin();
        _counter.store(static_cast<double>(_start), std::memory_order_relaxed); // Reset the counter to the start position
        _isPlaying.store(true, std::memory_order_relaxed); // Set the playing state to true
    }

    void stop() {
        stopRampL.begin();
        stopRampR.begin();
        playbackRateRamp.reset();
    }

    void pause() {
        _isPlaying.store(false, std::memory_order_relaxed); // Pause playback
    }


    // Get the current sample at the specified channel and frame index.
    // Handles output of mono to stereo by repeating the sample across channels.
    // This is useful for mono samples that need to be played in stereo.
    // If the channel index is out of bounds, it defaults to the first channel.
    float getAudioBufferSample(int channelIndex, int frameIndex) const {
        int cIndex = channelIndex > audioBufferView->getNumChannels()-1 ? 0 : channelIndex;
        return audioBufferView->getSample(cIndex, frameIndex);
    }

    void resume() {
        _isPlaying.store(true, std::memory_order_relaxed); // Resume playback
    }

    // Reset the player start and end positions to the beginning and end of the buffer and stops playback
    void reset() {
        _start = 0; // Reset start position to 0
        _end = audioBufferView ? audioBufferView->getNumFrames() : 0; // Reset end position to the length of the buffer
        _counter.store(static_cast<double>(_start), std::memory_order_relaxed); // Reset counter
        looping.store(false, std::memory_order_relaxed); 
        _isPlaying.store(false, std::memory_order_relaxed); 
    }

    bool isBufferValid() {
        return audioBufferView != nullptr && audioBufferView->getNumFrames() > 0; // Check if the audio buffer is valid
    }

    // Get the current position in the buffer
    double getPosition() const {
        return _counter.load(std::memory_order_relaxed); 
    }

    void setBuffer(shared_ptr<ChannelArrayBuffer<float>> buffer) {
        audioBufferView = buffer; // Set the audio buffer to play
        _counter.store(0.0, std::memory_order_relaxed); // Reset the counter to start from the beginning of the buffer
        _start = 0; // Reset start position
        _end = buffer->getNumFrames(); // Set end position to the length of the buffer
    }

    bool isPlaying() const {
        return _isPlaying.load(
            std::memory_order_relaxed); // Check if the player is currently
                                        // playing
    }
    bool hasEnded() const {
        return _ended.load(std::memory_order_relaxed); // Check if the player has ended
    }

    bool isLooping() const {
        return looping.load(std::memory_order_relaxed); // Check if the player is currently looping
    }

    void setLooping(bool loop) {
        looping.store(loop, std::memory_order_relaxed); // Set the looping state
    }

    uint64_t getLength() const {
        return audioBufferView ? audioBufferView->getNumFrames() : 0; // Get the length of the audio buffer
    }

    uint64_t getStart() const {
        return _start; // Get the start position
    }

    uint64_t getEnd() const {
        return _end; // Get the end position
    }

    void setStart(uint64_t start) {
        _start = start; // Set the start position
        if (_start >= _end) {
            _start = 0; // Reset to 0 if start is greater than or equal to end
        }
    }

    void setEnd(uint64_t end) {
        _end = end; // Set the end position
        if (_end <= _start) {
            _end = getLength(); // Reset to the length of the buffer if end is less than or equal to start
        }
    }

    void setOnPositionEndedCallback(std::function<void()> callback) {
        _onPositionEnded = callback; // Set the callback for when the position ends
    }

    // Playback rate control (1.0 = normal speed, 0.5 = half speed, 2.0 = double speed)
    float getPlaybackRate() const {
        return _playbackRate.load(std::memory_order_relaxed);
    }

    // Set the playback rate with an optional sliding effect. Sliding will apply the playback rate change over time.
    void setPlaybackRate(float rate, double durationMs = 0.0) {
        float validRate = rate > 0.0f ? rate : 1.0f; // Ensure playback rate is positive

        if(durationMs > 0.0) {
            playbackRateRamp.setStart(_playbackRate.load(std::memory_order_relaxed)); // Set the start of the ramp to the current playback rate
            playbackRateRamp.setEnd(validRate); // Set the end of the ramp to the new
            playbackRateRamp.setDurationMs(durationMs); // Set the duration of the ramp
            playbackRateRamp.begin(); // Begin the ramp
        }
        else {
            playbackRateRamp.setStart(validRate); // Set the start of the ramp to the current playback rate
            playbackRateRamp.setEnd(validRate); // Set the end of the ramp to the new
            _playbackRate.store(validRate, std::memory_order_relaxed);
        }
    }

private:
    int _channels = 2; // Number of audio channels
    float _sampleRate; // Sample rate for the audio context
    atomic<double> _counter = 0.0; // Fractional counter for smooth playback rate
    uint64_t _start = 0;
    uint64_t _end = 0;
    atomic<float> _playbackRate = 1.0f; // Playback rate (1.0 = normal speed)
    atomic<bool> looping = false; // Flag to indicate if the player is looping
    atomic<bool> _isPlaying = false; // Flag to indicate if the player is currently playing
    atomic<bool> _ended = false; // Flag to indicate if the player has ended
    double _lastCounter = 0.0; // Last counter value for tracking changes
    std::function<void()> _onPositionEnded;
    atomic<bool> _retrigger = false; // Flag to indicate if the player should retrigger on stop
    bool ampRampState = false;


    void _incrementCounter() {
        if(_isPlaying.load(std::memory_order_relaxed)){
            double currentCounter = _counter.load(std::memory_order_relaxed);
            double newCounter = currentCounter + static_cast<double>(_playbackRate.load(std::memory_order_relaxed));
            _counter.store(newCounter, std::memory_order_relaxed); // Increment by playback rate
        }

        // If the counter exceeds the end position, reset it to the start
        if(_counter.load(std::memory_order_relaxed) >= static_cast<double>(_end)){
            
            // If looping is enabled, reset the counter to the start
            if(looping.load(std::memory_order_relaxed)){
                _counter.store(static_cast<double>(_start), std::memory_order_relaxed); // Reset the counter to start
                _isPlaying.store(true, std::memory_order_relaxed); // Continue playing if looping is enabled
            } 
            
            // If not looping, stop playback and set the counter to the end
            // This allows the player to stop at the end of the buffer
            else {
                _counter.store(static_cast<double>(_end), std::memory_order_relaxed);
                _isPlaying.store(false, std::memory_order_relaxed);
                _ended.store(true, std::memory_order_relaxed);
            }

        }
    }

    
};