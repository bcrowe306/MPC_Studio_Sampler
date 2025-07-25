#pragma once
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/extended/AudioFileReader.h"
#include "property.h"
#include "sigslot/signal.hpp"
#include "core/item_lister.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include "core/devices/sampler_device.h"

using std::string;
using std::vector;
using std::filesystem::path;
using std::filesystem::directory_entry;

inline static std::string rootLocation = "/Users/brandoncrowe/Documents/Audio Samples";

struct BrowserHistoryItem {
    directory_entry entry; // Directory entry for the history item
    int scrollIndex; // Scroll index when this item was added to history
    BrowserHistoryItem(const directory_entry& entry, int scrollIndex) : entry(entry), scrollIndex(scrollIndex) {}
};

class Browser : public ItemLister {
public:
    Property<bool> loading = Property<bool>(false); // Property to indicate loading state
    Property<bool> autoPreview = Property<bool>(true); // Property to enable/disable auto-preview
    sigslot::signal<> onRefresh; // Signal emitted when the browser is refreshed or items are loaded
    shared_ptr<SamplerDevice> samplerDevice; // Sampler device for audio playback
    shared_ptr<lab::AudioBus> audioBus; // Audio bus for audio processing
    
    Browser(shared_ptr<AudioContext> audioContext, int initialPageSize = 5, int initialOffsetIndex = 0) : ItemLister(initialPageSize, initialOffsetIndex) 
    {
        _audioContext = audioContext; 
        samplerDevice = make_shared<SamplerDevice>(_audioContext);
        _audioContext->connect(audioContext->destinationNode(), samplerDevice->output, 0, 0); 
        _audioContext->synchronizeConnections();
        _visibleItems.reserve(_pageSize); 
        _items.reserve(100); 
        setPageSize(_pageSize);

        loadBrowserItems(rootLocation); // Load items from the default directory
        onScroll.connect([this](int newOffset) {
            if(autoPreview.get()) {
                previewItem(); // Automatically preview the selected item if auto-preview is enabled
            }
        });
        
    }

    void navigateToSelectedItem() {
        auto selectedItem = _items[_scrollIndex];
        if (selectedItem.is_directory()) {
            _navigationHistoryStack.push_back(BrowserHistoryItem(directory_entry(_currentDirectoryPath), _scrollIndex)); // Add the current directory to the navigation history
            loadBrowserItems(selectedItem.path().string()); // Load items from the selected directory
        }
    }
    void goBack() {
        if (_navigationHistoryStack.size() > 0) {
            auto previousDir = _navigationHistoryStack.back();
            loadBrowserItems(previousDir.entry.path().string(), previousDir.scrollIndex); // Load items from the previous directory
            _navigationHistoryStack.pop_back(); // Remove the last item from the history stack
        }
        else if (_navigationHistoryStack.size() == 0) {
            // If there is no history, load the root directory
            loadBrowserItems(rootLocation);
        } else {
            std::cerr << "No previous directory in history." << std::endl; // Handle case where no previous directory exists
        }
    }

    void setRootDirectory(const std::string& rootDir) {
        rootLocation = rootDir; // Set the new root directory
        _navigationHistoryStack.clear(); // Clear the navigation history stack
        loadBrowserItems(rootLocation, 0); // Load items from the new root directory
    }

    void stopPreview() {
        samplerDevice->stopSample();
    }

    void printNavigationHistory() const {
        std::cout << "Navigation History:" << std::endl;
        for (const auto& item : _navigationHistoryStack) {
            std::cout << " - " << item.entry.path().string() << " (Scroll Index: " << item.scrollIndex << ")" << std::endl;
        }
    }

    void previewItem(){
        auto selectedItem = _items[_scrollIndex];
        if (selectedItem.is_regular_file() && selectedItem.path().extension() == ".wav") {
            samplerDevice->loadAndPlaySample(selectedItem.path().string());
        }
    }

    void loadBrowserItems(const std::string& directoryPath, int newScrollIndex = -1) {
        
        // Check if the directory exists and is a valid directory
        std::filesystem::path dirPath(directoryPath);
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            return;
        }

        loading = true; // Set loading state to true
        _items.clear(); // Clear previous items
        _currentDirectoryName = dirPath.filename().string(); // Set the current directory name
        _currentDirectoryPath = dirPath.string(); // Set the current directory path

        vector<directory_entry> folders;
        vector<directory_entry> wavFiles;

        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_directory()) {
                folders.push_back(entry);
            } else if (entry.is_regular_file() && entry.path().extension() == ".wav") {
                wavFiles.push_back(entry);
            }
        }

        std::sort(folders.begin(), folders.end());
        std::sort(wavFiles.begin(), wavFiles.end());

        _items.insert(_items.end(), folders.begin(), folders.end());
        _items.insert(_items.end(), wavFiles.begin(), wavFiles.end());

        setItemCount((int)_items.size());
        setPageSize(_pageSize);
        _viewRange = {0, _pageSize, (int)_items.size()};
        

        // Scroll to the specified index if valid
        if(newScrollIndex >= 0 && newScrollIndex < (int)_items.size()) {
           scrollToIndex(newScrollIndex); 
        }else{
            scrollToIndex(0);
        }

        onRefresh();     // Emit the refresh signal
        loading = false; // Set loading state to false
    }

    vector<directory_entry>& getVisibleItems() {
        int start = _viewRange.start;
        int end = std::min(start + _viewRange.pageSize, (int)_items.size());
        _visibleItems.clear(); // Clear the previous visible items
        for (int i = start; i < end; ++i) {
            _visibleItems.push_back(_items[i]);
        }
        return _visibleItems;
    }

    std::filesystem::directory_entry getSelectedItem() const {
        if (_scrollIndex >= 0 && _scrollIndex < (int)_items.size()) {
            return _items[_scrollIndex]; // Return the currently selected item
        }
        return std::filesystem::directory_entry(); // Return an empty entry if no item is selected
    }

    string getCurrentDirectoryName() const {
        return _currentDirectoryName.empty() ? "Root" : _currentDirectoryName;
    }

private:
    shared_ptr<AudioContext> _audioContext; // Audio context for audio playback
    string _currentDirectoryPath; // Current directory path
    string _currentDirectoryName; // Current directory path
    vector<directory_entry> _items; // List of items in the browser
    vector<directory_entry> _visibleItems; // Currently visible items
    vector<BrowserHistoryItem> _navigationHistoryStack; // Selected items in the browser
};
