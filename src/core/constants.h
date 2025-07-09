#pragma once
static constexpr float A440_freq = 440.0f;
static constexpr int A440_note = 69;
static inline const int kTPQN = 480; // Ticks per quarter note
static inline const int kMaxTracks = 64; // Maximum number of tracks in a project
static inline const int kMaxBusses = 8; // Maximum number of busses in a project
static inline const int kMaxSequences = 64; // Maximum number of sequences in a project
static inline const int kMaxClips = 64; // Maximum number of clips in a sequence
static inline const int kMaxEffects = 8; // Maximum number of events in a clip
static const double kBPM_MAX = 300.0; // Maximum BPM
static const double kBPM_MIN = 30.0;  // Minimum BPM
static const int kDefaultSequenceLengthInTicks = 480 * 4 * 2; // Default sequence length in ticks (4 bars, 4 beats per bar, 480 ticks per beat) 
