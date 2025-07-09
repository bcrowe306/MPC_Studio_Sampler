#pragma once
#include "sigslot/signal.hpp"
#include "ui/widgets/widgets.h"
#include "ui/pages/page_widget.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "fmt/format.h"
#include "util.h"

using std::shared_ptr;
using std::make_shared;
using std::vector;

class DevicePage : public PageWidget {
    // Device-specific UI elements and layout
public:
    vector<shared_ptr<FunctionWidget>> functionWidgets;
    vector<shared_ptr<ParameterWidget>> parameterWidgets;
    vector<sigslot::connection> trackConnections;
    shared_ptr<ButtonWidget> outputWidget;
    shared_ptr<MeterWidget> meterWidget;
    shared_ptr<ButtonWidget> soloButtonWidget;
    shared_ptr<ButtonWidget> muteButtonWidget;
    shared_ptr<TextWidget> volumeLabelWidget;
    shared_ptr<HeaderSection> headerSection;
    shared_ptr<WaveformSection> waveformSection;
    DevicePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Device Page")
        : PageWidget(mpcSampler, x, y, width, height)
    {
        _title = title;
        createWidgets();
    }

    ~DevicePage() override {
        std::cout << "DevicePage destroyed\n";
    };

    void createWidgets(){
        for (int i = 0; i < 6; i++) {
          auto functionWidget = make_shared<FunctionWidget>(
              i * 60, 96 - 11, 60, 13, fmt::format("F{}", i + 1), false,
              "center");
          functionWidgets.push_back(functionWidget);
          this->add_child(functionWidget);
        }
        functionWidgets[3]->setLabel("Output");
        functionWidgets[4]->setLabel("Solo");
        functionWidgets[5]->setLabel("Mute");

        for (int i = 0; i < 4; i++) {
          auto parameterWidget = make_shared<ParameterWidget>(
              i * 60 + 60, 46, 60, 39, 0.5f, fmt::format("Param {}", i + 1),
              0.0f, 1.0f);
          this->add_child(parameterWidget);
          parameterWidgets.push_back(parameterWidget);
        }
        outputWidget = make_shared<ButtonWidget>(60 * 5 + 4, 68, 54, 13,
                                                      "Main", false, 11);
        meterWidget =
            make_shared<MeterWidget>(333, 13, 27, 39, .5, 0.9f, 0.7f);
        soloButtonWidget =
            make_shared<ButtonWidget>(303, 40, 15, 14, "s", false, 10);
        muteButtonWidget =
            make_shared<ButtonWidget>(317, 40, 15, 14, "m", false, 10);
        volumeLabelWidget = make_shared<TextWidget>(
            300, 55, 58, 11, fmt::format("{:.2f} dB", linearToDB(.5)), 11,
            "center");
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

    void onFrame() override {
        auto track = mpcSampler->project->selectedTrack();
        if (track) {
            auto levelMeters = track->getLevelMeters();
            float minDb = -60.0f;
            meterWidget->setMeters(mapFloat(levelMeters.left, minDb, 0.0f, 0.0f, 1.0f), mapFloat(levelMeters.right, minDb, 0.0f, 0.0f, 1.0f));
        }
    }

    void onTrackSelected(int trackIndex = -1) {

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
            trackConnections.push_back(selectedTrack->volume->onValueChanged.connect([this](float volume) {
                volumeLabelWidget->set_text(fmt::format("{:.2f} dB", linearToDB(volume)));
                meterWidget->setVolume(volume);
            }));
            volumeLabelWidget->set_text(fmt::format("{:.2f} dB", linearToDB(selectedTrack->volume->getValue())));
            meterWidget->setVolume(selectedTrack->volume->getValue());

            trackConnections.push_back(controlSurface->jogWheel->onOffset.connect([this, selectedTrack](int offset) {
                    selectedTrack->volume->setValue(selectedTrack->volume->getValue() + offset * 0.01f);
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

        }
        else {
            headerSection->rightTextWidget->set_text("No Track");
        }



        

    }
    void onActivated() override {
        signalConnections.push_back( mpcSampler->project->onTrackSelected.connect(std::bind(&DevicePage::onTrackSelected, this, std::placeholders::_1)) );
        onTrackSelected(); // Initialize with no track selected


        signalConnections.push_back(mpcSampler->project->bpm.onValueChanged.connect([this](float bpm) {
            headerSection->bpmWidget->set_text(fmt::format("{:.2f}", bpm));
        }));
        headerSection->bpmWidget->set_text(fmt::format("{:.2f}", mpcSampler->project->bpm.getValue()));


    }

    


    void draw(Vector offset) override {
       
    }
    
};