#pragma once
#include "fmt/base.h"
#include "knob_widget.h"
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <cairo.h>
#include <memory>
#include <algorithm>
#include "fmt/format.h"
#include "fmt/core.h"
#include <functional>

class ParameterWidget : public Widget {
public:
    ParameterWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
                    float value, string label, float min, float max, bool is_centered = false, bool selected = false, float thickness = 4)
        : Widget(x, y, width, height), _value(value), _label(label)
    {
        _min = min;
        _max = max;
        _is_centered = is_centered;
        _thickness = thickness;
        render();
    }

    void setValue(float value, float min = 0.0f, float max = 1.0f) {
        if(value != _value) {
            _value = std::clamp(value, min, max);
            render();
        }
    }
    void setName(string name) {
        if(name == _label) return; // Avoid unnecessary updates
        _label = name;
        render();
    }

    void setAll(float value, string label, string displayValue){
        if(value == _value && label == _label && displayValue == _displayValue) return; // Avoid unnecessary updates
        _value = value;
        _label = label;
        _displayValue = displayValue;
        render();
    }

    void setSelected(bool selected) {
        if (selected == _selected) return; // Avoid unnecessary updates
        _selected = selected;
        render();
    }
    float value() const {
        return _value;
    }
    void draw(Vector offset) override {
        // cairo_draw_rectangle(cr, position.x, position.y, width, height, true, true);
        if(_selected) {
            cairo_draw_triangle(cr, 12, (double)height/2, 3, 3, 90, true, true);
        } else {
            
        }
        cairo_draw_aligned_text(cr, _label, 0, 0, width, "center", fontSize);
        
        auto center_x = (width - knobSize) / 2.0f;
        cairo_draw_knob(cr, center_x,  11, knobSize, _value, _min, _max, _is_centered, _thickness);

        // Format the display value using the conversion function
        cairo_draw_aligned_text(cr, _displayValue, 0,  27, width, "center");
        // cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
    }

protected:
    bool _selected = false;
    float _value = 0.5f;
    int knobSize = 20;
    int fontSize = 11;
    string _label;
    string _displayValue;
    float _min = 0.0f;
    float _max = 1.0f;
    bool _is_centered = true;
    float _thickness = 4.0f;
    std::function<float(float)> valueConversionFunction = [](float value) {
        return value; // Default conversion function, can be overridden
    };
};