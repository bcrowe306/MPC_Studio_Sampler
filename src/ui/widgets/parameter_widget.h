#pragma once
#include "knob_widget.h"
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <memory>

class ParameterWidget : public Widget {
public:
    shared_ptr<KnobWidget> _knob; // Knob widget for the parameter
    ParameterWidget(unsigned int x, unsigned int y, unsigned int size,
                    float value, string label, float min, float max, bool is_centered = true, float thickness = 5)
        : Widget(x, y, size, size)
           {


        _label = label;
        render();
    }

    void setSettings(float value, float min, float max, bool is_centered) {
        _knob->setSettings(value, min, max, is_centered);
        render();
    }

    void draw(Vector offset) override {
        cairo_draw_text(cr, _label, position.x + offset.x, position.y + offset.y, 12);
        _knob->draw(offset);
    }

protected:
    string _label;
    string _displayValue;
};