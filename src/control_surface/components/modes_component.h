#pragma once
#include "control_surface/mpc_studio_black_surface.h"
#include "component.h"
#include "sigslot/signal.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>


class ModesComponent : public Component {
  // ModesComponent specific functionality
public:
  sigslot::signal<string> onModeChanged; // Signal emitted when the mode changes
  ModesComponent(shared_ptr<MPCStudioBlackControlSurface> controlSurface, string name) : Component(controlSurface) 
  {
    this->name = name; // Set the name for this component
  }

  ~ModesComponent() override = default;

  void onActivateComponent() override {
    activateCurrentModeComponents();
  };
  void onDeactivateComponent() override {
    deactivateCurrentModeComponents();
  };

  void addMode(const string &modeName,
               const vector<shared_ptr<Component>> &components) {
    _modes[modeName] = components; // Add a new mode with its components
    
  }

  void activateCurrentModeComponents(string modeName = "") {
    // Activate all components in the current mode
    for (const auto &component : _modes[modeName.empty() ? _currentMode : modeName]) {
      component->activate();
    }
  }

  void deactivateCurrentModeComponents(string modeName = "") {
    // Deactivate all components in the current mode
    for (const auto &component : _modes[modeName.empty() ? _currentMode : modeName]) {
      component->deactivate();
    }
  }

  void setMode(const string &modeName) {
    if (modeName == _currentMode) {
      return;
    }

    if (_modes.find(modeName) != _modes.end()) {
      _previousMode = _currentMode; // Store the previous mode
      deactivateCurrentModeComponents();
      _currentMode = modeName;      // Set the current mode
      activateCurrentModeComponents(modeName); // Activate components for the new mode
      onModeChanged(modeName);

    } else {
      std::cerr << "Mode '" << modeName  << "' not found.\n"; // Error if mode does not exist
    }
  }

  string getPreviousMode() const {
    return _previousMode; // Return the previous mode name
  }

  void revertToPreviousMode() {
    if (_previousMode != _currentMode) {
      setMode(_previousMode); // Revert to the previous mode
      _previousMode = "";
    } else {
      std::cerr << "Already in the previous mode: " << _currentMode
                << "\n"; // Error if already in previous mode
    }
  }

  string getCurrentMode() const {
    return _currentMode; // Return the current mode name
  }

protected:
  unordered_map<string, vector<shared_ptr<Component>>> _modes;
  string _currentMode = "default"; // Default mode name
  string _previousMode = ""; // Previous mode name, used for toggling modes
};