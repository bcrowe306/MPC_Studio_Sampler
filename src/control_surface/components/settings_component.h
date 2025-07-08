#pragma once
#include "component.h"
#include <memory>

class SettingsComponent : public Component {
public:
  SettingsComponent(
      shared_ptr<class MPCStudioBlackControlSurface> controlSurface)
      : Component(controlSurface) {}

  ~SettingsComponent() = default;

  void onActivateComponent() override {}

  void onDeactivateComponent() override {}
};