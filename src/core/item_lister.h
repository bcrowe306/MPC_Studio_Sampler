#include "sigslot/signal.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

struct ViewRange {
    int start; // Start index of the view range
    int pageSize;   // Size of the view range
    int count;

    ViewRange(int s, int e, int c) : start(s), pageSize(e - s), count(c) {
        // Ensure the range is valid
        if (start < 0 || pageSize < 0) {
            throw std::out_of_range("Invalid view range");
        }
    }
    bool isInRange(int index) const {
        return index >= start && index < start + pageSize;
    }
    bool isGreaterThanRange(int index) const {
        return index >= start + pageSize;
    }
    bool isLessThanRange(int index) const {
        return index < start;
    }
    void setRange(int s, int e) {
        start = std::clamp(s, 0, count);
        pageSize = std::clamp(e - s, 0, count - start);
    }
};

class ItemLister {
public:
    
    sigslot::signal<int> onScroll; // Signal emitted when scrolling occurs

    ItemLister(int initialPageSize = 5, int initialOffsetIndex = 0) : _pageSize(initialPageSize), _scrollIndex(initialOffsetIndex), _viewRange{0, initialPageSize, 0}
    {
        // Initialize the browser with default values
    }

    void setItemCount(int count) {
        _viewRange.count = count;
        isInitialized = true; // Mark the ItemLister as initialized
    }

    void setPageSize(int pageSize) {
        if (pageSize <= 0) {
            throw std::invalid_argument("Page size must be greater than zero");
        }
        _pageSize = pageSize;
        _viewRange.pageSize = pageSize; // Update view range size
        _viewRange.start = std::clamp(_scrollIndex, 0, static_cast<int>(_viewRange.count) - _pageSize);
    }
    int scroll(int offset){
        // Offset is the number of items to scroll, can be positive or negative
        scrollToIndex(_scrollIndex + offset);
        return _scrollIndex;
    }

    void scrollToIndex(int index) {
        int newScrollIndex = std::clamp(index, 0,
                                        static_cast<int>(_viewRange.count) - 1);
        if (newScrollIndex == _scrollIndex) {
            return; // No change in scroll index
        }
        _scrollIndex = newScrollIndex; // Update scroll index

        if (_viewRange.isGreaterThanRange(_scrollIndex)) {
            _viewRange.setRange(_scrollIndex - _pageSize + 1, _scrollIndex + 1);

        } else if (_viewRange.isLessThanRange(_scrollIndex)) {
            _viewRange.setRange(_scrollIndex, _scrollIndex + _pageSize);
        }
        onScroll(_scrollIndex); // Emit scroll signal with new offset
    }

    void page(int offset) {
        // Offset is the number of pages to scroll, can be positive or negative
        if (offset > 0){
            scroll(_pageSize);
        }
        else if (offset < 0) {
            scroll(-_pageSize);
        }
    }

    int getItemsCount() const {
        return _viewRange.count; // Return total number of items
    }

    int getPageSize() const {
        return _pageSize; // Return current page size
    }

    int getScrollIndex() const {
        return _scrollIndex; // Return current scroll index
    }

    ViewRange & getViewRange() {
        return _viewRange;
    }

protected:
    // Method to update the browser view based on current page size and offset index
    ViewRange _viewRange;
    int _pageSize; // Number of items displayed per page
    int _scrollIndex; // Current offset index for pagination
    bool isInitialized = false; // Flag to check if the browser is initialized
};