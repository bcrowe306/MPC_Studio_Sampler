#pragma once
#include "core/command.h"
#include "fmt/format.h"
#include "yaml-cpp/yaml.h"
#include <algorithm>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <string>

class ValueReceiverBase{
public:
    enum class ValueType {
        Int,
        Float,
        Double,
        String,
        Bool,
        Option,
    };
    ValueReceiverBase(std::string name) : _name(name) {
        _displayName = name; // Set the display name to the same as the name by default
    }
    virtual ~ValueReceiverBase() = default;
    virtual void serialize(YAML::Emitter &out) = 0;
    virtual void deserialize(YAML::Node &yaml) = 0;
    virtual void incrementValue(bool isCoarse) = 0;
    virtual void decrementValue(bool isCoarse) = 0;
    
    // Default implementation, can be overridden
    virtual float getUnitValue () const {
        return 0.0f; 
    }
    
    // Default implementation, can be overridden
    virtual std::string getStringValue() const {
      return ""; 
    }
    std::string getName() const {
        return _name;
    };
    void setName(const std::string &name) {
        _name = name;
    };

    uuids::uuid getId() const {
        return _id;
    };
    
    std::string getDisplayName() const {
        return _displayName;
    };
    void setDisplayName(const std::string &displayName) {
        _displayName = displayName;
    };
    ValueType getValueType() const {
        return _valueType;
    };

protected:
    uuids::uuid _id = generateUUID();
    std::string _name = "";
    std::string _displayName = ""; 
    ValueType _valueType = ValueType::Float; // Default value type
};


// Template class for value receivers that can handle different types of values

