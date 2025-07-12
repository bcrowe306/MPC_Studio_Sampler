#pragma once
#include "ui/widgets/widget.h"
#include "widgets/progress_bar_widget.h"
#include "widgets/text_widget.h"
#include "widgets/ui_helpers.h"
#include <memory>
#include <string>
#include "ui/widgets/label_button.h"

using std::string;

class HeaderSection : public Widget {
public:
    shared_ptr<TextWidget> bpmWidget;
    shared_ptr<LabelButton> inputQuantizeButton;
    shared_ptr<TextWidget> songPositionWidget;
    shared_ptr<LabelButton> sequenceNumberWidget;
    shared_ptr<TextWidget> leftTextWidget;
    shared_ptr<TextWidget> rightTextWidget;
    shared_ptr<ProgressBarWidget> sequenceProgressBar; // Progress bar widget for sequence progress

    bool metronomeEnabled = true; // Flag to control metronome icon visibility
    float sequenceProgress = 0.0f; // Progress of the sequence, 0.0 to 1.0
    void setMetronomeEnabled(bool enabled) {
        metronomeEnabled = enabled;
        render(); // Redraw to reflect the change
    }
    void setSequenceProgress(float progress){
        if (progress != sequenceProgress) {
            // Update the sequence progress, clamping it between 0 and 1
            sequenceProgress = std::clamp(progress, 0.0f, 1.0f); // Ensure progress is between 0 and 1
            render(); // Redraw to reflect the change
        }
    }
  HeaderSection()
      : Widget(0, 0, 360, 13) {

    bpmWidget = make_shared<TextWidget>(90, 0, 60, 11, "120.00 BPM", 11, "left");
    this->add_child(bpmWidget);

    inputQuantizeButton =  make_shared<LabelButton>(140, 0, 40, 11, "iQ", "1/16");
    this->add_child(inputQuantizeButton);

    songPositionWidget = make_shared<TextWidget>(180, 0, 50, 11, "1 . 1 . 1", 11, "center");
    this->add_child(songPositionWidget);

    sequenceNumberWidget = make_shared<LabelButton>(232, 0, 33, 11, "SEQ", "1");
    this->add_child(sequenceNumberWidget);

    sequenceProgressBar = make_shared<ProgressBarWidget>(270, 2, 28, 7);
    this->add_child(sequenceProgressBar);


    leftTextWidget = make_shared<TextWidget>(2, 0, 60, 11, "Left", 11, "left");
    this->add_child(leftTextWidget);

    rightTextWidget = make_shared<TextWidget>(width - 58, 0, 56, 11, "Right", 11, "left");
    this->add_child(rightTextWidget);

    render();
    }

   

    void draw(Vector offset) override {
        cairo_draw_horizontal_line(cr, 12, 0, width, 1, 1.0f);
        cairo_draw_metronome_icon(cr, 50, 0, 35, 10, metronomeEnabled, true);
        
    }
    
};