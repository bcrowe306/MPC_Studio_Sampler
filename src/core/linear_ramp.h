#pragma once
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <atomic>

class LRamp {
public:
  sigslot::signal<> onEnded; // Signal emitted when the ramp ends

    LRamp() = default; // Default constructor
    LRamp(double start, double end, double durationMs, double sampleRate)
        : _start(start), _end(end), _durationMs(durationMs), _currentValue(start),
            _sampleRate(sampleRate) {
        _calcCoefficients();
    }

  ~LRamp() {
    std::cout << "LRamp destroyed" << std::endl; // Debug output
  }
  // Process the ramp and return the current value. Call this function regularly
  // to update the ramp.
  double getValue() const {
    return _currentValue; // Return the current value of the ramp
  }

  double process() {
    if (!_processing) {
      return _currentValue;
    }

    if (_start == _end) {
      _processing = false;    // If start and end are the same, stop processing
      _currentValue = _start; // Reset current value to the start
      return _currentValue;
    }

    if (direction()) {
      if (_currentValue < _end) {
        _currentValue += _step; // Increment the current value
        return _currentValue;
      }
    } else {
      if (_currentValue > _end) {
        _currentValue += _step; // Increment the current value
        return _currentValue;
      }
    }

    onEnded(); // Emit the signal when the ramp ends
    _currentValue = _end;
    _processing = false;
    return _end;
  }

  bool direction() const {
    return _start <
           _end; // Return true if the ramp is increasing, false if decreasing
  }

  // Begin the ramp processing. Call this to start the ramp.
  // This resets the current value to the start and sets processing to true.
  // This allows the ramp to be processed in a loop until it reaches the end.
  void begin() {
    _processing = true;     // Start processing the ramp
    _currentValue = _start; // Reset current value to the start
  }

  void reset() {
    _processing = false;    // Stop processing the ramp
    _currentValue = _start; // Reset current value to the start
  }
  // Set the new start value
  void setStart(double newStart) {
    _start = newStart;
    _currentValue = _start;
    _calcCoefficients();
  }

  // Return the current start value
  double getStart() const { return _start; }

  // Set the new end value
  void setEnd(double newEnd) {
    _end = newEnd;
    _calcCoefficients();
  }

  // Return the current end value
  double getEnd() const { return _end; }

  // Return the current duration in milliseconds
  double getDurationMs() const { return _durationMs; }

  // Set the new duration in milliseconds
  void setDurationMs(double newDurationMs) {
    _durationMs = newDurationMs;
    _calcCoefficients();
  }

  // Set the sample rate for the ramp. Need to calculate time interval in
  // samples.
  void setSampleRate(double newSampleRate) {
    _sampleRate = newSampleRate;
    _calcCoefficients();
  }

  // Get the current sample rate
  double getSampleRate() const { return _sampleRate; }

  // Return the current processing state
  bool getState() const { return _processing; }

protected:
  void _calcCoefficients() {
    _step = (_end - _start) /
            ((_durationMs / 1000.0) * _sampleRate); // Recalculate the step size
  }
  double _start = 0.0;          // Start value of the ramp
  double _end = 1.0;            // End value of the ramp
  double _durationMs = 5.0;     // Duration of the ramp in milliseconds
  double _currentValue;         // Current value of the ramp
  double _step;                 // Step size for the ramp
  double _sampleRate = 48000.0; // Sample rate in Hz
  bool _processing = false;     // Flag to indicate if the ramp is currently processing
};