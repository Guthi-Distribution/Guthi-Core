#include "./pipe.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>

enum EMessage
{
    GetFile              = 0,
    CheckIfInCache       = 1,
    RequestFileMetadata  = 2,
    NoSuchResourceExists = 3,
    FileChanged          = 4
};

using Handle = void *;

Handle LaunchDaemon()
{
    // Initialize the demon
    PipeDesc desc        = {};
    desc.in_buffer_size  = 1024;
    desc.out_buffer_size = 1024;
    desc.mode            = PipeMode::Duplex;
    return InitProcessCommunication(desc);
}

void DispatchMessage(uint8_t *msg, uint32_t valid_length)
{
    // Read the first byte of the message
    EMessage msg_type = static_cast<EMessage>(msg[0]);
    switch (msg_type)
    {
    case GetFile:
        fprintf(stderr, "GetFile Request obtained");
        break;
    default:
        fprintf(stderr, "Unknown Request obtained");
        exit(-2);
    }
}

int main(int argc, char **argv)
{
    Handle  server   = LaunchDaemon();
    fprintf(stderr, "Daemon Launched"); // TODO :: Launch seperately using CreateProcess() in a detached mode 
    uint8_t msg[512] = {};
    while (true)
    {
        uint32_t bytes_read = ReadMessage(server, msg, 512);
        if (bytes_read)
        {
            DispatchMessage(msg, bytes_read);
        }
        else
            break;
        // Stop the heart bleed
        memset(msg, 0, sizeof(msg));
    }
    return 0;
}