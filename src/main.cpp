// #include <format>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include <shared_memory/shm.h>

int main() {
    SharedMemory memory("s_mem", O_CREAT, S_IWUSR | S_IWRITE);
    memory.write_data("Hello there\n");
}
