#pragma once

#include <string>
#include "sigslot/signal.hpp"
#include <algorithm> // For std::clamp

using std::string;

class ParameterBase {
public:
    ParameterBase(const string &name, const string &description = "")
        : _name(name), _description(description) {}
    virtual ~ParameterBase() = default;

    sigslot::signal<string> onNameChanged;
    sigslot::signal<string> onDisplayNameChanged;
    sigslot::signal<string> onDescriptionChanged;

    virtual void increment(bool fine = false) = 0;
    virtual void decrement(bool fine = false) = 0;
    virtual void serialize() = 0;
    virtual void deserialize() = 0;
    virtual void setName(const string &name) {
        _name = name;
        onNameChanged(name);
    }
    virtual void setDisplayName(const string &displayName) {
        _displayName = displayName;
        onDisplayNameChanged(displayName);
    }
    virtual void setDescription(const string &description) {
        _description = description;
        onDescriptionChanged(description);
    }
    const string &getName() const {
        return _name;
    }
    const string &getDescription() const {
        return _description;
    }
    const string &getDisplayName() const {
        return _displayName.empty() ? _name : _displayName;
    }

protected:
    string _name;
    string _description = "";
    string _displayName = "";
};


class FloatParameter : public ParameterBase {
public:
    float coarseOffset = 1.0f; // Coarse offset for increment/decrement
    float fineOffset = 0.1f;   // Fine offset for increment/decrement
    FloatParameter(const string &name, float defaultValue, float minValue, float maxValue, const string &description = "")
        : ParameterBase(name, description), _value(defaultValue), _min(minValue), _max(maxValue) {}

    sigslot::signal<float> onValueChanged;

    void setValue(float value) {
        if (value != _value) {
            _value = std::clamp(value, _min, _max);
            onValueChanged(_value);
        }
    }

    float getValue() const {
        return _value;
    }

    void increment(bool fine = false) override {
        setValue(_value + (fine ? fineOffset : coarseOffset));
    }

    void decrement(bool fine = false) override {
        setValue(_value - (fine ? fineOffset : coarseOffset));
    }

    void serialize() override {
        // Implement serialization logic
    }

    void deserialize() override {
        // Implement deserialization logic
    }
private:
    float _value;
    float _min;
    float _max; 
};

class IntParameter : public ParameterBase {
public:
    int coarseOffset = 1; // Coarse offset for increment/decrement
    int fineOffset = 1;   // Fine offset for increment/decrement
    IntParameter(const string &name, int defaultValue, int minValue, int maxValue, const string &description = "")
        : ParameterBase(name, description), _value(defaultValue), _min(minValue), _max(maxValue) {}
        
    sigslot::signal<int> onValueChanged;

    void setValue(int value) {
        if (value != _value) {
            _value = std::clamp(value, _min, _max);
            onValueChanged(_value);
        }
    }

    int getValue() const {
        return _value;
    }

    void increment(bool fine = false) override {
        setValue(_value + (fine ? fineOffset : coarseOffset));
    }

    void decrement(bool fine = false) override {
        setValue(_value - (fine ? fineOffset : coarseOffset));
    }

    void serialize() override {
        // Implement serialization logic
    }

    void deserialize() override {
        // Implement deserialization logic
    }       
private:
    int _value;
    int _min;
    int _max;
}; 

class BoolParameter : public ParameterBase {
public:
    BoolParameter(const string &name, bool defaultValue, const string &description = "")
        : ParameterBase(name, description), _value(defaultValue) {} 
    sigslot::signal<bool> onValueChanged;

    void setValue(bool value) {
        if (value != _value) {
            _value = value;
            onValueChanged(_value);
        }
    }

    bool getValue() const {
        return _value;
    }

    void increment(bool fine = false) override {
        setValue(!_value); // Toggle the boolean value
    }

    void decrement(bool fine = false) override {
        setValue(!_value);
    }

    void serialize() override {
        // Implement serialization logic
    }

    void deserialize() override {
        // Implement deserialization logic
    }
private:
    bool _value;
};


class StringParameter : public ParameterBase {
public:
    StringParameter(const string &name, const string &defaultValue, const string &description = "")
        : ParameterBase(name, description), _value(defaultValue) {}

    sigslot::signal<string> onValueChanged;

    void setValue(const string &value) {
        if (value != _value) {
            _value = value;
            onValueChanged(_value);
        }
    }

    const string &getValue() const {
        return _value;
    }