template <typename T>
class ValueReceiver : public IReceiver, public enable_shared_from_this<ValueReceiver<T>>,  public ValueReceiverBase 
{
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<T> onValueChanged;
  ValueReceiver(string name, T initialValue,
                shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), undoManager_(undoManager), ValueReceiverBase(name) {
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
        auto command = std::make_shared<Command>( this->shared_from_this(), UUIDToString(_id), _value, newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue, false); // Execute the action directly if no undo manager is present
      }
    }
  }

  // Override the = operator to set the value and emit signals
  ValueReceiver<T> &operator=(const T &value) {
    setValue(value); // Set the value and emit signals
    return *this; // Return the current instance
  }


  // Overload the copy assignment operator
  ValueReceiver<T> &operator=(const ValueReceiver<T> &other) {
    if (this != &other) {
      setValue(other.getValue()); // Set the value and emit signals
    }
    return *this; // Return the current instance
  }

  T getValue() const { return any_cast<T>(_value); }

  void incrementValue(bool isCoarse) override {
    
  }

  void decrementValue(bool isCoarse) override {
   
  }

  void doAction(any value, bool undo) override {
    // cast to T and set the value
    if (value.type() == typeid(T)) {
      _value = any_cast<T>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }
  void serialize(YAML::Emitter &out) override {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;

    out << YAML::EndMap;
  };

  void deserialize(YAML::Node &node) override {
    YAML::Node yaml = node[_name];
    if (!yaml) {
      std::cerr << "Invalid YAML node for ValueReceiver: " << _name
                << std::endl;
      return;
    }
    if (yaml.IsScalar()) {
      _value = yaml.as<T>();
      onValueChanged(_value);

    } 
    else if (yaml.IsMap()) {
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
class ValueReceiver<bool> : public IReceiver, public enable_shared_from_this<ValueReceiver<bool>>, public ValueReceiverBase 
{
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<bool> onValueChanged;
  ValueReceiver(string name, bool initialValue, shared_ptr<UndoManager> undoManager = nullptr) : undoManager_(undoManager) , ValueReceiverBase(name) 
  {
    _value.store(initialValue, memory_order_relaxed); // Use atomic for thread safety
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
            this->shared_from_this(), UUIDToString(_id), _value.load(memory_order_relaxed), newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue, false);
      }
    }
  }

  // Override the = operator to set the value and emit signals
  ValueReceiver<bool> &operator=(const bool &value) {
    setValue(value); // Set the value and emit signals
    return *this; // Return the current instance
  }

  bool getValue() const { return any_cast<bool>(_value.load(memory_order_relaxed)); }

  void incrementValue(bool isCoarse) override {
    // For boolean values, incrementing or decrementing doesn't make sense,
    // but we can toggle the value instead.
    setValue(true);
  }

  void decrementValue(bool isCoarse) override {
    // For boolean values, decrementing or incrementing doesn't make sense,
    // but we can toggle the value instead.
    setValue(false);
  }

  float getUnitValue() const override {
    // Convert boolean to float (0.0 for false, 1.0 for true)
    return _value.load(memory_order_relaxed) ? 1.0f : 0.0f;
  }

  std::string getStringValue() const override {
    // Convert boolean to string
    return _value.load(memory_order_relaxed) ? "on" : "off";
  }

  void doAction(any value, bool undo) override {
    // cast to T and set the value
    if (value.type() == typeid(bool)) {
      _value.store(any_cast<bool>(value));
      onValueChanged(_value.load(memory_order_relaxed));
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }
  virtual void serialize(YAML::Emitter &out) override {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value.load(memory_order_relaxed);
    out << YAML::EndMap;
  };

  virtual void deserialize(YAML::Node &node) override {
    YAML::Node yaml = node[_name];
    if (!yaml) {
      std::cerr << "Invalid YAML node for ValueReceiver: " << _name
                << std::endl;
      return;
    }
    if (yaml.IsScalar()) {
      _value.store(yaml.as<bool>(), memory_order_relaxed);
      onValueChanged(_value.load(memory_order_relaxed));

    } else if (yaml.IsMap()) {
      if (yaml["value"]) {
        _value.store(yaml["value"].as<bool>(), memory_order_relaxed);
        onValueChanged(_value.load(memory_order_relaxed));
      }
      if (yaml["id"]) {
        auto idStr = yaml["id"].as<string>();
        _id = GenerateFromString(idStr);
      }
    } else {
      throw std::runtime_error("Invalid YAML node for ValueReceiver");
    }
  };


protected:
  uuids::uuid _id = generateUUID();
  string _name = "";
  atomic<bool> _value;
  shared_ptr<UndoManager> undoManager_;
};

// Concepts for Numeric Types ==================

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <Numeric T>
class ValueReceiver<T> : public IReceiver,  public enable_shared_from_this<ValueReceiver<T>>, public ValueReceiverBase 
{
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<T> onValueChanged;
  ValueReceiver(
    string name, T initialValue,
    T min,
    T max,
    T coarseStep,
    T fineStep,
    shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), _min(min), _max(max), _coarseStep(coarseStep), _fineStep(fineStep), undoManager_(undoManager), ValueReceiverBase(name) 
  {
    _name = name;
  }

  void updateObservers() {
    // Notify observers of the value change
    onValueChanged(_value);
  }

  float getUnitValue() const override {
    // Convert the value to a unit value (0.0 to 1.0)
    if (_max == _min) return 0.0f; // Avoid division by zero
    return ((float)_value - (float)_min) / ((float)_max - (float)_min);
  }

  string getStringValue() const override {
    // Convert the value to a string representation
    return fmt::format("{}", _value);
  }

  void setValue(T newValue) {
    if (_value != newValue) {
      // Clamp the new value to the min and max range


      // No Clamp
      if(_min > _max){
        // New value is assumed to be a valid value
      }
      else if(_max == 0){
        newValue = std::max(newValue, _min); // Only clamp to min if max is 0
      }
      else {
        newValue = std::clamp(newValue, _min, _max); // Clamp to both min and max
      }

      if (undoManager_) {
        // Create a command to undo the change
        auto command = std::make_shared<Command>(
            this->shared_from_this(), UUIDToString(_id), _value, newValue);
        undoManager_->executeCommand(command);
      } else {
        doAction(newValue, false);
      }
    }
  }

  // Override the = operator to set the value and emit signals
  ValueReceiver<T> &operator=(const T &value) {
    setValue(value); // Set the value and emit signals
    return *this; // Return the current instance
  }

  T getValue() const { return any_cast<T>(_value); }

  void doAction(any value, bool undo) override {
    // cast to T and set the value
    if (value.type() == typeid(T)) {
      _value = any_cast<T>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }

  void incrementValue(bool isCoarse) override {
    T step = isCoarse ? _coarseStep : _fineStep;
    T newValue = _value + step;
    if (newValue > _max) {
      newValue = _max;
    } else if (newValue < _min) {
      newValue = _min;
    }
    setValue(newValue);
  }

  void decrementValue(bool isCoarse) override {
    T step = isCoarse ? _coarseStep : _fineStep;
    T newValue = _value - step;
    if (newValue > _max) {
      newValue = _max;
    } else if (newValue < _min) {
      newValue = _min;
    }
    setValue(newValue);
  }

  void setMin(T min) {
    _min = min;
    if (_value < _min) {
      _value = _min; // Clamp the current value to the new min
      onValueChanged(_value);
    }
  }

  void setMax(T max) {
    _max = max;
    if (_value > _max) {
      _value = _max; // Clamp the current value to the new max
      onValueChanged(_value);
    }
  }

  virtual void serialize(YAML::Emitter &out) override {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;
    out << YAML::EndMap;
  };

  void deserialize(YAML::Node &node) override {
    YAML::Node yaml = node[_name];
    if (!yaml) {
      std::cerr << "Invalid YAML node for ValueReceiver: " << _name << std::endl;
      return;
    }

    if (yaml.IsScalar()) {
      _value = yaml.as<T>();
      onValueChanged(_value);

    } 
    else if (yaml.IsMap()) {
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
class ValueOptionsReceiver : public IReceiver, public enable_shared_from_this<ValueOptionsReceiver<T>>, public ValueReceiverBase 
{
  using Command = ::Command; // Use the Command class defined above
public:
  sigslot::signal<T> onValueChanged;
  ValueOptionsReceiver(
    string name, 
    T initialValue,
    vector<T> options,
    vector<string> optionNames,
    shared_ptr<UndoManager> undoManager = nullptr)
      : _value(initialValue), _options(options), _optionNames(optionNames), undoManager_(undoManager), ValueReceiverBase(name) 
  {
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
        doAction(newValue, false);
      }
    }
  }

  float getUnitValue() const override {
    // Convert the value to a unit value (0.0 to 1.0)
    auto it = std::find(_options.begin(), _options.end(), _value);
    if (it != _options.end()) {
      size_t index = std::distance(_options.begin(), it);
      return static_cast<float>(index) / static_cast<float>(_options.size() - 1);
    }
    return 0.0f; // Default if not found
  }
  

  // Override the = operator to set the value and emit signals
  ValueOptionsReceiver<T> &operator=(const T &value) {
    setValue(value); // Set the value and emit signals
    return *this; // Return the current instance
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

  void incrementValue(bool isCoarse) override {
    size_t nextIndex = std::clamp((int)getSelectedOptionIndex() + 1, 0, (int)_options.size() - 1); // clamp
    setValue(_options[nextIndex]);
  }

  void decrementValue(bool isCoarse) override {
    size_t prevIndex = std::clamp((int)getSelectedOptionIndex() - 1, 0, (int)_options.size() - 1); // clamp
    setValue(_options[prevIndex]);
  }

  std::string getStringValue() const override {
    // Convert the value to a string representation
    auto it = std::find(_options.begin(), _options.end(), _value);
    if (it != _options.end()) {
      size_t index = std::distance(_options.begin(), it);
      if (index < _optionNames.size()) {
        return _optionNames[index];
      }
    }
    return ""; // Default if not found
  }

  void updateObservers() {
    // Notify observers of the value change
    onValueChanged(_value);
  }

  void doAction(any value, bool undo) override {
    // cast to T and set the value
    if (value.type() == typeid(T)) {
      _value = any_cast<T>(value);
      onValueChanged(_value);
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }

  virtual void serialize(YAML::Emitter &out) override {
    out << YAML::Key << _name;
    out << YAML::Value;

    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->_id);

    out << YAML::Key << "value";
    out << YAML::Value << this->_value;

    out << YAML::EndMap;
  };

  virtual void deserialize(YAML::Node &node) override {
    YAML::Node yaml = node[_name];
    if (!yaml) {
      std::cerr << "Invalid YAML node for ValueReceiver: " << _name << std::endl;
      return;
    }

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

protected:
  uuids::uuid _id = generateUUID();
  vector<T> _options; // List of options for the receiver
  vector<string> _optionNames; // Names for each option
  string _name = "";
  T _value;
  shared_ptr<UndoManager> undoManager_;
};


class VRFloatDB : public ValueReceiver<float> {
public:
  VRFloatDB(string name, float initialValue, shared_ptr<UndoManager> undoManager = nullptr)
      : ValueReceiver<float>(name, initialValue, -60.0f, 12.0f, 1.0f, 0.1f, undoManager) {}

  string getStringValue() const override {
    // Convert the value to a string representation in dB
    return fmt::format("{:.02f} dB", _value);
  }
};

class VRFloatTime : public ValueReceiver<float> {

public:
  VRFloatTime(string name, float initialValue = 0.0f, shared_ptr<UndoManager> undoManager = nullptr)
      : ValueReceiver<float>(name, initialValue, 0.0f, 1.0f, .01f, 0.001f, undoManager) {}

  float normalizedToTime(float normalized, float minTime = 0.001f, float maxTime = 10.0f) const {
    return minTime * std::pow(maxTime / minTime, normalized);
  }

  float timeToNormalized(float time, float minTime = 0.001f, float maxTime = 10.0f) const {
      return std::log(time / minTime) / std::log(maxTime / minTime);
  }

  float getUnitValue() const override {
    // Convert the value to a unit value (0.0 to 1.0)
    return _value;
  }

  string getStringValue() const override {
    // Convert the value to a string representation
    float timeInSeconds = normalizedToTime(_value);
    float seconds = timeInSeconds;
    float milliseconds = timeInSeconds * 1000.0f;
    if(timeInSeconds < 1.0f) {
      return fmt::format("{:.0f}ms", milliseconds);
    } else {
      return fmt::format("{:.02f}s", seconds);
    }
    
  }

  void doAction(any value, bool undo) override {
    // cast to T and set the value
    if (value.type() == typeid(float)) {
      _value = any_cast<float>(value);
      onValueChanged(normalizedToTime(_value));
    } else {
      throw std::runtime_error("Invalid type for ValueReceiver");
    }
  }
};

typedef ValueReceiver<std::string> VRString;
typedef ValueReceiver<bool> VRBool;
typedef ValueReceiver<float> VRFloat;
typedef ValueReceiver<double> VRDouble;
typedef ValueReceiver<int> VRInt;
typedef ValueOptionsReceiver<std::string> VRStringOptions;
typedef ValueOptionsReceiver<int> VRIntOptions;
typedef ValueOptionsReceiver<float> VRFloatOptions;
typedef ValueOptionsReceiver<double> VRDoubleOptions;
