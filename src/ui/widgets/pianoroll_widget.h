#pragma once
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <string>
#include <vector>
#include <iostream>

using std::vector;
using std::string;
struct NoteRange {
    int minPitch;
    int maxPitch;
    int range;
    int rangePlusOne() const {
        return range + 1; // Include both ends
    }
};

struct MidiNote {
    int id; // Unique identifier for the note
    int pitch;
    int velocity;
    int startTime;
    int duration;
};
class PianoRollWidget : public Widget {
public:
    vector<MidiNote> notes;
    int length = 480 * 8;
    int timeSigNumerator = 4;
    PianoRollWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
        : Widget(x, y, width, height) {
        // Initialize the piano roll widget

        for (int i = 0; i < 10; ++i) {
            MidiNote note;
            note.id = i; // Assign a unique ID
            note.pitch = i + 60;
            note.velocity = 100; // Default velocity
            note.startTime = i * 480; // Default start time
            note.duration = 300; // Default duration in ticks
            notes.push_back(note);
        }
    }

    NoteRange getNotesPitchRange() const {
        if (notes.empty()) return {0, 0, 0};
        int minPitch = notes[0].pitch;
        int maxPitch = notes[0].pitch;
        for (const auto &note : notes) {
            if (note.pitch < minPitch) minPitch = note.pitch;
            if (note.pitch > maxPitch) maxPitch = note.pitch;
        }
        return {minPitch, maxPitch, maxPitch - minPitch };
    }

    void setSelectedLine(int line) {
        if(line != _selectedLine) {

            // Check if the line is within the pitch range of the notes
            NoteRange pitchRange = getNotesPitchRange();
            if(line >= pitchRange.minPitch && line <= pitchRange.maxPitch) {
                _selectedLine = line;
                render();
            }
        }
        
    }
    void incrementOffset() {
        setViewOffset(_viewOffset + 1);
    }
    void decrementOffset() {
        setViewOffset(_viewOffset - 1);
    }

    void setViewOffset(int offset) {
        if(offset != _viewOffset) {

            int rows = generateRows();
            NoteRange pitchRange = getNotesPitchRange();
            // Ensure the offset does not exceed the pitch range
            if(offset >= 0 && offset < pitchRange.range) {
                _viewOffset = offset;
                render();
            }
            else {
                std::cerr << "View offset out of range: " << offset << std::endl;
            }
        }
    }


    void drawNote(int x, int y, int width, int height, bool muted = false, bool selected = false) {
        if(muted) {
            cairo_draw_rectangle(cr, x, y, width, height, false, true);
        }
        else {
            cairo_draw_rectangle(cr, x, y, width, height, true, true);
        }

        if(selected){
            int padding = 2;
            cairo_draw_rectangle(cr, x-padding, y-padding, width+ padding*2, height+padding*2, false, true);
        }
    }

    int generateRowHeight() const {
        NoteRange pitchRange = getNotesPitchRange();
        int rowHeight = (height - _headerHeight) / pitchRange.rangePlusOne() > _minRowHeight ? (height - _headerHeight) / pitchRange.rangePlusOne() : _minRowHeight;
        return rowHeight;
    }

    int generateRows() const {
        int rowHeight = generateRowHeight();
        int rows = (height - _headerHeight) / rowHeight;
        return rows;
    }

    void draw(Vector offset) override {

        double gridWidth = width - _gridStart;
        NoteRange pitchRange = getNotesPitchRange();
        int rowHeight = generateRowHeight();
        int rows = generateRows();

        cairo_draw_horizontal_line(cr, _headerHeight, 0, width);
        cairo_draw_vertical_line(cr, 0, 0, height);
        cairo_draw_vertical_line(cr, _gridStart, 0, height);
        cairo_draw_vertical_line(cr, width-1, 0, height);
        cairo_draw_horizontal_line(cr, height-1, 0, width);

        // Draw pitch lines
        for(int i = 1; i < rows; ++i) {
          double y = _headerHeight + i * rowHeight;
          cairo_draw_horizontal_line(cr, y, 0, _gridStart);
          cairo_draw_horizontal_dotted_line(cr, _gridStart, width, y);
        }

        // Draw bars
        int barsInLength = (int)(length / (timeSigNumerator * 480));
        for (int i = 0; i < barsInLength; i++) {
            double x = _gridStart + (i * gridWidth / barsInLength);
            std::cout << "barsInLength: " << barsInLength << ", i: " << i << ", x: " << x << std::endl;
            if (i != 0) {
                cairo_draw_vertical_line(cr, x, _headerHeight, height);
            }
        }
        // Draw beats in length
        int beatsInLength = (int)(length / 480);
        for(int i = 0; i < beatsInLength; i++) {
          double x = _gridStart + (i * gridWidth / beatsInLength);
          std::cout << "beatsInLength: " << beatsInLength << ", i: " << i << ", x: " << x << std::endl;
          if (i != 0) {
            cairo_draw_vertical_dotted_line(cr, x, _headerHeight, height);
          }
        }

        // Draw notes
        // for (const auto &note : notes) {
        //     // draw from bottom to top
        //     int yIndex = pitchRange.maxPitch - note.pitch;
        //     int noteHeight = rowHeight - 4; // Leave some padding
        //     double y = headerHeight + yIndex * rowHeight + 2; // +1 for padding
        //     double xStart = _gridStart + (note.startTime / (double)length) * (width - _gridStart) + 1;
        //     double xEnd = _gridStart + ((note.startTime + note.duration) / (double)length) * (width - _gridStart);
        //     // if (xEnd < _gridStart) continue; // Skip if note ends before the grid
        //     // if (xStart > width) continue; // Skip if note starts after the grid
        //     int width = xEnd - xStart;
        //     drawNote( xStart, y, width, noteHeight, false, _selectedNoteId == note.id);

        //     if (note.pitch == _selectedLine) {
        //         cairo_draw_rectangle(cr, 2, y+1, _gridStart-4, noteHeight - 2, true);
        //     }
        // }

        int firstNotePitch = _viewOffset + pitchRange.minPitch;
        int lastNotePitch = firstNotePitch + rows - 1;
        for(int i = 0; i < rows; ++i) {
            auto note = notes[i % notes.size()]; // Cycle through notes for demonstration
            if (note.pitch < firstNotePitch || note.pitch > lastNotePitch) continue;
            int yIndex = (lastNotePitch - note.pitch ); // Invert the index for drawing from bottom to top
            int noteHeight = rowHeight - 4; // Leave some padding
            double y = _headerHeight + yIndex * rowHeight + 2; // +1 for padding
            double x = 2; // Fixed x position for the label
            double xStart = _gridStart + (note.startTime / (double)length) * (width - _gridStart) + 1;
            double xEnd = _gridStart + ((note.startTime + note.duration) / (double)length) * (width - _gridStart);
            int width = xEnd - xStart;
            drawNote( xStart, y, width, noteHeight, false, _selectedNoteId == note.id);

            if (note.pitch == _selectedLine) {
                cairo_draw_rectangle(cr, 2, y+1, _gridStart-4, noteHeight - 2, true);
            }
        }
    }
protected:
    int _viewOffset = 0;
    int _selectedNoteId = 2;
    int _selectedLine = 60; // Midi note number for the selected line
    double _gridStart = 8.0; // Starting point of the grid
    double _headerHeight = 12.0; // Height of the header section
    int _minRowHeight = 8;
    cairo_t *_cr; // Cairo context for drawing
};