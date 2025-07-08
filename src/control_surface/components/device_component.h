#pragma once
#include "component.h"
#include <memory>

class DeviceComponent : public Component {
public:
  DeviceComponent(
      shared_ptr<class MPCStudioBlackControlSurface> controlSurface)
      : Component(controlSurface) {}

  ~DeviceComponent() = default;

  void onActivateComponent() override {}

  void onDeactivateComponent() override {}
};