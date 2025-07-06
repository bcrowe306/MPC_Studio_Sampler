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

class MeterWidget : public Widget {
public:
    MeterWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float volume = .5, float left = 1, float right = 0.7)
        : Widget(x, y, width, height), _volume(volume), _left(left), _right(right) {
        render();
    }

    void setVolume(float volume) {
        _volume = std::clamp(volume, 0.0f, 1.0f);
        render();
    }

    float getVolume() const {
        return _volume;
    }

    void setMeters(float left, float right) {
        _left = left;
        _right = right;
        render();
    }

    void setAll(float volume, float left, float right) {
        _volume = std::clamp(volume, 0.0f, 1.0f);
        _left = left;
        _right = right;
        render();
    }

    void draw(Vector offset) override {
        auto new_position = position + offset;
        auto padding = 2.0;
        auto leftHeight = height - (_left * height);
        auto rightHeight = height - (_right * height);

        auto volumeHeight = height - (_volume * height);

        auto meterWidth = ((double)width / 3) - (padding * 2);
        // draw left meter
        cairo_draw_rectangle(cr, 0, leftHeight, meterWidth+1, height, true, true);

        // draw right meter
        cairo_draw_rectangle(cr, meterWidth + padding +1, rightHeight, meterWidth, height, true, true);

        cairo_draw_horizontal_line(cr, volumeHeight, meterWidth*2, meterWidth*3 + padding);
        cairo_draw_vertical_line(cr, meterWidth*3 + padding-1, volumeHeight, height, 1.0, true);
    }

protected:
    float _volume;
    float _left;
    float _right;
    string _formatString = "{:.2f} dB";
};