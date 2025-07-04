#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <vector>
#include "sigslot/signal.hpp"
#include <string>
#include <fmt/format.h>

using std::string;

static const double BPM_MAX = 300.0; // Maximum BPM
static const double BPM_MIN = 30.0;  // Minimum BPM

struct TempoChange {
  int tick;   // Tick position of the tempo change
  double bpm; // New BPM
};

struct TimeSignature {
  int numerator;   // Numerator of the time signature (e.g., 4 for 4/4)
  int denominator; // Denominator of the time signature (e.g., 4 for 4/4)
};

struct SongPosition {
    int bar;          // Current bar number
    int beat;         // Current beat number within the bar
    int sixteenthNote; // Current sixteenth note within the beat

    string getSongPositionDisplay() const {
        return fmt::format("{:02}.{:02}.{:02}", bar, beat, sixteenthNote);
    }
};

class MidiClock {
    public:

        // Constructor

        MidiClock(double sampleRate, double bpm)
            : sampleRate(sampleRate), _bpm(bpm) {
            updateTiming();
        }

        // Signals
        sigslot::signal<int> onTick; // Signal for beat changes
        sigslot::signal<bool, bool, bool> onMetronomeTick; // Signal for metronome ticks
        sigslot::signal<string> onSongPositionDisplayChanged; // Signal for song position display
        sigslot::signal<TimeSignature> onTimeSignatureChanged; // Signal for time signature changes
        sigslot::signal<double> onBPMChanged; // Signal for BPM changes
        sigslot::signal<int> onSongPositionChanged; // Signal for song position changes
        void setBPM(double newBPM) {
            if (newBPM != _bpm) {
                _bpm = std::clamp(newBPM, BPM_MIN, BPM_MAX);
                updateTiming();
                onBPMChanged(_bpm);
            }
        }

        void incrementBPM(double increment) {
            setBPM(_bpm + increment);
        }

        void decrementBPM(double decrement) {
            setBPM(_bpm - decrement);
        }
        
        void offsetBPM(double offset) {
            setBPM(_bpm + offset);
        }

        double getBPM() const {
            return _bpm;
        }
    
        void setSongPosition(int bar, int beat, int sixteenthNote) {
            // Update the song position and emit the signal
            _ticks = (bar * _timeSignature.numerator * ppqn) + (beat * ppqn / (_timeSignature.denominator / 4)) + (sixteenthNote * (ppqn / 16));
            onSongPositionChanged(_ticks);
            onSongPositionDisplayChanged(generateDisplayString(bar, beat, sixteenthNote));
        }

        void setTimeSignature(int numerator, int denominator) {
            _timeSignature.numerator = numerator;
            _timeSignature.denominator = denominator;
            updateTiming();
            onTimeSignatureChanged(_timeSignature);
        }


        void setTimeSignature(const TimeSignature &timeSignature) {
            _timeSignature = timeSignature;
            updateTiming();
            onTimeSignatureChanged(_timeSignature);
        }

        TimeSignature getTimeSignature() const {
            return _timeSignature;
        }

        void setSampleRate(double newSampleRate) {
            if (newSampleRate != sampleRate) {
                // Adjust the samples per tick based on the new sample rate
                sampleRate = newSampleRate;
                updateTiming();
            }
        }
    
        // Call this every audio block
        void processBlock(double sampleRate, int blockSize) {
            setSampleRate(sampleRate);

            if (!_enabled) {
                return; // Skip processing if the clock is disabled
            }

            double ticksThisBlock = blockSize / _samplesPerTick;
            _tickAccumulator += ticksThisBlock;
    
            while (_tickAccumulator >= 1.0) {
                _tickAccumulator -= 1.0;
                _onTick();  // Callback or user hook
                ++_ticks;
            }
        }

        void reset() {
            _ticks = 0;
            _tickAccumulator = 0.0;
            onSongPositionChanged(_ticks);
            onSongPositionDisplayChanged(generateDisplayString(0, 0, 0));
        }

        void stop(bool doReset = true) {
            _enabled = false; // Disable the clock
            if (doReset) {
                reset(); // Reset the clock if requested
            }
        }

        void start() {
            _enabled = true; // Enable the clock
        }

        bool isEnabled() const {
            return _enabled; // Check if the clock is enabled
        }
    
    private:
        bool _enabled = false; // Whether the clock is enabled
        SongPosition generateSongPosition() const {
            int ticksPerBar = ppqn * _timeSignature.numerator;
            int ticksPerBeat = ppqn / (_timeSignature.denominator / 4);
            int ticksPerSixteenth = ticksPerBeat / 4;

            int bar = _ticks / ticksPerBar;
            int beat = (_ticks % ticksPerBar) / ticksPerBeat;
            int sixteenthNote = (_ticks % ticksPerBeat) / ticksPerSixteenth;

            return {bar, beat, sixteenthNote};
        }
        void _onTick(){
            int ticksPerBar = ppqn * _timeSignature.numerator;
            int ticksPerBeat = ppqn /  (_timeSignature.denominator / 4);
            int ticksPerSixteenth = ticksPerBar / 16;
            bool isBar = (_ticks % ticksPerBar) == 0;
            bool isBeat = (_ticks % ticksPerBeat) == 0;
            bool isHalfBeat = (_ticks % (ticksPerBeat / 2)) == 0;
            bool is16th = (_ticks % ticksPerSixteenth) == 0;
            int bar = _ticks / ticksPerBar;
            int beat = (_ticks % ticksPerBar) / ticksPerBeat;
            int sixteenthNote = (_ticks % ticksPerBeat) / ticksPerSixteenth;

            if(isHalfBeat){
                onMetronomeTick(isBar, isBeat, isHalfBeat);
            }

            if (is16th){
                onSongPositionDisplayChanged(generateDisplayString(bar, beat, sixteenthNote));
            }
            onTick(_ticks);
        }

        string generateDisplayString(int bar, int beat, int sixteenthNote) {
            // Generate a display string in the format "Bar.Beat.Sixteenth"
            return fmt::format("{:02}.{:02}.{:02}", bar + 1, beat + 1, sixteenthNote + 1);
        }

        double sampleRate;
        double _bpm;
        int ppqn = 480;
        TimeSignature _timeSignature = {4, 4}; // Default to 4/4 time signature
        double _samplesPerTick = 0.0;
        double _tickAccumulator = 0.0;
        int _ticks = 0;
    
        void updateTiming() {
            _samplesPerTick = (60.0 / (_bpm * ppqn)) * sampleRate;
        }
    };
    
