#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include "audio/choc_SampleBuffers.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include "core/linear_ramp.h"

using namespace lab;
using choc::audio::WAVAudioFileFormat;
typedef std::atomic<float> AFloat;
typedef std::atomic<bool> ABool;
typedef std::atomic<int> AInt;
typedef std::shared_ptr<choc::buffer::ChannelArrayBuffer<float>> AudioBufferPtr;

struct BufferStream {
    float _counter = 0.0f;
    int _start = 0;
    int _end = 0;
    bool _isPlaying = false; // Flag to indicate if the node is currently playing
    bool _ended = false;     // Flag to indicate if the node has ended
    float _playbackRate = 1.0f; // Playback
    bool looping = false;       // Flag to indicate if the node is looping
    LRamp _downRamp = LRamp(1.0, 0.0, 5.0, 48000.0); // Volume ramp for the stream
    LRamp _upRamp = LRamp(0.0, 1.0, 5.0, 48000.0); // Volume ramp for the stream
    LRamp _playbackRateRamp = LRamp(1.0, 1.0, 2.0, 48000.0); // Playback rate ramp

    BufferStream(){
        _downRamp.onEnded.connect([this]() {
            _isPlaying = false; // Stop playing when the down ramp ends
            _ended = true; // Set ended state to true
            _counter = static_cast<float>(_start); // Reset counter to start
        });
    }

    void setPlaybackRate(float rate, double durationMs = 0.0) {
        if(durationMs > 0.0) {
            _playbackRateRamp.setStart(_playbackRate); // Set the start of the ramp to the current playback rate
            _playbackRateRamp.setEnd(rate); // Set the end of the ramp to the new
            _playbackRateRamp.setDurationMs(durationMs); // Set the duration of the ramp
            _playbackRateRamp.begin(); // Begin the ramp
        }
        else {
            _playbackRateRamp.setStart(rate); // Set the start of the ramp to the current playback rate
            _playbackRateRamp.setEnd(rate); // Set the end of the ramp to the new
            _playbackRate = rate;
        }
    }

    float getPlaybackRate() const {
        return _playbackRate; // Get the playback rate
    }

    void play(){
        _counter = static_cast<float>(_start); // Reset counter to start
        _isPlaying = true; // Set the playing state to true
        _ended = false; // Reset ended state
    }

    bool isEnded() const {
        return _ended; // Check if the node has ended
    }

    void stop() {
        _downRamp.begin(); // Begin the down ramp to stop playback
    }

    void setStart(int start) {
        _start = start; // Set the start position
        _counter = static_cast<double>(_start); // Reset counter to start
    }

    int getStart() const {
        return _start; // Get the start position
    }

    void setEnd(int end) {
        _end = end; // Set the end position
        if (_end <= _start) {
            _end = _start + 1; // Ensure end is greater than start
        }
    }
    bool isPlaying() const {
        return _isPlaying; // Check if the node is currently playing
    }

    int getEnd() const {
        return _end; // Get the end position
    }

    void setLooping(bool loop) {
        looping = loop; // Set the looping state
    }

    float interpolateSample(double currentCounter, int channelIndex, AudioBufferPtr &buffer) {
        uint64_t currentFrame = static_cast<uint64_t>(currentCounter);
        uint64_t nextFrame = currentFrame + 1;
        float fraction = static_cast<float>(currentCounter - static_cast<double>(currentFrame));

        // Ensure we don't read past the buffer bounds
        if (nextFrame >= buffer->getNumFrames() || nextFrame >= _end) {
            nextFrame =
                currentFrame; // Reset next frame to current if out of bounds
        }

        // Get samples for interpolation
        float currentSample = getAudioBufferSample(channelIndex, currentFrame, buffer);
        float nextSample = getAudioBufferSample(channelIndex, nextFrame, buffer);

        // Linear interpolation (when fraction is 0, this equals currentSample)
        return currentSample + fraction * (nextSample - currentSample);
    }

    float getAudioBufferSample(int channelIndex, int frameIndex, AudioBufferPtr &buffer) {
        int cIndex = channelIndex > buffer->getNumChannels() - 1
                         ? 0
                         : channelIndex;
        return buffer->getSample(cIndex, frameIndex);
    }

