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

class FunctionWidget : public Widget {
public:
    FunctionWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
                   string label = "F1", bool selected = false, string align = "center", int fontSize = 11)
        : Widget(x, y, width, height), _label(label), _selected(selected), _align(align), _fontSize(fontSize) {
        render();
    }

    void setLabel(string label) {
        if (label == _label) return; // Avoid unnecessary updates
        _label = label;
        render();
    }

    void setSelected(bool selected) {
        if (selected == _selected) return; // Avoid unnecessary updates
        _selected = selected;
        render();
    }

    void setAlign(string align) {
        if (align == _align) return; // Avoid unnecessary updates
        _align = align;
        render();
    }

    void setFontSize(int fontSize) {
        if (fontSize == _fontSize) return; // Avoid unnecessary updates
        _fontSize = fontSize;
        render();
    }
    string label() const {
        return _label;
    }
    void draw(Vector offset) override {
        auto padding = 2;
        auto new_position = position + offset;
        cairo_draw_rounded_rectangle(cr, padding, 0, width-padding*2, height + 5, 7, 1, _selected, true);
        cairo_draw_aligned_text(cr, _label, 0,0, width, _align, _fontSize, !_selected);

    }
protected:
    string _label;
    bool _selected;
    string _align;
    int _fontSize;

    
};