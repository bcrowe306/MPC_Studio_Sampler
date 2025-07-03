// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef METERNODE_H
#define METERNODE_H

#include "util.h"
#include "LabSound/LabSound.h"
#include <array>

using namespace lab;


    class MeterNode : public AudioBasicInspectorNode {


public:

    // Members
    int windowSize = 128;

    // Contructors
    MeterNode(AudioContext &ac);
    virtual ~MeterNode();

    static const char *static_name() { return "Meter"; }
    virtual const char *name() const override { return static_name(); }
    static AudioNodeDescriptor *desc();

    // Methods
    virtual double tailTime(ContextRenderLock &r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock &r) const override {
      return 0;
    }
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;
    std::array<float, 6> rmsDb() { return _rmsDb; }
    std::array<float, 6> db() { return _db; }
    std::array<float, 6> dbLinear();
    std::array<float, 6> rmsDbLinear();

  private:
    std::array<float, 6> _rmsDb = {-72.f, -72.f, -72.f, -72.f, -72.f, -72.f};
    std::array<float, 6> _rmsDbLinear;
    std::array<float, 6> _db = {-72.f, -72.f, -72.f, -72.f, -72.f, -72.f};
    std::array<float, 6> _dbLinear;
    std::array<float, 6> power = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

    float db_decay_factor = .999995f;
    float rms_decay_factor = .999f;
    float dB_min = -72.f;
    float dB_max = 0.f;
    const float kMinPower = 0.000125f;
    float applyDecay(float current_dBFS, float previous_dBFS, float decay_factor);
    float db_to_linear_ratio(float db_val)
    {
      return pow(10.0, db_val / 10.0);
    }

    float linearToUnit(float linearDb)
    {
       return (linearDb + 1.f) / 2;
    }
};





#endif // METERNODE_H
