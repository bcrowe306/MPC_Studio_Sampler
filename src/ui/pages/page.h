#pragma once
#include "ui/widgets/widget.h"
#include <memory>
#include <string>
#include <vector>
#include "core/mpc_sampler.h"

using std::shared_ptr;


class Page : public Widget {
public:
    std::string _title;
    shared_ptr<MPCSampler> mpcSampler;
    Page(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height) : Widget(x, y, width, height), mpcSampler(mpcSampler)
    {

    }
    virtual ~Page() = default;

    // Is called at every frame update per the frame rate of the application
    // Override this method to handle frame updates for the page ie animations, visual updates, etc.
    virtual void onFrame() {
    }
};