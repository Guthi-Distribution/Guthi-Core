
// #include <format>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "./filesystem/fs.hpp"
#include "./event/event.hpp"

// Lets get started with file sharing then

int handler_function() {
    return 1;
}

int main(int argc, char *argv[]) {
    printf("Hello from Guthi : A framework for distributed application development\n");

    // File cache
    auto &cache = FileSystem::FileCache::GetLocalFileCache();

    auto  file  = FileSystem::OSFile::OpenFile("../../../../Platform/src/test.txt", FileSystem::FileMode::In);
    FileSystem::FileBufReader reader(&file);

    int                       a;
    float                     f;
    char                      str[32] = {};
    reader.Scanf("%d %f %s", &a, &f, str);
    std::cout << "a -> " << a << " and f -> " << f << " and str -> " << str << std::endl;

    // TODO :: Implement unit testing framework for automated testing approach
}
