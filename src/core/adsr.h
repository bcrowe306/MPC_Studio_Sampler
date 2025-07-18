#pragma once

#include <stdio.h>
#include <string>
#include "LabSound/LabSound.h"
#include "LabSound/extended/FunctionNode.h"
#include <memory>

enum envState { env_idle = 0, env_attack, env_decay, env_sustain, env_release };

class ADSR {
public:
  enum class ADSRMode {
    ONESHOT, // One-shot mode
    ADSR     // ADSR mode
  };
  std::shared_ptr<lab::AudioContext> audioContext; // Audio context for the ADSR
  std::shared_ptr<lab::FunctionNode> functionNode; // Function node for processing

  ADSR(std::shared_ptr<lab::AudioContext> audioContext);
  ~ADSR();
  void processFunction(lab::ContextRenderLock &r, lab::FunctionNode *me, int channel, float *buffer, int bufferSize);
  float process(void);
  float process(int sampleCount);
  float getOutput(void);
  int getState(void);
  void gate(int on);
  void setAttackRate(float rate);
  void setDecayRate(float rate);
  void setReleaseRate(float rate);
  void setSustainLevel(float level);
  void setTargetRatioA(float targetRatio);
  void setTargetRatioDR(float targetRatio);
  void setMode(ADSRMode mode);
  ADSRMode getMode() const { return mode; } // Get the current ADSR mode
  void setAll(float attackSeconds, float decaySeconds, float sustainLevel, float releaseSeconds);

  // Set attack, decay, and release times in seconds
  void setAttackTimeSeconds(float seconds);
  void setDecayTimeSeconds(float seconds);
  void setReleaseTimeSeconds(float seconds);
  void resetEnvelope();

protected:
  ADSRMode mode = ADSRMode::ADSR; // ADSR mode (One Shot or ADSR)
  int state;
  float sampleRate;
  float output;
  float attackRate;
  float decayRate;
  float releaseRate;
  float attackCoef;
  float decayCoef;
  float releaseCoef;
  float sustainLevel;
  float targetRatioA;
  float targetRatioDR;
  float attackBase;
  float decayBase;
  float releaseBase;
  std::string name;
  float calcCoef(float rate, float targetRatio);
};

inline float ADSR::process() {
  switch (state) {
  case env_idle:
    break;

  case env_attack:
    output = attackBase + output * attackCoef;
    if (output >= 1.0) {
      output = 1.0;

      // Transition to decay or sustain state based on mode
      if(mode == ADSRMode::ADSR) {
        state = env_decay; // Transition to decay state
      } else {
        state = env_sustain; // Transition to sustain state
      }
    }
    break;

  case env_decay:
    output = decayBase + output * decayCoef;
    if (output <= sustainLevel) {
      output = sustainLevel;
      state = env_sustain;
    }
    break;

  case env_sustain:
    break;

  case env_release:
    if(mode == ADSRMode::ONESHOT) {
      state = env_idle; // Transition to idle state if in one-shot mode
      break;
    }
    
    output = releaseBase + output * releaseCoef;
    if (output <= 0.0) {
      output = 0.0;
      state = env_idle;
    }
    break;
  }
  return output;
}

inline float ADSR::process(int sampleCount) {
  float retVal = 0;

  if (state != env_idle) {
    for (int i = 0; i < sampleCount; i++)
      retVal = process();
  }

  return retVal;
}
inline void ADSR::setAll(float attackSeconds, float decaySeconds, float sustainLevel, float releaseSeconds) {
  setAttackTimeSeconds(attackSeconds);
  setDecayTimeSeconds(decaySeconds);
  setSustainLevel(sustainLevel);
  setReleaseTimeSeconds(releaseSeconds);
}
inline void ADSR::gate(int gate) {

  if (gate)
    state = env_attack;
  else if (state != env_idle)
    state = env_release;
}

inline int ADSR::getState() { return state; }

inline void ADSR::resetEnvelope() {
  state = env_idle;
  output = 0.0;
}

inline float ADSR::getOutput() { return output; }