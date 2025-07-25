#pragma once
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include "ui/pages/page_widget.h"
#include "ui/widgets/widgets.h"
#include "widgets/button_widget.h"
#include "widgets/label_button.h"
#include "widgets/pianoroll_widget.h"
#include <memory>
#include <string>
#include <vector>
#include "core/project.h"

class SequencePage : public PageWidget {
    // Device-specific UI elements and layout
public:
    
    shared_ptr<HeaderSection> headerSection;

    shared_ptr<ButtonWidget> startButton;
    shared_ptr<ButtonWidget> endButton;
    shared_ptr<ButtonWidget> noteButton;
    shared_ptr<ButtonWidget> velocityButton;

    shared_ptr<ButtonWidget> modeWidget;
    shared_ptr<ButtonWidget> outputWidget;
    shared_ptr<MeterWidget> meterWidget;
    shared_ptr<ButtonWidget> soloButtonWidget;
    shared_ptr<ButtonWidget> muteButtonWidget;
    shared_ptr<TextWidget> volumeLabelWidget;
    shared_ptr<PianoRollWidget> pianoRollWidget;


    std::string songPositionDisplay = "00:00:00";
    SequencePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Sequence Page")
        : PageWidget(mpcSampler, x, y, width, height), _title(title) 
    {
        _createWidgets();
        

    }

    void onFrame() override {
        auto track = mpcSampler->selectedTrack();
        if (track) {
          auto levelMeters = track->getLevelRMSdB();
          float minDb = -60.0f;
          meterWidget->setMeters(
              mapFloat(levelMeters.left, minDb, 0.0f, 0.0f, 1.0f),
              mapFloat(levelMeters.right, minDb, 0.0f, 0.0f, 1.0f));
        }
        auto selectedSeq = mpcSampler->sequencer->getSelectedSequence();
        if (selectedSeq) {
          auto sp = selectedSeq->getSongPositionDisplay();
          if (sp != songPositionDisplay) {
            songPositionDisplay = sp;
            headerSection->songPositionWidget->set_text(songPositionDisplay);
          }
          headerSection->sequenceProgressBar->setProgress(
              selectedSeq->getSongPositionProgress());
        }
    }
    void onTrackSelected(int trackIndex) override {
        setupSequenceControls();
    }

    void _createWidgets(){
        startButton = make_shared<ButtonWidget>(2, 15, 54, 11, "Start");
        endButton = make_shared<ButtonWidget>(2, 29, 54, 11, "Length");
        noteButton = make_shared<ButtonWidget>(2, 43, 54, 11, "Note: 60");
        velocityButton = make_shared<ButtonWidget>(2, 57, 54, 11, "Vel: 127");
        modeWidget = make_shared<ButtonWidget>(2, 71, 54, 11, "Mode", false);


        outputWidget = make_shared<ButtonWidget>(60 * 5 + 4, 68, 54, 13, "Main", false, 11);
        meterWidget = make_shared<MeterWidget>(333, 13, 27, 39, .5, 0.9f, 0.7f);
        soloButtonWidget = make_shared<ButtonWidget>(303, 40, 15, 14, "s", false, 10);
        muteButtonWidget = make_shared<ButtonWidget>(317, 40, 15, 14, "m", false, 10);
        volumeLabelWidget = make_shared<TextWidget>( 300, 55, 58, 11, fmt::format("{:.2f} dB", linearToDB(.5)), 11, "center");
        pianoRollWidget = make_shared<PianoRollWidget>(60, 13, 240, 96-26);
        headerSection = make_shared<HeaderSection>();
        this->add_child(outputWidget);
        this->add_child(meterWidget);
        this->add_child(soloButtonWidget);
        this->add_child(muteButtonWidget);
        this->add_child(volumeLabelWidget);
        this->add_child(headerSection);
        this->add_child(pianoRollWidget);

        this->add_child(startButton);
        this->add_child(endButton);
        this->add_child(noteButton);
        this->add_child(velocityButton);
        this->add_child(modeWidget);
    }

    void setupSequenceControls (){

        // Midi notes
        int selectedTrackIndex = mpcSampler->selectedTrackIndex();
        auto selectedSequence = mpcSampler->sequencer->getSelectedSequence();
        auto midiClip = selectedSequence->getClip(selectedTrackIndex);
        pianoRollWidget->setMidiClip(midiClip);
        std::cout << "Setting up sequence controls for track index: " << selectedTrackIndex << std::endl;

        // Sequence Labels
        addSequenceConnection(selectedSequence->onSequenceChanged.connect([this]() {
            setSeqenceLabels();
        }));
        setSeqenceLabels();

        // QLink Controls
        // TODO: Need state here. Make Qlink controls do different things based on state
        addSequenceConnection(controlSurface->qlinkEncoders->onOffsetSlow.connect([this](int qlinkIndex, int offset) {
            auto seq = mpcSampler->sequencer->getSelectedSequence();
            if(qlinkIndex == 0){
                if (offset > 0) {
                    seq->incrementStartTickBeat();
                } else if (offset < 0) {
                    seq->decrementStartTickBeat();
                }
            } else if (qlinkIndex == 1) {
                if (offset > 0) {
                    seq->incrementEndTickBar();
                } else if (offset < 0) {
                    seq->decrementEndTickBar();
                }
            }
        }));
    }

