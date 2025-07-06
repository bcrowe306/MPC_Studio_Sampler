#pragma once
#include "ui/widgets/widget.h"
#include <string>

class SequencePage : public Widget {
    // Device-specific UI elements and layout
public:
    SequencePage(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Sequence Page")
        : Widget(x, y, width, height), _title(title) {
        render();
    }
    void draw(Vector offset) override {
       
    }
protected:
    std::string _title;
    
};