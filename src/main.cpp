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
    

    auto display = make_shared<Display>(std::bind(&MPCStudioBlackDevice::sendImageBuffer, mpcDevice.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    auto page1 = make_shared<TextWidget>(0, 0, 360, 96, "MPC Studio Blank Page 1", 24);
    display->add_page("page1", page1);
    display->initialize();
    display->show_page("page1");


    shared_ptr<OneColorButtonControl> playButton = std::dynamic_pointer_cast<OneColorButtonControl>(MPCStudioBlackControls["playButton"]);
    shared_ptr<OneColorButtonControl> stopButton = std::dynamic_pointer_cast<OneColorButtonControl>(MPCStudioBlackControls["stopButton"]);


        std::cin.get(); // Wait for user input to exit

    std::cout << "MIDI ports closed. Exiting program.\n";
    return 0;
}
