#pragma once

#include "core/mpc_sampler.h"
#include "ui/pages/pages.h"
#include "ui/widgets/widgets.h"
#include "widgets/widget.h"
#include <memory>
#include <string>
#include <unordered_map>
using std::shared_ptr;
using std::unordered_map;
using std::string;

class Display {
public:
    unordered_map<string, shared_ptr<Widget>> pages;
    EncodeSurfaceCallback encode_surface_callback;
    shared_ptr<MPCSampler> mpcSampler;
    string current_page;
    Display(shared_ptr<MPCSampler> mpcSampler, EncodeSurfaceCallback encode_surface_callback) 
        : mpcSampler(mpcSampler), encode_surface_callback(encode_surface_callback), current_page("") {
        // Initialize the display with an empty set of pages

    }
    ~Display() {
        
    }

    void initialize() {
        // Initialize all pages
        for (auto &[name, page] : pages) {
            if (page) {
                page->initialize(encode_surface_callback);
            }
        }

        // Connect the displayPage parameter to the show_page method
        mpcSampler->project->displayPage.onValueChanged.connect(std::bind(&Display::show_page, this, std::placeholders::_1));
        auto initialPage = mpcSampler->project->displayPage.getValue();
        show_page(initialPage);
    }

    void add_page(const string &name, shared_ptr<Widget> page) {
        if (page) {
            pages[name] = page;
        }
    }
    shared_ptr<Widget> get_page(const string &name) {
        auto it = pages.find(name);
        if (it != pages.end()) {
            return it->second;
        }
        return nullptr;
    }

    void show_page(const string &name) {
        // show page by name and activating it. only one page can be active at a time
        if (current_page == name) {
            return; // Already showing this page
        }
        for(auto &[page_name, page] : pages) {
            if(page_name == name) {
                page->activate();
                current_page = name;
                page->render();
            } else {
                page->deactivate();
            }
        }
    }
};


static inline shared_ptr<Display> create_display(shared_ptr<MPCSampler> mpcSampler, EncodeSurfaceCallback encode_surface_callback) {
    auto display = std::make_shared<Display>(mpcSampler, encode_surface_callback);
    auto devicePage = make_shared<DevicePage>(mpcSampler, 0, 0, 360, 96);
    auto sequencePage = make_shared<SequencePage>(mpcSampler, 0, 0, 360, 96);

    display->add_page("devicePage", devicePage);
    display->add_page("sequencePage", sequencePage);
    display->initialize();
    return display;
}