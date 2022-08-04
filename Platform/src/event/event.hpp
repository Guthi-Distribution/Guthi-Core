#pragma once
#include <sys/inotify.h>

namespace Event
{
enum EventType {
    NONE = 0,
    CHANGED,
    DELETED,
};

/*
TODO:
    - Event Type Implementation
    - Event Category?
    - Event Listener
    - Buffered Event(Queue)
    - Debug Data
    - Event Dispatch
    - Inheritance as a base event?
*/

struct Event {
    Event() {
    }
};
} // namespace Event