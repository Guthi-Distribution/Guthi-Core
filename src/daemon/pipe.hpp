// Create a named pipe between two processes that transfer data in one direction
#include <cstdint>

#ifdef _WIN32
#define GUTHI_PIPE_NAME "\\\\.\\pipe\\GUTHI_PIPE"
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

void    *InitProcessCommunication(PipeDesc &desc); // For the server
void    *ConnectAsClient();

uint32_t WriteMessage(void *handle, uint8_t *msg, uint32_t len);
uint32_t ReadMessage(void *handle, uint8_t *msg, uint32_t max_allowed_read);