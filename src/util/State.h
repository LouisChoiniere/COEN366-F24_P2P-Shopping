#pragma once

// Generic state and event base classes
class State {
public:
    virtual ~State() = default;
    virtual std::string getName() const = 0;
};
