#pragma once
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <string>

using std::string;

class LabelButton : public Widget {
public:
    LabelButton(unsigned int x, unsigned int y, unsigned int width, unsigned int height, string label, string value="")
        : Widget(x, y, width, height), _label(label), _value(value) {
        render();
    }

    void setLabel(const string &label) {
        if (label == _label) return; // Avoid unnecessary updates
        _label = label;
        render();
    }

    void setSelected(bool selected) {
        if (selected == _selected) return; // Avoid unnecessary updates
        _selected = selected;
        render();
    }
    string label() const {
        return _label;
    }

    string value() const {
        return _value;
    }
    void setValue(const string &value) {
        if (value == _value) return; // Avoid unnecessary updates
        _value = value;
        render();
    }
    void draw(Vector offset) override {
        // get text extents from both label and value
        auto label_extents = cairo_text_extents_t();
        auto value_extents = cairo_text_extents_t();
        cairo_text_extents(cr, _label.c_str(), &label_extents);
        cairo_text_extents(cr, _value.c_str(), &value_extents);
        cairo_draw_rectangle(cr, 0, 0, label_extents.width + 4, height, _selected, true);
        cairo_draw_text(cr, _label, 2, 1 + label_extents.height, _font_size,
                        !_selected);
        cairo_draw_rectangle(cr, label_extents.width+5, 0, value_extents.width + 6, height, false, true);
        cairo_draw_text(cr, _value, label_extents.width + 6, 2 + value_extents.height, _font_size, true);
    }

protected:
    bool _selected = false; // Whether the button is selected'
    unsigned int _font_size = 10;
    string _label;
    string _value;
};