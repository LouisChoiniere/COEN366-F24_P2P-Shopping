#pragma once

class Event {
public:
    virtual ~Event() = default;
    virtual std::string getName() const = 0;
};
