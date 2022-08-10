#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace Guthi
{
enum EventType {
    NONE = 0,
    CHANGED,
    DELETED,
};

enum EventStatus {
    ERROR,
    COMPLETED
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
    - Event Queue
*/

struct Event {
    // TODO: is an event identifier needed?
    //  maybe not because we are using objects
    EventType type;
    bool      handle_status;
    int       mask; // a number to mask the event to not occur

    Event() = default;

    Event(EventType _type) {
        type = _type;
    }

    virtual EventType   GetType() = 0;
    virtual const char *GetName() = 0;

    ~Event() {
    }
};

#define DefineMethods(type)                                                                                            \
    static EventType GetStaticType() {                                                                                 \
        return EventType::type;                                                                                        \
    }                                                                                                                  \
    virtual EventType GetType() const override {                                                                       \
        return GetStaticType();                                                                                        \
    }                                                                                                                  \
    virtual const char *GetName() const override {                                                                     \
        return #type;                                                                                                  \
    }

struct EventHandler {
  public:
    EventHandler(Event *_event) : event(_event) {
    }

    template <class T> bool handle_event(std::function<EventStatus()> handler) {
        if (event->type == T::GetStaticType) {
            event->handle_status = handler();

            return true;
        }

        return false;
    }

  private:
    Event *event;
};

struct EventQueueEntry {
    EventHandler     handler;
    EventQueueEntry *next;
    std::string      debug_string;

    EventQueueEntry() = default;

    EventQueueEntry(Event *event) : handler(event) {
    }
};

struct EventQueue {
    void         push_event(Event *e);

    EventHandler pop_event();

    EventHandler get_event();

  private:
    EventQueueEntry *head = nullptr;
    EventQueueEntry *tail = nullptr;
};

} // namespace Guthi