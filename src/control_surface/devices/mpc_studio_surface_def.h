#pragma once
#include <array>
#include <string>
#include <unordered_map>
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

inline  unsigned char REC_BUTTON = 0x49;
inline  unsigned char PLAY_BUTTON = 0x52;
inline  unsigned char STOP_BUTTON = 0x51;
inline  unsigned char PLAY_START = 0x53; 
inline  unsigned char OVERDUB = 0x50;
inline  unsigned char STEP_LEFT = 0x44;
inline  unsigned char STEP_RIGHT = 0x45;
inline  unsigned char GOTO = 0x46;
inline  unsigned char BAR_LEFT = 0x47;
inline  unsigned char BAR_RIGHT = 0x48;
inline  unsigned char TAP_TEMPO = 0x35;
inline  unsigned char LEFT = 0x41;
inline  unsigned char RIGHT = 0x42;
inline  unsigned char UP = 0x38;
inline  unsigned char DOWN = 0x39;
inline  unsigned char UNDO = 0x43;
inline  unsigned char SHIFT = 0x31;
inline  unsigned char PLUS = 0x36;
inline  unsigned char MINUS = 0x37;
inline  unsigned char WINDOW = 0x33;
inline  unsigned char MAIN = 0x34;
inline  unsigned char BROWSER = 0x32;
inline  unsigned char NUMERIC = 0x6F;
inline  unsigned char JOG_WHEEL = 0x64; 
inline  unsigned char PROJECT = 0x2C;
inline  unsigned char SEQ = 0x2D;
inline  unsigned char PROG = 0x2E;
inline  unsigned char SAMPLE = 0x2F; 
inline  unsigned char NO_FILTER = 0x30;
inline  unsigned char PROG_EDIT = 0x02;
inline  unsigned char PROG_MIX = 0x03;
inline  unsigned char SEQ_EDIT = 0x05;
inline  unsigned char SAMPLE_EDIT = 0x06;
inline  unsigned char SONG = 0x08;
inline  unsigned char FULL_LEVEL = 0x27;
inline  unsigned char SIXTEEN_LEVEL = 0x28;
inline  unsigned char STEP_SEQ = 0x29;
inline  unsigned char NEXT_SEQ = 0x2A;
inline  unsigned char TRACK_MUTE = 0x2B;
inline  unsigned char PAD_BACK_A = 0x23;
inline  unsigned char PAD_BACK_B = 0x24;
inline  unsigned char PAD_BACK_C = 0x25;
inline  unsigned char PAD_BACK_D = 0x26;
inline  unsigned char PAD_ASSIGN = 0x70;
inline  unsigned char F1_BUTTON = 0x0C;
inline  unsigned char F2_BUTTON = 0x0D;
inline  unsigned char F3_BUTTON = 0x0E;
inline  unsigned char F4_BUTTON = 0x0F;
inline  unsigned char F5_BUTTON = 0x21;
inline  unsigned char F6_BUTTON = 0x22;
inline  unsigned char QLINK_TRIGGER = 0x71;
inline  unsigned char ERASE = 0x09;
inline  unsigned char NOTE_REPEAT = 0x0B;
inline  unsigned char QLINK1 = 0x10;
inline  unsigned char QLINK2 = 0x11;
inline  unsigned char QLINK3 = 0x12;
inline  unsigned char QLINK4 = 0x13;
inline  unsigned char QLINK_SCROLL = 0x65;
inline  unsigned char QLINK1_TOUCH = 0x54;
inline  unsigned char QLINK2_TOUCH = 0x55;
inline  unsigned char QLINK3_TOUCH = 0x56;
inline  unsigned char QLINK4_TOUCH = 0x57;
inline  unsigned char PAD1 = 0x25;
inline  unsigned char PAD2 = 0x24;
inline  unsigned char PAD3 = 0x2A;
inline  unsigned char PAD4 = 0x52;
inline  unsigned char PAD5 = 0x28;
inline  unsigned char PAD6 = 0x26;
inline  unsigned char PAD7 = 0x2E;
inline  unsigned char PAD8 = 0x2C;
inline  unsigned char PAD9 = 0x30;
inline  unsigned char PAD10 = 0x2F;
inline  unsigned char PAD11 = 0x2D;
inline  unsigned char PAD12 = 0x2B;
inline  unsigned char PAD13 = 0x31;
inline  unsigned char PAD14 = 0x37;
inline  unsigned char PAD15 = 0x33;
inline  unsigned char PAD16 = 0x35;

inline std::array<unsigned char, 16> PAD_BUTTONS = {
    PAD1, PAD2, PAD3, PAD4,
    PAD5, PAD6, PAD7, PAD8,
    PAD9, PAD10, PAD11, PAD12,
    PAD13, PAD14, PAD15, PAD16
};

inline std::array<unsigned char, 4> QLINKS = {
    QLINK1, QLINK2, QLINK3, QLINK4
};

inline std::array<unsigned char, 4> QLINK_TOUCH = {
    QLINK1_TOUCH, QLINK2_TOUCH, QLINK3_TOUCH, QLINK4_TOUCH
};

inline std::array<unsigned char, 6> F_KEYS = {
    F1_BUTTON, F2_BUTTON, F3_BUTTON, F4_BUTTON,
    F5_BUTTON, F6_BUTTON
};

inline std::unordered_map<unsigned char, int > PAD_MAP = {
    {PAD1, 0}, {PAD2, 1}, {PAD3, 2}, {PAD4, 3},
    {PAD5, 4}, {PAD6, 5}, {PAD7, 6}, {PAD8, 7},
    {PAD9, 8}, {PAD10, 9}, {PAD11, 10}, {PAD12, 11},
    {PAD13, 12}, {PAD14, 13}, {PAD15, 14}, {PAD16, 15}
};