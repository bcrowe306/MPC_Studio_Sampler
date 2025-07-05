#include <iostream>
#include "control_surface/devices/mpc_studio_black.h"
#include "ui/widgets/widgets.h"
#include <memory>
#include "control_surface/mpc_studio_black_surface.h"
#include "ui/display.h"
#include "core/mpc_sampler.h"
using std::shared_ptr;
using std::make_shared;


int main(int, char **) {


    shared_ptr<MPCSampler> mpc_sampler = make_shared<MPCSampler>();
    mpc_sampler->initialize(); // Initialize the MPC Sampler

    shared_ptr<MPCStudioBlackControlSurface> controlSurface = make_shared<MPCStudioBlackControlSurface>();
    auto mpcDevice = controlSurface->device; // Get the MPC Studio Black device
    
    float volume = 0.5f; // Initial volume value
    auto display = make_shared<Display>(std::bind(&MPCStudioBlackDevice::sendImageBuffer, mpcDevice.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    auto knobWidget = make_shared<KnobWidget>(0, 0, 90, volume, 0.0f, 1.0f);
    display->add_page("page1", knobWidget);
    display->initialize();
    display->show_page("page1");

    controlSurface->qlinkEncoder1->onOffset.connect([&](int offset) {
        volume += offset * 0.01f; // Adjust volume based on encoder offset
        volume = std::clamp(volume, 0.0f, 1.0f); // Clamp volume between 0 and 1
        knobWidget->setValue(volume); // Update knob widget value
        std::cout << "Volume adjusted to: " << volume << std::endl;
    });

    controlSurface->qlinkEncoder4->onOffset.connect([&](int offset) {
        volume += offset * 0.01f; // Adjust volume based on encoder offset
        volume = std::clamp(volume, 0.0f, 1.0f); // Clamp volume between 0 and 1
        knobWidget->setValue(volume); // Update knob widget value
        std::cout << "Volume adjusted to: " << volume << std::endl;
    });

        std::cin.get(); // Wait for user input to exit

    std::cout << "MIDI ports closed. Exiting program.\n";
    return 0;
}
