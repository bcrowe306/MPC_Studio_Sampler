#pragma once
#include "page.h"
#include <string>

class SettingsPage : public Page {
    // Device-specific UI elements and layout
public:
    SettingsPage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Settings Page")
        : Page(mpcSampler, x, y, width, height), _title(title) {
        render();
    }
    void draw(Vector offset) override {
       
    }
protected:
    std::string _title;
    
};