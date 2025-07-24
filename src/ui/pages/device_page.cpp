#include "device_page.h"
#include <iostream>
#include "fmt/format.h"
#include "util.h"
#include "router.h"
#include "core/project.h"

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
        for(auto &connection : sequenceConnections) {
            connection.disconnect();
        }
        sequenceConnections.clear();
    });
}

DevicePage::~DevicePage() {
    std::cout << "DevicePage destroyed\n";
}

void DevicePage::createWidgets(){
    

    for (int i = 0; i < 4; i++) {
      auto parameterWidget = make_shared<ParameterWidget>(
          i * 60 + 60, 46, 60, 39, 0.5f, fmt::format("Param {}", i + 1),
          0.0f, 1.0f);
      this->add_child(parameterWidget);
      parameterWidgets.push_back(parameterWidget);
    }

    outputWidget = make_shared<ButtonWidget>(60 * 5 + 4, 68, 54, 13,  "Main", false, 11);
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
    auto track = mpcSampler->selectedTrack();
    if (track) {
        auto levelMeters = track->getLevelRMSdB();
        float minDb = -60.0f;
        meterWidget->setMeters(mapFloat(levelMeters.left, minDb, 0.0f, 0.0f, 1.0f), mapFloat(levelMeters.right, minDb, 0.0f, 0.0f, 1.0f));
    }
    auto selectedSeq = mpcSampler->sequencer->getSelectedSequence();
    if (selectedSeq) {
        auto sp = selectedSeq->getSongPositionDisplay();
        if( sp != songPositionDisplay) {
            songPositionDisplay = sp;
            headerSection->songPositionWidget->set_text(songPositionDisplay);
        }
        headerSection->sequenceProgressBar->setProgress(selectedSeq->getSongPositionProgress());
    }
    updateParameterWidgets();
}

void DevicePage::onSequenceSelected(int sequenceIndex) {
    for(auto &connection : sequenceConnections) {
        connection.disconnect();
    }
    sequenceConnections.clear();
    headerSection->sequenceNumberWidget->setValue(fmt::format("{}", mpcSampler->sequencer->getSelectedSequenceIndex() + 1));
}

void DevicePage::onQlinkControlsAdjusted(int index, int offset) {
    auto track = mpcSampler->selectedTrack();
    auto device = track->getDevice();
    if (!device) {
        return;
    }

    auto params = device->parameters;
    int paramIndex = index + (parameterBank.get() * 4);
    if (paramIndex < 0 || paramIndex >= params.size()) {
        std::cerr << "Invalid parameter index: " << paramIndex << std::endl;
        return;
    }

    // Adjust the parameter value based on the offset
    if(offset > 0) {
        params[paramIndex]->incrementValue(true);
    } else if(offset < 0) {
        params[paramIndex]->decrementValue(true);
    }
    for (int i = 0; i < parameterWidgets.size(); i++) {
        parameterWidgets[i]->setSelected( i == index);
    }
}

void DevicePage::onTrackSelected(int trackIndex) {

    for(auto &connection : trackConnections) {
        connection.disconnect();
    }
    trackConnections.clear();


    
    auto selectedTrack = mpcSampler->selectedTrack();

    selectedTrack->onTrackDeviceUpdated.connect([this]() {
        // set the waveform data for the waveform section
        auto waveformData = mpcSampler->selectedTrack()->getWaveformData();
        if (waveformData) {
            waveformSection->setWaveformData(waveformData);
        } else {
            waveformSection->setWaveformData(nullptr); // Clear waveform data if no track is selected
        }
    });

    if (selectedTrack) {

        headerSection->rightTextWidget->set_text(selectedTrack->name->getValue());

        // set the waveform data for the waveform section
        auto waveformData = mpcSampler->selectedTrack()->getWaveformData();
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

        trackConnections.push_back(selectedTrack->onTrackDeviceUpdated.connect([this]() {
            auto st = mpcSampler->selectedTrack();
            if (st) {
                headerSection->leftTextWidget->set_text(st->getDeviceTypeName());
            }
        }));
    }
    else {
        headerSection->rightTextWidget->set_text("No Track");
    }

    updateParameterWidgets();
}

void DevicePage::onActivated() {
    functionWidgets[2]->setLabel("InputQ");
    functionWidgets[3]->setLabel("Output");
    functionWidgets[4]->setLabel("Solo");
    functionWidgets[5]->setLabel("Mute");

    addConnection(project->metronomeEnabled.onValueChanged.connect([this](bool enabled) {
        headerSection->setMetronomeEnabled(enabled);
    }));
    headerSection->setMetronomeEnabled(project->metronomeEnabled.getValue());

    // Input Quantize
    addConnection(controlSurface->f3Button->onPressed.connect([this]() {
       project->inputQuantize.setValue(!project->inputQuantize.getValue());
    }));

    addConnection(project->inputQuantize.onValueChanged.connect([this](bool inputQuantize) {
        headerSection->inputQuantizeButton->setSelected(inputQuantize);
        
    }));


    addConnection(project->bpm.onValueChanged.connect([this](float bpm) {
        headerSection->bpmWidget->set_text(fmt::format("{:.2f}", bpm));
    }));
    headerSection->bpmWidget->set_text(fmt::format("{:.2f}", project->bpm.getValue()));

    addConnection(controlSurface->qlinkEncoders->onOffset.connect([this](int index, int offset) {
        onQlinkControlsAdjusted(index, offset);
    }));

    addConnection(controlSurface->qlinkScrollEncoder->onOffset.connect([this](int offset) {
        if(controlSurface->shiftButton->isPressed){
            if (offset > 0) {
                project->bpm.incrementValue(true); // Increment BPM by coarse value
            } else if (offset < 0) {
                project->bpm.decrementValue(true); // Decrement BPM by coarse value
            }
        }
        else {
            offsetParameterBank(offset); // Adjust the parameter bank by offset
        }
    }));

    addConnection(controlSurface->qlinkEncoderTouches->onPressed.connect([this](int index) {
        onQlinkEncoderTouched(index);
    }));    

    parameterBank.onValueChanged.connect([this](int) {
        updateParameterWidgets();
    });
    parameterBank.emit();

}

void DevicePage::draw(Vector offset) {
   
}

void DevicePage::updateParameterWidgets() {
    auto selectedTrack = mpcSampler->selectedTrack();
    if(selectedTrack){
        auto device = selectedTrack->getDevice();
        if (device){
            auto params = device->parameters;
            // update the params according to bank and bankSize making sure to be in bounds
            for (int i = 0; i < _bankSize; i++) {
                
                int paramIndex = i + (parameterBank.get() * _bankSize);
                if (paramIndex < 0 || paramIndex >= params.size()) {
                    parameterWidgets[i]->setAll(0.0f, "", ""); // Show widget if in bounds
                } else {
                    parameterWidgets[i]->setAll(
                        params[paramIndex]->getUnitValue(), 
                        params[paramIndex]->getDisplayName(), 
                        params[paramIndex]->getStringValue()); // Show widget if in bounds
                }
            }
        } else {
            for (auto &widget : parameterWidgets) {
                for (int i = 0; i < _bankSize; i++) {
                    widget->setAll(0.0f, "", ""); // Clear widgets if no device
                }
            }
        }
        
    } 
}

void DevicePage::onQlinkEncoderTouched(int index){
    if(hasDevice()){
        for(int i = 0; i < parameterWidgets.size(); i++) {
            parameterWidgets[i]->setSelected(i == index); // Highlight the touched parameter widget
        }

    }
}
