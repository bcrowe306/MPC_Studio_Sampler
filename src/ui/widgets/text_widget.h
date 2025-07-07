#pragma once
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <string>
using std::string;

class TextWidget : public Widget {
public:
  TextWidget(unsigned int x, unsigned int y, unsigned int width,
             unsigned int height, string text, unsigned int font_size = 16, std::string alignment = "left")
      : Widget(x, y, width, height) {
    this->_text = text;
    this->_font_size = font_size;
    this->_alignment = alignment;
    render();
  }

  void draw(Vector offset) override {

    auto new_position = position + offset;
    cairo_draw_aligned_text(cr, _text, 0, 0, width,  _alignment, _font_size, true);
  }

  void set_text(const string &text) {
    if (text == _text) return; // Avoid unnecessary updates
    _text = text;
    render();
  }

  void set_font_size(unsigned int size) {
    if (size == _font_size) return; // Avoid unnecessary updates
    _font_size = size;
    render();
  }

protected:
  unsigned int _font_size = 16;
  string _alignment = "left"; // "left", "center", "right"
  string _text = "Hello MPC Studio!";
};