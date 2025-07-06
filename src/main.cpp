#include <iostream>
#include "audio/choc_MIDI.h"
#include "control_surface/devices/mpc_studio_black.h"
#include "ui/widgets/widgets.h"
#include "ui/pages/pages.h"
#include <memory>
#include <string>
#include "control_surface/mpc_studio_black_surface.h"
#include "ui/display.h"
#include "core/mpc_sampler.h"
#include "core/command.h"
#include "core/value_receiver.h"
#include "containers/choc_Value.h"
#include "widgets/text_widget.h"
#include <thread>
#include <vector>
using std::shared_ptr;
using std::make_shared;


int main(int, char **) {


    // MPC Sampler - Audio and engine backend
    shared_ptr<MPCSampler> mpc_sampler = make_shared<MPCSampler>();
    mpc_sampler->initialize();


    // Control Surface - MPC Studio Black
    shared_ptr<MPCStudioBlackControlSurface> controlSurface = make_shared<MPCStudioBlackControlSurface>();
    auto mpcDevice = controlSurface->device;
   
    // UI - Display and Widgets
    auto display = make_shared<Display>(std::bind(&MPCStudioBlackDevice::sendImageBuffer, mpcDevice.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    string deviceText = "MPC Studio Black Is the Best Device on the planet";
    auto textPage = make_shared<TextWidget>(0, 10, 360, 60, deviceText);
    auto devicePage = make_shared<DevicePage>(0, 0, 360, 96, "MPC Studio Black");
    auto rect = make_shared<RectangleWidget>(200, 10, 10, 10);
    auto sequencePage = make_shared<SequencePage>(0, 0, 360, 96, "MPC Studio Black");

    std::vector<shared_ptr<FunctionWidget>> functionWidgets;
    for (int i = 0; i < 6; i++) {
        auto functionWidget = make_shared<FunctionWidget>(i * 60, 96-12, 60, 13, fmt::format("F{}", i+1), false, "center");
        functionWidgets.push_back(functionWidget);
        devicePage->add_child(functionWidget);
    }

    std::vector<shared_ptr<ParameterWidget>> parameterWidgets;
    for (int i = 0; i < 4; i++){
        auto parameterWidget = make_shared<ParameterWidget>(i * 60 + 60, 42, 60, 40, 0.5f, fmt::format("Param {}", i+1), 0.0f, 1.0f);
        devicePage->add_child(parameterWidget);
        parameterWidgets.push_back(parameterWidget);
    }
    auto buttonWidget = make_shared<ButtonWidget>(60 * 5 + 4, 96-30, 54, 13, "Main", false, 11);
    auto meterWidget = make_shared<MeterWidget>(332, 12, 28, 40, 0.5f, 0.9f, 0.7f);
    devicePage->add_child(buttonWidget);
    devicePage->add_child(meterWidget);
    display->add_page("devicePage", devicePage);
    display->add_page("rect", rect);
    display->add_page("sequencePage", sequencePage);
    display->add_page("textPage", textPage);
    display->show_page("devicePage");

    display->initialize();

    controlSurface->progEditButton->onPressed.connect([&]() {
        display->show_page("devicePage");
        // Here you can add logic to handle the program edit button press
        // For example, you might want to toggle a specific mode or update the UI
    });

    controlSurface->seqEditButton->onPressed.connect([&]() {
        display->show_page("sequencePage");
        // Here you can add logic to handle the sequence edit button press
        // For example, you might want to toggle a specific mode or update the UI
    });
    

    controlSurface->functionButtons->onPressed.connect([&](int index) {
        for(int i = 0; i < functionWidgets.size(); i++) {
            functionWidgets[i]->setSelected(i == index);
        }
        
    });
    controlSurface->qlinkEncoders->onOffset.connect([&](int index, float offset) {
        auto paramValue = parameterWidgets[index]->value();
        if(offset > 0.0f) {
            parameterWidgets[index]->setValue(paramValue + 0.01f);
        } else {
            parameterWidgets[index]->setValue(paramValue - 0.01f);
        }
    });

    controlSurface->qlinkEncoderTouches->onPressed.connect([&](int index) {
       for(int i = 0; i < parameterWidgets.size(); i++) {
            parameterWidgets[i]->setSelected(i == index);
        }
    });

    controlSurface->jogWheel->onOffset.connect([&](float offset) {
        // Handle jog wheel offset
        // For example, you might want to adjust a parameter value or scroll through a list
        auto currentParam = meterWidget->getVolume();
        if(offset > 0.0f) {
            meterWidget->setVolume(currentParam + 0.01f);
        } else {
            meterWidget->setVolume(currentParam - 0.01f);
        }
    });

    std::cin.get(); // Wait for user input to proceed

    // // Application loop
    // while (true) {

    // if (mpc_sampler->project->undoManager->hasFlushableCommands.load(std::memory_order_acquire)) {
    //     mpc_sampler->project->undoManager->flushToUndoStack();
    // }
    //     std::this_thread::sleep_for(std::chrono::milliseconds(80));

        
    
    // }

    std::cout << "MIDI ports closed. Exiting program.\n";
    return 0;
}
