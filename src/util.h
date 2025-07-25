#pragma once
#include "uuid.h"
#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include <utility>   // for std::pair

using namespace uuids;
using namespace std;

static inline unsigned int from_msb_lsb(unsigned char msb, unsigned char lsb) {
    return ((msb & 0x7F) << 7) | (lsb & 0x7F);
}


inline static std::pair<int, int> msblsb(int number) {
    int msb = std::clamp(number / 128, 0, 127);
    int lsb = number % 128;
    return {msb, lsb};
}

static inline void to_msb_lsb(int value, unsigned char &msb, unsigned char &lsb) {
    msb = std::clamp(value / 128, 0, 127);
    lsb = value % 128;
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


inline float dBToLinear(float dB) { return std::pow(10.0f, dB / 20.0f); }

inline float linearToDB(float linearValue) {
    return 20.0f * log10f(linearValue);
    // return 10.0f * std::log10(linearValue);
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

// Function to convert ms into samples with given sample rate
static inline int msToSamples(float ms, float sampleRate) {
    return static_cast<int>((ms / 1000.0f) * sampleRate);
}

static inline int getEncoderOffsetAmount(uint8_t value) {
    int seventhBit = (value & 0b1000000) >> 6;
    int offsetAmount = (value & 0b0111111);
    if (bool(seventhBit)) { // Convert to signed value
        offsetAmount -= 64;
        offsetAmount = offsetAmount; // Make it negative
    } else {
        offsetAmount = offsetAmount; // No change needed
    }
    return offsetAmount;
}

inline float normalizedToFrequency(float normalized, float minFreq = 20.0f,  float maxFreq = 20000.0f) {
    return minFreq * std::pow(maxFreq / minFreq, normalized);
}

inline float frequencyToNormalized(float frequency, float minFreq = 20.0f, float maxFreq = 20000.0f) {
    return std::log(frequency / minFreq) / std::log(maxFreq / minFreq);
}

inline float normalizedToTime(float normalized, float minTime = 0.001f, float maxTime = 10.0f) {
    return minTime * std::pow(maxTime / minTime, normalized);
}

inline float timeToNormalized(float time, float minTime = 0.001f, float maxTime = 10.0f) {
    return std::log(time / minTime) / std::log(maxTime / minTime);
}


// skew < 1 = more sensitivity in low end
// skew > 1 = more sensitivity in high end
inline float skewedNormalizedToFrequency(float normalized, float minFreq, float maxFreq, float skew = .5f) {
    float curved = std::pow(normalized, skew);
    return minFreq * std::pow(maxFreq / minFreq, curved);
}

// skew < 1 = more sensitivity in low end
// skew > 1 = more sensitivity in high end
inline float frequencyToSkewedNormalized(float frequency, float minFreq, float maxFreq, float skew) {
    frequency = std::clamp(frequency, minFreq, maxFreq);
    float base = std::log(frequency / minFreq) / std::log(maxFreq / minFreq);
    return std::pow(base, 1.0f / skew);
}

#include <cstdlib> // for getenv
#include <filesystem>
#include <iostream>
#include <string>

#if defined(_WIN32)
#include <shlobj.h> // for SHGetKnownFolderPath
#include <windows.h>
#elif defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#elif defined(__linux__)
#include <pwd.h>
#include <unistd.h>
#endif

inline std::filesystem::path getDocumentsFolder() {
#if defined(_WIN32)
    PWSTR path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
    if (SUCCEEDED(hr)) {
        std::wstring wpath(path);
        CoTaskMemFree(path);
        return std::filesystem::path(wpath);
    }
    return std::filesystem::path(); // fallback or error
#elif defined(__APPLE__) || defined(__linux__)
    const char *homeDir = getenv("HOME");
    if (!homeDir) {
        struct passwd *pwd = getpwuid(getuid());
        if (pwd)
          homeDir = pwd->pw_dir;
    }
    if (homeDir) {
        std::filesystem::path documents =
            std::filesystem::path(homeDir) / "Documents";
        return documents;
    }
    return std::filesystem::path(); // fallback or error
#else
    return std::filesystem::path(); // unsupported OS
#endif
}
