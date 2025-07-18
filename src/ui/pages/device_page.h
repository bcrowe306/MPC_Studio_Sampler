#pragma once
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include "ui/widgets/widgets.h"
#include "ui/pages/page_widget.h"
#include <Security/Security.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <memory>

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
    uint32 parameterBank = 0; // Parameter bank for the device
    DevicePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Device Page");
    
    ~DevicePage() override;
    
    void createWidgets();
    void onFrame() override;
    void onSequenceSelected(int sequenceIndex = -1) override;
    void onTrackSelected(int trackIndex = -1) override;
    void onActivated() override;
    void onQlinkControlsAdjusted(int index, int offset);
    void draw(Vector offset) override;
    
};