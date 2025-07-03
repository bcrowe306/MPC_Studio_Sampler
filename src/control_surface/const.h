#pragma once
#include <array>
#include <string>
using std::string;

inline string MIDI_OUT_NAME = "MPC Studio Black MPC Private";
inline string MIDI_IN_NAME = "MPC Studio Black MPC Private";
inline unsigned char MIDI_CHANNEL = 0;
inline unsigned char SYSEX_START = 0xF0;
inline unsigned char SYSEX_END = 0xF7;
// f0 47 7f 3d 62 00 01 61 f7 # Private Mode

inline unsigned char SYSEX_MANUFACTURER_ID = 0x47; // Akai Professional
inline unsigned char SYSEX_DEVICE_ID = 0x7F; // Device ID, 0x7F is for all devices
inline unsigned char SYSEX_MODEL_ID = 0x3D; // MPC Studio Black
inline unsigned char SYSEX_COMMAND_ID_MODE = 0x62; // Private Mode command
inline unsigned char SYSEX_COMMAND_ID_SCREEN = 0x04; // Screen command
inline unsigned char SYSEX_PRIVATE_MODE_ID = 0x61; // Private Mode
inline unsigned char SYSEX_PUBLIC_MODE_ID = 0x02; // Public Mode 

inline unsigned int DISPLAY_WIDTH = 360;
inline unsigned int DISPLAY_HEIGHT = 96;

inline std::array<unsigned char, 4> SYSEX_MESSAGE_HEADER = {
    SYSEX_START, 
    SYSEX_MANUFACTURER_ID, 
    SYSEX_DEVICE_ID,
    SYSEX_MODEL_ID
};

