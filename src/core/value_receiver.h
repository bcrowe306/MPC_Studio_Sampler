#pragma once
#include "core/command.h"
#include <concepts>
#include <cstddef>
#include <string>

template <typename T>
class ValueReceiver : public IReceiver,
                      public enable_shared_from_this<ValueReceiver<T>> {
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<T> onValueChanged;
  ValueReceiver(string name, T initialValue,
                shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), undoManager_(undoManager) {
    _name = name;
  }

  void updateObservers() {
    // Notify observers of the value change
    onValueChanged(_value);
  }

  void setValue(T newValue) {
    if (_value != newValue) {

      if (undoManager_) {
        // Create a command to undo the change
        auto command = std::make_shared<Command>(
            this->shared_from_this(), UUIDToString(_id), _value, newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue);
      }
    }
  }

  T getValue() const { return any_cast<T>(_value); }

  void incrementValue(bool isCoarse) {
    
  }

  void decrementValue(bool isCoarse) {
   
  }

  void doAction(any value) override {
    // cast to T and set the value
    if (value.type() == typeid(T)) {
      _value = any_cast<T>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }
  virtual void serialize(YAML::Emitter &out) {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;
  };

  virtual void deserialize(YAML::Node &yaml) {
    if (yaml.IsScalar()) {
      _value = yaml.as<T>();
      onValueChanged(_value);

    } else if (yaml.IsMap()) {
      if (yaml["value"]) {
        _value = yaml["value"].as<T>();
        onValueChanged(_value);
      }
      if (yaml["id"]) {
        auto idStr = yaml["id"].as<string>();
        _id = GenerateFromString(idStr);
      }
    } else {
      throw std::runtime_error("Invalid YAML node for ValueReceiver");
    }
  };

  void printSerialization() {
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    this->serialize(out);
    std::cout << out.c_str() << std::endl;
  }

protected:
  uuids::uuid _id = generateUUID();
  string _name = "";
  T _value;
  shared_ptr<UndoManager> undoManager_;
};



// Specialization for bool type
// This specialization is needed because bool is not a numeric type and has different behavior
template <>
class ValueReceiver<bool> : public IReceiver,
                      public enable_shared_from_this<ValueReceiver<bool>> {
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<bool> onValueChanged;
  ValueReceiver(string name, bool initialValue,
                shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), undoManager_(undoManager) {
    _name = name;
  }


  void updateObservers() {
    // Notify observers of the value change
    onValueChanged(_value);
  }

  void setValue(bool newValue) {
    if (_value != newValue) {

      if (undoManager_) {
        // Create a command to undo the change
        auto command = std::make_shared<Command>(
            this->shared_from_this(), UUIDToString(_id), _value, newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue);
      }
    }
  }

  bool getValue() const { return any_cast<bool>(_value); }

  void incrementValue(bool isCoarse) {
    // For boolean values, incrementing or decrementing doesn't make sense,
    // but we can toggle the value instead.
    setValue(!_value);
  }

  void decrementValue(bool isCoarse) {
    // For boolean values, decrementing or incrementing doesn't make sense,
    // but we can toggle the value instead.
    setValue(!_value);
  }

  void doAction(any value) override {
    // cast to T and set the value
    if (value.type() == typeid(bool)) {
      _value = any_cast<bool>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }
  virtual void serialize(YAML::Emitter &out) {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;
  };

  virtual void deserialize(YAML::Node &yaml) {
    if (yaml.IsScalar()) {
      _value = yaml.as<bool>();
      onValueChanged(_value);

    } else if (yaml.IsMap()) {
      if (yaml["value"]) {
        _value = yaml["value"].as<bool>();
        onValueChanged(_value);
      }
      if (yaml["id"]) {
        auto idStr = yaml["id"].as<string>();
        _id = GenerateFromString(idStr);
      }
    } else {
      throw std::runtime_error("Invalid YAML node for ValueReceiver");
    }
  };

  void printSerialization() {
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    this->serialize(out);
    std::cout << out.c_str() << std::endl;
  }

protected:
  uuids::uuid _id = generateUUID();
  string _name = "";
  bool _value;
  shared_ptr<UndoManager> undoManager_;
};

// Concepts for Numeric Types ==================

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <Numeric T>
class ValueReceiver<T> : public IReceiver,
                      public enable_shared_from_this<ValueReceiver<T>> {
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<T> onValueChanged;
  ValueReceiver(
    string name, T initialValue,
    T min,
    T max,
    T coarseStep = 0.01,
    T fineStep = 0.001,
    shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), _min(min), _max(max), _coarseStep(coarseStep), _fineStep(fineStep), undoManager_(undoManager) {
    _name = name;
  }

  void updateObservers() {
    // Notify observers of the value change
    onValueChanged(_value);
  }

  void setValue(T newValue) {
    if (_value != newValue) {

      if (undoManager_) {
        // Create a command to undo the change
        auto command = std::make_shared<Command>(
            this->shared_from_this(), UUIDToString(_id), _value, newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue);
      }
    }
  }

  T getValue() const { return any_cast<T>(_value); }

  void doAction(any value) override {
    // cast to T and set the value
    if (value.type() == typeid(T)) {
      _value = any_cast<T>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }

  void incrementValue(bool isCoarse) {
    T step = isCoarse ? _coarseStep : _fineStep;
    T newValue = _value + step;
    if (newValue > _max) {
      newValue = _max;
    } else if (newValue < _min) {
      newValue = _min;
    }
    setValue(newValue);
  }

  void decrementValue(bool isCoarse) {
    T step = isCoarse ? _coarseStep : _fineStep;
    T newValue = _value - step;
    if (newValue > _max) {
      newValue = _max;
    } else if (newValue < _min) {
      newValue = _min;
    }
    setValue(newValue);
  }

  virtual void serialize(YAML::Emitter &out) {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "min";
    out << YAML::Value << this->_min;

    out << YAML::Key << "max";
    out << YAML::Value << this->_max;

    out << YAML::Key << "coarseStep";
    out << YAML::Value << this->_coarseStep;

    out << YAML::Key << "fineStep";
    out << YAML::Value << this->_fineStep;

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;
  };

  virtual void deserialize(YAML::Node &yaml) {
    if (yaml.IsScalar()) {
      _value = yaml.as<T>();
      onValueChanged(_value);

    } else if (yaml.IsMap()) {
      if (yaml["value"]) {
        _value = yaml["value"].as<T>();
        onValueChanged(_value);
      }
      if (yaml["id"]) {
        auto idStr = yaml["id"].as<string>();
        _id = GenerateFromString(idStr);
      }
      if (yaml["min"]) {
        _min = yaml["min"].as<T>();
      }
      if (yaml["max"]) {
        _max = yaml["max"].as<T>();
      }
      if (yaml["coarseStep"]) {
        _coarseStep = yaml["coarseStep"].as<T>();
      }
      if (yaml["fineStep"]) {
        _fineStep = yaml["fineStep"].as<T>();
      }
    } else {
      throw std::runtime_error("Invalid YAML node for ValueReceiver");
    }
  };

  void printSerialization() {
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    this->serialize(out);
    std::cout << out.c_str() << std::endl;
  }

protected:
  T _min;
  T _max;
  T _coarseStep;
  T _fineStep;
  uuids::uuid _id = generateUUID();
  string _name = "";
  T _value;
  shared_ptr<UndoManager> undoManager_;
};

template <typename T>
class ValueOptionsReceiver : public IReceiver,
                      public enable_shared_from_this<ValueOptionsReceiver<T>> {
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<T> onValueChanged;
  ValueOptionsReceiver(
    string name, 
    T initialValue,
    vector<T> options,
    vector<string> optionNames,
    shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), _options(options), _optionNames(optionNames), undoManager_(undoManager) {
    _name = name;
  }

  void setValue(T newValue) {
    if (_value != newValue) {

      if (undoManager_) {
        // Create a command to undo the change
        auto command = std::make_shared<Command>(
            this->shared_from_this(), UUIDToString(_id), _value, newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue);
      }
    }
  }

  T getValue() const { return any_cast<T>(_value); }

  size_t getOptionCount() const {
    return _options.size();
  }

  vector<T> *getOptions() const {
    return const_cast<vector<T> *>(&_options);
  }

  vector<string> *getOptionNames() const {
    return const_cast<vector<string> *>(&_optionNames);
  }

  size_t getSelectedOptionIndex() const {
    auto it = std::find(_options.begin(), _options.end(), _value);
    if (it != _options.end()) {
      return std::distance(_options.begin(), it);
    }
    return -1; // Not found
  }

  void incrementValue(bool isCoarse) {
    size_t currentIndex = getSelectedOptionIndex();
    if (currentIndex == -1) return; // Value not found in options

    size_t nextIndex = (currentIndex + 1) % _options.size(); // Wrap around
    setValue(_options[nextIndex]);
  }

  void decrementValue(bool isCoarse) {
    size_t currentIndex = getSelectedOptionIndex();
    if (currentIndex == -1) return; // Value not found in options

    size_t prevIndex = (currentIndex - 1 + _options.size()) % _options.size(); // Wrap around
    setValue(_options[prevIndex]);
  }

  void updateObservers() {
    // Notify observers of the value change
    onValueChanged(_value);
  }

  void doAction(any value) override {
    // cast to T and set the value
    if (value.type() == typeid(T)) {
      _value = any_cast<T>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }

  
  virtual void serialize(YAML::Emitter &out) {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;

    out << YAML::Key << "options";
    out << YAML::Value << YAML::BeginSeq;
    for (const auto &option : _options) {
      out << option;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "optionNames";
    out << YAML::Value << YAML::BeginSeq;
    for (const auto &name : _optionNames) {
      out << name;
    }
    out << YAML::EndSeq;
  };

  virtual void deserialize(YAML::Node &yaml) {
    if (yaml.IsScalar()) {
      _value = yaml.as<T>();
      onValueChanged(_value);

    } else if (yaml.IsMap()) {
      if (yaml["value"]) {
        _value = yaml["value"].as<T>();
        onValueChanged(_value);
      }
      if (yaml["id"]) {
        auto idStr = yaml["id"].as<string>();
        _id = GenerateFromString(idStr);
      }
      if (yaml["options"]) {
        _options.clear();
        for (const auto &option : yaml["options"]) {
          _options.push_back(option.as<T>());
        }
      }
      if (yaml["optionNames"]) {
        _optionNames.clear();
        for (const auto &name : yaml["optionNames"]) {
          _optionNames.push_back(name.as<string>());
        }
      }
    } else {
      throw std::runtime_error("Invalid YAML node for ValueReceiver");
    }
  };

  void printSerialization() {
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    this->serialize(out);
    std::cout << out.c_str() << std::endl;
  }

protected:
  uuids::uuid _id = generateUUID();
  vector<T> _options; // List of options for the receiver
  vector<string> _optionNames; // Names for each option
  string _name = "";
  T _value;
  shared_ptr<UndoManager> undoManager_;
};

typedef ValueReceiver<std::string> VRString;
typedef ValueReceiver<bool> VRBool;
typedef ValueReceiver<float> VRFloat;
typedef ValueReceiver<int> VRInt;
