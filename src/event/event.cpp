#include "event/event.h"

namespace Guthi
{

// void EventQueue::push_event(Event *e, Handler handler) 

EventHandler EventQueue::pop_event() {
    EventHandler e = head->handler;
    head           = head->next;

    return e;
}

EventHandler EventQueue::get_event() {
    return head->handler;
}

} // namespace Guthi
