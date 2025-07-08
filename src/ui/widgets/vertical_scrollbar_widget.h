#pragma once
#include "widget.h"
#include "widgets/ui_helpers.h"

class VerticalScrollbarWidget : public Widget {
public:
    int itemsCount = 0;
    int pageSize = 0;
    int offsetIndex = 0;

    VerticalScrollbarWidget(int x, int y, int width, int height, int itemsCount = 31, int pageSize = 5, int offsetIndex = 0)
        : Widget(x, y, width, height), itemsCount(itemsCount), pageSize(pageSize), offsetIndex(offsetIndex) 
        {

        }

    void scrollUp() {
        if (offsetIndex > 0) {
            --offsetIndex;
        }
    }

    void scrollDown() {
        if (offsetIndex < std::max(0, itemsCount - pageSize)) {
            ++offsetIndex;
        }
    }

    void pageUp() {
        offsetIndex = std::max(0, offsetIndex - pageSize);
    }

    void pageDown() {
        offsetIndex = std::min(std::max(0, itemsCount - pageSize), offsetIndex + pageSize);
    }
    void setItemsCount(int newItemsCount) {
        if (itemsCount != newItemsCount) {
            render();
        }
    }

    void setPageSize(int newPageSize) {
        if (pageSize != newPageSize) {
            render();
        }
    }

    void setOffsetIndex(int newOffsetIndex) {
        int maxOffset = std::max(0, itemsCount - pageSize);
        int clampedOffset = std::clamp(newOffsetIndex, 0, maxOffset);
        if (offsetIndex != clampedOffset) {
            offsetIndex = clampedOffset;
            render();
        }
    }
    
    // Optionally, override Widget's draw/render method to display the scrollbar
    void draw(Vector offset) override {
        // Draw the vertical bars
        double rectHeight = static_cast<double>(height) * pageSize / itemsCount;
        double rectY = static_cast<double>(offsetIndex) * height / itemsCount;
        cairo_draw_vertical_line(cr, 0, 0, height - 1);
        cairo_draw_rectangle(cr, 0, rectY, width - 2, rectHeight, true, true);
    }
};