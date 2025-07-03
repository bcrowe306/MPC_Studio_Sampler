#include "LabSound/LabSound.h"
#include "meter_node.h"
#include <algorithm>

using namespace lab;



static AudioSettingDescriptor s_meterNodeSetting[] = {{nullptr}};

AudioNodeDescriptor *MeterNode::desc() {
  static AudioNodeDescriptor d{nullptr, s_meterNodeSetting, 1};
  return &d;
}

MeterNode::MeterNode(AudioContext &ac) : AudioBasicInspectorNode(ac, *desc()) {
  initialize();
}

MeterNode::~MeterNode() { uninitialize(); }

void MeterNode::process(ContextRenderLock &r, int bufferSize) {
  AudioBus *outputBus = output(0)->bus(r);
  AudioBus *inputBus = input(0)->bus(r);

  if (!isInitialized() || !input(0)->isConnected() || !inputBus) {
    if (outputBus)
      outputBus->zero();
    return;
  }

  // calculate the power of this buffer; Uses preallocated array<float, 6> for simultaneously calculating up to 6 channels of db and rms.
  {
    int start = static_cast<int>(bufferSize) - static_cast<int>(windowSize);
    int end = static_cast<int>(bufferSize);
    if (start < 0)
      start = 0;

    int numberOfChannels = inputBus->numberOfChannels();
    for (int c = 0; c < numberOfChannels; ++c) {
      const float *data = inputBus->channel(c)->data();
      for (int i = start; i < end; ++i) {
        if(numberOfChannels <= _rmsDb.size()){
          float p = data[i];
          float abs_sample = std::max(std::abs(p), std::numeric_limits<float>::min());
          float new_db = 20.0f * log10f(abs_sample);
          _db[c] = applyDecay(new_db, _db[c], db_decay_factor);
          power[c] += p * p;
        }
        
      }
    }
    for (size_t i = 0; i < power.size(); i++)
    {
      auto &p = power[i];
      float rms = sqrtf(p / bufferSize);
      if (std::isinf(p) || std::isnan(p) || p < kMinPower)
        p = kMinPower;
      rms = std::max(rms, std::numeric_limits<float>::min());
      float new_rmsDb = 20.0f * logf(rms) / logf(10.0f);
      _rmsDb[i] = applyDecay(new_rmsDb, _rmsDb[i], rms_decay_factor);
    }

    // Reset power to 0;
    power.fill(0.f);
  }
  // to here

  if (inputBus != outputBus) {
    outputBus->copyFrom(*inputBus);
  }
}

void MeterNode::reset(ContextRenderLock &) {}
float MeterNode::applyDecay(float current_dBFS, float previous_dBFS, float decay_factor)
{
  float decayed_dBFS;
  if (current_dBFS < previous_dBFS)
  {
    decayed_dBFS= previous_dBFS *decay_factor + current_dBFS * (1.0f - decay_factor);
  }else{
    decayed_dBFS = current_dBFS;
  }
  return std::max(decayed_dBFS, dB_min);
}

std::array<float, 6> MeterNode::dbLinear()
{
  for (size_t i = 0; i < _rmsDb.size(); i++)
  {
    _dbLinear[i] = db_to_linear_ratio(_db[i]);
  }
  return _dbLinear;
};

std::array<float, 6> MeterNode::rmsDbLinear() {
  for (size_t i = 0; i < _rmsDb.size(); i++)
  {
    _rmsDbLinear[i] = db_to_linear_ratio(_rmsDb[i]);
  }
  return _rmsDbLinear;
};