    void incrementCounter() {
        _counter += _playbackRate; // Increment the counter by the playback rate
        _downRamp.process();
        _playbackRate = _playbackRateRamp.process();


        if(_counter >= static_cast<double>(_end)){
            if(looping){
                _counter = static_cast<double>(_start); // Reset counter to start
                _isPlaying = true; // Continue playing if looping is enabled
            } else {
                _counter = static_cast<double>(_end);
                _isPlaying = false;
                _ended = true;
            }
        }
    }

    float process(int channelIndex, AudioBufferPtr &buffer) {
        if(!_isPlaying) {
            return 0.0f; // If not playing, return silence
        }
        else{
            float output = interpolateSample(_counter, channelIndex, buffer);
            if(_downRamp.getState()) {
                output *= _downRamp.getValue(); // Apply down ramp if active
            }
            return output;
        }
    }
};

class BufferNode : public AudioScheduledSourceNode {

public:
    BufferNode(AudioContext &ac) : AudioScheduledSourceNode(ac, {nullptr, nullptr, 2}) 
    {
        _audioContext = &ac; // Set the audio context
        initialize();
        for(auto &stream : _streams) {
            stream._downRamp.setSampleRate(_audioContext->sampleRate()); // Initialize playback rate for each stream
            stream._upRamp.setSampleRate(_audioContext->sampleRate()); // Initialize playback rate for each stream
        }
    }
    
    virtual ~BufferNode() { uninitialize(); };

    static const char *static_name() { return "Buffer"; }
    virtual const char *name() const override { return static_name(); }
    static AudioNodeDescriptor *desc() {
        static AudioNodeDescriptor d{nullptr, nullptr, 1};
        return &d;
    };


    virtual void process(ContextRenderLock &r, int bufferSize) override {
        AudioBus * outputBus = output(0)->bus(r);

        if (!isInitialized() || !outputBus->numberOfChannels())
        {
            outputBus->zero();
            return;
        }

        int quantumFrameOffset = _self->_scheduler._renderOffset;
        int nonSilentFramesToProcess = _self->_scheduler._renderLength;

        if (!nonSilentFramesToProcess)
        {
            outputBus->zero();
            return;
        }

        float * lChannel = outputBus->channel(0)->mutableData();
        float * rChannel = outputBus->channel(1)->mutableData();

        for (int frame = 0; frame < nonSilentFramesToProcess; ++frame)
        {
            for(int channel = 0; channel < outputBus->numberOfChannels(); ++channel)
            {
                if(!isLoading()){
                    float samp = 0.0f;
                    for(auto &stream : _streams) {
                        samp += stream.process(channel, _audioBuffer); // Process each stream for the current channel
                    }
                    if(channel == 0) {
                        lChannel[frame] = samp;
                    } else if(channel == 1) {
                        rChannel[frame] = samp;
                    }

                }
                else {
                    if(channel == 0) {
                        lChannel[frame] = 0.0f; // If loading, output silence
                    } else if(channel == 1) {
                        rChannel[frame] = 0.0f; // If loading, output silence
                    }
                }
                
            }

            for(auto &stream : _streams) {
                stream.incrementCounter(); // Increment the counter for each stream
            }
            
        }

        _now += double(bufferSize) / r.context()->sampleRate();
        outputBus->clearSilentFlag();
    };


    virtual void reset(ContextRenderLock &r) override
    {
        // No-op
    }

    double now() const { return _now; }

    void play(){
        int playingStreamIndex = getPlayingStreamIndex();
        if(playingStreamIndex > -1) {
            _streams[playingStreamIndex].stop(); // Resume playback for the first playing stream
            _streams[(playingStreamIndex + 1) % _streams.size()].play(); // Restart playback for the next stream
        }
        else {
            _streams[0].play(); // Start playback for the first stream
        }
        
    }

    void play(uint64_t start, uint64_t end) {
        _start = start; // Set the start position
        _end = end; // Set the end position
        for(auto &stream : _streams) {
            stream.setStart(start); // Set the start position for each stream
            stream.setEnd(end); // Set the end position for each stream
            stream.play(); // Start playback for each stream
        }
    }

    int getPlayingStreamIndex() const {
        for (int i = 0; i < _streams.size(); ++i) {
            if (_streams[i].isPlaying()) {
                return i; // Return the index of the first playing stream
            }
        }
        return -1; // No streams are playing
    }