    void setSeqenceLabels(){
        startButton->setLabel(fmt::format("Start: {}", mpcSampler->sequencer->getSelectedSequence()->getStartInBars() + 1 ));
        endButton->setLabel(fmt::format("Length: {}", mpcSampler->sequencer->getSelectedSequence()->getLengthInBars() ));
        pianoRollWidget->setSequenceEnd(mpcSampler->sequencer->getSelectedSequence()->getEndInTicks());
    }

    void onSequenceSelected(int sequenceIndex) override {
        for (auto &connection : sequenceConnections) {
          connection.disconnect();
        }
        sequenceConnections.clear();
        headerSection->sequenceNumberWidget->setValue(fmt::format(
            "{}",
            mpcSampler->sequencer->getSelectedSequenceIndex() + 1));
        setupSequenceControls();
    }

    void onActivated() override{
        functionWidgets[0]->setLabel("Quantize");
        functionWidgets[1]->setLabel("Double");
        functionWidgets[2]->setLabel("Clear");
        functionWidgets[3]->setLabel("Mute");
        functionWidgets[4]->setLabel("Duplicate");
        functionWidgets[5]->setLabel("More");

        // Qlink Touch Controls
        controlSurface->qlinkEncoderTouches->onPressed.connect([this](int qlinkIndex) {
            if (qlinkIndex == 0) {
                startButton->setSelected(true);
                endButton->setSelected(false);
                noteButton->setSelected(false);
                velocityButton->setSelected(false);
            }
            else if(qlinkIndex == 1) {
                startButton->setSelected(false);
                endButton->setSelected(true);
                noteButton->setSelected(false);
                velocityButton->setSelected(false);
            }
            else if(qlinkIndex == 2) {
                startButton->setSelected(false);
                endButton->setSelected(false);
                noteButton->setSelected(true);
                velocityButton->setSelected(false);
            }
            else if(qlinkIndex == 3) {
                startButton->setSelected(false);
                endButton->setSelected(false);
                noteButton->setSelected(false);
                velocityButton->setSelected(true);
            }
        });

       

        // Jog Wheel Controls
        addConnection(controlSurface->jogWheel->onOffset.connect([this](int offset) {
            if (offset > 0) {
                pianoRollWidget->scrollGridRight();
            } else if (offset < 0) {
                pianoRollWidget->scrollGridLeft();
            }
        }));

        // Directional Controls
        addConnection(controlSurface->upButton->onPressed.connect([this]() {
            pianoRollWidget->incrementBaseNote();
        }));

        addConnection(controlSurface->downButton->onPressed.connect([this]() {
            pianoRollWidget->decrementBaseNote();
        }));

        addConnection(controlSurface->leftButton->onPressed.connect([this]() {
            pianoRollWidget->scrollGridLeft();
        }));

        addConnection(controlSurface->rightButton->onPressed.connect([this]() {
            pianoRollWidget->scrollGridRight();
        }));


        // Function Buttons
        addConnection(controlSurface->functionButtons->onPressed.connect([this](int index) {
            if (index == 0) { // Assuming index 0 is for "Quantize"
                auto seq = mpcSampler->sequencer->getSelectedSequence();
                // seq->setInputQuantize(!seq->isInputQuantizeEnabled());
                // headerSection->inputQuantizeButton->setSelected(seq->isInputQuantizeEnabled());
            } else if (index == 2) { // Assuming index 2 is for "Clear"
                auto seq = mpcSampler->sequencer->getSelectedSequence();
                int trackIndex = mpcSampler->selectedTrackIndex();
                seq->clearTrackEvents(trackIndex);

            } else if (index == 3) { // Assuming index 3 is for "Mute"
                // auto seq = mpcSampler->project->sequencer->getSelectedSequence();
                // seq->toggleMute();
            } else if (index == 4) { // Assuming index 4 is for "Duplicate"
                // auto seq = mpcSampler->project->sequencer->getSelectedSequence();
                // seq->duplicateSequence();
            } else if (index == 5) { // Assuming index 5 is for "More"
                // Handle more options
            }
            if (index == 1) { // Assuming index 1 is for "Double"
                auto seq = mpcSampler->sequencer->getSelectedSequence();
                seq->doubleSequence();
                pianoRollWidget->setSequenceEnd(seq->getEndInTicks());
            }
        }));


        if(project.expired()){
            // Handle expired project case
            return;
        }
        auto projectPtr = project.lock();

        addConnection(projectPtr->metronomeEnabled->onValueChanged.connect(
            [this](bool enabled) {
              headerSection->setMetronomeEnabled(enabled);
            }));
        headerSection->setMetronomeEnabled(projectPtr->metronomeEnabled->getValue());

        // BPM Label and control
        addConnection(projectPtr->bpm->onValueChanged.connect([this](float bpm) {
            headerSection->bpmWidget->set_text(fmt::format("{:.2f}", bpm));
        }));
        headerSection->bpmWidget->set_text(fmt::format("{:.2f}", projectPtr->bpm->getValue()));

        addConnection(controlSurface->qlinkEncoder4->onOffset.connect([projectPtr](int offset) {
            if(offset > 0) {
                projectPtr->metronomeVolumeDb->incrementValue(true);
            } else if(offset < 0) {
                projectPtr->metronomeVolumeDb->decrementValue(true);
            }
        }));


        setSeqenceLabels();
    }


    void draw(Vector offset) override {
       
    }
protected:
    std::string _title;
    
};