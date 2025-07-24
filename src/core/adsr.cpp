
#include "adsr.h"
#include <iostream>
#include <math.h>

  

  ADSR::ADSR(std::shared_ptr<lab::AudioContext> audioContext)  : audioContext(audioContext) 
  {

    sampleRate = audioContext->sampleRate();
    functionNode = std::make_shared<lab::FunctionNode>(*audioContext.get());
    functionNode->start(0.0);
    functionNode->setFunction([this](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
      processFunction(r, me, channel, buffer, bufferSize); // Process the ADSR function
    });

    resetEnvelope();
    setAttackRate(0);
    setDecayRate(0);
    setReleaseRate(0);
    setSustainLevel(1.f);
    setTargetRatioA(0.3);
    setTargetRatioDR(0.0001);
    setRetriggerRate(sampleRate * 0.01); // Set default retrigger rate to 10 ms
  }

  ADSR::~ADSR() {
    std::cout << "ADSR Destructor called" << std::endl; // Debug output for destructor
  }

void ADSR::processFunction(lab::ContextRenderLock &r, lab::FunctionNode *me, int channel, float *buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; ++i) {
        float amp = process();
          buffer[i] = amp;
    }
}

void ADSR::setMode(ADSRMode mode) {
  this->mode = mode;
}

void ADSR::setAttackRate(float rate) {
  attackRate = rate;
  attackCoef = calcCoef(rate, targetRatioA);
  attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
}

void ADSR::setDecayRate(float rate) {
  decayRate = rate;
  decayCoef = calcCoef(rate, targetRatioDR);
  decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
}

void ADSR::setReleaseRate(float rate) {
  releaseRate = rate;
  releaseCoef = calcCoef(rate, targetRatioDR);
  releaseBase = -targetRatioDR * (1.f - releaseCoef);
}

void ADSR::setRetriggerRate(float rate) {
  retriggerRate = rate;
  retriggerCoef = calcCoef(rate, targetRatioDR);
  retriggerBase = -targetRatioDR * (1.f - retriggerCoef);
 
}

float ADSR::calcCoef(float rate, float targetRatio) {
  return expf(-logf((1.f + targetRatio) / targetRatio) / rate);
}

void ADSR::setSustainLevel(float level) {
  sustainLevel = level;
  decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
}

void ADSR::setAttackTimeSeconds(float seconds) {
  setAttackRate(seconds * sampleRate);
}

void ADSR::setDecayTimeSeconds(float seconds) {
  setDecayRate(seconds * sampleRate);
}

void ADSR::setReleaseTimeSeconds(float seconds) {
  setReleaseRate(seconds * sampleRate);
}

void ADSR::setTargetRatioA(float targetRatio) {
  if (targetRatio < 0.000000001)
    targetRatio = 0.000000001; // -180 dB
  targetRatioA = targetRatio;
  attackBase = (1.f + targetRatioA) * (1.f - attackCoef);
}

void ADSR::setTargetRatioDR(float targetRatio) {
  if (targetRatio < 0.000000001)
    targetRatio = 0.000000001; // -180 dB
  targetRatioDR = targetRatio;
  decayBase = (sustainLevel - targetRatioDR) * (1.f - decayCoef);
  releaseBase = -targetRatioDR * (1.f - releaseCoef);
}