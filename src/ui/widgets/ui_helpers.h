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
    auto newY = y + font_size-1; // Adjust Y position to baseline
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


inline static void cairo_draw_triangle(cairo_t *cr, double x, double y, double width, double height, double rotation, bool fill = false, bool color = true) {
    cairo_set_color(cr, color);
    cairo_move_to(cr, x + width / 2, y);
    cairo_line_to(cr, x + width, y + height);
    cairo_line_to(cr, x, y + height);
    cairo_close_path(cr);
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

inline static void cairo_draw_folder_icon(cairo_t *cr, double x, double y, double width, double height, bool color = true) {
cairo_set_color(cr, color);

// Draw folder base
double tab_height = height * 0.28;
double tab_width = width * 0.38;
double body_y = y + tab_height;
double body_height = height - tab_height;

// Folder tab
cairo_move_to(cr, x, y + tab_height);
cairo_line_to(cr, x + tab_width, y + tab_height);
cairo_line_to(cr, x + tab_width + width * 0.12, y);
cairo_line_to(cr, x + width * 0.7, y);
cairo_line_to(cr, x + width, y + tab_height);
cairo_line_to(cr, x, y + tab_height);
cairo_close_path(cr);
cairo_fill(cr);

// Folder body
cairo_move_to(cr, x, body_y);
cairo_line_to(cr, x + width, body_y);
cairo_line_to(cr, x + width, y + height);
cairo_line_to(cr, x, y + height);
cairo_close_path(cr);
cairo_fill(cr);
}

inline static void cairo_draw_wav_icon(cairo_t *cr, double x, double y, double width, double height, bool color = true) {
    // No file rectangle, no "WAV" text, just waveform

    // Draw waveform (simple sine wave)
    cairo_set_color(cr, color);
    double margin = width * 0.13;
    double wave_x = x + margin;
    double wave_y = y + height * 0.60;
    double wave_w = width - 2 * margin;
    double wave_h = height * 0.18;
    int points = 18;
    cairo_move_to(cr, wave_x, wave_y);
    for (int i = 0; i <= points; ++i) {
        double t = (double)i / points;
        double wx = wave_x + t * wave_w;
        double wy = wave_y - std::sin(t * 2 * M_PI) * wave_h;
        cairo_line_to(cr, wx, wy);
    }
    cairo_stroke(cr);
}

inline static void cairo_draw_vertical_scrollbar(cairo_t *cr, double x, double y, double width, double height, int itemsCount, int pageSize, int offsetIndex, bool color = true) {
    cairo_set_color(cr, color);
    double rectHeight = static_cast<double>(height) * pageSize / itemsCount;
    double rectY = static_cast<double>(offsetIndex) * height / itemsCount;
    cairo_rectangle(cr, x, y + rectY, width - 2, rectHeight);
    cairo_fill(cr);
    cairo_rectangle(cr, x + width - 2, y, 1, height);
    cairo_stroke(cr);
}

inline static void cairo_draw_metronome_icon(cairo_t *cr, double x, double y, double width, double height, bool selected = false, bool color = true) {
    cairo_set_color(cr, color);

    // Draw metronome body
    if(selected){
        cairo_rectangle(cr, x, y, width, height);
        cairo_fill(cr);
    }
    else {
        cairo_rectangle(cr, x, y, width-1, height);
        cairo_stroke(cr);
    }
    

    // draw two inner circles evenly spaced inside the rectangle
    double innerRadius = height * 0.20;
    double innerY = y + height * 0.5;
    double innerX1 = x + width * 0.47;
    double innerX2 = x + width * 0.71;
    cairo_set_color(cr, !selected); // Invert color for inner circles
    cairo_arc(cr, innerX1, innerY, innerRadius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_arc(cr, innerX2, innerY, innerRadius, 0, 2 * M_PI);
    cairo_fill(cr);
}


inline static void cairo_draw_horizontal_progress_bar(cairo_t *cr, double x, double y, double width, double height, float progress, bool color = true) {
    cairo_set_color(cr, color);
    cairo_rectangle(cr, x, y, width, height);
    cairo_stroke(cr);

    // Draw filled part based on progress
    progress = std::clamp(progress, 0.0f, 1.0f); // Ensure progress is between 0 and 1
    cairo_rectangle(cr, x, y, width * progress, height);
    cairo_fill(cr);
}

inline static void cairo_draw_horizontal_dotted_line(cairo_t *cr, double x1, double x2, double y, bool color = true) {
    double dash_length = 2;
    double gap_length = 6.0;
    cairo_set_color(cr, color);
    cairo_set_dash(cr, &dash_length, 1, gap_length);
    cairo_move_to(cr, x1, y);
    cairo_line_to(cr, x2, y);
    cairo_stroke(cr);
    cairo_set_dash(cr, nullptr, 0, 0); // Reset dash pattern
}

inline static void cairo_draw_vertical_dotted_line(cairo_t *cr, double x, double y1, double y2, bool color = true) {
    double dash_length = 2;
    double gap_length = 6.0;
    cairo_set_color(cr, color);
    cairo_set_dash(cr, &dash_length, 1, gap_length);
    cairo_move_to(cr, x, y1);
    cairo_line_to(cr, x, y2);
    cairo_stroke(cr);
    cairo_set_dash(cr, nullptr, 0, 0); // Reset dash pattern
}