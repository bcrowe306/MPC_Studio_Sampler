#pragma once
#include "page_widget.h"
#include "router.h"
using std::shared_ptr;



PageWidget::PageWidget(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y, unsigned int width, unsigned int height) : Widget(x, y, width, height), mpcSampler(mpcSampler)
{

}

// Is called at every frame update per the frame rate of the application
// Override this method to handle frame updates for the page ie animations,
// visual updates, etc.

void PageWidget::addConnection(const sigslot::connection &connection) {
  // Add a signal connection to the page
  signalConnections.push_back(connection);
}

void PageWidget::onDeactivated() {
        // Deactivate all signal connections when the page is deactivated
        for (auto &connection : signalConnections) {
            connection.disconnect();
        }
        signalConnections.clear();
        deactivatedSignal(); // Emit the deactivated signal
    }
