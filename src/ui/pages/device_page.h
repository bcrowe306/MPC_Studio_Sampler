#pragma once
#include "core/midi_utils.h"
#include "sigslot/signal.hpp"
#include "ui/widgets/widgets.h"
#include "ui/pages/page_widget.h"
#include <string>
#include <vector>
#include <memory>

using std::shared_ptr;
using std::vector;

class DevicePage : public PageWidget {
    // Device-specific UI elements and layout
public:
    vector<shared_ptr<FunctionWidget>> functionWidgets;
    vector<shared_ptr<ParameterWidget>> parameterWidgets;
    vector<sigslot::connection> trackConnections;
    vector<sigslot::connection> seqConnections;
    shared_ptr<ButtonWidget> outputWidget;
    shared_ptr<MeterWidget> meterWidget;
    shared_ptr<ButtonWidget> soloButtonWidget;
    shared_ptr<ButtonWidget> muteButtonWidget;
    shared_ptr<TextWidget> volumeLabelWidget;
    shared_ptr<HeaderSection> headerSection;
    shared_ptr<WaveformSection> waveformSection;
    std::string songPositionDisplay = "00:00:00";
    
    DevicePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Device Page");
    
    ~DevicePage() override;
    
    void createWidgets();
    void onFrame() override;
    void onSeqSelected();
    void onTrackSelected(int trackIndex = -1);
    void onActivated() override;
    void draw(Vector offset) override;
};