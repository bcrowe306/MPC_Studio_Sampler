#include "browser_page.h"
#include "router.h"

// Constructor for the browser page
BrowserPage::BrowserPage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y,
            unsigned int width, unsigned int height,
            const std::string &title)
    : PageWidget(mpcSampler, x, y, width, height), _title(title) 
{
}

void BrowserPage::onActivated() {
    functionWidgets[0]->setLabel("Root");
    functionWidgets[1]->setLabel("Next");
    functionWidgets[2]->setLabel("Scroll");
    functionWidgets[3]->setLabel("Auto");
    functionWidgets[4]->setLabel("Prev");
    functionWidgets[5]->setLabel("Load");
    // Connect signals and set up the page when activated
    signalConnections.push_back(controlSurface->jogWheel->onOffset.connect([this](int offset) {
        mpcSampler->browser->scroll(offset);
    }));
    signalConnections.push_back(controlSurface->upButton->onPressed.connect([this]() {
        mpcSampler->browser->scroll(-1);
    }));
    signalConnections.push_back(controlSurface->downButton->onPressed.connect([this]() {
        mpcSampler->browser->scroll(1);
    }));
    
    signalConnections.push_back(controlSurface->leftButton->onPressed.connect([this]() {
        mpcSampler->browser->goBack();
    }));

    signalConnections.push_back(controlSurface->rightButton->onPressed.connect([this]() {
        mpcSampler->browser->navigateToSelectedItem();
    }));

    signalConnections.push_back(controlSurface->plusButton->onPressed.connect([this]() {
        if(controlSurface->shiftButton->isPressed) {
            mpcSampler->browser->page(1);
        } else {
            mpcSampler->browser->scroll(1);
        }
    }));

    signalConnections.push_back(controlSurface->minusButton->onPressed.connect([this]() {
        if(controlSurface->shiftButton->isPressed) {
            mpcSampler->browser->page(-1);
        } else {
            mpcSampler->browser->scroll(-1);
        }   
    }));

    signalConnections.push_back(controlSurface->f4Button->onPressed.connect([this]() {
        mpcSampler->browser->autoPreview = !mpcSampler->browser->autoPreview.get(); // Toggle auto-preview
    }));

    signalConnections.push_back(controlSurface->f5Button->onPressed.connect([this]() {
        mpcSampler->browser->previewItem();
    }));

    signalConnections.push_back(controlSurface->f6Button->onPressed.connect([this]() {
        mpcSampler->browser->stopPreview(); // Stop previewing the current item
        auto selectedItem = mpcSampler->browser->getSelectedItem();
        auto track = mpcSampler->selectedTrack();
        if (selectedItem.is_regular_file() && selectedItem.path().extension() == ".wav") {
            bool success = track->loadSample(selectedItem.path().string());
            if(success){
                router->pop();
            }
        }
    }));
    
    signalConnections.push_back( mpcSampler->browser->onScroll.connect( [this](int newOffset) 
        { 
            updateBrowserView(); 
        }
    ));

    signalConnections.push_back( mpcSampler->browser->onRefresh.connect( [this]() 
        { 
            updateBrowserView(); 
        }
    ));

    signalConnections.push_back( mpcSampler->browser->autoPreview.onValueChanged.connect( [this](bool enabled) 
        { 
            functionWidgets[3]->setSelected(enabled);
        }
    ));
    updateBrowserView(); // Initial update to set the view
    functionWidgets[3]->setSelected(mpcSampler->browser->autoPreview.get()); // Set initial state of auto-preview button
}

void BrowserPage::updateBrowserView(){
    int newOffset = mpcSampler->browser->getScrollIndex();
    _items = mpcSampler->browser->getVisibleItems();
    auto viewRange = mpcSampler->browser->getViewRange();
    _selectedViewIndex = newOffset - viewRange.start;
    _scrollIndex = newOffset;
    _itemsCount = mpcSampler->browser->getItemsCount();
    _currentDirectoryName = mpcSampler->browser->getCurrentDirectoryName();
    _selectedItemName = _items.empty() ? "No item selected"  : _items[_selectedViewIndex].path().filename().string();
    render();
}

void BrowserPage::draw(Vector offset) {
    cairo_draw_text(cr, fmt::format("{:.12}", _currentDirectoryName), 1, 10, _fontSize);
    cairo_draw_text(cr, fmt::format("{:.60}", _selectedItemName), 90, 10, _fontSize);
    cairo_draw_horizontal_line(cr, 12, 0, width);

    for(int i = 0; i < _items.size(); i++) {
        int start_y = 13 + i * 13;
        bool isSelected = (i == _selectedViewIndex);
        directory_entry itemEntry = _items[i];
        if(isSelected){
            cairo_draw_rectangle(cr, 1, start_y + 1, width - 10, 12,true);
        }
        if(itemEntry.is_directory()) {
            cairo_draw_folder_icon(cr, 2, start_y + 2, 8, 10, !isSelected);
            cairo_draw_text(cr, fmt::format("{:.50}", itemEntry.path().filename().string()), 13, start_y + _fontSize, _fontSize,!isSelected);
        } else if(itemEntry.path().extension() == ".wav") {
            // Draw WAV icon and filename
            cairo_draw_wav_icon(cr, 1, start_y, 8, 11, !isSelected);
            cairo_draw_text(cr, fmt::format("{:.50}", itemEntry.path().filename().string()), 13, start_y + _fontSize, _fontSize,!isSelected);
        }
    }
    cairo_draw_vertical_scrollbar(cr, width - 8, 13, 8, height - 25,
                                  _itemsCount, _pageSize, _scrollIndex);
}
