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
  HeaderSection(string leftText = "XF Kick",
                string rightText = "Track 1")
      : Widget(0, 0, 360, 13), _leftText(leftText), _rightText(rightText) {

    bpmWidget = make_shared<TextWidget>(60, 0, 60, 11, "120.00 BPM", 11, "center");
    this->add_child(bpmWidget);

    inputQuantizeButton =  make_shared<LabelButton>(130, 0, 60, 11, "InQ", "1/16");
    this->add_child(inputQuantizeButton);

    songPositionWidget = make_shared<TextWidget>(175, 0, 60, 11, "1 . 1 . 1", 11, "center");
    this->add_child(songPositionWidget);

    sequenceNumberWidget = make_shared<LabelButton>(240, 0, 60, 11, "SEQ", "1");
    this->add_child(sequenceNumberWidget);

    render();
    }

    void setLeftText(const string &text) {
        if(text == _leftText) return; // Avoid unnecessary updates
        if(text.empty()) return; // Avoid empty text
        _leftText = text;
        render();
    }  
    void setRightText(const string &text) {
        if(text == _rightText) return; // Avoid unnecessary updates
        if(text.empty()) return; // Avoid empty text
        _rightText = text;
        render();
    }

    void draw(Vector offset) override {
        cairo_draw_horizontal_line(cr, 12, 0, width, 1, 1.0f);
        cairo_draw_text(cr, _leftText, 2, 10, 11);
        cairo_draw_text(cr, _rightText, width - 60, 10, 11);
    }
protected:
    string _leftText;
    string _rightText;

    
};