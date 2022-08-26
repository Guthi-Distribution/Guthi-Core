// #include <format>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "./filesystem/fs.hpp"
#include "./event/event.h"

// Lets get started with file sharing then

Guthi::EventStatus handler_function(void *test) {
    // return 1;
}

int main(int argc, char *argv[]) {
    printf("Hello from Guthi : A framework for distributed application development\n");

    Guthi::Event event;
    Guthi::EventQueue queue;
    queue.push_event(&event, handler_function);
    queue.get_event();
}
