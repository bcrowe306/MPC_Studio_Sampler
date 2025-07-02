#include "Widget.h"
#include <string>
using std::string;

class TextWidget : public Widget {
public:
  TextWidget(unsigned int x, unsigned int y, unsigned int width,
             unsigned int height, string text, unsigned int font_size = 16)
      : Widget(x, y, width, height) {
    this->_text = text;
    this->_font_size = font_size;
    render();
  }

  void draw(Vector offset) override {

    auto new_position = position + offset;
    cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, _font_size);
    cairo_move_to(cr, new_position.x, new_position.y + _font_size);
    cairo_show_text(cr, _text.c_str()); // Draw text on the surface
  }

  void set_text(const string &text) {
    _text = text;
    render();
  }

  void set_font_size(unsigned int size) {
    _font_size = size;
    render();
  }

protected:
  unsigned int _font_size = 16;
  string _text = "Hello MPC Studio!";
};