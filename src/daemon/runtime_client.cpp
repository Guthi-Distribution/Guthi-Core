#include "./pipe.hpp"
#include <cstdio>

int main(int argc, char **argv)
{
    Handle pipe = ConnectAsClient();
    sleep(2);
    fprintf(stderr, "Clinet : Connected with the daemon");
    // Send the echo hello world message
    const char echo_msg[] = "HUTIJ\x06\x05\x00HelloWorld";
    WriteMessage(pipe, (uint8_t *)echo_msg, 512);
    sleep(1);
    char read_buffer[512] = {};
    for (uint32_t i = 0; i < 50; ++i)
    {
        int bytes_read = ReadNonBlocking(pipe,(uint8_t*)read_buffer,512);
        if (bytes_read)
            fprintf(stderr, "Message received : %.*s.\n", bytes_read - 8, read_buffer + 8);
        else
            fprintf(stderr, "Non blocking\n");
        sleep(1);
    }
    sleep(2);
    return 0;
}
