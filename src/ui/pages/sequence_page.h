#pragma once
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include "ui/pages/page_widget.h"
#include "ui/widgets/widgets.h"
#include "widgets/pianoroll_widget.h"
#include <memory>
#include <string>
#include <vector>

class SequencePage : public PageWidget {
    // Device-specific UI elements and layout
public:
    
    shared_ptr<HeaderSection> headerSection;
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
        functionWidgets[0]->setLabel("Quantize");
        functionWidgets[1]->setLabel("Double");
        functionWidgets[2]->setLabel("Clear");
        functionWidgets[3]->setLabel("Mute");   
        functionWidgets[4]->setLabel("Duplicate");
        functionWidgets[5]->setLabel("More");
    }

    void onFrame() override {
        auto track = mpcSampler->project->selectedTrack();
        if (track) {
          auto levelMeters = track->getLevelRMSdB();
          float minDb = -60.0f;
          meterWidget->setMeters(
              mapFloat(levelMeters.left, minDb, 0.0f, 0.0f, 1.0f),
              mapFloat(levelMeters.right, minDb, 0.0f, 0.0f, 1.0f));
        }
        auto selectedSeq =
            mpcSampler->project->sequencer->getSelectedSequence();
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

    void _createWidgets(){
        outputWidget = make_shared<ButtonWidget>(60 * 5 + 4, 68, 54, 13,  "Main", false, 11);
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
    }

    void onSequenceSelected(int sequenceIndex) override {
        for (auto &connection : sequenceConnections) {
          connection.disconnect();
        }
        sequenceConnections.clear();
        headerSection->sequenceNumberWidget->setValue(fmt::format(
            "{}",
            mpcSampler->project->sequencer->getSelectedSequenceIndex() + 1));
    }

    void onActivated() override{
        functionWidgets[2]->setLabel("InputQ");
        functionWidgets[3]->setLabel("Output");
        functionWidgets[4]->setLabel("Solo");
        functionWidgets[5]->setLabel("Mute");

        addConnection(mpcSampler->project->metronomeEnabled.onValueChanged.connect([this](bool enabled) {
            headerSection->setMetronomeEnabled(enabled);
        }));
        headerSection->setMetronomeEnabled(mpcSampler->project->metronomeEnabled.getValue());

        addConnection(controlSurface->jogWheel->onOffset.connect([this](int offset) {
            if (offset > 0) {
                pianoRollWidget->incrementOffset();
            } else if (offset < 0) {
                pianoRollWidget->decrementOffset();
            }
        }));


        // Input Quantize
        addConnection(controlSurface->f3Button->onPressed.connect([this]() {
        mpcSampler->project->inputQuantize.setValue(!mpcSampler->project->inputQuantize.getValue());
        }));

        addConnection(mpcSampler->project->inputQuantize.onValueChanged.connect([this](bool inputQuantize) {
            headerSection->inputQuantizeButton->setSelected(inputQuantize);
            
        }));



        signalConnections.push_back(mpcSampler->project->bpm.onValueChanged.connect([this](float bpm) {
            headerSection->bpmWidget->set_text(fmt::format("{:.2f}", bpm));
        }));
        headerSection->bpmWidget->set_text(fmt::format("{:.2f}", mpcSampler->project->bpm.getValue()));

        signalConnections.push_back(controlSurface->qlinkEncoder4->onOffset.connect([this](int offset) {
            if(offset > 0) {
                mpcSampler->project->metronomeVolumeDb.incrementValue(true);
            } else if(offset < 0) {
                mpcSampler->project->metronomeVolumeDb.decrementValue(true);
            }
        }));


        
    }


    void draw(Vector offset) override {
       
    }
protected:
    std::string _title;
    
};