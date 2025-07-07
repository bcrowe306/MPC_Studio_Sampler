#pragma once

#include "control_surface/components/component.h"
#include "sigslot/signal.hpp"


template <typename T>
class Property {
public:

    // Signals
    sigslot::signal<T> onValueChanged; // Signal emitted when the property value changes
    sigslot::signal<T> beforeValueChanged; // Signal emitted before the property value changes

    Property(const T& value) : _value(value) {}

    // Getter for the property value
    
    // override the = operator to set the value and emit signals
    Property& operator=(const T& value) {
        set(value); // Set the value and emit signals
        return *this; // Return the current instance
    }

    // Getter for the property value
    T get() const {
        return _value; // Return the current property value
    }
    void set(const T &value) {
        if (_value != value) {
          beforeValueChanged(value); // Emit signal before changing the value
          _value = value;            // Update the property value
          onValueChanged(value);     // Emit signal after changing the value
        }
    }
    
    void emit(){
        onValueChanged(_value); // Emit the current value
    }

protected:

    
    T _value; // The property value
};