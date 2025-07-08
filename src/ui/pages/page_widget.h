#pragma once
#include "core/mpc_sampler.h"
#include "sigslot/signal.hpp"
#include "ui/widgets/widget.h"
#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;


class PageWidget : public Widget {
public:
    std::string _title;
    shared_ptr<MPCSampler> mpcSampler;
    std::vector<sigslot::connection> signalConnections;
    PageWidget(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height) : Widget(x, y, width, height), mpcSampler(mpcSampler)
    {

    }
    virtual ~PageWidget() = default;

    // Is called at every frame update per the frame rate of the application
    // Override this method to handle frame updates for the page ie animations, visual updates, etc.
    virtual void onFrame() {
    }
    void onDeactivated() override {
        // Deactivate all signal connections when the page is deactivated
        for (auto &connection : signalConnections) {
            connection.disconnect();
        }
        signalConnections.clear();
    }
};