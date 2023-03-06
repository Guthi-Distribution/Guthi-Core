#include "./pipe.hpp"
#include "./daemon_api.hpp"

void *GetDaemonHandle()
{
    return ConnectAsClient();
}

void PostMessageToDaemon(void *handle, uint8_t *msg, uint32_t len)
{
    WriteMessage(handle, msg, len);
}

void GetMessageFromDaemon(void *handle, uint8_t *msg, uint32_t *len, uint32_t max_allowed_read)
{
    *len = ReadMessage(handle, msg, max_allowed_read);
}
