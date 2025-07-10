#pragma once
#include "audio/choc_MIDI.h"
#include "core/midi_utils.h"
#include "sequence.h"
#include "sigslot/signal.hpp"
#include "core/playhead.h"
#include <iostream>
#include <memory>

using std::shared_ptr;
using std::make_shared;



class Sequencer : public enable_shared_from_this<Sequencer> {
    // Sequencer class to manage multiple sequences and their states
public:
    sigslot::signal<int> onSequenceSelected;
    sigslot::signal<int, ShortMessage &> onMidiOutput; // Signal for MIDI output from the sequencer
    vector<sigslot::connection> sequenceConnections; // Connections for sequence signals
    vector<shared_ptr<Sequence>> sequences; // List of sequences in the sequencer
    
    Sequencer() {
        sequences.reserve(kMaxSequences); // Reserve space for maximum sequences
        _createSequences(); // Create sequences
        selectSequence(0); // Select the first sequence by default
    }
    ~Sequencer() = default;

    void setLQValue(LQ_VALUE value) {
        for(auto &seq : sequences) {
            if (seq) {
                seq->setLQValue(value);
            }
        }
    }

    void setLaunchQuantization(bool enabled) {
        for(auto &seq : sequences) {
            if (seq) {
                seq->setLaunchQuantization(enabled);
            }
        }
    }

    void setInputQuantize(bool enabled) {
        for(auto &seq : sequences) {
            if (seq) {
                seq->inputQuantize = enabled; // Set input quantization for each sequence
            }
        }
    }

    void setQuantizationValue(QUANTIZATION_VALUE value) {
        for(auto &seq : sequences) {
            if (seq) {
                seq->quantizationValue = value; // Set quantization value for each sequence
            }
        }
    }

    void setQuantizationValue(int value) {
        for(auto &seq : sequences) {
            if (seq) {
                seq->quantizationValue = static_cast<QUANTIZATION_VALUE>(value); // Set quantization value for each sequence
            }
        }
    }

    void selectSequence(int sequenceIndex) {
        if (sequenceIndex < 0 || sequenceIndex >= static_cast<int>(sequences.size())) {
            
            return; // Invalid sequence ID
        }
        
        
        if (sequences[sequenceIndex] == nullptr) {
            return; // Sequence is null
        }
        
        for(int i = 0; i < sequences.size(); i++) {
            if(i != sequenceIndex) {
                sequences[i]->setNextState(Sequence::SequenceState::Stopped); // Stop all other sequences
            }else{
                sequences[i]->setNextState(Sequence::SequenceState::Playing); // Set the selected sequence to playing state
                
            }
        }
        _currentSequenceIndex = sequenceIndex;
        onSequenceSelected(_currentSequenceIndex); // Emit signal when a sequence is selected
    }

    // Returns the currently selected sequence
    shared_ptr<Sequence> getSelectedSequence() {
        if (_currentSequenceIndex < 0 || _currentSequenceIndex >= static_cast<int>(sequences.size())) {
            return nullptr; // Invalid sequence ID
        }
        return sequences[_currentSequenceIndex]; // Return the currently selected sequence
    }

    // Returns the index of the currently selected sequence
    int getSelectedSequenceIndex() const {
        return _currentSequenceIndex; // Return the index of the currently selected sequence
    }

    // Selects the next sequence in the list, wrapping around if necessary
    void nextSequence() {
        _currentSequenceIndex = (_currentSequenceIndex + 1) % sequences.size(); // Increment the sequence index
        selectSequence(_currentSequenceIndex); // Select the next sequence
    }

    // Selects the previous sequence in the list, wrapping around if necessary
    void previousSequence() {
        _currentSequenceIndex = (_currentSequenceIndex - 1 + sequences.size()) % sequences.size(); // Decrement the sequence index
        selectSequence(_currentSequenceIndex); // Select the previous sequence
    }

    void onMidiInput(int trackIndex, ShortMessage &msg) {
        if (trackIndex < 0 || trackIndex >= static_cast<int>(sequences.size())) {
            return; // Invalid track index
        }

        // Forward MIDI input to the sequences
        for(auto &seq : sequences) {
            if (seq) {
                seq->onMidiInput(trackIndex, msg); 
            }
        }
    }

    void midiOutFromSequences(int trackIndex, ShortMessage &msg) {
        onMidiOutput(trackIndex, msg); // Forward MIDI output from the sequence to the main output
    }

    void onTick(int ticks) {
        // Tick all sequences
        for (auto &seq : sequences) {
            if (seq) {
                seq->onTick(ticks); // Call the tick method for each sequence
            }
        }
    }

    void onPrecountTick(int precountTick, int precountTickLength){
        // Handle precount tick for all sequences
        for (auto &seq : sequences) {
            if (seq) {
                seq->onPrecountTick(precountTick, precountTickLength); // Call the precount tick method for each sequence
            }
        }
    }

    void onPlayheadStateChanged(Playhead::PlayheadState state){
        if(state == Playhead::PlayheadState::PLAYING) {
            auto seq = getSelectedSequence();
            if (seq) {
                seq->setNextState(Sequence::SequenceState::Playing); // Set the sequence to playing state
            }
        } 
        else if(state == Playhead::PlayheadState::STOPPED) {
            auto seq = getSelectedSequence();
            if (seq) {
                seq->setNextState(Sequence::SequenceState::Playing); // Set the sequence to playing state
                seq->resetToStart();
            }
        }
        else if (state == Playhead::PlayheadState::RECORDING) {
            auto seq = getSelectedSequence();
            if (seq) {
                seq->setNextState(Sequence::SequenceState::Recording); // Set the sequence to recording state
            }
        } 
        else if (state == Playhead::PlayheadState::PRECOUNT) {
            auto seq = getSelectedSequence();
            if (seq) {
                seq->setNextState(
                    Sequence::SequenceState::Recording); // Set the sequence to
                                                         // recording state
            }
        }
    }
    

protected:
    int _currentSequenceIndex; // Index of the currently active sequence
    
    void _createSequences(){
        for(int i = 0; i < kMaxSequences; i++) {
             auto seq = make_shared<Sequence>(i);
             sequenceConnections.push_back(seq->midiOut.connect([this](int trackIndex, ShortMessage &msg) {
                 midiOutFromSequences(trackIndex, msg); // Connect sequence MIDI output to the sequencer
             }));
            
            sequences.push_back(seq);
        }
     }
   

};