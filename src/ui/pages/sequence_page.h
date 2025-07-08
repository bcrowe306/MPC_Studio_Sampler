#pragma once
#include "page_widget.h"
#include <string>

class SequencePage : public PageWidget {
    // Device-specific UI elements and layout
public:
    SequencePage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Sequence Page")
        : PageWidget(mpcSampler, x, y, width, height), _title(title) {
    }
    void draw(Vector offset) override {
       
    }
protected:
    std::string _title;
    
};