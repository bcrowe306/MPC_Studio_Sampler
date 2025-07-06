#pragma once
#include "fmt/base.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <algorithm>
#include <cairo.h>
#include <functional>
#include <memory>
#include <string>

class ButtonWidget : public Widget {
    // Button-specific properties and methods
public:
    ButtonWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
                 string label = "Button", bool selected = false, int fontSize = 11)
        : Widget(x, y, width, height), _label(label), _selected(selected), _fontSize(fontSize) {
        render();
    }
    void setLabel(string label) {
        _label = label;
        render();
    }
    void setSelected(bool selected) {
        _selected = selected;
        render();
    }
   
    void setFontSize(int fontSize) {
        _fontSize = fontSize;
        render();
    }
    string label() const {
        return _label;
    }
    void draw(Vector offset) override {
        auto new_position = position + offset;
        
        cairo_draw_rectangle(cr, 0, 0, width-2, height , _selected, true);
        // center vertically text
        auto text_extents = cairo_text_extents_t();
        
        cairo_draw_aligned_text(cr, _label, 0, 0, width-2, "center", _fontSize, !_selected);
    }
protected:
    string _label;
    bool _selected;
    int _fontSize;
    // Additional properties specific to ButtonWidget can be added here
};