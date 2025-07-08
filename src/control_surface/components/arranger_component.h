#pragma once
#include "component.h"
#include <memory>

class ArrangerComponent : public Component {
public:
  ArrangerComponent(
      shared_ptr<class MPCStudioBlackControlSurface> controlSurface)
      : Component(controlSurface) {}

  ~ArrangerComponent() = default;

  void onActivateComponent() override {}

  void onDeactivateComponent() override {}
};