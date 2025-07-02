#pragma once
#include "cairo.h"
#include <vector>
#include <memory>
#include <functional>

using std::function;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::vector;

// typedef for a function that encodes a cairo surface

typedef function<void(cairo_surface_t *, unsigned int, unsigned int)> EncodeSurfaceCallback;

struct Vector {
    unsigned int x, y;
    Vector(unsigned int x, unsigned int y) : x(x), y(y) {}
    Vector() : x(0), y(0) {}
    Vector(const Vector &other) : x(other.x), y(other.y) {}
    Vector &operator=(const Vector &other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }
    Vector operator+(const Vector &other) const {
        return Vector(x + other.x, y + other.y);
    }
    Vector operator-(const Vector &other) const {
        return Vector(x - other.x, y - other.y);
    }
};

class Widget {
    
public:
    
    cairo_surface_t *surface = nullptr;
    cairo_t *cr = nullptr;
    // Function to encode the surface into a format suitable for transmission
    EncodeSurfaceCallback encode_surface_callback;

    // Constructors
    Widget(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    Widget(Vector position, unsigned int width, unsigned int height);
    Widget(Vector position, Vector size);
    Widget(unsigned int width, unsigned int height);
    Widget(); // Default constructor with default size

    virtual ~Widget() {
        if (cr) {
            cairo_destroy(cr);
        }
        if (surface) {
            cairo_surface_destroy(surface);
        }
    }

    virtual void draw(Vector offset);

    void initialize(EncodeSurfaceCallback encode_surface_callback); 
    void activate();
    void deactivate();
    void clear();
    void render(Vector offset = Vector(0, 0));
    int get_x() const { return position.x; }
    int get_y() const { return position.y; }
    Vector get_position() const { return position; }
    int get_width() const { return cairo_image_surface_get_width(surface); }
    int get_height() const { return cairo_image_surface_get_height(surface); }
    void add_child(shared_ptr<Widget> child);

    

protected:
    
    Vector position;
    bool _active = true;
    bool _enabled = true;
    vector<shared_ptr<Widget>> children;
private:
    void encode_surface(Vector offset);
};