#pragma once
#include "util.h"
#include "widget.h"
#include <algorithm>
#include <cairo.h>
#include <iostream>
#include <string>


class RectangleWidget : public Widget {
    std::string color;

public:
    RectangleWidget(float x, float y, float width, float height)
        : Widget(x,y,width,height) {}

    void setPosition(float x, float y) {
        this->position.x = x;
        this->position.y = y;
        render();
    }

    void draw(Vector offset) override {
        auto new_position = position + offset;
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_rectangle(cr, new_position.x, new_position.y, width, height);
        cairo_paint(cr);
    }
};