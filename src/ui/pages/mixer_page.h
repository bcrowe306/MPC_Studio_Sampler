#pragma once
#include "page_widget.h"
#include "widgets/ui_helpers.h"
#include <string>

class MixerPage : public PageWidget {
    // Device-specific UI elements and layout
public:
    MixerPage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Mixer Page")
        : PageWidget(mpcSampler, x, y, width, height), _title(title) {
    }
    void draw(Vector offset) override {
       cairo_draw_text(cr, _title, 2, 12, 12);
    }
protected:
    std::string _title;
    
};