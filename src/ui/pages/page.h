#pragma once
#include "ui/widgets/widget.h"
#include <memory>
#include <string>
#include <vector>
#include "core/mpc_sampler.h"

using std::shared_ptr;


class Page : public Widget {
public:
    shared_ptr<MPCSampler> mpcSampler;
    Page(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height) : Widget(x, y, width, height), mpcSampler(mpcSampler)
    {

    }
    virtual ~Page() = default;
    // Page-specific members and methods    
};