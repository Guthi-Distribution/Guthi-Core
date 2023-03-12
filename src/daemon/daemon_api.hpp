#include <cstdint>

#if defined(_WIN32) 
typedef void *Handle; 
#elif defined(__linux__)
typedef int Handle; 
#else 
#error "Platform unsupported" 
#endif 

extern "C"
{
    Handle GetDaemonHandle(); // Connect to daemon, pass unmodified to C runtime
    void   PostMessageToDaemon(Handle, uint8_t *msg, uint32_t len);
    void   GetMessageFromDaemon(Handle, uint8_t *msg, uint32_t *len, uint32_t max_bytes_allowed);

    // Return 0 on no message and -1 for error or connection termination
    int32_t   ReadNonBlockingMessageFromDaemon(Handle, uint8_t* msg, uint32_t max_bytes_allowed); 
}
