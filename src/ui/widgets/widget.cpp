#include "widget.h"
#include <iostream>

Widget::Widget(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    : position(x, y) {
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create(surface);
    this->width = width;
    this->height = height;
}

Widget::Widget(Vector position, Vector size)
    : Widget(position.x, position.y, size.x, size.y) {
    this->width = size.x;
    this->height = size.y;
    position = position; // Initialize position
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create(surface);
    }

Widget::Widget() { // Default size
    position = Vector(0, 0); // Default position
    width = 360; // Default width
    height = 96; // Default height
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 360, 96);
    cr = cairo_create(surface);
}

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
}

void Widget::activate() {
    _active = true;
    for (const auto &child : children) {
        if (child) {
            child->activate();
        }
    }
    onActivated(); // Call the activation callback
}

void Widget::deactivate() {
    _active = false;
    for (const auto &child : children) {
        if (child) {
            child->deactivate();
        }
    }
    onDeactivated(); // Call the deactivation callback
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
        std::cout << "Encoding surface at offset (" << offset.x << ", " << offset.y << ") with size (" 
                  << width << "x" << height << ")" << std::endl;

        // Process the surface data as needed, e.g., send it to the display
        // This is a placeholder for actual encoding logic
        // For example, you could convert the RGB data to a specific format
    }
}