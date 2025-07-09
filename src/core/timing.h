#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <vector>
#include "sigslot/signal.hpp"
#include <string>
#include <fmt/format.h>
#include "core/constants.h"
#include "midi_utils.h"

using std::string;


class MidiClock {
    public:

        // Constructor

        MidiClock(double sampleRate, double bpm)
            : sampleRate(sampleRate), _bpm(bpm) {
            updateTiming();
        }

        // Signals
        sigslot::signal<> onTick; // Signal for beat changes
        sigslot::signal<TimeSignature> onTimeSignatureChanged; // Signal for time signature changes
        sigslot::signal<double> onBPMChanged; // Signal for BPM changes
        void setBPM(double newBPM) {
            if (newBPM != _bpm) {
                _bpm = std::clamp(newBPM, kBPM_MIN, kBPM_MAX);
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

            double samplesThisBlock = blockSize / _samplesPerTick;
            _samplesAccumulator += samplesThisBlock;
    
            while (_samplesAccumulator >= 1.0) {
                _samplesAccumulator -= 1.0;
                _onTick();  // Callback or user hook
                
            }
        }

        void reset() {
            _samplesAccumulator = 0.0;
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

        void _onTick(){
            onTick();
        }


        double sampleRate;
        double _bpm;
        TimeSignature _timeSignature = {4, 4}; // Default to 4/4 time signature
        double _samplesPerTick = 0.0;
        double _samplesAccumulator = 0.0;
    
        void updateTiming() {
            _samplesPerTick = (60.0 / (_bpm * kTPQN)) * sampleRate;
        }
    };
    
