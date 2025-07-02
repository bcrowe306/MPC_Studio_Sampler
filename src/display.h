#include "cairo.h"


inline void drawText(cairo_t *cr, const char *text, double x, double y, double fontSize) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 1, 1, 1); // Set text color to white
    cairo_set_font_size(cr, fontSize);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
}