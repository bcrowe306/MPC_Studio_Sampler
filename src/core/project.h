#pragma once
#include "LabSound/LabSound.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include "core/parameter.h"
#include "core/track.h"
#include <array>

inline const int kMaxTracks = 64; // Maximum number of tracks in a project
inline const int kMaxBusses = 8; // Maximum number of busses in a project

class Project {
public:
    Project() {
        // Initialize the project with an empty track list
        tracks = std::vector<std::shared_ptr<Track>>();
        tracks.reserve(kMaxTracks); // Reserve space for 64 tracks
    };
    ~Project() = default;

    void serialize() {
        // Implement serialization logic if needed
    }

    void deserialize() {
        // Implement deserialization logic if needed
    }

    void addTrack(std::shared_ptr<Track> track) {
        if (tracks.size() >= kMaxTracks) {
            std::cout << "Maximum number of tracks reached. Cannot add more tracks." << std::endl;
            return;
        }
        if (!track) {
            std::cout << "Cannot add a null track." << std::endl;
            return;
        }
        tracks.push_back(track);
    }

    const std::vector<std::shared_ptr<Track>>& getTracks() const {
        return tracks;
    }

private:
    std::vector<std::shared_ptr<Track>> tracks; // List of tracks in the project
};