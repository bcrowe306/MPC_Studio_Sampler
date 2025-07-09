#include "control_surface/mpc_studio_black_surface.h"
#include "control_surface/components/components.h"

static inline shared_ptr<MPCStudioBlackControlSurface> build_surface(shared_ptr<MPCSampler> mpc_sampler) {
    // Create the control surface
    auto controlSurface = make_shared<MPCStudioBlackControlSurface>(mpc_sampler);
    
    // Create components
    controlSurface->sessionComponent = make_shared<SessionComponent>(controlSurface);
    controlSurface->pageComponent = make_shared<PageComponent>(controlSurface);
    controlSurface->transportComponent = make_shared<TransportComponent>(controlSurface);
    
    // Activate components
    controlSurface->sessionComponent->activate();
    controlSurface->pageComponent->activate();
    controlSurface->transportComponent->activate();

    // controlSurface->recordButton->onPressed.connect([&]() {
    //     bool isActive = controlSurface->sessionComponent->isActive();
    //     if (isActive) {
    //         controlSurface->sessionComponent->deactivate();
    //     } else {
    //         controlSurface->sessionComponent->activate();
    //     }
    // });

    return controlSurface;
}