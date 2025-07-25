#pragma once 
#include "yaml-cpp/yaml.h"
#include <string>

class Serializable {
public:
    virtual ~Serializable() = default;

    // Serialize the object to a YAML emitter
    virtual void serialize(YAML::Emitter &out) = 0;

    // Deserialize the object from a YAML node
    virtual void deserialize(const YAML::Node &yaml) = 0;

};