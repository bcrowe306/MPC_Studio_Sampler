#pragma once
#include "cairo.h"
#include <string>
#include "util.h"

using std::string;
inline static void cairo_draw_text(cairo_t *cr, const string &text, double x, double y, double font_size, bool color = true) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_size);
    cairo_move_to(cr, x, y);
    if (color) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set text color to black
    }
    else {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    }
    cairo_show_text(cr, text.c_str());
}

inline static void cairo_draw_line(cairo_t *cr, double x1, double y1, double x2, double y2, double line_width = 1.0, bool color = true) {
    cairo_set_line_width(cr, line_width);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    if (color) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set line color to black
    }
    else {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set line color to white
    }
    cairo_stroke(cr);
}

inline static void cairo_draw_rectangle(cairo_t *cr, double x, double y, double width, double height, bool fill = false, bool color = true) {
    if (color) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set rectangle color to black
    }
    else {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set rectangle color to white
    }
    cairo_rectangle(cr, x, y, width, height);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

inline static void cairo_draw_circle(cairo_t *cr, double x, double y, double radius, bool fill = false, bool color = true) {
    if (color) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set circle color to black
    }
    else {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set circle color to white
    }
    cairo_arc(cr, x, y, radius, 0.0, 2 * M_PI);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

inline static void cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y, double width, double height, double radius, bool fill = false, bool color = true) {
    if (color) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set rectangle color to black
    }
    else {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set rectangle color to white
    }
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - radius, y + radius, radius, -M_PI / 2.0, 0.0);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0.0, M_PI / 2.0);
    cairo_arc(cr, x + radius, y + height - radius, radius, M_PI / 2.0, M_PI);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, -M_PI / 2.0);
    cairo_close_path(cr);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}



inline static void cairo_draw_basic_knob(double x, double y, int size, float value, float min, float max, cairo_t *cr, float thickness = 5, int angleRange = 300, int startAngle = 120) {
    auto _unitValue = mapFloat(value, min, max, 0.0f, 1.0f);
    auto center_x = x + (float)size / 2;
    auto center_y = y + (float)size / 2;
    auto radius = (size - thickness) / 2 - 2; // Adjust radius to fit within the widget
    double sangle = angleToRad(90 + ((360.0 - angleRange) / 2.0));
    double eangle = angleToRad(startAngle + angleRange * _unitValue + 2);

    cairo_set_line_width(cr, thickness);
    cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    cairo_arc(cr, center_x, center_y, radius, sangle, eangle);
    cairo_stroke(cr);
  }

  inline static void cairo_draw_center_knob(double x, double y, int size, float value, float min, float max, cairo_t *cr, float thickness = 5, int angleRange = 300, int startAngle = 120) {
    auto _unitValue = mapFloat(value, min, max, 0.0f, 1.0f);
    auto center_x = x + (float)size / 2;
    auto center_y = y + (float)size / 2;
    auto radius =
        (size - thickness) / 2 - 2; // Adjust radius to fit within the widget
    double sangle = angleToRad(90 + ((360.0 - angleRange) / 2.0));
    double eangle = angleToRad(startAngle + (angleRange * _unitValue + 2) / 2);

    cairo_set_line_width(cr, thickness);
    cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    // Draw positive arc
    if (_unitValue > .5) {
      cairo_arc(cr, center_x, center_y, radius, angleToRad(270),
                angleToRad(270 + (angleRange * _unitValue + 2) -
                           (float)angleRange / 2));
      cairo_stroke(cr);
    } else if (_unitValue == .5) {

    } else {
      cairo_arc_negative(cr, center_x, center_y, radius, angleToRad(270),
                         angleToRad(270 + (angleRange * _unitValue + 2) -
                                    (float)angleRange / 2));
      cairo_stroke(cr);
    }

    // Draw the outline arc
    cairo_set_line_width(cr, 1);
    cairo_arc(cr, center_x, center_y, radius + thickness / 2, sangle,
              angleToRad(angleRange + startAngle + 2));
    cairo_stroke(cr);
  }

inline static void cairo_draw_knob(cairo_t *cr, double x, double y, int size, float value, float min, float max, bool is_centered = false, float thickness = 5) {
   if( is_centered) {
       cairo_draw_center_knob(x, y, size, value, min, max, cr, thickness);
   } else {
       cairo_draw_basic_knob(x, y, size, value, min, max, cr, thickness);
   }
}


inline static void cairo_draw_top_rounded_rectangle(cairo_t *cr, double x, double y, double width, double height, double radius, bool fill = false, bool color = true) {
    if (color) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set rectangle color to black
    }
    else {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set rectangle color to white
    }
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - radius, y + radius, radius, -M_PI / 2.0, 0.0);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI / 2.0, M_PI);
    cairo_line_to(cr, x + width - radius, y + height);
    cairo_line_to(cr, x + radius, y + height);
    cairo_line_to(cr, x + width - radius, y + height);
    cairo_close_path(cr);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}   
