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

// Message format 
// Every message should start with these 5 magic bytes 0x48 0x55 0x54 0x49 0x4A  

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

void DaemonMainEntry() {
	fprintf(stderr, "Reached to daemon main func,,, "); 
	exit(-5); 
}

bool MessageVerified(uint8_t* msg, uint32_t length) {
	if (length <= 5) 
		return false; 
	constexpr static uint8_t magic_bytes = {0x48, 0x55, 0x54, 0x49, 0x4A}; 
	if (magic_bytes, msg, 5) 
		return false; 
	return true; 
}
void DispatchMessage(Handle client, uint8_t *msg, uint32_t valid_length)
{
    if (!MessageVerified(msg, valid_length)) {
	fprintf(stderr, "Fail to verify the message"); 
	return; 
    }
    
    // Read the first byte of the message
    EMessage msg_type = static_cast<EMessage>(msg[0] - '1');
    uint8_t buffer[] = "I got the message"; 
    switch (msg_type)
    {
    case GetFile:
        fprintf(stderr, "GetFile Request obtained");\
	WriteMessage(client,buffer,sizeof(buffer));  
        break;
    default:
        fprintf(stderr, "Unknown Request obtained");
        exit(-2);
    }
}

int main(int argc, char **argv)
{
    Handle  server   = LaunchAsDaemon(argc, argv);
    fprintf(stderr, "Daemon Launched\n"); // TODO :: Launch seperately using CreateProcess() in a detached mode 
    uint8_t msg[512] = {};
    // Wait for other end of the named pipe 
    fprintf(stderr, "Waiting for the message\n"); 
    Handle client = WaitForConnection(server); 
    while (true)
    {
        uint32_t bytes_read = ReadMessage(client, msg, 512);
        if (bytes_read)
        {
            DispatchMessage(client, msg, bytes_read);
        }
        else
            break;
        // Stop the heart bleed
        memset(msg, 0, sizeof(msg));
    }
    return 0;
}
