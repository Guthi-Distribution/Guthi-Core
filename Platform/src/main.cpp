
// #include <format>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "event/event.hpp"
#include "filesystem/fs.hpp"

// Lets get started with file sharing then

int main(int argc, char *argv[]) {
    printf("Hello from Guthi : A framework for distributed application development\n");

    // File cache
    auto &cache = FileSystem::FileCache::GetLocalFileCache();

    Event event;
    printf("%d\n", event.inotify_fd);

    return 0;
}
