#pragma once
#include <memory>
#include <functional>
#include "LabSound/LabSound.h"
#include "audio/choc_MIDI.h"
#include "util.h"
#include "core/playhead.h"
using std::shared_ptr;
using std::make_shared;



class MetronomeNode {
    public:

       
        shared_ptr<lab::AudioContext> audioContext; // Audio context for the metronome
        shared_ptr<lab::SampledAudioNode> clickSampleNode;
        shared_ptr<lab::GainNode> clickGainNode;
        shared_ptr<lab::GainNode> clickVelocityNode;
        shared_ptr<lab::AudioBus> sampleBus;
        shared_ptr<Playhead> playhead; // Playhead for the metronome

        MetronomeNode(shared_ptr<lab::AudioContext> audioContext, shared_ptr<Playhead> playhead) : audioContext(audioContext), playhead(playhead) {

            clickSampleNode = make_shared<lab::SampledAudioNode>(*audioContext.get());
            clickGainNode = make_shared<lab::GainNode>(*audioContext.get());
            clickVelocityNode = make_shared<lab::GainNode>(*audioContext.get());
            clickSampleNode->schedule(0.0);
            sampleBus = lab::MakeBusFromFile("assets/BVKER - The Astro Perc 08.wav", false);
            clickSampleNode->setBus(sampleBus);
            audioContext->connect(clickVelocityNode, clickSampleNode, 0, 0);
            audioContext->connect(clickGainNode, clickVelocityNode, 0, 0);
            audioContext->connect(audioContext->destinationNode(), clickGainNode, 0, 0);
            audioContext->synchronizeConnections();

            
        }

        ~MetronomeNode() {

            audioContext->disconnect(clickGainNode, clickVelocityNode, 0, 0);
            audioContext->disconnect(audioContext->destinationNode(), clickGainNode, 0, 0);
            audioContext->synchronizeConnections();
        }

        void playClick(float note, float velocity) {
            
            auto baseFreq = choc::midi::noteNumberToFrequency(60); // Middle C frequency
            auto frequency = choc::midi::noteNumberToFrequency(note) / baseFreq; // Calculate frequency based on note
            clickSampleNode->playbackRate()->setValue(frequency);
            clickVelocityNode->gain()->setValue(velocity);
            clickSampleNode->start(0.0);
        }
        void onMetronomeTick(bool isBar, bool isBeat, bool isHalfBeat) {
            // Handle metronome tick events
            bool shouldPlay = false;
            if(playhead->getState() == Playhead::PlayheadState::PRECOUNT) {
                shouldPlay = true;
            } 
            else{
                if(!_enabled){
                    shouldPlay = false; // Do not play clicks if metronome is disabled
                }
                else{
                    shouldPlay = true;
                }
            }

            if(shouldPlay == false) return; // If not enabled, do not play clicks

            if(isBar) {
                playClick(67, .75f); // Play bar click at note 67 (G4) with full velocity
            } else if(isBeat) {
                playClick(60, 0.5f); // Play beat click at note 60 (C4) with reduced velocity
            } else if(isHalfBeat && _halfBeatEnabled) {
                playClick(59, 0.2f); // Play half beat click at note 59 (B3) with lower velocity
            }
        }

        void setVolume(float newVolume) {
            _volume = newVolume;
            clickGainNode->gain()->setValue(newVolume);
        }

        void setVolumeDb(float newVolumeDb) {
            std::cout << "Setting metronome volume to " << newVolumeDb << " dB" << std::endl;
            _volume = dBToLinear(newVolumeDb);
            clickGainNode->gain()->setValue(_volume);
        }
        void setEnabled(bool enabled) {
            _enabled = enabled;
        }
        void setHalfBeatEnabled(bool enabled) {
            _halfBeatEnabled = enabled;
        }
        void toggleEnabled() {
            _enabled = !_enabled; // Toggle the enabled state
        }

    protected:
        bool _enabled = true; // Metronome enabled state
        bool _halfBeatEnabled = false; // Half beat enabled state
        float _volume = 1.0f; // Volume of the metronome
      
};