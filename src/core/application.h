#pragma once

#include "control_surface/build_surface.h"
#include "core/mpc_sampler.h"
#include "core/project.h"
#include "core/project_manager.h"
#include "sigslot/signal.hpp"
#include "ui/display.h"
#include "util.h"
#include "yaml-cpp/yaml.h"
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

using std::make_shared;
using std::shared_ptr;

class Application {
public:
    std::atomic<bool> running {true}; // Atomic boolean to control the application loop
    sigslot::connection projectSaveConnection;
    shared_ptr<ProjectManager> projectManager;
    
    Application() {
        // Initialize application components
        mpcSampler = make_shared<MPCSampler>();
        projectManager = make_shared<ProjectManager>(mpcSampler);
        controlSurface = build_surface(mpcSampler);
        controlSurface->saveProject.connect([this]() {
            projectManager->saveProject();
        });
        projectManager->createProjectDirectories();
        projectManager->onProjectLoaded.connect([this]() {
            onProjectLoaded();
        });

        display = create_display(
            mpcSampler, 
            controlSurface, 
            projectManager,
            std::bind(&MPCStudioBlackDevice::sendImageBuffer, controlSurface->device.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        controlSurface->router = display->router; // Set the router for the control surface
        projectManager->newProject("");
        activate_surface(controlSurface); // Activate the control surface components
    }


    void onProjectLoaded(){
        projectSaveConnection.disconnect(); // Disconnect any existing project save connections
        display->uninitialize();
        project.reset(); // Reset the project pointer
        project = projectManager->currentProject;
        display->initialize(project);
        projectSaveConnection = project->saveProject.connect([this](const std::string& projectName) {
            projectManager->saveProject();
        });
        mpcSampler->onProjectLoaded();
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