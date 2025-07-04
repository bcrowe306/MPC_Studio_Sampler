#include "LabSound/LabSound.h"
#include "timing.h"
#include "parameter.h"


#include <memory>

using std::shared_ptr;
using std::make_shared;

class Playhead{
    public:
    
        shared_ptr<MidiClock> midiClock; // MIDI clock for timing
        shared_ptr<lab::FunctionNode> playheadNode; // Node representing the playhead
        shared_ptr<lab::AudioContext> audioContext; // Audio context for the playhead
        Playhead(shared_ptr<lab::AudioContext> audioContext) : audioContext(audioContext) {
            // Initialize playhead with the audio context
            playheadNode = make_shared<lab::FunctionNode>(*audioContext.get());
            midiClock = make_shared<MidiClock>(audioContext->sampleRate(), 120.0); // Initialize MidiClock with 24 PPQN and 120 BPM
            playheadNode->setFunction([this](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
                midiClock->processBlock(r.context()->sampleRate(), bufferSize);
            });
            playheadNode->start(0.0); // Start the playhead node at time 0.0
        };
        ~Playhead() = default;
    
        void processBlock(lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
            // Process the audio block for the playhead
            midiClock->processBlock(audioContext->sampleRate(), bufferSize);
        };

        void setBPM(double bpm) {
            midiClock->setBPM(bpm); // Set the BPM for the MIDI clock
        };

        void start() {
            midiClock->start(); // Start the MIDI clock
        };

        void stop() {
            midiClock->stop(_returnToZero); // Stop the MIDI clock
        };

        void togglePlaying() {
            if (midiClock->isEnabled()) {
                stop(); // Stop if currently playing
            } else {
                start(); // Start if currently stopped
            }
        };

        void setReturnToZero(bool returnToZero) {
            _returnToZero = returnToZero; // Set the flag to return to zero position
        };

        void setTimeSigNumerator(int numerator) {
            midiClock->setTimeSignature(numerator, midiClock->getTimeSignature().denominator); // Set the numerator of the time signature
        };

        void setTimeSigDenominator(int denominator) {
            midiClock->setTimeSignature(midiClock->getTimeSignature().numerator, denominator); // Set the denominator of the time signature
        };
        void setTimeSignature(const TimeSignature &timeSignature) {
            midiClock->setTimeSignature(timeSignature); // Set the time signature
        };
    private:
        bool _returnToZero = true; // Flag to return to zero position
        
};