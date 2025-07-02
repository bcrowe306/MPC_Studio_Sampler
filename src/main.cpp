#include <iostream>
#include "Widget.h"
#include "rtmidi/RtMidi.h"
#include "device/mpc_studio_black.h"
#include "ui/Widgets.h"
#include <memory>
#include "device/button_control.h"
#include "device/encoder_control.h"

using std::shared_ptr;
using std::make_shared;

int main(int, char **) {

    shared_ptr<RtMidiOut> midiout = make_shared<RtMidiOut>();
    shared_ptr<RtMidiIn> midiin = make_shared<RtMidiIn>();

    MPCStudioBlack mpc(midiout, midiin);
    auto recordButton = make_shared<ButtonControl>(Control::Type::NOTE, 0, 0x49, "Record Button");
    auto qlink1 = make_shared<EncoderControl>(0, 0x13, "Q-Link 1");
    mpc.registerControl(recordButton);
    mpc.registerControl(qlink1);
    auto display = make_shared<Display>(std::bind(&MPCStudioBlack::sendImageBuffer, &mpc, std::placeholders::_1, 0, 0));
    auto page1 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 1", 24);
    auto page2 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 2", 24);
    auto page3 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 3", 24);
    auto page4 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 4", 24);
    auto page5 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 5", 24);

    display->add_page("page1", page1);
    display->add_page("page2", page2);
    display->add_page("page3", page3);
    display->add_page("page4", page4);
    display->add_page("page5", page5);

    display->initialize();
    display->show_page("page1");
    
    mpc.onMidiMessage = [&](choc::midi::ShortMessage msg) {
        std::cout << "Midi Msg:" << msg.toHexString() << "\n";

        
    };

    std::cin.get(); // Wait for user input to exit

    std::cout << "MIDI ports closed. Exiting program.\n";
    return 0;
}
