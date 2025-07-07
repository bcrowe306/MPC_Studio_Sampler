#pragma once
#include "ui/widgets/label_button.h"
#include "ui/widgets/widget.h"
#include "widgets/text_widget.h"
#include "widgets/ui_helpers.h"
#include <memory>
#include <string>

class WaveformSection : public Widget {
public:
    WaveformSection(unsigned int x, unsigned int y, unsigned int width, unsigned int height, std::string title = "Waveform")
        : Widget(x, y, width, height), _title(title) {
        render();
    }

    void draw(Vector offset) override {
        cairo_draw_rectangle(cr, 0, 0, width, height, false, true);
        cairo_draw_text(cr, _title, 2, 10, 11);
    }
    void setTitle(const std::string &title) {
        _title = title;
        render();
    }
protected:
    std::string _title;
};