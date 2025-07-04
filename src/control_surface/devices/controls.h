#include "control_surface/controls/controls.h"
#include "mpc_studio_surface_def.h"
#include <unordered_map>
#include <string>
#include <memory>

using std::string;
using std::shared_ptr;
using std::make_shared;

using std::unordered_map;

static inline unordered_map<string, shared_ptr<Control>> MPCStudioBlackControls = {
    {"recordButton", make_shared<OneColorButtonControl>(REC_BUTTON, "Record Button")},
    {"playButton", make_shared<OneColorButtonControl>(PLAY_BUTTON, "Play Button")},
    {"stopButton", make_shared<OneColorButtonControl>(STOP_BUTTON, "Stop Button")},
    {"playStart", make_shared<OneColorButtonControl>(PLAY_START, "Play Start Button")},
    {"overdub", make_shared<OneColorButtonControl>(OVERDUB, "Overdub Button")},
    {"stepLeft", make_shared<OneColorButtonControl>(STEP_LEFT, "Step Left Button")},
    {"stepRight", make_shared<OneColorButtonControl>(STEP_RIGHT, "Step Right Button")},
    {"goto", make_shared<OneColorButtonControl>(GOTO, "Goto Button")},
    {"barLeft", make_shared<OneColorButtonControl>(BAR_LEFT, "Bar Left Button")},
    {"barRight", make_shared<OneColorButtonControl>(BAR_RIGHT, "Bar Right Button")},
    {"tapTempo", make_shared<OneColorButtonControl>(TAP_TEMPO, "Tap Tempo Button")},
    {"leftArrow", make_shared<OneColorButtonControl>(LEFT, "Left Arrow Button")},
    {"rightArrow", make_shared<OneColorButtonControl>(RIGHT, "Right Arrow Button")},
    {"upArrow", make_shared<OneColorButtonControl>(UP, "Up Arrow Button")},
    {"downArrow", make_shared<OneColorButtonControl>(DOWN, "Down Arrow Button")},
    {"undo", make_shared<OneColorButtonControl>(UNDO, "Undo Button")},
    {"shift", make_shared<OneColorButtonControl>(SHIFT, "Shift Button")},
    {"plus", make_shared<OneColorButtonControl>(PLUS, "Plus Button")},
    {"minus", make_shared<OneColorButtonControl>(MINUS, "Minus Button")},
    {"window", make_shared<OneColorButtonControl>(WINDOW, "Window Button")},
    {"main", make_shared<OneColorButtonControl>(MAIN, "Main Button")},
    {"browser", make_shared<OneColorButtonControl>(BROWSER, "Browser Button")},
    {"numeric", make_shared<OneColorButtonControl>(NUMERIC, "Numeric Button")},
    {"jogWheel", make_shared<EncoderControl>(0, JOG_WHEEL, "Jog Wheel")},
    {"project", make_shared<OneColorButtonControl>(PROJECT, "Project Button")},
    {"seq", make_shared<OneColorButtonControl>(SEQ, "Seq Button")},
    {"prog", make_shared<OneColorButtonControl>(PROG, "Prog Button")},
    {"sample", make_shared<OneColorButtonControl>(SAMPLE, "Sample Button")},
    {"noFilter", make_shared<OneColorButtonControl>(NO_FILTER, "No Filter Button")},
    {"f1Button", make_shared<PlainButtonControl>(F1_BUTTON, "F1 Button")},
    {"f2Button", make_shared<PlainButtonControl>(F2_BUTTON, "F2 Button")},
    {"f3Button", make_shared<PlainButtonControl>(F3_BUTTON, "F3 Button")},
    {"f4Button", make_shared<PlainButtonControl>(F4_BUTTON, "F4 Button")},
    {"f5Button", make_shared<PlainButtonControl>(F5_BUTTON, "F5 Button")},
    {"f6Button", make_shared<PlainButtonControl>(F6_BUTTON, "F6 Button")},
    {"qlinkTrigger", make_shared<OneColorButtonControl>(QLINK_TRIGGER, "Q-Link Trigger Button")},
    {"erase", make_shared<OneColorButtonControl>(ERASE, "Erase Button")},
    {"noteRepeat", make_shared<OneColorButtonControl>(NOTE_REPEAT, "Note Repeat Button")},
    {"qlink1", make_shared<EncoderControl>(0, QLINK1, "Q-Link 1")},
    {"qlink2", make_shared<EncoderControl>(0, QLINK2, "Q-Link 2")},
    {"qlink3", make_shared<EncoderControl>(0, QLINK3, "Q-Link 3")},
    {"qlink4", make_shared<EncoderControl>(0, QLINK4, "Q-Link 4")},
    {"qlink_touch1", make_shared<PlainButtonControl>(QLINK1_TOUCH, "Q-Link Touch 1 Button")},
    {"qlink_touch2", make_shared<PlainButtonControl>(QLINK2_TOUCH, "Q-Link Touch 2 Button")},
    {"qlink_touch3", make_shared<PlainButtonControl>(QLINK3_TOUCH, "Q-Link Touch 3 Button")},
    {"qlink_touch4", make_shared<PlainButtonControl>(QLINK4_TOUCH, "Q-Link Touch 4 Button")},
    {"qlinkScroll", make_shared<EncoderControl>(0, QLINK_SCROLL, "Q-Link Scroll Button")},
    {"padBackA", make_shared<TwoColorButtonControl>(PAD_BACK_A, "Pad Back A Button")},
    {"padBackB", make_shared<TwoColorButtonControl>(PAD_BACK_B, "Pad Back B Button")},
    {"padBackC", make_shared<TwoColorButtonControl>(PAD_BACK_C, "Pad Back C Button")},
    {"padBackD", make_shared<TwoColorButtonControl>(PAD_BACK_D, "Pad Back D Button")},
    {"padAssign", make_shared<OneColorButtonControl>(PAD_ASSIGN, "Pad Assign Button")},
    {"pad1", make_shared<PadControl>(9, PAD1, 0, "Pad 1")},
    {"pad2", make_shared<PadControl>(9, PAD2, 1, "Pad 2")},
    {"pad3", make_shared<PadControl>(9, PAD3, 2, "Pad 3")},
    {"pad4", make_shared<PadControl>(9, PAD4, 3, "Pad 4")},
    {"pad5", make_shared<PadControl>(9, PAD5, 4, "Pad 5")},
    {"pad6", make_shared<PadControl>(9, PAD6, 5, "Pad 6")},
    {"pad7", make_shared<PadControl>(9, PAD7, 6, "Pad 7")},
    {"pad8", make_shared<PadControl>(9, PAD8, 7, "Pad 8")},
    {"pad9", make_shared<PadControl>(9, PAD9, 8, "Pad 9")},
    {"pad10", make_shared<PadControl>(9, PAD10, 9, "Pad 10")},
    {"pad11", make_shared<PadControl>(9, PAD11, 10, "Pad 11")},
    {"pad12", make_shared<PadControl>(9, PAD12, 11, "Pad 12")},
    {"pad13", make_shared<PadControl>(9, PAD13, 12, "Pad 13")},
    {"pad14", make_shared<PadControl>(9, PAD14, 13, "Pad 14")},
    {"pad15", make_shared<PadControl>(9, PAD15, 14, "Pad 15")},
    {"pad16", make_shared<PadControl>(9, PAD16, 15, "Pad 16")}
};

