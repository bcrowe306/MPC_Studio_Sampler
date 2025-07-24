#pragma once
#include "fmt/format.h"
#include "page_widget.h"
#include "sections/titlebar_section.h"
#include "widgets/vertical_scrollbar_widget.h"
#include "widgets/widgets.h"
#include "widgets/ui_helpers.h"
#include <filesystem>
#include <memory>
#include <string>

class BrowserPage : public PageWidget {
    // Device-specific UI elements and layout
public:
    // Constructor for the browser page
    BrowserPage(shared_ptr<MPCSampler> mpcSampler, unsigned int x, unsigned int y,
                unsigned int width, unsigned int height,
                const std::string &title = "Browser Page");

    void onActivated() override;
    void updateBrowserView();
    void draw(Vector offset) override;

protected:
    string _currentDirectoryName = "Samples";
    string _selectedItemName = "No item selected";
    int _fontSize = 10;
    std::string _title;
    int _pageSize = 5; // Number of items displayed per page
    int _selectedViewIndex = 0; // Index of the currently selected item
    int _scrollIndex = 0; // Current scroll index
    int _itemsCount = 0; // Total number of items in the browser
    vector<std::filesystem::directory_entry> _items; // List of items in the browser
};