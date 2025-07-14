#pragma once
#include "widget.h"
#include "widgets/ui_helpers.h"
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "core/constants.h"
#include "core/sequencer/midi_clip.h"

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
    MidiClip *midiClip;
    int _sequenceStart = 0;
    
    int timeSigNumerator = 4;
    PianoRollWidget(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
        : Widget(x, y, width, height) {
        // Initialize the piano roll widget

    }

    NoteRange getNotesPitchRange() const {
        if (midiClip->events.empty()) return {0, 0, 0};
        int minPitch = midiClip->events[0].pitch;
        int maxPitch = midiClip->events[0].pitch;
        for (const auto &event : midiClip->events) {
            if (event.pitch < minPitch) minPitch = event.pitch;
            if (event.pitch > maxPitch) maxPitch = event.pitch;
        }
        return {minPitch, maxPitch, maxPitch - minPitch };
    }

    void setMidiClip(MidiClip *clip) {
        midiClip = clip;
        auto range = getNotesPitchRange();
        _baseNote = range.minPitch;
        _selectedLine = range.minPitch;
        render();
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

    void setSequenceEnd(int endInTicks) {
        if(endInTicks != _sequenceEnd) {
            _sequenceEnd = endInTicks;
            render();
        }
    }

    void incrementBaseNote() {
        setBaseNote(_baseNote + 1);
    }

    void decrementBaseNote() {
        setBaseNote(_baseNote - 1);
    }

    void setBaseNote(int offset) {
        if(offset != _baseNote) {
            _baseNote = offset;
            render();
        }
    }

    void scrollGridRight(){
        _startTick += kTPQN; // Scroll right by one beat
        if(_startTick + _gridLengthTicks > _sequenceEnd) {
            _startTick = _sequenceEnd - _gridLengthTicks; // Prevent scrolling past the end
        }
        render();
    }

    void scrollGridLeft(){
        _startTick -= kTPQN; // Scroll left by one beat
        if (_startTick < 0) {
            _startTick = 0; // Prevent negative start tick
        }
        render();
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

    void draw(Vector offset) override {

        double gridWidth = width - _gridStart;
        int gridHeight = height - _headerHeight - _hScrollBarHeight;
        NoteRange pitchRange = getNotesPitchRange();
        int rows = _rowCount;
        int rowHeight = gridHeight / rows;

        cairo_draw_horizontal_line(cr, _hScrollBarHeight, 0, width);
        cairo_draw_horizontal_line(cr, _hScrollBarHeight + _headerHeight, 0, width);

        cairo_draw_vertical_line(cr, 0, 0, height);
        cairo_draw_vertical_line(cr, _gridStart, 0, height);
        cairo_draw_vertical_line(cr, width-1, 0, height);
        cairo_draw_horizontal_line(cr, height-1, 0, width);

        // Draw scroll bar
        int scrollBarWidth = width - _gridStart;
        int scrollBarHandleWidth = scrollBarWidth * (_gridLengthTicks / (double)_sequenceEnd);
        int scrollBarX = _gridStart + (_startTick / (double)_sequenceEnd) * scrollBarWidth;
        cairo_draw_rectangle(cr, scrollBarX, 0, scrollBarHandleWidth, _hScrollBarHeight, true, true);


        



        // Draw pitch lines
        for(int i = 1; i < rows; ++i) {
          double y = _headerHeight + _hScrollBarHeight + i * rowHeight;
          cairo_draw_horizontal_line(cr, y, 0, _gridStart);

          int notePitch = _baseNote + (rows - i - 1); // Invert the pitch for drawing
          if(notePitch % 12 == 0) {
              cairo_draw_horizontal_line(cr, y, _gridStart, width);

          }else{
              cairo_draw_horizontal_dotted_line(cr, _gridStart, width, y);

          }
        }

        // Draw bars
        int sequenceBarCount = (int)(_sequenceEnd / (timeSigNumerator * kTPQN));
        double gridBarCount = (double)(_gridLengthTicks / ((double)kTPQN * timeSigNumerator));
        for (int i = 0; i < sequenceBarCount; i++) {
            int barStartTick = i * timeSigNumerator * kTPQN;
            if (barStartTick >= _startTick + _gridLengthTicks || barStartTick < _startTick) continue; // Prevent out of bounds
            int offsetX = _startTick / (double)_gridLengthTicks * gridWidth;
            double x = _gridStart + (i * gridWidth / gridBarCount) - offsetX;
            if (i != 0) {
                cairo_draw_vertical_line(cr, x, _headerHeight + _hScrollBarHeight, height);
            }
            // Draw bar number
            std::string barNumber = std::to_string(i + 1);
            cairo_draw_text(cr, barNumber, x + 2, _headerHeight + _hScrollBarHeight - 2, 10);
        }
        // Draw beats in length
        int beatsInLength = (int)(_gridLengthTicks / kTPQN);
        for(int i = 0; i < beatsInLength; i++) {
          double x = _gridStart + (i * gridWidth / beatsInLength);
          if (i != 0) {
            cairo_draw_vertical_dotted_line(cr, x, _headerHeight + _hScrollBarHeight, height);
          }
        }

        // Draw notes
        if(midiClip == nullptr) {
            std::cout << "MidiClip is null, cannot draw notes." << std::endl;
            return;
        }
        for(auto &event : midiClip->events) {
          int viewMinPitch = _baseNote;
          int viewMaxPitch = _baseNote + _rowCount - 1;
          if (event.pitch < viewMinPitch || event.pitch > viewMaxPitch)continue; // Skip notes outside the visible pitch range

          if (event.startTick < _startTick || event.startTick + event.duration > _gridLengthTicks + _startTick) continue; // Skip notes outside the visible range
          int yIndex = viewMaxPitch - event.pitch;  // Invert the index for drawing
          int noteHeight = rowHeight - 4; // Leave some padding
          double y = _headerHeight + _hScrollBarHeight + yIndex * rowHeight + 2; // +1 for padding
          double xStart =  _gridStart + ((event.startTick - _startTick) / (double)_gridLengthTicks) * (width - _gridStart) + 1;
          double xEnd = _gridStart + ((event.startTick + event.duration - _startTick) / (double)_gridLengthTicks) * (width - _gridStart);
          int width = xEnd - xStart;
          drawNote(xStart, y, width, noteHeight, false,
                   _selectedNoteId == event.pitch);
        }

    }
protected:
    int _baseNote = 60; // Middle C
    int _rowCount = 6; // Number of rows in the piano roll
    int _selectedNoteId = 2;
    int _selectedLine = 60; // Midi note number for the selected line
    double _gridStart = 8.0; // Starting point of the grid
    int _startTick = 0; // Starting tick for the piano roll
    double _headerHeight = 12.0; // Height of the header section
    double _hScrollBarHeight = 4.0; // Height of the horizontal scroll bar
    int _minRowHeight = 8;
    int _gridLengthTicks = kTPQN * 8;
    int _sequenceEnd = 16 * kTPQN; // Default sequence length in ticks
    cairo_t *_cr; // Cairo context for drawing
};