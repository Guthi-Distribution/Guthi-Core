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
    ERROR = 0,
    COMPLETED
};

typedef std::function<EventStatus(void *)> Handler;

struct Event {
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

    void bind_handler(Handler _handler) {
        handler = _handler;
    }

    template <class T> bool handle_event(void *param) {
        if (event->type == T::GetStaticType) {
            event->handle_status = handler(param);

            return true;
        }

        return false;
    }

  private:
    Event  *event;
    Handler handler;
};

struct EventQueueEntry {
    EventHandler     handler;
    EventQueueEntry *next;
    std::string      debug_string;

    EventQueueEntry() = default;

    EventQueueEntry(Event *event) : handler(event) {
    }
};

/*
    Stores all the unhandled events
*/
struct EventQueue {
    void         push_event(Event *e, Handler handler);

    EventHandler pop_event();

    EventHandler get_event();

  private:
    EventQueueEntry *head = nullptr;
    EventQueueEntry *tail = nullptr;
};

} // namespace Guthi