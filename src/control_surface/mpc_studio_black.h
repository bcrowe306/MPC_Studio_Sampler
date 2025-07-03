#pragma once
#include "const.h"
#include "rtmidi/RtMidi.h"
#include "util.h"
#include <string>
#include <vector>
#include "audio/choc_MIDI.h"
#include <functional>
#include "cairo.h"
#include "control.h"
#include <memory>

using std::shared_ptr;
using std::vector;
using std::string;

class MPCStudioBlack {
public:
    std::function <void(choc::midi::ShortMessage)> onMidiMessage;

public:
    MPCStudioBlack(shared_ptr<RtMidiOut> midiOut, shared_ptr<RtMidiIn> midiIn);
    ~MPCStudioBlack();
    void openOutputPort();
    void openInputPort();
    void sendSysExMessage(const std::vector<unsigned char> &message);
    void switchToPrivateMode();
    void switchToPublicMode();
 
    void setDisplayScreen(unsigned int pixel_count, unsigned int x, unsigned int y, vector<unsigned char> &pixel_data);

    static void midiInCallback(
        double deltatime, std::vector<unsigned char> *message, void *userData) {
      MPCStudioBlack *instance = static_cast<MPCStudioBlack *>(userData);
      if (message->size() > 0 && (*message)[0] == SYSEX_START) {
        // Handle SysEx message
        instance->handleSysExMessage(*message);
      } else {
        // Handle other MIDI messages if needed
        choc::midi::ShortMessage midiMessage(message->data(), message->size());
        instance->signalControls(midiMessage);
        if(instance->onMidiMessage) {
          instance->onMidiMessage(midiMessage);
        }
      }
    };

    void registerControl(shared_ptr<Control> control);

    void signalControls(choc::midi::ShortMessage &msg);

    void sendImageBuffer(cairo_surface_t *surface, unsigned int x_pos, unsigned int y_pos); 

  private:
    vector<shared_ptr<Control>> _controlRegistry;
    vector<unsigned char> encoded_line;
    shared_ptr<RtMidiOut> midiOut;
    shared_ptr<RtMidiIn> midiIn;
    void handleError(const RtMidiError &error);
    void handleSysExMessage(const std::vector<unsigned char> &message);
    void sendSysExCommand(unsigned char commandId, const std::vector<unsigned char> &data);
};