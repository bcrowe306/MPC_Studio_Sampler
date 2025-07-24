#pragma once
#include "LabSound/LabSound.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/FunctionNode.h"
#include <cstdint> // For int64_t and uint64_t
#include <memory>
#include <sys/types.h>
#include <array>
#include <cmath>
#include "sigslot/signal.hpp"
using std::shared_ptr;

class TapTempoNode {
public:

    shared_ptr<lab::AudioContext> context; // Audio context for the node
    shared_ptr<lab::FunctionNode> functionNode; // Function node for tap tempo functionality
    sigslot::signal<double> onTempoCalculated; // Signal emitted when a new tempo is calculated
    TapTempoNode(shared_ptr<lab::AudioContext> ctx)
        : context(ctx), functionNode(std::make_shared<lab::FunctionNode>(*ctx.get())) {
        // Initialize the tap tempo nod
        functionNode->setFunction([this](lab::ContextRenderLock &r, lab::FunctionNode *me, int channelCount, float *output, int framesToProcess) {
            processBlock(r, me, channelCount, output, framesToProcess);
        });
        functionNode->start(0);
        context->connect(context->destinationNode(), functionNode, 0, 0); // Connect the function node to the audio context destination
        context->synchronizeConnections(); // Synchronize connections after setup
    }

    void processBlock(lab::ContextRenderLock &r, lab::FunctionNode *me, int channelCount, float *output, int framesToProcess) {
        for (int i = 0; i < framesToProcess; ++i) {
            _sampleCounter++;
            _sampleRate = r.context()->sampleRate();
            if (_sampleCounter - _lastTapTime > _sampleRate * 2) {
                // Reset tap intervals if no tap has occurred in the last 3 seconds
                _tapIntervals.fill(0);
                _tapCount = 0;
                _lastTapTime = 0; // Reset last tap time to current sample counter
            }
        }
        
    }

    void tap(){
        uint64_t currentTime = _sampleCounter;
        if (_lastTapTime == 0) {
            _lastTapTime = currentTime; // Initialize last tap time on first tap
            return;
        }
        auto index = _tapCount % 3; // Use modulo to cycle through the tap intervals
        _tapIntervals[index] = currentTime - _lastTapTime;
        _tapCount++; // Cycle through tap intervals
        _lastTapTime = currentTime;
        calculateTempo(); // Calculate the tempo after each tap
    }



    void calculateTempo() {
        if (_tapCount == 0) return; // No taps to calculate tempo

        uint64_t totalInterval = 0;
        int nonZeroCount = 0; // Count of non-zero intervals
        for (int i = 0; i < _tapIntervals.size(); ++i) {
            if (_tapIntervals[i] == 0) continue; // Skip zero intervals
            totalInterval += _tapIntervals[i];
            nonZeroCount++; // Count only non-zero intervals
        }
        uint64_t averageInterval = totalInterval / nonZeroCount; // Calculate the average interval

        // Calculate BPM from the average interval
        double bpm = 60 / (averageInterval / _sampleRate); // Convert to BPM
        // Here you can use the calculated BPM value as needed
        bpm = std::round(bpm); // Round to nearest whole number
        if(_tapCount >= 3) {
            onTempoCalculated(bpm); // Emit signal with the calculated BPM
        }

    }

private:
    float _sampleRate = 48000.0f; // Default sample rate
    uint64_t _sampleCounter = 0;
    uint64_t _lastTapTime = 0; // Timestamp of the last tap
    int _tapCount = 0; // Number of taps detected
    std::array<uint64_t, 3> _tapIntervals = {0, 0, 0};

};