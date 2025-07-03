#include "mpc_studio_black.h"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread> // Required for std::this_thread::sleep_for

using std::to_string;

MPCStudioBlack::MPCStudioBlack(shared_ptr<RtMidiOut> midiOut, shared_ptr<RtMidiIn> midiIn)
    : midiOut(midiOut), midiIn(midiIn) 
{
    encoded_line.reserve(120); // Reserve space for 120 bytes (360 pixels / 3 bytes per pixel)
    // Set the callback for MIDI input
    midiIn->setCallback(midiInCallback, this);
    openOutputPort();
    openInputPort();

    // Switch to Private Mode on initialization
    switchToPrivateMode();
}

MPCStudioBlack::~MPCStudioBlack() {
    // Set the device back to public mode before destruction
    switchToPublicMode();
    midiIn->cancelCallback();
    midiOut->closePort();
    midiIn->closePort();
}

void MPCStudioBlack::openOutputPort() {
    auto nDevices = midiOut->getPortCount();
    for (unsigned int i = 0; i < nDevices; ++i) {
        try {
            auto name = midiOut->getPortName(i);
            if (name == MIDI_OUT_NAME) {
                std::cout << "Found MIDI output device: " << name << "\n";
                midiOut->openPort(i);
                std::cout << "Opened MIDI output port: " << name << "\n";
                return;
            }
        } catch (RtMidiError &error) {
            handleError(error);
        }
    }
}

void MPCStudioBlack::openInputPort() {
    auto nDevices = midiIn->getPortCount();
    for (unsigned int i = 0; i < nDevices; ++i) {
        try {
            auto name = midiIn->getPortName(i);
            if (name == MIDI_IN_NAME) {
                std::cout << "Found MIDI input device: " << name << "\n";
                midiIn->openPort(i);
                std::cout << "Opened MIDI input port: " << name << "\n";
                return;
            }
        } catch (RtMidiError &error) {
            handleError(error);
        }
    }
}

void MPCStudioBlack::sendSysExMessage(const std::vector<unsigned char> &message) {
    // Append the SYSEX HEADER 
    std::vector<unsigned char> sysexMessage;
    sysexMessage.insert(sysexMessage.end(), SYSEX_MESSAGE_HEADER.begin(), SYSEX_MESSAGE_HEADER.end());
    sysexMessage.insert(sysexMessage.end(), message.begin(), message.end());
    sysexMessage.push_back(SYSEX_END); // Append SYSEX END byte
    try {
        midiOut->sendMessage(&sysexMessage);
    } catch (RtMidiError &error) {
        handleError(error);
    }
}

void MPCStudioBlack::sendSysExCommand(unsigned char commandId, const std::vector<unsigned char> &data) {
    std::vector<unsigned char> message = {commandId};
    size_t dataSize = data.size();
    // 2 char array for MSB and LSB
    unsigned char msbLsb[2];
    to_msb_lsb(dataSize, msbLsb[0], msbLsb[1]);
    message.push_back(msbLsb[0]); // MSB
    message.push_back(msbLsb[1]); // LSB
    // Append the data bytes    
    message.insert(message.end(), data.begin(), data.end());
    sendSysExMessage(message);
}

void MPCStudioBlack::handleSysExMessage(const std::vector<unsigned char> &message) {
   
    // Process the SysEx message
}

void MPCStudioBlack::switchToPrivateMode() {
    std::vector<unsigned char> data = {SYSEX_PRIVATE_MODE_ID};
    sendSysExCommand(SYSEX_COMMAND_ID_MODE, data);
}

void MPCStudioBlack::switchToPublicMode() {
    std::vector<unsigned char> data = {SYSEX_PUBLIC_MODE_ID};
    sendSysExCommand(SYSEX_COMMAND_ID_MODE, data);
}

void MPCStudioBlack::handleError(const RtMidiError &error) {
    std::cerr << "MIDI Error: " << error.getMessage() << "\n";
    if (error.getType() == RtMidiError::WARNING) {
        std::cerr << "This is a warning, continuing execution.\n";
    } else {
        std::cerr << "Exiting due to error.\n";
        exit(EXIT_FAILURE);
    }
}

void MPCStudioBlack::registerControl(shared_ptr<Control> control) {
    if (control) {
        control->setMidiOutPort(midiOut);
        _controlRegistry.push_back(control);
    }
}

void MPCStudioBlack::signalControls(choc::midi::ShortMessage &msg) {
    for (const auto &control : _controlRegistry) {
        control->midiInput(msg);
    }
}

void MPCStudioBlack::setDisplayScreen(unsigned int pixel_count, unsigned int x, unsigned int y, vector<unsigned char> &pixel_data) {
    unsigned char pixel_count_msblsb[2];
    unsigned char x_msb_lsb[2];
    unsigned char y_msb_lsb[2];
    to_msb_lsb(pixel_count, pixel_count_msblsb[0], pixel_count_msblsb[1]);
    to_msb_lsb(x, x_msb_lsb[0], x_msb_lsb[1]);
    to_msb_lsb(y, y_msb_lsb[0], y_msb_lsb[1]);
    std::vector<unsigned char> message = {
         // Command ID for screen update
        pixel_count_msblsb[0],   // MSB of pixel count
        pixel_count_msblsb[1],   // LSB of pixel count
        x_msb_lsb[0],            // MSB of x position
        x_msb_lsb[1],            // LSB of x position
        y_msb_lsb[0],            // MSB of y position
        y_msb_lsb[1]             // LSB of y position
    };
    // Append pixel data
    message.insert(message.end(), pixel_data.begin(), pixel_data.end());
    // Send the SysEx message
    sendSysExCommand(SYSEX_COMMAND_ID_SCREEN, message);

}
    

void MPCStudioBlack::sendImageBuffer(cairo_surface_t *surface, unsigned int x_pos, unsigned int y_pos){

    auto width = cairo_image_surface_get_width(surface);
    auto height = cairo_image_surface_get_height(surface);
    auto stride = cairo_image_surface_get_stride(surface);
    auto image_buffer = cairo_image_surface_get_data(surface);
    int pixel_on_threshold = 180; // Threshold for pixel color to be considered "on"


    // Log the dimensions of the image
    // std::cout << "Image: pos: (" << to_string(x_pos) << "," << to_string(y_pos) << "): " << to_string(width) << "x" << to_string(height) << " with stride: " << to_string(stride) << "\n";

    unsigned char bit = 0x00;
    unsigned char bitmap[3] = {0x30, 0x0C, 0x03};

    for (int y = 0; y < height; ++y) {
        int pixel_index = 0;
        for (int x = 0; x < stride; x+=4) {
            auto bit_stride = pixel_index % 3;
            // Set pixel color to white
            
            auto pixel = image_buffer + y * stride + x;
            // convert rgb to bit on/off
            if (pixel[0] > pixel_on_threshold && pixel[1] > pixel_on_threshold && pixel[2] > pixel_on_threshold) {
                bit = bit | bitmap[bit_stride];
            } else {
                bit = bit | 0x00; // Set bit to off
            }
            if(bit_stride == 2 && x > 0) {
                encoded_line.push_back(bit);
                bit = 0x00; // Reset bit for next byte
            }
            pixel_index++;
        }
        // send the encoded line to the display
        // wait for about 1ms to ensure the display is ready
        setDisplayScreen(width, x_pos, y + y_pos, encoded_line);
        encoded_line.clear(); // Clear the encoded line for the next row
    }
}
