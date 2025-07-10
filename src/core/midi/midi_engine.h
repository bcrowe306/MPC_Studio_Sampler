#pragma once
#include "rtmidi/RtMidi.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <any>
#include "sigslot/signal.hpp"
#include "audio/choc_MIDI.h"

using choc::midi::ShortMessage;

namespace MPCSBC {

struct MidiCallbackData{
    std::string deviceName;
    std::any midiEngine;
};

struct MidiInputDevice {
    std::unique_ptr<RtMidiIn> midi_in;
    std::string name;
    unsigned int port_number;
    bool enabled;
    
    MidiInputDevice(const std::string& device_name, unsigned int port_num)
        : name(device_name), port_number(port_num), enabled(false) {
        midi_in = std::make_unique<RtMidiIn>();
    }
};

struct MidiOutputDevice {
    std::unique_ptr<RtMidiOut> midi_out;
    std::string name;
    unsigned int port_number;
    bool enabled;
    
    MidiOutputDevice(const std::string& device_name, unsigned int port_num)
        : name(device_name), port_number(port_num), enabled(false) {
        midi_out = std::make_unique<RtMidiOut>();
    }
};

class MidiEngine {
public:
    using MidiCallback = std::function<void(const ShortMessage&, double)>;
    sigslot::signal<const std::string &, const ShortMessage&, double> onMidiInput; 
    MidiEngine(bool enableAll = false);
    ~MidiEngine();
    
    // Device management
    void refresh_devices();
    void enable_input_device(const std::string& device_name);
    void disable_input_device(const std::string& device_name);
    void enable_output_device(const std::string& device_name);
    void disable_output_device(const std::string& device_name);
    
    // Device queries
    std::vector<std::string> get_input_device_names() const;
    std::vector<std::string> get_output_device_names() const;
    bool is_input_device_enabled(const std::string& device_name) const;
    bool is_output_device_enabled(const std::string& device_name) const;

    
    // MIDI message handling
    void set_midi_callback(MidiCallback callback);
    void send_midi_message(const std::string& device_name, const std::vector<unsigned char>& message);
    
    // Static callback for RTMidi
    static void midi_input_callback(double timestamp, std::vector<unsigned char>* message, void* user_data);
    
private:
    std::unordered_map<std::string, std::unique_ptr<MidiInputDevice>> input_devices_;
    std::unordered_map<std::string, MidiCallbackData> input_callback_data_;
    std::unordered_map<std::string, std::unique_ptr<MidiOutputDevice>> output_devices_;
    
    MidiCallback midi_callback_;
    
    void scan_input_devices();
    void scan_output_devices();
    void propagate_midi_message(const std::string& device_name, ShortMessage &message, double timestamp);
};

} // namespace MPCSBC