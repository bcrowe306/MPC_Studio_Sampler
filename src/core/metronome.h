#pragma once
#include <memory>
#include <functional>
#include "LabSound/LabSound.h"
#include "audio/choc_MIDI.h"
#include "parameter.h"
using std::shared_ptr;
using std::make_shared;



class MetronomeNode {
    public:

        BoolParameter enabled = BoolParameter("Enabled", true, "Enable or disable the metronome");
        FloatParameter volume = FloatParameter("Volume", 0.75f, 0.0f, 6.0f);
        BoolParameter halfBeatEnabled = BoolParameter("Half Beat Enabled", false, "Enable or disable half beat clicks");
        shared_ptr<lab::AudioContext> audioContext; // Audio context for the metronome
        shared_ptr<lab::SampledAudioNode> clickSampleNode;
        shared_ptr<lab::GainNode> clickGainNode;
        shared_ptr<lab::GainNode> clickVelocityNode;
        shared_ptr<lab::AudioBus> sampleBus;
        MetronomeNode(shared_ptr<lab::AudioContext> audioContext) : audioContext(audioContext) {

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

            volume.onValueChanged.connect([this](float newVolume) {
                clickGainNode->gain()->setValue(newVolume);
            });
        }

        ~MetronomeNode() {

            audioContext->disconnect(clickGainNode, clickVelocityNode, 0, 0);
            audioContext->disconnect(audioContext->destinationNode(), clickGainNode, 0, 0);
            audioContext->synchronizeConnections();
        }

        void playClick(float note, float velocity) {
            if(!enabled.getValue()) {
                return; // Do not play if metronome is disabled
            }
            auto baseFreq = choc::midi::noteNumberToFrequency(60); // Middle C frequency
            auto frequency = choc::midi::noteNumberToFrequency(note) / baseFreq; // Calculate frequency based on note
            clickSampleNode->playbackRate()->setValue(frequency);
            clickVelocityNode->gain()->setValue(velocity);
            clickSampleNode->start(0.0);
        }
        void onMetronomeTick(bool isBar, bool isBeat, bool isHalfBeat) {
            // Handle metronome tick events
            
            if(isBar) {
                playClick(67, .75f); // Play bar click at note 67 (G4) with full velocity
            } else if(isBeat) {
                playClick(60, 0.5f); // Play beat click at note 60 (C4) with reduced velocity
            } else if(isHalfBeat && halfBeatEnabled.getValue()) {
                playClick(59, 0.2f); // Play half beat click at note 59 (B3) with lower velocity
            }
        }

    protected:

      void setVolume(float newVolume) {
            volume.setValue(newVolume);
            clickGainNode->gain()->setValue(newVolume);
      }
};