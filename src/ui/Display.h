#pragma once

#include "control_surface/mpc_studio_black_surface.h"
#include "core/mpc_sampler.h"
#include "ui/pages/pages.h"
#include "ui/widgets/widgets.h"
#include "widgets/widget.h"
#include <memory>
#include <string>
#include <unordered_map>
#include "sigslot/signal.hpp"
using std::shared_ptr;
using std::unordered_map;
using std::string;

class Display {
public:
    unordered_map<string, shared_ptr<PageWidget>> pages;
    
    EncodeSurfaceCallback encode_surface_callback;
    shared_ptr<MPCSampler> mpcSampler;
    shared_ptr<MPCStudioBlackControlSurface> controlSurface;
    string current_page;
    sigslot::signal<> onFrame;
    Display(shared_ptr<MPCSampler> mpcSampler, shared_ptr<MPCStudioBlackControlSurface> controlSurface, EncodeSurfaceCallback encode_surface_callback) 
        : mpcSampler(mpcSampler), controlSurface(controlSurface), encode_surface_callback(encode_surface_callback)
    {
        // Initialize the display with an empty set of pages

    }
    ~Display() {
        
    }

    void initialize() {
        // Initialize all pages
        for (auto &[name, page] : pages) {
            if (page) {
                page->initialize(encode_surface_callback);
                page->controlSurface = controlSurface; // Set the control surface for each page
            }
        }

        // Connect the displayPage parameter to the show_page method
        mpcSampler->project->displayPage.onValueChanged.connect(std::bind(&Display::show_page, this, std::placeholders::_1));
        auto initialPage = mpcSampler->project->displayPage.getValue();
        show_page(initialPage);
    }

    void add_page(const string &name, shared_ptr<PageWidget> page) {
        if (page) {
            pages[name] = page;
        }
    }
    shared_ptr<PageWidget> get_page(const string &name) {
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

                // Connect onFrame signal to the page's onFrame method
                _pageConnections[page_name] = onFrame.connect(std::bind(&PageWidget::onFrame, page.get()));
                page->activate();
                current_page = name;
                page->render();
            } else {
                _pageConnections[page_name].disconnect();
                page->deactivate();
            }
        }
    }
protected:
    unordered_map<string, sigslot::scoped_connection> _pageConnections; // Connections for each page's onFrame signal

};


static inline shared_ptr<Display> create_display(shared_ptr<MPCSampler> mpcSampler, shared_ptr<MPCStudioBlackControlSurface> controlSurface, EncodeSurfaceCallback encode_surface_callback) {
    auto display = std::make_shared<Display>(mpcSampler, controlSurface, encode_surface_callback);
    auto devicePage = make_shared<DevicePage>(mpcSampler, 0, 0, 360, 96);
    auto sequencePage = make_shared<SequencePage>(mpcSampler, 0, 0, 360, 96);
    auto arrangerPage = make_shared<ArrangerPage>(mpcSampler, 0, 0, 360, 96);
    auto projectPage = make_shared<ProjectPage>(mpcSampler, 0, 0, 360, 96);
    auto mixerPage = make_shared<MixerPage>(mpcSampler, 0, 0, 360, 96);
    auto performPage = make_shared<PerformPage>(mpcSampler, 0, 0, 360, 96);
    auto browserPage = make_shared<BrowserPage>(mpcSampler, 0, 0, 360, 96);
    auto settingsPage = make_shared<SettingsPage>(mpcSampler, 0, 0, 360, 96);

    display->add_page("devicePage", devicePage);
    display->add_page("sequencePage", sequencePage);
    display->add_page("arrangerPage", arrangerPage);
    display->add_page("projectPage", projectPage);
    display->add_page("mixerPage", mixerPage);
    display->add_page("performPage", performPage);
    display->add_page("browserPage", browserPage);
    display->add_page("settingsPage", settingsPage);
    display->initialize();
    return display;
}