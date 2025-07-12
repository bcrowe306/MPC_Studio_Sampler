#include <iostream>
#include <memory>
#include <string>
#include "control_surface/controls/control.h"
#include "ui/display.h"
#include "core/mpc_sampler.h"
#include <thread>
#include <vector>
#include <atomic>
#include "control_surface/build_surface.h"

using std::shared_ptr;
using std::make_shared;

static void applicationLoop(std::atomic<bool> &running, shared_ptr<MPCSampler> mpc_sampler, shared_ptr<MPCStudioBlackControlSurface> controlSurface, shared_ptr<Display> display) {
    // Application loop
    while (running.load()) {
        if (mpc_sampler->project->undoManager->hasFlushableCommands.load(std::memory_order_acquire)) {
            mpc_sampler->project->undoManager->flushToUndoStack();
        }
        display->router->onFrame(); // Call the display's onFrame method to update the UI
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}


int main(int, char **) {


    // MPC Sampler - Audio and engine backend
    shared_ptr<MPCSampler> mpc_sampler = make_shared<MPCSampler>();

     // Control Surface - MPC Studio Black Midi device
     shared_ptr<MPCStudioBlackControlSurface> controlSurface = build_surface(mpc_sampler);
     auto mpcDevice = controlSurface->device;

     // Class to draw on the MPC Studio Black display
     auto display = create_display(mpc_sampler, controlSurface, std::bind(&MPCStudioBlackDevice::sendImageBuffer, mpcDevice.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // Launch the application loop in a separate thread. need boolean flag to control the loop
    std::atomic<bool> running(true);
    std::thread appThread(applicationLoop, std::ref(running), mpc_sampler, controlSurface, display);

    

    std::cin.get(); // Wait for user input to proceed

    // Join the application thread before exiting
    running = false;
    if (appThread.joinable()) {
        appThread.join();
    }
    
    controlSurface->device->close();

    std::cout << "MIDI ports closed. Exiting program.\n";
    return 0;
}
