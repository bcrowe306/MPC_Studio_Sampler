#pragma once

#include "sigslot/signal.hpp"

struct NoteKey {
  uint8_t pitch;
  uint8_t channel;

  bool operator==(const NoteKey &other) const {
    return pitch == other.pitch && channel == other.channel;
  }
};

namespace std {
template <> struct hash<NoteKey> {
  size_t operator()(const NoteKey &k) const {
    return (k.pitch << 8) | k.channel;
  }
};
} // namespace std

struct MidiNote {
  uint32_t id;
  int pitch;
  int velocity;
  int startTick;
  int durationTicks;
  uint8_t channel;
  bool muted = false;
};

struct ControlChange {
  uint8_t controllerNumber;
  uint8_t value;
  int tick;
  uint8_t channel;
};

struct PitchBend {
  int tick;
  uint8_t channel;
  int value; // -8192 to +8191
};

struct ProgramChange {
  int tick;
  uint8_t channel;
  uint8_t program;
};

struct ChannelAftertouch {
  int tick;
  uint8_t channel;
  uint8_t pressure;
};

struct PolyAftertouch {
  int tick;
  uint8_t channel;
  uint8_t pitch;
  uint8_t pressure;
};

class MidiClip {
public:
  std::vector<MidiNote> notes;
  std::vector<ControlChange> ccMessages;
  std::vector<PitchBend> pitchBends;
  std::vector<ProgramChange> programChanges;
  std::vector<ChannelAftertouch> channelAftertouches;
  std::vector<PolyAftertouch> polyAftertouches;

  int tpqn = 480;
  double bpm = 120.0;

private:
  std::unordered_map<NoteKey, std::pair<int, int>> activeNotes;
  uint32_t nextNoteId = 1;

public:
  // Convert real time to tick time
  int timeToTicks(double timeSec) const {
    double quarterNotes = (timeSec * bpm) / 60.0;
    return static_cast<int>(quarterNotes * tpqn + 0.5);
  }

  // Record incoming MIDI event
  void recordMidiEvent(uint8_t status, uint8_t data1, uint8_t data2,
                       uint8_t channel, double timeSec) {
    uint8_t statusType = status & 0xF0;
    int tickTime = timeToTicks(timeSec);

    switch (statusType) {
    case 0x90: // Note On
      if (data2 > 0) {
        activeNotes[{data1, channel}] = {tickTime, data2};
      } else {
        goto note_off;
      }
      break;

    note_off:
    case 0x80: {
      NoteKey key{data1, channel};
      auto it = activeNotes.find(key);
      if (it != activeNotes.end()) {
        int startTick = it->second.first;
        int velocity = it->second.second;
        int duration = tickTime - startTick;

        notes.push_back(MidiNote{.id = nextNoteId++,
                                 .pitch = data1,
                                 .velocity = velocity,
                                 .startTick = startTick,
                                 .durationTicks = duration,
                                 .channel = channel,
                                 .muted = false});

        activeNotes.erase(it);
      }
      break;
    }

    case 0xB0: // Control Change
      ccMessages.push_back(ControlChange{.controllerNumber = data1,
                                         .value = data2,
                                         .tick = tickTime,
                                         .channel = channel});
      break;

    case 0xC0: // Program Change
      programChanges.push_back(ProgramChange{
          .tick = tickTime, .channel = channel, .program = data1});
      break;

    case 0xD0: // Channel Pressure
      channelAftertouches.push_back(ChannelAftertouch{
          .tick = tickTime, .channel = channel, .pressure = data1});
      break;

    case 0xE0: { // Pitch Bend
      int bend = ((data2 << 7) | data1) - 8192;
      pitchBends.push_back(
          PitchBend{.tick = tickTime, .channel = channel, .value = bend});
      break;
    }

    case 0xA0: // Polyphonic Aftertouch
      polyAftertouches.push_back(PolyAftertouch{.tick = tickTime,
                                                .channel = channel,
                                                .pitch = data1,
                                                .pressure = data2});
      break;

    default:
      // Skip unsupported (e.g., SysEx)
      break;
    }
  }

  // Playback everything
  void playBack(double currentTimeSec, MidiOutput &output) {
    int currentTick = timeToTicks(currentTimeSec);

    for (const auto &note : notes) {
      if (note.muted)
        continue;
      if (note.startTick == currentTick)
        output.sendNoteOn(note.channel, note.pitch, note.velocity);
      if (note.startTick + note.durationTicks == currentTick)
        output.sendNoteOff(note.channel, note.pitch);
    }

    for (const auto &cc : ccMessages)
      if (cc.tick == currentTick)
        output.sendControlChange(cc.channel, cc.controllerNumber, cc.value);

    for (const auto &bend : pitchBends)
      if (bend.tick == currentTick)
        output.sendPitchBend(bend.channel, bend.value);

    for (const auto &pc : programChanges)
      if (pc.tick == currentTick)
        output.sendProgramChange(pc.channel, pc.program);

    for (const auto &at : channelAftertouches)
      if (at.tick == currentTick)
        output.sendChannelAftertouch(at.channel, at.pressure);

    for (const auto &pat : polyAftertouches)
      if (pat.tick == currentTick)
        output.sendPolyAftertouch(pat.channel, pat.pitch, pat.pressure);
  }

  // Mute a note by unique ID
  void muteNoteById(uint32_t id, bool mute = true) {
    for (auto &note : notes) {
      if (note.id == id) {
        note.muted = mute;
        return;
      }
    }
  }
};