    void increment(bool fine = false) override {
        // Implement string increment logic
    }

    void decrement(bool fine = false) override {
        // Implement string decrement logic
    }

    void serialize() override {
        // Implement serialization logic
    }

    void deserialize() override {
        // Implement deserialization logic
    }
private:
    string _value;
};

class IntOptionsParameter : public ParameterBase {
    std::vector<int> _options;
    int _currentIndex;

public:
    IntOptionsParameter(const string &name, const std::vector<int> &options, int defaultIndex, const string &description = "")
        : ParameterBase(name, description), _options(options), _currentIndex(defaultIndex) {}

    sigslot::signal<int> onValueChanged;

    void setValue(int index) {
        if (index != _currentIndex) {
            _currentIndex = std::clamp(index, 0, static_cast<int>(_options.size()) - 1);
            onValueChanged(_options[_currentIndex]);
        }
    }

    int getValue() const {
        return _options[_currentIndex];
    }

    void increment(bool fine = false) override {
        setValue(_cycleIndex(true));
    }

    void decrement(bool fine = false) override {
        setValue(_cycleIndex(false));
    }

    void serialize() override {
        // Implement serialization logic
    }

    void deserialize() override {
        // Implement deserialization logic
    }   
    const std::vector<int> &getOptions() const {
        return _options;
    }
    int getCurrentIndex() const {
        return _currentIndex;
    }
    int getCount() const {
        return _options.size();
    }
private:
    int _cycleIndex (bool forward = true) {
        auto index = _currentIndex;
        if (forward) {
            index = (index + 1) % _options.size();
        } else {
            index = (index - 1 + _options.size()) % _options.size();
        }
        return index;
    }
};

class FloatOptionsParameter : public ParameterBase {
    std::vector<float> _options;
    int _currentIndex;  
public:
    FloatOptionsParameter(const string &name, const std::vector<float> &options, int defaultIndex, const string &description = "")
        : ParameterBase(name, description), _options(options), _currentIndex(defaultIndex) {}   
    sigslot::signal<float> onValueChanged;
    void setValue(int index) {
        if (index != _currentIndex) {
            _currentIndex = std::clamp(index, 0, static_cast<int>(_options.size()) - 1);
            onValueChanged(_options[_currentIndex]);
        }
    }   
    float getValue() const {
        return _options[_currentIndex];
    }
    void increment(bool fine = false) override {
        setValue(_cycleIndex(true));
    }
    void decrement(bool fine = false) override {
        setValue(_cycleIndex(false));
    }
    void serialize() override {
        // Implement serialization logic
    }
    void deserialize() override {
        // Implement deserialization logic
    }
    const std::vector<float> &getOptions() const {
        return _options;
    }
    int getCurrentIndex() const {
        return _currentIndex;
    }
    int getCount() const {
        return _options.size();
    }
private:
    int _cycleIndex (bool forward = true) {
        auto index = _currentIndex;
        if (forward) {
            index = (index + 1) % _options.size();
        } else {
            index = (index - 1 + _options.size()) % _options.size();
        }
        return index;
    }
};

class StringOptionsParameter : public ParameterBase {
    std::vector<string> _options;
    int _currentIndex;
public:
    StringOptionsParameter(const string &name, const std::vector<string> &options, int defaultIndex, const string &description = "")
        : ParameterBase(name, description), _options(options), _currentIndex(defaultIndex) {}
    sigslot::signal<string> onValueChanged;
    void setValue(int index) {
        if (index != _currentIndex) {
            _currentIndex = std::clamp(index, 0, static_cast<int>(_options.size()) - 1);
            onValueChanged(_options[_currentIndex]);
        }
    }
    const string &getValue() const {
        return _options[_currentIndex];
    }
    void increment(bool fine = false) override {
        setValue(_cycleIndex(true));
    }
    void decrement(bool fine = false) override {
        setValue(_cycleIndex(false));
    }
    void serialize() override {
        // Implement serialization logic
    }
    void deserialize() override {
        // Implement deserialization logic
    }
    const std::vector<string> &getOptions() const {
        return _options;
    }
    int getCurrentIndex() const {
        return _currentIndex;
    }
    int getCount() const {
        return _options.size();
    }
private:
    int _cycleIndex (bool forward = true) {
        auto index = _currentIndex;
        if (forward) {
            index = (index + 1) % _options.size();
        } else {
            index = (index - 1 + _options.size()) % _options.size();
        }
        return index;
    }
};
