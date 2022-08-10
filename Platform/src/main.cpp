
// #include <format>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "event/event.hpp"
#include "filesystem/fs.hpp"

// Lets get started with file sharing then

int handler_function() {
    return 1;
}

int main(int argc, char *argv[]) {
    printf("Hello from Guthi : A framework for distributed application development\n");

    // File cache
    auto &cache = FileSystem::FileCache::GetLocalFileCache();

    return 0;
}
