#pragma once
#include "ui/widgets/widget.h"
#include "widgets/text_widget.h"
#include "widgets/ui_helpers.h"
#include <memory>
#include <string>
#include "ui/widgets/label_button.h"

using std::string;

class TitleBarSection : public Widget {
public:
    shared_ptr<TextWidget> leftTextWidget;
    shared_ptr<TextWidget> centerTextWidget;
    shared_ptr<TextWidget> rightTextWidget;
  TitleBarSection()
      : Widget(0, 0, 360, 13) {

    centerTextWidget = make_shared<TextWidget>(60, 0, 240, 11, "Center", 11, "center");
    this->add_child(centerTextWidget);

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