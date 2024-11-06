#pragma once

#include <iostream>

class Event {
public:
    virtual ~Event() = default;
    virtual std::string getName() const = 0;
};
