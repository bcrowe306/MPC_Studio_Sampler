#pragma once
#include "component.h"
#include <memory>

class SequenceComponent : public Component {
public:
  SequenceComponent(
      shared_ptr<class MPCStudioBlackControlSurface> controlSurface)
      : Component(controlSurface) {}

  ~SequenceComponent() = default;

  void onActivateComponent() override {}

  void onDeactivateComponent() override {}
};