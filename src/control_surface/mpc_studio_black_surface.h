#include "devices/mpc_studio_black.h"
#include "devices/controls.h"
#include <memory>
#include "devices/mpc_studio_surface_def.h"


class MPCStudioBlackControlSurface : public enable_shared_from_this<MPCStudioBlackControlSurface>{
public:
    shared_ptr<RtMidiOut> midiout = make_shared<RtMidiOut>();
    shared_ptr<RtMidiIn> midiin = make_shared<RtMidiIn>();
    shared_ptr<MPCStudioBlackDevice> device;

    // Controls
    shared_ptr<OneColorButtonControl> playButton = make_shared<OneColorButtonControl>(PLAY_BUTTON, "Play Button");
    shared_ptr<OneColorButtonControl> stopButton = make_shared<OneColorButtonControl>(STOP_BUTTON, "Stop Button");
    shared_ptr<OneColorButtonControl> recordButton = make_shared<OneColorButtonControl>(REC_BUTTON, "Record Button");
    shared_ptr<OneColorButtonControl> overdubButton = make_shared<OneColorButtonControl>(OVERDUB, "Overdub Button");
    shared_ptr<OneColorButtonControl> playStartButton = make_shared<OneColorButtonControl>(PLAY_START, "Play Start Button");
    shared_ptr<PlainButtonControl> stepLeftButton = make_shared<PlainButtonControl>(STEP_LEFT, "Step Left Button");
    shared_ptr<PlainButtonControl> stepRightButton = make_shared<PlainButtonControl>(STEP_RIGHT, "Step Right Button");
    shared_ptr<PlainButtonControl> gotoButton = make_shared<PlainButtonControl>(GOTO, "Goto Button");
    shared_ptr<PlainButtonControl> barLeftButton = make_shared<PlainButtonControl>(BAR_LEFT, "Bar Left Button");
    shared_ptr<PlainButtonControl> barRightButton = make_shared<PlainButtonControl>(BAR_RIGHT, "Bar Right Button");
    shared_ptr<OneColorButtonControl> tapTempoButton = make_shared<OneColorButtonControl>(TAP_TEMPO, "Tap Tempo Button");
    shared_ptr<PlainButtonControl> leftButton = make_shared<PlainButtonControl>(LEFT, "Left Button");
    shared_ptr<PlainButtonControl> rightButton = make_shared<PlainButtonControl>(RIGHT, "Right Button");
    shared_ptr<PlainButtonControl> upButton = make_shared<PlainButtonControl>(UP, "Up Button");
    shared_ptr<PlainButtonControl> downButton = make_shared<PlainButtonControl>(DOWN, "Down Button");
    shared_ptr<OneColorButtonControl> undoButton = make_shared<OneColorButtonControl>(UNDO, "Undo Button");
    shared_ptr<OneColorButtonControl> shiftButton = make_shared<OneColorButtonControl>(SHIFT, "Shift Button");
    shared_ptr<PlainButtonControl> minusButton = make_shared<PlainButtonControl>(MINUS, "Minus Button");
    shared_ptr<PlainButtonControl> plusButton = make_shared<PlainButtonControl>(PLUS, "Plus Button");
    shared_ptr<OneColorButtonControl> windowButton = make_shared<OneColorButtonControl>(WINDOW, "Window Button");
    shared_ptr<OneColorButtonControl> mainButton = make_shared<OneColorButtonControl>(MAIN, "Main Button");
    shared_ptr<OneColorButtonControl> browserButton = make_shared<OneColorButtonControl>(BROWSER, "Browser Button");
    shared_ptr<EncoderControl> jogWheel = make_shared<EncoderControl>(0, JOG_WHEEL, "Jog Wheel");
    shared_ptr<PlainButtonControl> numericButton = make_shared<PlainButtonControl>(NUMERIC, "Numeric Button");
    shared_ptr<OneColorButtonControl> projectButton = make_shared<OneColorButtonControl>(PROJECT, "Project Button");
    shared_ptr<OneColorButtonControl> seqButton = make_shared<OneColorButtonControl>(SEQ, "Sequence Button");
    shared_ptr<OneColorButtonControl> progButton = make_shared<OneColorButtonControl>(PROG, "Program Button");
    shared_ptr<OneColorButtonControl> sampleButton = make_shared<OneColorButtonControl>(SAMPLE, "Sample Button");
    shared_ptr<OneColorButtonControl> noFilterButton = make_shared<OneColorButtonControl>(NO_FILTER, "No Filter Button");
    shared_ptr<TwoColorButtonControl> progEditButton = make_shared<TwoColorButtonControl>(PROG_EDIT, "Program Edit Button");
    shared_ptr<TwoColorButtonControl> progMixButton = make_shared<TwoColorButtonControl>(PROG_MIX, "Program Mix Button");
    shared_ptr<TwoColorButtonControl> seqEditButton = make_shared<TwoColorButtonControl>(SEQ_EDIT, "Sequence Edit Button");
    shared_ptr<TwoColorButtonControl> sampleEditButton = make_shared<TwoColorButtonControl>(SAMPLE_EDIT, "Sample Edit Button");
    shared_ptr<TwoColorButtonControl> songButton = make_shared<TwoColorButtonControl>(SONG, "Song Button");
    shared_ptr<TwoColorButtonControl> fullLevelButton = make_shared<TwoColorButtonControl>(FULL_LEVEL, "Full Level Button");
    shared_ptr<OneColorButtonControl> sixteenLevelsButton = make_shared<OneColorButtonControl>(SIXTEEN_LEVEL, "Sixteen Levels Button");
    shared_ptr<TwoColorButtonControl> stepSeqButton = make_shared<TwoColorButtonControl>(STEP_SEQ, "Step Sequence Button");
    shared_ptr<TwoColorButtonControl> nextSeqButton = make_shared<TwoColorButtonControl>(NEXT_SEQ, "Next Sequence Button");
    shared_ptr<TwoColorButtonControl> trackMuteButton = make_shared<TwoColorButtonControl>(TRACK_MUTE, "Track Mute Button");
    shared_ptr<TwoColorButtonControl> padBankAButton = make_shared<TwoColorButtonControl>(PAD_BACK_A, "Pad Bank A Button");
    shared_ptr<TwoColorButtonControl> padBankBButton = make_shared<TwoColorButtonControl>(PAD_BACK_B, "Pad Bank B Button");
    shared_ptr<TwoColorButtonControl> padBankCButton = make_shared<TwoColorButtonControl>(PAD_BACK_C, "Pad Bank C Button");
    shared_ptr<TwoColorButtonControl> padBankDButton = make_shared<TwoColorButtonControl>(PAD_BACK_D, "Pad Bank D Button");
    shared_ptr<TwoColorButtonControl> padAssignButton = make_shared<TwoColorButtonControl>(PAD_ASSIGN, "Pad Assign Button");
    shared_ptr<OneColorButtonControl> noteRepeatButton = make_shared<OneColorButtonControl>(NOTE_REPEAT, "Note Repeat Button");
    shared_ptr<PlainButtonControl> eraseButton = make_shared<PlainButtonControl>(ERASE, "Erase Button");
    shared_ptr<OneColorButtonControl> qlinkTriggerButton = make_shared<OneColorButtonControl>(QLINK_TRIGGER, "Q-Link Trigger Button");
    shared_ptr<PlainButtonControl> f1Button = make_shared<PlainButtonControl>(F1_BUTTON, "F1 Button");
    shared_ptr<PlainButtonControl> f2Button = make_shared<PlainButtonControl>(F2_BUTTON, "F2 Button");
    shared_ptr<PlainButtonControl> f3Button = make_shared<PlainButtonControl>(F3_BUTTON, "F3 Button");
    shared_ptr<PlainButtonControl> f4Button = make_shared<PlainButtonControl>(F4_BUTTON, "F4 Button");
    shared_ptr<PlainButtonControl> f5Button = make_shared<PlainButtonControl>(F5_BUTTON, "F5 Button");
    shared_ptr<PlainButtonControl> f6Button = make_shared<PlainButtonControl>(F6_BUTTON, "F6 Button");
    shared_ptr<EncoderControl> qlinkEncoder1 = make_shared<EncoderControl>(0, QLINK1, "Q-Link Encoder 1");
    shared_ptr<PlainButtonControl> qlinkTouch = make_shared<PlainButtonControl>(QLINK1_TOUCH, "Q-Link Encoder 1 Button");
    shared_ptr<EncoderControl> qlinkEncoder2 = make_shared<EncoderControl>(0, QLINK2, "Q-Link Encoder 2");
    shared_ptr<PlainButtonControl> qlinkEncoder2Touch = make_shared<PlainButtonControl>(QLINK2_TOUCH, "Q-Link Encoder 2 Button");
    shared_ptr<EncoderControl> qlinkEncoder3 = make_shared<EncoderControl>(0, QLINK3, "Q-Link Encoder 3");
    shared_ptr<PlainButtonControl> qlinkEncoder3Touch = make_shared<PlainButtonControl>(QLINK3_TOUCH, "Q-Link Encoder 3 Button");
    shared_ptr<EncoderControl> qlinkEncoder4 = make_shared<EncoderControl>(0, QLINK4, "Q-Link Encoder 4");
    shared_ptr<PlainButtonControl> qlinkEncoder4Touch = make_shared<PlainButtonControl>(QLINK4_TOUCH, "Q-Link Encoder 4 Button");
    shared_ptr<EncoderControl> qlinkScrollEncoder = make_shared<EncoderControl>(0, QLINK_SCROLL, "Q-Link Scroll Encoder");
    shared_ptr<PadControl> pad1 = make_shared<PadControl>(9, PAD1, 0, "Pad 1");
    shared_ptr<PadControl> pad2 = make_shared<PadControl>(9, PAD2, 1, "Pad 2");
    shared_ptr<PadControl> pad3 = make_shared<PadControl>(9, PAD3, 2, "Pad 3");
    shared_ptr<PadControl> pad4 = make_shared<PadControl>(9, PAD4, 3, "Pad 4");
    shared_ptr<PadControl> pad5 = make_shared<PadControl>(9, PAD5, 4, "Pad 5");
    shared_ptr<PadControl> pad6 = make_shared<PadControl>(9, PAD6, 5, "Pad 6");
    shared_ptr<PadControl> pad7 = make_shared<PadControl>(9, PAD7, 6, "Pad 7");
    shared_ptr<PadControl> pad8 = make_shared<PadControl>(9, PAD8, 7, "Pad 8");
    shared_ptr<PadControl> pad9 = make_shared<PadControl>(9, PAD9, 8, "Pad 9");
    shared_ptr<PadControl> pad10 = make_shared<PadControl>(9, PAD10, 9, "Pad 10");
    shared_ptr<PadControl> pad11 = make_shared<PadControl>(9, PAD11, 10, "Pad 11");
    shared_ptr<PadControl> pad12 = make_shared<PadControl>(9, PAD12, 11, "Pad 12");
    shared_ptr<PadControl> pad13 = make_shared<PadControl>(9, PAD13, 12, "Pad 13");
    shared_ptr<PadControl> pad14 = make_shared<PadControl>(9, PAD14, 13, "Pad 14");
    shared_ptr<PadControl> pad15 = make_shared<PadControl>(9, PAD15, 14, "Pad 15");
    shared_ptr<PadControl> pad16 = make_shared<PadControl>(9, PAD16, 15, "Pad 16");


