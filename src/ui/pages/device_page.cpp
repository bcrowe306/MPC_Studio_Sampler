#include "device_page.h"
#include <iostream>
#include "fmt/format.h"
#include "util.h"
#include "router.h"

using std::make_shared;

DevicePage::DevicePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
           const std::string &title)
    : PageWidget(mpcSampler, x, y, width, height)
{
    _title = title;
    createWidgets();
    deactivatedSignal.connect([this]() {

        // Clear track connections when the page is deactivated
        for (auto &connection : trackConnections) {
            connection.disconnect();
        }
        trackConnections.clear();
        for(auto &connection : seqConnections) {
            connection.disconnect();
        }
        seqConnections.clear();
    });
}

DevicePage::~DevicePage() {
    std::cout << "DevicePage destroyed\n";
}

void DevicePage::createWidgets(){
    for (int i = 0; i < 6; i++) {
      auto functionWidget = make_shared<FunctionWidget>(
          i * 60, 96 - 11, 60, 13, fmt::format("F{}", i + 1), false,
          "center");
      functionWidgets.push_back(functionWidget);
      this->add_child(functionWidget);
    }
    

    for (int i = 0; i < 4; i++) {
      auto parameterWidget = make_shared<ParameterWidget>(
          i * 60 + 60, 46, 60, 39, 0.5f, fmt::format("Param {}", i + 1),
          0.0f, 1.0f);
      this->add_child(parameterWidget);
      parameterWidgets.push_back(parameterWidget);
    }
    outputWidget = make_shared<ButtonWidget>(60 * 5 + 4, 68, 54, 13,
                                                  "Main", false, 11);
    meterWidget = make_shared<MeterWidget>(333, 13, 27, 39, .5, 0.9f, 0.7f);
    soloButtonWidget = make_shared<ButtonWidget>(303, 40, 15, 14, "s", false, 10);
    muteButtonWidget = make_shared<ButtonWidget>(317, 40, 15, 14, "m", false, 10);
    volumeLabelWidget = make_shared<TextWidget>( 300, 55, 58, 11, fmt::format("{:.2f} dB", linearToDB(.5)), 11, "center");
    headerSection = make_shared<HeaderSection>();
    waveformSection = make_shared<WaveformSection>(60, 12, 240, 34, "Waveform");
    this->add_child(outputWidget);
    this->add_child(meterWidget);
    this->add_child(soloButtonWidget);
    this->add_child(muteButtonWidget);
    this->add_child(volumeLabelWidget);
    this->add_child(headerSection);
    this->add_child(waveformSection);
}

void DevicePage::onFrame() {
    auto track = mpcSampler->project->selectedTrack();
    if (track) {
        auto levelMeters = track->getLevelRMSdB();
        float minDb = -60.0f;
        meterWidget->setMeters(mapFloat(levelMeters.left, minDb, 0.0f, 0.0f, 1.0f), mapFloat(levelMeters.right, minDb, 0.0f, 0.0f, 1.0f));
    }
    auto selectedSeq = mpcSampler->project->sequencer->getSelectedSequence();
    if (selectedSeq) {
        auto sp = selectedSeq->getSongPositionDisplay();
        if( sp != songPositionDisplay) {
            songPositionDisplay = sp;
            headerSection->songPositionWidget->set_text(songPositionDisplay);
        }
        headerSection->sequenceProgressBar->setProgress(selectedSeq->getSongPositionProgress());
    }
}

void DevicePage::onSeqSelected(){
    for(auto &connection : seqConnections) {
        connection.disconnect();
    }
    seqConnections.clear();
}

void DevicePage::onTrackSelected(int trackIndex) {

    for(auto &connection : trackConnections) {
        connection.disconnect();
    }
    trackConnections.clear();

    
    auto selectedTrack = mpcSampler->project->selectedTrack();
    if (selectedTrack) {

        headerSection->rightTextWidget->set_text(selectedTrack->name->getValue());

        // set the waveform data for the waveform section
        auto waveformData = mpcSampler->project->selectedTrack()->getWaveformData();
        if (waveformData) {
            waveformSection->setWaveformData(waveformData);
        } else {
            waveformSection->setWaveformData(nullptr); // Clear waveform data if no track is selected
        }

        // Volume
        trackConnections.push_back(selectedTrack->volumeDb->onValueChanged.connect([this](float volume) {
            volumeLabelWidget->set_text(fmt::format("{:.2f} dB", volume));
            meterWidget->setVolume(volume);
        }));
        volumeLabelWidget->set_text(fmt::format("{:.2f} dB", selectedTrack->volumeDb->getValue()));
        meterWidget->setVolume(selectedTrack->volumeDb->getValue());

        trackConnections.push_back(controlSurface->jogWheel->onOffset.connect([this, selectedTrack](int offset) {
                selectedTrack->volumeDb->setValue(selectedTrack->volumeDb->getValue() + offset);
        }));

        // Solo
        trackConnections.push_back(controlSurface->f5Button->onPressed.connect([this, selectedTrack]() {
            selectedTrack->solo->setValue(!selectedTrack->solo->getValue());
        }));
        trackConnections.push_back(selectedTrack->solo->onValueChanged.connect([this](bool solo) {
            soloButtonWidget->setSelected(solo);
        }));
        soloButtonWidget->setSelected(selectedTrack->solo->getValue());

        // Mute
        trackConnections.push_back(controlSurface->f6Button->onPressed.connect([this, selectedTrack]() {
            selectedTrack->mute->setValue(!selectedTrack->mute->getValue());
        }));
        trackConnections.push_back(selectedTrack->mute->onValueChanged.connect([this](bool mute) {
            muteButtonWidget->setSelected(mute);
        }));
        muteButtonWidget->setSelected(selectedTrack->mute->getValue());

        headerSection->leftTextWidget->set_text(selectedTrack->getDeviceTypeName());

        trackConnections.push_back(selectedTrack->onDeviceUpdate.connect([this]() {
            auto st = mpcSampler->project->selectedTrack();
            if (st) {
                headerSection->leftTextWidget->set_text(st->getDeviceTypeName());
            }
        }));
    }
    else {
        headerSection->rightTextWidget->set_text("No Track");
    }

}

void DevicePage::onActivated() {

    addConnection(mpcSampler->project->metronomeEnabled.onValueChanged.connect([this](bool enabled) {
        headerSection->setMetronomeEnabled(enabled);
    }));
    headerSection->setMetronomeEnabled(mpcSampler->project->metronomeEnabled.getValue());

    addConnection( mpcSampler->project->onTrackSelected.connect(std::bind(&DevicePage::onTrackSelected, this, std::placeholders::_1)) );
    onTrackSelected(); // Initialize with no track selected

    addConnection(mpcSampler->project->sequencer->onSequenceSelected.connect([this](int index) {
        onSeqSelected();
    }));
    onSeqSelected();

    // Sequence Number
    addConnection(mpcSampler->project->sequencer->onSequenceSelected.connect([this](int sequenceIndex) {
        headerSection->sequenceNumberWidget->setValue(fmt::format("{}", sequenceIndex + 1));
    }));
    headerSection->sequenceNumberWidget->setValue(fmt::format("{}", mpcSampler->project->sequencer->getSelectedSequenceIndex() + 1));

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


    functionWidgets[2]->setLabel("InputQ");
    functionWidgets[3]->setLabel("Output");
    functionWidgets[4]->setLabel("Solo");
    functionWidgets[5]->setLabel("Mute");
}

void DevicePage::draw(Vector offset) {
   
}
