#include "midi_engine.h"
#include <any>
#include <iostream>
#include <algorithm>
#include "core/constants.h"


namespace MPCSBC {

MidiEngine::MidiEngine(bool enableAll) {
    refresh_devices();
    if (enableAll) {
        for (const auto& [name, device] : input_devices_) {
            enable_input_device(name);
        }
        for (const auto& [name, device] : output_devices_) {
            enable_output_device(name);
        }
    }
}

MidiEngine::~MidiEngine() {
    // Disable all devices before destruction
    for (auto& [name, device] : input_devices_) {
        if (device->enabled) {
            disable_input_device(name);
        }
    }
    
    for (auto& [name, device] : output_devices_) {
        if (device->enabled) {
            disable_output_device(name);
        }
    }
}

void MidiEngine::refresh_devices() {
    // Clear existing devices
    input_devices_.clear();
    output_devices_.clear();
    
    // Scan for new devices
    scan_input_devices();
    scan_output_devices();
}

void MidiEngine::scan_input_devices() {
    try {
        RtMidiIn temp_midi_in;
        unsigned int port_count = temp_midi_in.getPortCount();
        
        for (unsigned int i = 0; i < port_count; ++i) {
            std::string port_name = temp_midi_in.getPortName(i);
            if (!port_name.empty() && port_name != MPC_STUDIO_BLACK_DEVICE_NAME) { // Exclude MPC Studio Black Private
                auto device = std::make_unique<MidiInputDevice>(port_name, i);
                input_devices_[port_name] = std::move(device);
            }
        }
    } catch (RtMidiError& error) {
        std::cerr << "Error scanning MIDI input devices: " << error.getMessage() << std::endl;
    }
}

void MidiEngine::scan_output_devices() {
    try {
        RtMidiOut temp_midi_out;
        unsigned int port_count = temp_midi_out.getPortCount();
        
        for (unsigned int i = 0; i < port_count; ++i) {
            std::string port_name = temp_midi_out.getPortName(i);
            if (!port_name.empty() && port_name != MPC_STUDIO_BLACK_DEVICE_NAME) { // Exclude MPC Studio Black Private
                auto device = std::make_unique<MidiOutputDevice>(port_name, i);
                output_devices_[port_name] = std::move(device);
            }
        }
    } catch (RtMidiError& error) {
        std::cerr << "Error scanning MIDI output devices: " << error.getMessage() << std::endl;
    }
}

void MidiEngine::enable_input_device(const std::string& device_name) {
    auto it = input_devices_.find(device_name);
    if (it == input_devices_.end()) {
        std::cerr << "Input device not found: " << device_name << std::endl;
        return;
    }
    
    auto& device = it->second;
    if (device->enabled) {
        return; // Already enabled
    }
    
    try {
        MidiCallbackData callback_data = { device_name, std::any(this) };
        input_callback_data_[device_name] = callback_data;
        device->midi_in->setCallback(&MidiEngine::midi_input_callback, &input_callback_data_[device_name]);
        device->midi_in->openPort(device->port_number, device_name);
        device->midi_in->ignoreTypes(false, false, false); // Don't ignore sysex, timing, or active sensing
        device->enabled = true;
        
        std::cout << "Enabled MIDI input device: " << device_name << std::endl;
    } catch (RtMidiError& error) {
        std::cerr << "Error enabling MIDI input device " << device_name << ": " << error.getMessage() << std::endl;
    }
}

void MidiEngine::disable_input_device(const std::string& device_name) {
    auto it = input_devices_.find(device_name);
    if (it == input_devices_.end()) {
        std::cerr << "Input device not found: " << device_name << std::endl;
        return;
    }
    
    auto& device = it->second;
    if (!device->enabled) {
        return; // Already disabled
    }
    
    try {
       
        device->midi_in->closePort();
        auto callback_it = input_callback_data_.find(device_name);
        if (callback_it != input_callback_data_.end()) {
            device->midi_in->setCallback(nullptr); // Remove the callback
            input_callback_data_.erase(callback_it);
        }
        device->enabled = false;
        
        std::cout << "Disabled MIDI input device: " << device_name << std::endl;
    } catch (RtMidiError& error) {
        std::cerr << "Error disabling MIDI input device " << device_name << ": " << error.getMessage() << std::endl;
    }
}

void MidiEngine::enable_output_device(const std::string& device_name) {
    auto it = output_devices_.find(device_name);
    if (it == output_devices_.end()) {
        std::cerr << "Output device not found: " << device_name << std::endl;
        return;
    }
    
    auto& device = it->second;
    if (device->enabled) {
        return; // Already enabled
    }
    
    try {
        device->midi_out->openPort(device->port_number, device_name);
        device->enabled = true;
        
        std::cout << "Enabled MIDI output device: " << device_name << std::endl;
    } catch (RtMidiError& error) {
        std::cerr << "Error enabling MIDI output device " << device_name << ": " << error.getMessage() << std::endl;
    }
}

void MidiEngine::disable_output_device(const std::string& device_name) {
    auto it = output_devices_.find(device_name);
    if (it == output_devices_.end()) {
        std::cerr << "Output device not found: " << device_name << std::endl;
        return;
    }
    
    auto& device = it->second;
    if (!device->enabled) {
        return; // Already disabled
    }
    
    try {
        device->midi_out->closePort();
        device->enabled = false;
        
        std::cout << "Disabled MIDI output device: " << device_name << std::endl;
    } catch (RtMidiError& error) {
        std::cerr << "Error disabling MIDI output device " << device_name << ": " << error.getMessage() << std::endl;
    }
}

std::vector<std::string> MidiEngine::get_input_device_names() const {
    std::vector<std::string> names;
    names.reserve(input_devices_.size());
    
    for (const auto& [name, device] : input_devices_) {
        names.push_back(name);
    }
    
    return names;
}

std::vector<std::string> MidiEngine::get_output_device_names() const {
    std::vector<std::string> names;
    names.reserve(output_devices_.size());
    
    for (const auto& [name, device] : output_devices_) {
        names.push_back(name);
    }
    
    return names;
}

bool MidiEngine::is_input_device_enabled(const std::string& device_name) const {
    auto it = input_devices_.find(device_name);
    return it != input_devices_.end() && it->second->enabled;
}

bool MidiEngine::is_output_device_enabled(const std::string& device_name) const {
    auto it = output_devices_.find(device_name);
    return it != output_devices_.end() && it->second->enabled;
}

void MidiEngine::set_midi_callback(MidiCallback callback) {
    midi_callback_ = std::move(callback);
}

void MidiEngine::send_midi_message(const std::string& device_name, const std::vector<unsigned char>& message) {
    auto it = output_devices_.find(device_name);
    if (it == output_devices_.end()) {
        std::cerr << "Output device not found: " << device_name << std::endl;
        return;
    }
    
    auto& device = it->second;
    if (!device->enabled) {
        std::cerr << "Output device not enabled: " << device_name << std::endl;
        return;
    }
    
    try {
        device->midi_out->sendMessage(&message);
    } catch (RtMidiError& error) {
        std::cerr << "Error sending MIDI message to " << device_name << ": " << error.getMessage() << std::endl;
    }
}

void MidiEngine::midi_input_callback(double timestamp, std::vector<unsigned char>* message, void* user_data) {
    if (!message || !user_data) {
        return;
    }
    
    MidiEngine* engine = std::any_cast<MidiEngine*>(static_cast<MidiCallbackData*>(user_data)->midiEngine);
    std::string device_name = static_cast<MidiCallbackData*>(user_data)->deviceName;
    if(message->size() == 3){
        choc::midi::ShortMessage short_message(message->data(), message->size());
        std::cout << "Msg: " << short_message.toHexString() << std::endl;
        engine->onMidiInput(device_name, short_message, timestamp);
    }
}


} // namespace MPCSBC
