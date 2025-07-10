#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/FunctionNode.h"
#include "sigslot/signal.hpp"
#include <string>
#include <fmt/format.h>
#include "core/constants.h"
#include "midi_utils.h"
#include "LabSound/LabSound.h"
#include <memory>

using std::string;
using std::shared_ptr;
using std::make_shared;


class Timer {
    public:

        // Constructor

        Timer(shared_ptr<lab::AudioContext> ac, double sampleRate, double bpm)
            : sampleRate(sampleRate), _bpm(bpm) {
            timerNode = make_shared<lab::FunctionNode>(*ac.get());
            timerNode->setFunction([this](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
                processBlock(r.context()->sampleRate(), bufferSize);
            });
            timerNode->start(0.0); // Start the timer node at time 0
            updateTiming();
        }

        // Signals
        sigslot::signal<int> onTick; // Signal transport
        sigslot::signal<TimeSignature> onTimeSignatureChanged; // Signal for time signature changes
        sigslot::signal<double> onBPMChanged; // Signal for BPM changes

        std::shared_ptr<lab::FunctionNode> timerNode; // Node representing the timer

        void setBPM(double newBPM) {
            if (newBPM != _bpm) {
                _bpm = std::clamp(newBPM, kBPM_MIN, kBPM_MAX);
                updateTiming();
                onBPMChanged(_bpm);
            }
        }

        void incrementBPM(double increment) {
            setBPM(_bpm + increment);
        }

        void decrementBPM(double decrement) {
            setBPM(_bpm - decrement);
        }
        
        void offsetBPM(double offset) {
            setBPM(_bpm + offset);
        }

        double getBPM() const {
            return _bpm;
        }
    
        

        void setTimeSignature(int numerator, int denominator) {
            _timeSignature.numerator = numerator;
            _timeSignature.denominator = denominator;
            updateTiming();
            onTimeSignatureChanged(_timeSignature);
        }


        void setTimeSignature(const TimeSignature &timeSignature) {
            _timeSignature = timeSignature;
            updateTiming();
            onTimeSignatureChanged(_timeSignature);
        }

        TimeSignature getTimeSignature() const {
            return _timeSignature;
        }

        void setSampleRate(double newSampleRate) {
            if (newSampleRate != sampleRate) {
                // Adjust the samples per tick based on the new sample rate
                sampleRate = newSampleRate;
                updateTiming();
            }
        }
    
        // Call this every audio block
        void processBlock(double sampleRate, int blockSize) {
            setSampleRate(sampleRate);

            

            double samplesThisBlock = blockSize / _samplesPerTick;
            _samplesAccumulator += samplesThisBlock;
    
            while (_samplesAccumulator >= 1.0) {
                
                _samplesAccumulator -= 1.0;
                _onTick();  // Callback or user hook
                
            }
        }

        void reset() {
            _samplesAccumulator = 0.0;
        }


    
    private:
        void _onTick(){
            onTick(_tickCounter);
            _tickCounter++;
        }


        double sampleRate;
        double _bpm;
        TimeSignature _timeSignature = {4, 4}; // Default to 4/4 time signature
        double _samplesPerTick = 0.0;
        double _samplesAccumulator = 0.0;
        int _tickCounter = 0;
    
        void updateTiming() {
            _samplesPerTick = (60.0 / (_bpm * kTPQN)) * sampleRate;
        }
    };
    
