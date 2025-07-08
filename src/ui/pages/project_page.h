#pragma once
#include "core/project.h"
#include "page_widget.h"
#include "widgets/ui_helpers.h"
#include <string>

class ProjectPage : public PageWidget {
    // Device-specific UI elements and layout
public:
    ProjectPage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Project Page")
        : PageWidget(mpcSampler, x, y, width, height), _title(title) {
    }
    void draw(Vector offset) override {
       cairo_draw_text(cr, _title, 2, 12, 12);
    }
protected:
    std::string _title;
    
};