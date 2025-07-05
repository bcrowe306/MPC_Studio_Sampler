#include "widget.h"
#include <cairo.h>
#include <string>
#include <algorithm>
#include <iostream>
#include "util.h"
using std::string;

class KnobWidget : public Widget {
public:
  
  KnobWidget(unsigned int x, unsigned int y, unsigned int size,
             float value, float min, float max, bool is_centered = false, float thickness = 5)
      : Widget(x, y, size, size), 
        _thickness(thickness), _is_centered(is_centered) {
    this->_value = value;
    this->_min = min;
    this->_max = max;
    render();
  }
  double angleToRad(double angle) {
    return angle * (M_PI / 180.0);
  }

  void setCentered(bool is_centered) {
    _is_centered = is_centered;
  }

  void setSettings(float value, float min, float max, bool is_centered) {
    _value = value;
    _min = min;
    _max = max;
    _is_centered = is_centered;
    _unitValue = mapFloat(_value, _min, _max, 0.0f, 1.0f);
    render();
  }

  void drawBasicKnob(Vector offset) {
    auto new_position = position + offset;
    auto center_x = new_position.x + width / 2;
    auto center_y = new_position.y + height / 2;
    auto radius = (width - _thickness) / 2 - 2; // Adjust radius to fit within the widget
    std::cout << "Radius: " << radius << std::endl;
    double sangle = angleToRad(90 + ((360.0 - angleRange) / 2.0));
    double eangle = angleToRad(startAngle + angleRange * _unitValue + 2);

    cairo_set_line_width(cr, _thickness);
    cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    cairo_arc(cr, center_x, center_y, radius, sangle, eangle);
    cairo_stroke(cr);
  }

  void drawCenterKnob(Vector offset) {
    auto new_position = position + offset;
    auto center_x = new_position.x + width / 2;
    auto center_y = new_position.y + height / 2;
    auto radius = (width - _thickness) / 2 - 2; // Adjust radius to fit within the widget
    std::cout << "Radius: " << radius << std::endl;
    double sangle = angleToRad(90 + ((360.0 - angleRange) / 2.0));
    double eangle = angleToRad(startAngle + (angleRange * _value + 2) / 2);

    cairo_set_line_width(cr, _thickness);
    cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    // Draw positive arc
    if (_unitValue > .5){
      cairo_arc(
          cr, center_x, center_y, radius, angleToRad(270),
          angleToRad(270 + (angleRange * _unitValue + 2) - (float)angleRange / 2));
      cairo_stroke(cr);
    }
    else if(_unitValue == .5){

    }
    else {
      cairo_arc_negative(
          cr, center_x, center_y, radius, angleToRad(270),
          angleToRad(270 + (angleRange * _unitValue + 2) - (float)angleRange / 2));
      cairo_stroke(cr);
    }

    // Draw the outline arc
    cairo_set_line_width(cr, 1);
    cairo_arc(cr, center_x, center_y, radius + _thickness/2, sangle, angleToRad( angleRange + startAngle + 2));
    cairo_stroke(cr);
  }
  void draw(Vector offset) override {
    if( !_is_centered) {
      drawBasicKnob(offset);
    } else {
      drawCenterKnob(offset);
    }
  }

  void setValue(float value) {
    if(value != _value) {
      // Clamp value to the range [0.0, 1.0]
      _value = std::clamp(value, 0.0f, 1.0f);
      _unitValue = mapFloat(_value, _min, _max, 0.0f, 1.0f);
      render();
    }
  }


protected:
  float _min = 0.0f; // Minimum value of the knob
  float _max = 1.0f; // Maximum value of the knob
  int angleRange = 300; // Range of the knob in degrees
  int startAngle = 120; // Starting angle in degrees
  float _thickness; // Thickness of the knob arc
  float _value = 0.5;
  bool _is_centered = true; // Whether the knob is centered
  float _unitValue = 0.5; // Value in the range [0.0, 1.0]
};