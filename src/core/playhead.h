#pragma once
#include "LabSound/LabSound.h"
#include "core/value_receiver.h"
#include "timing.h"
#include <memory>

using std::shared_ptr;

class Playhead{
    public:
        // Signals
        enum class PlayheadState { PLAYING, STOPPED, RECORDING, PRECOUNT };
        
        sigslot::signal<bool, bool, bool> onMetronomeTick; // Signal for metronome ticks
        sigslot::signal<string> onSongPositionDisplayChanged; // Helper Signal for song position display
        sigslot::signal<int> onSongPositionChanged; // Signal for song position changes
        sigslot::signal<int> onTick;
        sigslot::signal<int, int> onPrecountTick;
        sigslot::signal<PlayheadState> onPlayheadStateChanged; // Signal for playhead state changes

        
        VRBool precount = VRBool("precount", true); // Value receiver for precount state
        VRInt precountBars = VRInt("precountBars", 1, 1, 4); // Value receiver for number of bars in precount

        shared_ptr<Timer> timer; // MIDI clock for timing
 

        // Constructor
        Playhead(shared_ptr<Timer> timer) :  timer(timer) {
            // Initialize playhead with the audio context
            
            // Process the tick when the MIDI clock ticks
            timer->onTick.connect([this](int tick) {
                processTick(); 
            });
        };

        ~Playhead() = default;


        void setBPM(double bpm) {
            timer->setBPM(bpm); // Set the BPM for the MIDI clock
        };

        double getBPM() const {
            return timer->getBPM(); // Get the current BPM from the MIDI clock
        };

        void start() {
            setState(PlayheadState::PLAYING); // Set the playhead state to playing
        };

        void stop() {
            setState(PlayheadState::STOPPED); // Set the playhead state to stopped
            
        };

        void record(){
            setState(PlayheadState::RECORDING); // Set the playhead state to recording
        }

        void togglePlaying() {
            if (isPlaying()) {
                stop(); // Stop if currently playing
            } else {
                start(); // Start if currently stopped
            }
        };

        void toggleRecording() {
            if(isRecording()) {
                setState(PlayheadState::PLAYING); // Stop recording if currently recording
            } else {
                setState(PlayheadState::RECORDING); // Start recording if currently stopped
            }
        };

        bool isPlaying() const {
            return _state == PlayheadState::PLAYING || _state == PlayheadState::RECORDING || _state == PlayheadState::PRECOUNT; // Check if the playhead is in playing, recording, or precount state
        };

        bool isRecording () const {
            return _state == PlayheadState::RECORDING || _state == PlayheadState::PRECOUNT;
        }

        void setReturnToZero(bool returnToZero) {
            _returnToZero = returnToZero; // Set the flag to return to zero position
        };

        void setTimeSigNumerator(int numerator) {
            timer->setTimeSignature(numerator, timer->getTimeSignature().denominator); // Set the numerator of the time signature
        };

        void setTimeSigDenominator(int denominator) {
            timer->setTimeSignature(timer->getTimeSignature().numerator, denominator); // Set the denominator of the time signature
        };
        void setTimeSignature(const TimeSignature &timeSignature) {
            timer->setTimeSignature(timeSignature); // Set the time signature
        };

        void setState(PlayheadState settingState) {
            if(settingState == PlayheadState::RECORDING ){
                switch(_state) {
                    case PlayheadState::PLAYING:
                        // If currently playing, switch to recording state
                        _state = PlayheadState::RECORDING;
                        break;
                    case PlayheadState::STOPPED:
                        // If currently stopped, switch to precount if precount is enabled
                        if (precount.getValue()) {
                            _state = PlayheadState::PRECOUNT;
                            _precountTicks = 0; // Reset precount ticks
                        }else{
                            _state = PlayheadState::RECORDING; // Switch to recording state
                        }
                        break;
                    case PlayheadState::RECORDING:
                        // Already in recording state, do nothing
                        return;
                    case PlayheadState::PRECOUNT:
                        _state = settingState; // If in precount, switch to recording state
                        break;
                }
            }
            else if (settingState == PlayheadState::PLAYING){
                if(_state == PlayheadState::PRECOUNT){
                    // ignore the request to switch to playing state from precount
                    return;
                }
                else if(_state == PlayheadState::STOPPED){
                    _state = PlayheadState::PLAYING; // Set the playhead state to playing
                }
                else if(_state == PlayheadState::RECORDING){
                    _state = PlayheadState::PLAYING; // Set the playhead state to playing
                }
            }
            else if(settingState == PlayheadState::STOPPED){
                    _state = PlayheadState::STOPPED;
                    // timer->stop(); // Stop the MIDI clock
                    _ticks = _returnToZero ? 0 : _ticks; // Reset ticks when stopped
            }
            else{
                _state = settingState; // Set the playhead state to the requested state
            }
            onPlayheadStateChanged(_state);
        };

        void playingState(){
            auto _timeSignature = timer->getTimeSignature(); // Get the current time signature
            int ticksPerBar = kTPQN * _timeSignature.numerator;
            int ticksPerBeat = kTPQN / (_timeSignature.denominator / 4);
            int ticksPerSixteenth = ticksPerBar / 16;
            bool isBar = (_ticks % ticksPerBar) == 0;
            bool isBeat = (_ticks % ticksPerBeat) == 0;
            bool isHalfBeat = (_ticks % (ticksPerBeat / 2)) == 0;
            bool is16th = (_ticks % ticksPerSixteenth) == 0;
            int bar = _ticks / ticksPerBar;
            int beat = (_ticks % ticksPerBar) / ticksPerBeat;
            int sixteenthNote = (_ticks % ticksPerBeat) / ticksPerSixteenth;

            if (isHalfBeat) {
                onMetronomeTick(isBar, isBeat, isHalfBeat);
            }

            if (is16th) {
                onSongPositionDisplayChanged(
                    generateDisplayString(bar, beat, sixteenthNote));
            }
            this->onTick(_ticks); // Emit tick signal with current ticks
            ++_ticks;
        }

