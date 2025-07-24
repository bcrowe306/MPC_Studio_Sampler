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
#include "router.h"

using std::shared_ptr;
using std::unordered_map;
using std::string;

typedef shared_ptr<unordered_map<string, shared_ptr<PageWidget>>> PageCollection;

class Project;

class Display {
public:
    EncodeSurfaceCallback encode_surface_callback;
    shared_ptr<MPCSampler> mpcSampler;
    shared_ptr<MPCStudioBlackControlSurface> controlSurface;
    shared_ptr<Router> router;
    
    Display(
        shared_ptr<Router> router,
        shared_ptr<MPCSampler> mpcSampler,
        shared_ptr<MPCStudioBlackControlSurface> controlSurface,
        EncodeSurfaceCallback encode_surface_callback)
        : router(router), mpcSampler(mpcSampler), controlSurface(controlSurface),
          encode_surface_callback(encode_surface_callback) {
        // Initialize the display with an empty set of pages
    }
    ~Display() {
        
    }

    void initialize(shared_ptr<Project> project) {
        // Initialize all pages
        for (auto &[name, page] : *router->get_pages()) {
            if (page) {
                page->initialize(encode_surface_callback);
                page->controlSurface = controlSurface; // Set the control surface for each page
                page->router = router; // Set the router for each page
                page->project = project; // Set the project for each page
            }
        }

        router->push("devicePage"); // Show the device page by default
    }

    void uninitialize() {
        // Uninitialize all pages
        router->deactivateAll(); // Deactivate all pages
    }

    
protected:

};


static inline shared_ptr<Display> create_display(
    shared_ptr<MPCSampler> mpcSampler, 
    shared_ptr<MPCStudioBlackControlSurface> controlSurface, 
    EncodeSurfaceCallback encode_surface_callback) {
    auto router = std::make_shared<Router>();

    auto display = std::make_shared<Display>(router, mpcSampler, controlSurface, encode_surface_callback);
    auto devicePage = make_shared<DevicePage>(mpcSampler, 0, 0, 360, 96);
    auto sequencePage = make_shared<SequencePage>(mpcSampler, 0, 0, 360, 96);
    auto arrangerPage = make_shared<ArrangerPage>(mpcSampler, 0, 0, 360, 96);
    auto projectPage = make_shared<ProjectPage>(mpcSampler, 0, 0, 360, 96);
    auto mixerPage = make_shared<MixerPage>(mpcSampler, 0, 0, 360, 96);
    auto performPage = make_shared<PerformPage>(mpcSampler, 0, 0, 360, 96);
    auto browserPage = make_shared<BrowserPage>(mpcSampler, 0, 0, 360, 96);
    auto settingsPage = make_shared<SettingsPage>(mpcSampler, 0, 0, 360, 96);

    router->add_page("devicePage", devicePage);
    router->add_page("sequencePage", sequencePage);
    router->add_page("arrangerPage", arrangerPage);
    router->add_page("projectPage", projectPage);
    router->add_page("mixerPage", mixerPage);
    router->add_page("performPage", performPage);
    router->add_page("browserPage", browserPage);
    router->add_page("settingsPage", settingsPage);
    return display;
}

