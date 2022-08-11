#include "event.hpp"

namespace Guthi
{

void EventQueue::push_event(Event *e, Handler handler) {
    EventQueueEntry *entry = new EventQueueEntry(e);
    entry->handler.bind_handler(handler);
    if (!head && !tail) {
        head = entry;
        tail = entry;
        return;
    }

    tail->next  = entry;
    entry->next = nullptr;

    nullptr;
}

EventHandler EventQueue::pop_event() {
    EventHandler e = head->handler;
    head           = head->next;

    return e;
}

EventHandler EventQueue::get_event() {
    return head->handler;
}

} // namespace Guthi
