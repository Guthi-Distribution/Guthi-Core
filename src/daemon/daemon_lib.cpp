#include "./pipe.hpp"
#include "./daemon_api.hpp"

Handle GetDaemonHandle()
{
    return ConnectAsClient();
}

void PostMessageToDaemon(Handle handle, uint8_t *msg, uint32_t len)
{
    WriteMessage(handle, msg, len);
}

void GetMessageFromDaemon(Handle handle, uint8_t *msg, uint32_t *len, uint32_t max_allowed_read)
{
    *len = ReadMessage(handle, msg, max_allowed_read);
}

int32_t ReadNonBlockingMessageFromDaemon(Handle handle, uint8_t* msg, uint32_t max_allowed_read) {
	// Read optional message
	return ReadNonBlocking(handle, msg, max_allowed_read); 
}
