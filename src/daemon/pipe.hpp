#pragma once

// Create a named pipe between two processes that transfer data in one direction
#include <cstdint>

#ifdef _WIN32
#define GUTHI_PIPE_NAME "\\\\.\\pipe\\GUTHI_PIPE"
#elif defined(__linux__)
#define GUTHI_PIPE_NAME "./GUTHI_PIPE"
#else
#error "Unsupported Platform"
#endif 

// handles to pipe
#ifdef _WIN32
typedef void* Handle;
#elif defined(__linux__)
typedef int  Handle;
#else
#error "Unsupported platform"
#endif

enum class PipeMode
{
    Duplex,
    Inbound,
    Outbound
};

struct PipeDesc
{
    uint32_t in_buffer_size;
    uint32_t out_buffer_size;
    uint32_t default_timeout;
    PipeMode mode;
};

Handle InitProcessCommunication(PipeDesc &desc); // For the server
Handle ConnectAsClient();

uint32_t WriteMessage(Handle handle,const uint8_t *msg, uint32_t len);
uint32_t ReadMessage(Handle handle, uint8_t *msg, uint32_t max_allowed_read);
int32_t ReadNonBlocking(Handle handle, uint8_t* msg, uint32_t max_allowed_read); 


Handle WaitForConnection(Handle );
Handle LaunchAsDaemon(int argc,char *argv[]);
void DaemonMainEntry(); 

