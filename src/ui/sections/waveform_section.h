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
        if (_waveformData && !_waveformData->empty()) {
            size_t numSamples = _waveformData->size();
            float xStep = static_cast<float>(width) / numSamples;
            float centerY = height / 2.0f;
            float halfHeight = (height - 8) / 2.0f;

            for (size_t i = 0; i < numSamples; ++i) {
                float value = (*_waveformData)[i];
                float x = i * xStep;
                float y1 = centerY - value * halfHeight;
                float y2 = centerY + value * halfHeight;
                cairo_move_to(cr, x, y1);
                cairo_line_to(cr, x, y2);
            }
            cairo_set_source_rgb(cr, 1, 1, 1.0); // waveform color
            cairo_stroke(cr);
        }
    }
    void setWaveformData(std::vector<float> *data) {
        _waveformData = data;
        render();
    }
    void setTitle(const std::string &title) {
        _title = title;
        render();
    }
protected:
    std::string _title;
    std::vector<float> *_waveformData;
};