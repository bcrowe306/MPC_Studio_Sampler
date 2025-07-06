#pragma once
#include "cairo.h"
#include <string>
#include "util.h"

using std::string;
inline static void cairo_set_color(cairo_t *cr, bool color = true) {
    if (color) {
        cairo_set_source_rgb(cr, 1, 1, 1); // Set color to black
    }
    else {
        cairo_set_source_rgb(cr, 0, 0, 0); // Set color to white
    }
}

inline static void cairo_draw_text(cairo_t *cr, const string &text, double x, double y, double font_size, bool color = true) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_size);
    cairo_move_to(cr, x, y);
    cairo_set_color(cr, color);
    cairo_show_text(cr, text.c_str());
}

inline static void cairo_draw_aligned_text(cairo_t *cr, const string &text, double x, double y, double width, string alignment, double font_size = 10, bool color = true) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_size);
    auto newX = x;
    auto newY = y + font_size; // Adjust Y position to baseline
    if(alignment == "center") {
        cairo_text_extents_t extents;
        cairo_text_extents(cr, text.c_str(), &extents);
        newX = x + (width - extents.width) / 2; // Center the text
    } else if(alignment == "right") {
        cairo_text_extents_t extents;
        cairo_text_extents(cr, text.c_str(), &extents);
        newX = x + width - extents.width; // Right align the text
    } else {
        newX = x; // Left align the text
    }
    cairo_draw_text(cr, text, newX, newY, font_size, color);
}

inline static void cairo_draw_line(cairo_t *cr, double x1, double y1, double x2, double y2, double line_width = 1.0, bool color = true) {
    cairo_set_line_width(cr, line_width);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_set_color(cr, color);
    cairo_stroke(cr);
}

inline static void cairo_draw_horizontal_line(cairo_t *cr, double y, double x1, double x2, double line_width = 1.0, bool color = true) {
    cairo_set_line_width(cr, line_width);
    cairo_move_to(cr, x1, y);
    cairo_line_to(cr, x2, y);
    cairo_set_color(cr, color);
    cairo_stroke(cr);
}

inline static void cairo_draw_vertical_line(cairo_t *cr, double x, double y1, double y2, double line_width = 1.0, bool color = true) {
    cairo_set_line_width(cr, line_width);
    cairo_move_to(cr, x, y1);
    cairo_line_to(cr, x, y2);
    cairo_set_color(cr, color);
    cairo_stroke(cr);
}

inline static void cairo_draw_rectangle(cairo_t *cr, double x, double y, double width, double height, bool fill = false, bool color = true) {
    cairo_move_to(cr, x, y);
    cairo_set_color(cr, color);
    cairo_rectangle(cr, x, y, width, height);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

inline static void cairo_draw_circle(cairo_t *cr, double x, double y, double radius, bool fill = false, bool color = true) {
    cairo_set_color(cr, color);
    cairo_arc(cr, x, y, radius, 0.0, 2 * M_PI);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

inline static void cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y, double width, double height, double radius, double thickness=1, bool fill = false, bool color = true) {
    cairo_set_color(cr, color);
    cairo_set_line_width(cr, thickness);
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

    // Calculate arc start position
    double start_x = center_x + radius * std::cos(sangle);
    double start_y = center_y + radius * std::sin(sangle);
    cairo_move_to(cr, start_x, start_y);
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
    // Calculate arc start position
    double start_x = center_x + radius * std::cos(angleToRad(270));
    double start_y = center_y + radius * std::sin(angleToRad(270));

    cairo_move_to(cr, start_x, start_y);
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
    cairo_set_color(cr, color);
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

#include <cmath> // for M_PI

inline static void roundrect(cairo_t *cr, double x, double y, double width, double height, double r) {
    cairo_arc(cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_arc(cr, x + width - r, y + r, r, 3 * M_PI / 2, 0);
    cairo_arc(cr, x + width - r, y + height - r, r, 0, M_PI / 2);
    cairo_arc(cr, x + r, y + height - r, r, M_PI / 2, M_PI);
    cairo_close_path(cr);
}
