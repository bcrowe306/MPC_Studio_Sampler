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
#include <cmath>
#include <iostream>

class MeterWidget : public Widget {
public:
    MeterWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float volume = .5, float left = 1, float right = 0.7)
        : Widget(x, y, width, height), _volume(volume) 
    {
        calculateHeights(left, right);
        render();

    }

    void setVolume(float volume) {
        if(volume == _volume) return; // Avoid unnecessary updates
        _volume = std::clamp(volume, 0.0f, 1.0f);
        render();
    }

    float getVolume() const {
        return _volume;
    }

    void setMeters(float left, float right) {
        auto newLeftHeight =  std::round(height - (left * height));
        auto newRightHeight =  std::round(height - (right * height));

        if(newLeftHeight == _leftHeight && newRightHeight == _rightHeight) return; // Avoid unnecessary updates
        _leftHeight = newLeftHeight;
        _rightHeight = newRightHeight;
        render();
    }
    
    void setAll(float volume, float left, float right) {
        auto newLeftHeight = std::round(height - (left * height));
        auto newRightHeight = std::round(height - (right * height));

        if(newLeftHeight == _leftHeight && newRightHeight == _rightHeight && volume == _volume) return; // Avoid unnecessary updates
        _leftHeight = newLeftHeight;
        _rightHeight = newRightHeight;
        // Ensure volume is clamped between 0 and 1
        _volume = std::clamp(volume, 0.0f, 1.0f);
        render();
    }

    void calculateHeights(float left, float right) {
        _leftHeight = std::round(height - (left * height));
        _rightHeight = std::round(height - (right * height));
    }

    void draw(Vector offset) override {
        auto new_position = position + offset;
        auto padding = 2.0;

        auto volumeHeight = height - (_volume * height);

        auto meterWidth = ((double)width / 3) - (padding * 2);
        // draw left meter
        cairo_draw_rectangle(cr, 0, _leftHeight, meterWidth+1, height, true, true);

        // draw right meter
        cairo_draw_rectangle(cr, meterWidth + padding +1, _rightHeight, meterWidth, height, true, true);

        cairo_draw_horizontal_line(cr, volumeHeight, meterWidth*2, meterWidth*3 + padding);
        cairo_draw_vertical_line(cr, meterWidth*3 + padding-1, volumeHeight, height, 1.0, true);
    }

protected:
    float _volume;
    float _leftHeight;
    float _rightHeight;
    string _formatString = "{:.2f} dB";
};