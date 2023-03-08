#include <cstdint>

void *GetDaemonHandle(); // Connect to daemon 
void  PostMessageToDaemon(uint8_t *msg, uint32_t len);
void  GetMessageFromDaemon(uint8_t *msg, uint32_t *len, uint32_t max_bytes_allowed);
