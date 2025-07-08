#pragma once
#include "component.h"
#include <memory>

class PerformComponent : public Component {
public:
  PerformComponent(
      shared_ptr<class MPCStudioBlackControlSurface> controlSurface)
      : Component(controlSurface) {}

  ~PerformComponent() = default;

  void onActivateComponent() override {}

  void onDeactivateComponent() override {}
};