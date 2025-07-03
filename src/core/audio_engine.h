#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H
#include "LabSound/LabSound.h"
#include "LabSound/backends/AudioDevice_RtAudio.h"
#include <memory>
using namespace lab;
using std::vector;
using std::shared_ptr;

using namespace lab;

class AudioEngine
{
private:
    /* data */
public:
    AudioStreamConfig inputConfig;
    AudioStreamConfig outputConfig;
    vector<AudioDeviceInfo> audioDevices;
    shared_ptr<lab::AudioDevice_RtAudio> audioDevice;
    AudioDeviceInfo defaultOutputInfo, defaultInputInfo;
    shared_ptr<AudioContext> context;
    shared_ptr<AudioDestinationNode> destinationNode;
    AudioEngine(/* args */);
    ~AudioEngine();
    void init();
    shared_ptr<AudioContext> activate();
    void deactivate();
  };



#endif