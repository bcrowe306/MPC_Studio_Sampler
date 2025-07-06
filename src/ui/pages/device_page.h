#pragma once
#include "ui/widgets/widget.h"
#include <string>

class DevicePage : public Widget {
    // Device-specific UI elements and layout
public:
    DevicePage(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Device Page")
        : Widget(x, y, width, height), _title(title) {
        render();
    }
    void draw(Vector offset) override {
       
    }
protected:
    std::string _title;
    
};