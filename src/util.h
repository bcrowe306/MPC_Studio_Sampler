#pragma once
#include "uuid.h"
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

using namespace uuids;
using namespace std;

static inline unsigned int from_msb_lsb(unsigned char msb, unsigned char lsb) {
    return ((msb & 0x7F) << 7) | (lsb & 0x7F);
}

static inline void to_msb_lsb(unsigned int value, unsigned char &msb, unsigned char &lsb) {
    if (value > 16383) {
        throw std::out_of_range("Value must be between 0 and 16383 (14-bit).");
    }
    lsb = value & 0x7F;  // lower 7 bits
    msb = (value >> 7) & 0x7F;  // upper 7 bits
}

static inline float midiValueToFloat(uint8_t value) {
    // Convert MIDI value (0-127) to a float in the range [0.0, 1.0]
    // clamp value to 0-127 range
    value = std::clamp(value, (uint8_t)0, (uint8_t)127);
    return static_cast<float>(value) / 127.0f;
}   

static inline float mapFloat(float value, float inMin, float inMax, float outMin, float outMax) {
    // Map a float value from one range to another
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}


inline float dBToLinear(float dB) { return std::pow(10.0f, dB / 10.0f); }

inline float linearToDB(float linearValue) {
    return 10.0f * std::log10(linearValue);
}

inline float linearToPercentage(float linearValue, float maxLinearValue) {
    return (linearValue / maxLinearValue) * 100.0f;
}

static inline uuid generateUUID() {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};

    return gen();
};

static inline uuid GenerateFromString(string uuid_string) {
    auto result = uuid::from_string(uuid_string);
    if (result.has_value()) {
        return result.value();
    } else {
        return uuid{};
    }
};

static inline string UUIDToString(uuid id) { return uuids::to_string(id); };

static inline double angleToRad(double angle) { return angle * (M_PI / 180.0); }