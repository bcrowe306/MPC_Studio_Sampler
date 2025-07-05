#include "widget.h"
#include <iostream>

Widget::Widget(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    : position(x, y) {
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create(surface);
    this->width = width;
    this->height = height;
}
Widget::Widget(Vector position, unsigned int width, unsigned int height)
    : position(position) {
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    this->width = width;
    this->height = height;
    cr = cairo_create(surface);
    
}
Widget::Widget(unsigned int width, unsigned int height)
    : Widget(0, 0, width, height) {
        this->width = width;
        this->height = height;
        position = Vector(0, 0); // Initialize position
        surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
        cr = cairo_create(surface);
    }

Widget::Widget(Vector position, Vector size)
    : Widget(position.x, position.y, size.x, size.y) {
    this->width = size.x;
    this->height = size.y;
    position = position; // Initialize position
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create(surface);
    }

Widget::Widget() : Widget(0, 0, 360, 96) {
    // Default constructor initializes a widget with default size
    this->width = 360;
    this->height = 96;
    position = Vector(0, 0);
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create(surface);
} // Default constructor with default size

void Widget::clear() {
    if (cr) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set background color to black
        cairo_paint(cr); // Fill the surface with the background color
    }
}

void Widget::draw(Vector offset) {
    // This method should be overridden by derived classes to implement specific drawing logic
    // For now, it does nothing
}

void Widget::render(Vector offset) {
    // draw self first
    if(_active){
        
        if (cr) {
            clear();
            draw(offset); // Call the derived class's draw method

            // Encode and send the surface if the callback is set
            if(encode_surface_callback) {
                encode_surface_callback(surface, position.x + offset.x, position.y + offset.y);
            }
        }
    
        // draw children
        for (const auto &child : children) {
            if (child) {
                child->render(position + offset);
            }
        }
    }
    
}

void Widget::initialize(EncodeSurfaceCallback encode_surface_callback) {
    this->encode_surface_callback = encode_surface_callback;
    for (const auto &child : children) {
        if (child) {
            child->initialize(encode_surface_callback);
        }
    }
    render(); // Initial render to set up the surface
}

void Widget::activate() {
    _active = true;
    for (const auto &child : children) {
        if (child) {
            child->activate();
        }
    }
}

void Widget::deactivate() {
    _active = false;
    for (const auto &child : children) {
        if (child) {
            child->deactivate();
        }
    }
}

void Widget::add_child(shared_ptr<Widget> child) {
    if (child) {
        children.push_back(child);
    }
}

void Widget::encode_surface(Vector offset) {
    if (surface) {
        unsigned char *data = cairo_image_surface_get_data(surface);
        int width = cairo_image_surface_get_width(surface);
        int height = cairo_image_surface_get_height(surface);
        int stride = cairo_image_surface_get_stride(surface);

        // Process the surface data as needed, e.g., send it to the display
        // This is a placeholder for actual encoding logic
        // For example, you could convert the RGB data to a specific format
    }
}