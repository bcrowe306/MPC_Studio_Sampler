#pragma once
#include <cmath>
#include "constants.h"
#include <algorithm>
#include "fmt/format.h"
#include <string>

using std::string;

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
      int tick;         // Current tick position
      int lengthInTicks;


      float getSongPositionInBeatTime() const {
          return static_cast<float>(tick) / (float)kTPQN; // Convert tick to beat time
      }

      float getSongPositionInMs(float bpm, float sampleRate = 48000.0) const {
          return sampleRate;
      }

  
      string getSongPositionDisplay() const {
          return fmt::format("{:02}.{:02}.{:02}", bar + 1, beat + 1, sixteenthNote + 1);
      }

      void updateFromTick(int tick, int timeSignatureNumerator, int timeSignatureDenominator) {
          this->tick = tick;
          int ticksPerBar = kTPQN * timeSignatureNumerator;
          int ticksPerBeat = kTPQN / (timeSignatureDenominator / 4);
          int ticksPerSixteenth = ticksPerBar / 16;
          this-> bar = tick / ticksPerBar;
          this-> beat = (tick % ticksPerBar) / ticksPerBeat;
          this-> sixteenthNote = (tick % ticksPerBeat) / ticksPerSixteenth;
      }
  };
  

enum class QUANTIZATION_VALUE{
    NONE = 0,
    SIXTY_FOURTH = 30,
    THIRTY_SECOND_TRIPLET = 40,
    THIRTY_SECOND = 60,
    SIXTEEN_TRIPLET = 80,
    SIXTEENTH = 120,
    EIGHTH_TRIPLET = 160,
    EIGHTH = 240,
    QUARTER_TRIPLET = 320,
    QUARTER = 480,
    HALF = 960,
    BAR = 1920,
    BAR_2 = 3840
};

inline int quantizeTick(int delta, QUANTIZATION_VALUE resolution = QUANTIZATION_VALUE::SIXTEENTH, float strength = 1.0) {

  int grid_tick_size = std::round(kTPQN * static_cast<int>(resolution));
  int quantized_delta = std::round(static_cast<float>(delta) / grid_tick_size) * grid_tick_size;
  int note_spread = quantized_delta - delta;
  int shift_amount = std::round(note_spread * strength);
  int new_delta = shift_amount + delta;
  return new_delta;
}

// Convert ticks to beat time
inline float ticksToBeatTime(int ticks){
    return static_cast<float>(ticks) / kTPQN; 
}

// Convert beat time to ticks
inline int beatTimeToTicks(float beatTime) {
    return static_cast<int>(std::round(beatTime * kTPQN)); 
}

// Convert bars to ticks (assuming 4 beats per bar)
inline int barsToTicks(int bars) {
    return bars * kTPQN * 4; 
}

inline int ticksToBars(int ticks) {
    return ticks / (kTPQN * 4); // Convert ticks to bars
}


/// Converts a MIDI note (usually in the range 0-127) to a frequency in Hz.
inline float noteNumberToFrequency (int note)          { return A440_freq * std::pow (2.0f, (static_cast<float> (note) - A440_note) * (1.0f / 12.0f)); }

/// Converts a MIDI note (usually in the range 0-127) to a frequency in Hz.
inline float noteNumberToFrequency (float note)        { return A440_freq * std::pow (2.0f, (note - static_cast<float> (A440_note)) * (1.0f / 12.0f)); }

/// Converts a frequency in Hz to an equivalent MIDI note number.
inline float frequencyToNoteNumber (float frequency)   { return static_cast<float> (A440_note) + (12.0f / std::log (2.0f)) * std::log (frequency * (1.0f / A440_freq)); }

inline string generateDisplayString(int currentTick, int timeSignatureNumerator, int timeSignatureDenominator) {
    int ticksPerBeat = kTPQN / timeSignatureDenominator;
    int sixteenthNote = (currentTick % ticksPerBeat) / (ticksPerBeat / 16);
    int beat = (currentTick / ticksPerBeat) % timeSignatureNumerator;
    int bar = currentTick / (ticksPerBeat * timeSignatureNumerator);
    return fmt::format("{:02d}.{:02d}.{:02d}", bar + 1, beat + 1, sixteenthNote + 1);
}

inline bool isTickSixteenth(int currentTick) {
    return currentTick % static_cast<int>(QUANTIZATION_VALUE::SIXTEENTH) == 0;
}

inline SongPosition getSongPositionFromTick(int tick){
    int bars = ticksToBars(tick);
    int beats = (tick % (kTPQN * 4)) / kTPQN;
    int sixteenthNotes = (tick % kTPQN) / (kTPQN / 16);
    return {bars, beats, sixteenthNotes};
}