    MPCStudioBlackControlSurface()
    {
        device = make_shared<MPCStudioBlackDevice>(midiout, midiin);
        initialize();
    };
    ~MPCStudioBlackControlSurface() = default;

    void initialize() {
            // Register controls
            device->registerControl(playButton);
            device->registerControl(stopButton);
            device->registerControl(recordButton);
            device->registerControl(overdubButton);
            device->registerControl(playStartButton);
            device->registerControl(stepLeftButton);
            device->registerControl(stepRightButton);
            device->registerControl(gotoButton);
            device->registerControl(barLeftButton);
            device->registerControl(barRightButton);
            device->registerControl(tapTempoButton);
            device->registerControl(leftButton);
            device->registerControl(rightButton);
            device->registerControl(upButton);
            device->registerControl(downButton);
            device->registerControl(undoButton);
            device->registerControl(shiftButton);
            device->registerControl(minusButton);
            device->registerControl(plusButton);
            device->registerControl(windowButton);
            device->registerControl(mainButton);
            device->registerControl(browserButton);
            device->registerControl(jogWheel);
            device->registerControl(numericButton);
            device->registerControl(projectButton);
            device->registerControl(seqButton);
            device->registerControl(progButton);
            device->registerControl(sampleButton);
            device->registerControl(noFilterButton);
            device->registerControl(progEditButton);
            device->registerControl(progMixButton);
            device->registerControl(seqEditButton);
            device->registerControl(sampleEditButton);
            device->registerControl(songButton);
            device->registerControl(fullLevelButton);
            device->registerControl(sixteenLevelsButton);
            device->registerControl(stepSeqButton);
            device->registerControl(nextSeqButton);
            device->registerControl(trackMuteButton);
            device->registerControl(padBankAButton);
            device->registerControl(padBankBButton);
            device->registerControl(padBankCButton);
            device->registerControl(padBankDButton);
            device->registerControl(padAssignButton);
            device->registerControl(noteRepeatButton); 
            device->registerControl(eraseButton);
            device->registerControl(qlinkTriggerButton);
            device->registerControl(f1Button);
            device->registerControl(f2Button);
            device->registerControl(f3Button);
            device->registerControl(f4Button);
            device->registerControl(f5Button);
            device->registerControl(f6Button);
            device->registerControl(qlinkEncoder1);
            device->registerControl(qlinkTouch);
            device->registerControl(qlinkEncoder2);
            device->registerControl(qlinkEncoder2Touch);
            device->registerControl(qlinkEncoder3);
            device->registerControl(qlinkEncoder3Touch);
            device->registerControl(qlinkEncoder4);
            device->registerControl(qlinkEncoder4Touch);
            device->registerControl(qlinkScrollEncoder); 
        }

    void uninitialize(){
    }
};

