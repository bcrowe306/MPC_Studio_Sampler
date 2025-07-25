#pragma once
#include <unordered_map>
#include <string>

using std::string;
using std::unordered_map;

enum class DeviceType {
    Undefined,
    Sampler,
    Synthesizer
};

inline unordered_map<string, DeviceType> deviceTypeMap = {
    {"undefined", DeviceType::Undefined},
    {"sampler", DeviceType::Sampler},
    {"synthesizer", DeviceType::Synthesizer}
};

inline unordered_map<DeviceType, string> deviceTypeToStringMap = {
    {DeviceType::Undefined, "undefined"},
    {DeviceType::Sampler, "sampler"},
    {DeviceType::Synthesizer, "synthesizer"}
};
