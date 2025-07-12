#pragma once
#include "control_surface/mpc_studio_black_surface.h"
#include "core/mpc_sampler.h"
#include "sigslot/signal.hpp"
#include "ui/widgets/widget.h"
#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;

// Forward declaration of Router class
class Router;

class PageWidget : public Widget {
public:
    std::string _title;
    shared_ptr<MPCSampler> mpcSampler;
    shared_ptr<Router> router;
    std::vector<sigslot::connection> signalConnections;
    shared_ptr<MPCStudioBlackControlSurface> controlSurface;
    sigslot::signal<> deactivatedSignal; // Signal emitted when the page is deactivated
    PageWidget(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    virtual ~PageWidget() = default;

    // Is called at every frame update per the frame rate of the application
    // Override this method to handle frame updates for the page ie animations, visual updates, etc.
    virtual void onFrame() {
    }

    void addConnection(const sigslot::connection &connection);

    void onDeactivated() override ;
};