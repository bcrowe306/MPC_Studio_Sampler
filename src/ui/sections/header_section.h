#pragma once
#include "ui/widgets/widget.h"
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
  HeaderSection()
      : Widget(0, 0, 360, 13) {

    bpmWidget = make_shared<TextWidget>(60, 0, 60, 11, "120.00 BPM", 11, "center");
    this->add_child(bpmWidget);

    inputQuantizeButton =  make_shared<LabelButton>(130, 0, 60, 11, "InQ", "1/16");
    this->add_child(inputQuantizeButton);

    songPositionWidget = make_shared<TextWidget>(175, 0, 60, 11, "1 . 1 . 1", 11, "center");
    this->add_child(songPositionWidget);

    sequenceNumberWidget = make_shared<LabelButton>(240, 0, 60, 11, "SEQ", "1");
    this->add_child(sequenceNumberWidget);

    leftTextWidget = make_shared<TextWidget>(2, 0, 60, 11, "Left", 11, "left");
    this->add_child(leftTextWidget);

    rightTextWidget = make_shared<TextWidget>(width - 60, 0, 58, 11, "Right", 11, "left");
    this->add_child(rightTextWidget);

    render();
    }

   

    void draw(Vector offset) override {
        cairo_draw_horizontal_line(cr, 12, 0, width, 1, 1.0f);
        
    }
    
};