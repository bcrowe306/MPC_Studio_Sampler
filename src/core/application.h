#pragma once

#include "control_surface/build_surface.h"
#include "core/mpc_sampler.h"
#include "ui/display.h"
#include "core/project.h"
#include <atomic>

#include <memory>
#include <thread>

using std::make_shared;
using std::shared_ptr;

class Application {
public:
    std::atomic<bool> running {true}; // Atomic boolean to control the application loop
   
    Application() {
        // Initialize application components
        mpcSampler = make_shared<MPCSampler>();
        controlSurface = build_surface(mpcSampler);
        display = create_display(
            mpcSampler, 
            controlSurface, 
            std::bind(&MPCStudioBlackDevice::sendImageBuffer, controlSurface->device.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        controlSurface->router = display->router; // Set the router for the control surface
        newProject(); // Create a new project
        activate_surface(controlSurface); // Activate the control surface components
    }

    
    void newProject() {
        display->uninitialize(); // Uninitialize the display before creating a new project
        project = make_shared<Project>(mpcSampler);
        display->initialize(project); // Initialize the display with the new project
    }

    void run() {
        appThread = std::thread(&Application::applicationLoop, this);
        appThread.detach(); // Detach the thread to run independently

        std::cin.get();     // Wait for user input to proceed

        // Join the application thread before exiting
        running = false;
        if (appThread.joinable()) {
          appThread.join();
        }
        controlSurface->device->close();

        std::cout << "MIDI ports closed. Exiting program.\n";
    }

    void applicationLoop(){
        // Application loop
        while (running.load()) {
            if (mpcSampler->undoManager->hasFlushableCommands.load(std::memory_order_acquire)) {
                mpcSampler->undoManager->flushToUndoStack();
            }
            display->router->onFrame(); // Call the display's onFrame method to update the UI
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }

private:
    std::thread appThread; // Thread for the application loop
    shared_ptr<MPCSampler> mpcSampler;
    shared_ptr<MPCStudioBlackControlSurface> controlSurface;
    shared_ptr<Display> display;
    shared_ptr<Project> project;
};