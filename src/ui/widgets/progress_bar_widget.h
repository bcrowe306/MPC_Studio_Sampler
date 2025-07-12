#pragma once
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <algorithm>
using std::string;

class ProgressBarWidget : public Widget {
public:
  ProgressBarWidget(unsigned int x, unsigned int y, unsigned int width,  unsigned int height) : Widget(x, y, width, height) {
    render();
  }

  void setProgress(float progress) {
    if (progress != _progress) {
      
      _progress = std::clamp(progress, 0.0f, 1.0f); // Ensure progress is between 0 and 1
      
      // Only render if the viewable width has changed
      int width = static_cast<int>(width * _progress); 
      if (width != _progressWidth) {
        _progressWidth = width; 
        render(); 
      }
    }
  }

  void draw(Vector offset) override {

    cairo_draw_horizontal_progress_bar(cr, 0, 0, width-1, height, _progress);
  }


protected:
  float _progress = 0.0f; // Progress value between 0.0 and 1.0
  int _progressWidth = 0; // Width of the progress bar
};