        void stoppedState(){
            _precountTicks = 0; // Reset precount ticks
        }

        PlayheadState getState() const {
            return _state; // Return the current playhead state
        }

        void recordingState(){
            auto _timeSignature = timer->getTimeSignature(); // Get the current time signature
            int ticksPerBar = kTPQN * _timeSignature.numerator;
            int ticksPerBeat = kTPQN / (_timeSignature.denominator / 4);
            int ticksPerSixteenth = ticksPerBar / 16;
            bool isBar = (_ticks % ticksPerBar) == 0;
            bool isBeat = (_ticks % ticksPerBeat) == 0;
            bool isHalfBeat = (_ticks % (ticksPerBeat / 2)) == 0;
            bool is16th = (_ticks % ticksPerSixteenth) == 0;
            int bar = _ticks / ticksPerBar;
            int beat = (_ticks % ticksPerBar) / ticksPerBeat;
            int sixteenthNote = (_ticks % ticksPerBeat) / ticksPerSixteenth;

            if (isHalfBeat) {
                onMetronomeTick(isBar, isBeat, isHalfBeat);
            }

            if (is16th) {
                onSongPositionDisplayChanged(
                    generateDisplayString(bar, beat, sixteenthNote));
            }
            onTick(_ticks); // Emit tick signal with current ticks
            ++_ticks;
        }

        void precountState(){
            auto _timeSignature = timer->getTimeSignature(); // Get the current time signature
            int ticksPerBar = kTPQN * _timeSignature.numerator;
            int ticksPerBeat = kTPQN / (_timeSignature.denominator / 4);
            int ticksPerSixteenth = ticksPerBar / 16;
            bool isBar = (_precountTicks % ticksPerBar) == 0;
            bool isBeat = (_precountTicks % ticksPerBeat) == 0;
            bool isHalfBeat = (_precountTicks % (ticksPerBeat / 2)) == 0;
            bool is16th = (_precountTicks % ticksPerSixteenth) == 0;
            int bar = _precountTicks / ticksPerBar;
            int beat = (_precountTicks % ticksPerBar) / ticksPerBeat;
            int sixteenthNote = (_precountTicks % ticksPerBeat) / ticksPerSixteenth;

            if (isHalfBeat) {
                onMetronomeTick(isBar, isBeat, isHalfBeat);
            }

            if (is16th) {
                // onSongPositionDisplayChanged(
                //     generateDisplayString(bar, beat, sixteenthNote));
            }

            // Do precount logic
            if(_precountTicks >= (precountBars.getValue() * ticksPerBar) - 1) {
                // If the precount has reached the specified number of bars, switch to recording state
                onPrecountTick(_precountTicks, precountBars.getValue() * ticksPerBar); // Emit precount tick signal
                setState(PlayheadState::RECORDING); // Set the playhead state to recording
                _precountTicks = 0; // Reset precount ticks
            } else {
                onPrecountTick(_precountTicks, precountBars.getValue() * ticksPerBar); // Emit precount tick signal
                _precountTicks++; // Increment ticks during precount
            }
        }

        void processTick(){
            switch(_state) {
                case PlayheadState::PLAYING:
                    playingState(); // Handle playing state
                    break;
                case PlayheadState::STOPPED:
                    stoppedState(); // Handle stopped state
                    break;
                case PlayheadState::RECORDING:
                    recordingState(); // Handle recording state
                    break;
                case PlayheadState::PRECOUNT:
                    precountState(); // Handle precount state
                    break;
            }
        }
        string generateDisplayString(int bar, int beat, int sixteenthNote) {
            // Generate a display string in the format "Bar.Beat.Sixteenth"
            return fmt::format("{:02}.{:02}.{:02}", bar + 1, beat + 1,
                               sixteenthNote + 1);
        }

        void setSongPosition(int bar, int beat, int sixteenthNote) {
            // Update the song position and emit the signal
            auto _timeSignature = timer->getTimeSignature(); // Get the current time signature
            _ticks = (bar * _timeSignature.numerator * kTPQN) + (beat * kTPQN / (_timeSignature.denominator / 4)) + (sixteenthNote * (kTPQN / 16));
            onSongPositionChanged(_ticks);
            onSongPositionDisplayChanged(generateDisplayString(bar, beat, sixteenthNote));
        }

        SongPosition generateSongPosition() const {
            auto _timeSignature = timer->getTimeSignature(); // Get the current time signature
            int ticksPerBar = kTPQN * _timeSignature.numerator;
            int ticksPerBeat = kTPQN / (_timeSignature.denominator / 4);
            int ticksPerSixteenth = ticksPerBeat / 4;

            int bar = _ticks / ticksPerBar;
            int beat = (_ticks % ticksPerBar) / ticksPerBeat;
            int sixteenthNote = (_ticks % ticksPerBeat) / ticksPerSixteenth;

            return {bar, beat, sixteenthNote};
        }

    private:
        int _ticks = 0; // Current tick count
        int _precountTicks = 0; // Ticks during precount
        PlayheadState _state = PlayheadState::STOPPED; // Current state of the playhead
        bool _returnToZero = true; // Flag to return to zero position
        
};