    void stop() {
        for(auto &stream : _streams) {
            stream.stop(); // Stop each stream
        }
    }

    bool isPlaying() const {
        return getPlayingStreamIndex() > -1; // Check if any stream is currently playing
    }

    bool isLoading() const {
        return _isLoading; // Check if the node is currently loading audio data
    }

    bool isEnded() const {
        for(auto &stream : _streams) {
            if(!stream.isEnded()) {
                return false; // If any stream has not ended, return false
            }
        }
        return true; // All streams have ended
    }

    void loadSample(const std::string &filePath) {
        if(filePath.empty()) {
            return;
        }
        _loadThread = std::thread(&BufferNode::_doLoadSample, this, filePath);
        _loadThread.detach(); // Detach the thread to allow it to run independently
    }

    void setPlaybackRate(float rate, double durationMs = 0.0) {
        _playbackRate = rate; // Set the playback rate
        for(auto &stream : _streams) {
            stream.setPlaybackRate(rate, durationMs); // Set the playback rate for each stream
        }
    }

    void setPlaybackRateFromNote(float note, float baseNote = 60.0f, double durationMs = 0.0) {
        float rate = pow(2.0f, (note - baseNote) / 12.0f); // Calculate playback rate based on MIDI note
        setPlaybackRate(rate, durationMs); // Set the calculated playback rate
    }

    void setStart(int start) {
        _start = start; // Set the start position
        for(auto &stream : _streams) {
            stream.setStart(start); // Set the start position for each stream
        }
    }

    int getStart() const {
        return _start; // Get the start position
    }

    void setRange(int start, int end) {
        setStart(start); // Set the start position
        setEnd(end); // Set the end position
    }

    void setEnd(int end) {
        _end = end; // Set the end position
        if (_end <= _start) {
            _end = _start + 1; // Ensure end is greater than start
        }
        for(auto &stream : _streams) {
            stream.setEnd(end); // Set the end position for each stream
        }
    }

    int getEnd() const {
        return _end; // Get the end position
    }

    void setBuffer(AudioBufferPtr buffer) {
        _isLoading = true; // Set loading state to true
        _audioBuffer = buffer; // Set the audio buffer
        if (_audioBuffer) {
            _start = 0; // Reset start position
            _end = _audioBuffer->getNumFrames(); // Set end position to the length of the buffer
            for(auto &stream : _streams) {
                stream.setStart(_start); // Set start position for each stream
                stream.setEnd(_end); // Set end position for each stream
            }
        }
        _isLoading = false; // Set loading state to false after loading
    }

    void setLooping(bool loop) {
        looping = loop; // Set the looping state
        for(auto &stream : _streams) {
            stream.setLooping(loop); // Set the looping state for each stream
        }
    }

    bool isLooping() const {
        return looping; // Check if the node is currently looping
    }

private:
    virtual bool propagatesSilence(ContextRenderLock &r) const override 
    {
        return !isPlayingOrScheduled() || hasFinished();
    }
    virtual double tailTime(ContextRenderLock &r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock &r) const override { return 0; }

    void _doLoadSample(const std::string &filePath) {
        if(filePath.empty()) {
            return;
        }
        _isLoading = true; // Set loading state to true
        auto sampleData = WAVAudioFileFormat<false>().createReader(filePath)->loadFileContent(
            _audioContext->sampleRate()
        );
        _audioBuffer = std::make_shared<choc::buffer::ChannelArrayBuffer<float>>(sampleData.frames);
        setStart(0);
        setEnd(_audioBuffer->getNumFrames() - 1); // Set end position to the length of the sample
        _isLoading = false; // Set loading state to false after loading
    }


  double _now = 0.0;
  AudioContext *_audioContext = nullptr; // Pointer to the audio context
  int _start = 0;
  int _end = 0;
  float _playbackRate = 1.0f; // Playback rate
  AudioBufferPtr _audioBuffer; // Sample buffer for audio data
  bool _isLoading = false; // Flag to indicate if the node is currently loading audio data
  bool looping = false; // Flag to indicate if the node is looping
  std::thread _loadThread; // Thread for loading audio data
  std::array<BufferStream, 2> _streams = {BufferStream(), BufferStream()}; // Streams for stereo output
};