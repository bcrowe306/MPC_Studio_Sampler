#pragma once
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include "ui/widgets/widgets.h"
#include "ui/pages/page_widget.h"
#include <Security/Security.h>
#include <_types/_uint32_t.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <memory>
#include "core/property.h"

using std::shared_ptr;
using std::vector;

class DevicePage : public PageWidget {
    // Device-specific UI elements and layout
public:
    shared_ptr<HeaderSection> headerSection;
    vector<shared_ptr<ParameterWidget>> parameterWidgets;
    shared_ptr<ButtonWidget> outputWidget;
    shared_ptr<MeterWidget> meterWidget;
    shared_ptr<ButtonWidget> soloButtonWidget;
    shared_ptr<ButtonWidget> muteButtonWidget;
    shared_ptr<TextWidget> volumeLabelWidget;
    shared_ptr<WaveformSection> waveformSection;
    std::string songPositionDisplay = "00:00:00";
    Property<int> parameterBank = Property<int>(0); // Parameter bank for the device

    DevicePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Device Page");
    
    ~DevicePage() override;
    
    void createWidgets();
    void onFrame() override;
    void onSequenceSelected(int sequenceIndex = -1) override;
    void onTrackSelected(int trackIndex = -1) override;
    void onActivated() override;
    void onQlinkControlsAdjusted(int index, int offset);
    void onQlinkEncoderTouched(int index);
    void updateParameterWidgets();
    void draw(Vector offset) override;
    bool hasDevice() const {
        auto track = mpcSampler->selectedTrack();
        return track && track->getDevice() != nullptr; // Check if the selected track has a device
    }

    void incrementParameterBank() {
        offsetParameterBank(1);
    }

    void decrementParameterBank() {
        offsetParameterBank(-1);
    }
    
    void offsetParameterBank(int offset) {
        if(hasDevice()){
            int paramSize = mpcSampler->selectedTrack()->getDevice()->parameters.size();
            int maxIndex = (paramSize / _bankSize) - 1;
            maxIndex = paramSize % _bankSize == 0 ? maxIndex : maxIndex + 1; // Ensure we have a full bank of parameters
            parameterBank = std::clamp(parameterBank.get() + offset, 0, maxIndex); // Adjust the parameter bank by offset, clamping to 0-maxIndex
        }
    }

private:
    int _bankSize = 4;
};