#include <iostream>
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/AudioFileReader.h"
#include "LabSound/extended/FunctionNode.h"
#include "Widget.h"
#include "rtmidi/RtMidi.h"
#include "control_surface/mpc_studio_black.h"
#include "ui/Widgets.h"
#include <memory>
#include "control_surface/button_control.h"
#include "control_surface/encoder_control.h"
#include "LabSound/LabSound.h"
#include "core/audio_engine.h"
#include "core/timing.h"
#include <string>
#include "core/devices/sampler_device.h"

using std::string;
using std::shared_ptr;
using std::make_shared;




int main(int, char **) {


    shared_ptr<RtMidiOut> midiout = make_shared<RtMidiOut>();
    shared_ptr<RtMidiIn> midiin = make_shared<RtMidiIn>();
    
    MPCStudioBlack mpc(midiout, midiin);
    auto recordButton = make_shared<OneColorButtonControl>( 0x49, "Record Button");
    auto qlink1 = make_shared<EncoderControl>(0, 0x13, "Q-Link 1");
    mpc.registerControl(recordButton);
    mpc.registerControl(qlink1);
    auto display = make_shared<Display>(std::bind(&MPCStudioBlack::sendImageBuffer, &mpc, std::placeholders::_1, 0, 0));
    auto page1 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 1", 24);
    display->add_page("page1", page1);
    display->initialize();
    display->show_page("page1");

    recordButton->sendColor(OneColorButtonControl::Colors::ON);

    
    AudioEngine audioEngine;
    auto context = audioEngine.activate();

    string audioFilePath = "/Users/brandoncrowe/Documents/Audio Samples/DECAP - Drums That Knock X/Hihats/DECAP hihat 219 short.wav";

    auto samplerDevice = make_shared<SamplerDevice>(context, audioFilePath);

    auto midiClock = MidiClock(context->sampleRate(), 120.0); // Initialize MidiClock with 24 PPQN and 120 BPM
    midiClock.setBPM(140.0f); // Set the tempo to 140 BPM

    auto playhead = make_shared<lab::FunctionNode>(*context.get(), 1);
    playhead->setFunction([&](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
          midiClock.processBlock(context->sampleRate(), bufferSize);
    });
    midiClock.onSongPositionDisplayChanged.connect([&](string display) {
        // std::cout << "Position: " << display << std::endl;
    });

    midiClock.onMetronomeTick.connect([&](bool isBar, bool isBeat, bool isHalfBeat) {
        if(isBeat){
            samplerDevice->trigger(); // Start playback on beat
        }
    });

    

    context->connect(context->destinationNode(), playhead, 0, 0); // Connect the sampler output to the context destination
    playhead->start(0.0f); // Start the playhead immediately
    context->synchronizeConnections();
    
    recordButton->onPressed.connect([&]() {
    });

    qlink1->onOffset.connect([&](int offset) {
        midiClock.offsetBPM(offset);
        auto newBPM = midiClock.getBPM();
        std::cout << "New BPM: " << newBPM << std::endl; // Adjust the BPM based on the encoder offset
    });

    std::cin.get(); // Wait for user input to exit

    std::cout << "MIDI ports closed. Exiting program.\n";
    return 0;
}
