#pragma once
#include "page_widget.h"
#include "router.h"
using std::shared_ptr;



PageWidget::PageWidget(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height) : Widget(x, y, width, height), mpcSampler(mpcSampler)
{
  // Create function button widgets
  for (int i = 0; i < 6; i++) {
      auto functionWidget = make_shared<FunctionButtonWidget>(
          i * 60, 96 - 11, 60, 13, fmt::format("F{}", i + 1), false,
          "center");
      functionWidgets.push_back(functionWidget);
      this->add_child(functionWidget);
  }
}

// Is called at every frame update per the frame rate of the application
// Override this method to handle frame updates for the page ie animations,
// visual updates, etc.

void PageWidget::addConnection(const sigslot::connection &connection) {
  // Add a signal connection to the page
  signalConnections.push_back(connection);
}

void PageWidget::addTrackConnection(const sigslot::connection &connection) {
  // Add a track connection to the page
  trackConnections.push_back(connection);
}

void PageWidget::addSequenceConnection(const sigslot::connection &connection) {
    // Add a sequence connection to the page
    sequenceConnections.push_back(connection);
}

void PageWidget::onPreActivated() {
    addConnection(mpcSampler->onTrackSelected.connect(std::bind(&PageWidget::onTrackSelected, this, std::placeholders::_1))); 
    onTrackSelected(); // Initialize with no track selected
    addConnection(mpcSampler->sequencer->onSequenceSelected.connect(std::bind(&PageWidget::onSequenceSelected, this, std::placeholders::_1)));
    onSequenceSelected(); // Initialize with no sequence selected
}

void PageWidget::onDeactivated() {
        // Deactivate all signal connections when the page is deactivated
        for (auto &connection : trackConnections) {
            connection.disconnect();
        }
        trackConnections.clear();
        for (auto &connection : sequenceConnections) {
            connection.disconnect();
        }
        sequenceConnections.clear();
        
        for (auto &connection : signalConnections) {
            connection.disconnect();
        }
        signalConnections.clear();

        deactivatedSignal(); // Emit the deactivated signal
    }
