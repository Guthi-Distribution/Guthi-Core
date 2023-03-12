#include <Windows.h>
#include <cstdio>
#include <cassert>

#include "./pipe.hpp"

constexpr const char* service_name = "GuthiService"; 

void DumpWin32ErrorMessage(const char *operation)
{
    char  buffer[512] = {};
    DWORD err_val     = GetLastError();
    fprintf(stderr, "Operation : %s failed -> %d.\n", operation, err_val);
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err_val, 0, buffer, 512, nullptr);
    fprintf(stderr, "%s\n", buffer);
}

// It is the main code to be run by the daemon
void *InitProcessCommunication(PipeDesc &desc)
{
    DWORD  open_mode   = PIPE_ACCESS_DUPLEX;

    HANDLE pipe_handle = CreateNamedPipe(GUTHI_PIPE_NAME, open_mode, PIPE_WAIT, 1, desc.out_buffer_size,
                                         desc.in_buffer_size, desc.default_timeout, nullptr);

    if (pipe_handle == INVALID_HANDLE_VALUE)
    {
        DumpWin32ErrorMessage("Pipe Creation Failed");
        exit(-11);
    }

    return pipe_handle;
}

void *ConnectAsClient()
{
    HANDLE pipe = CreateFile(GUTHI_PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE)
    {
	DumpWin32ErrorMessage("OpeningNamedFile() failed\n"); 
        exit(-5);
    }
    return pipe;
}

uint32_t WriteMessage(void *hnd, const uint8_t *msg, uint32_t len)
{
    DWORD  bytes_written = 0;
    HANDLE handle        = (HANDLE)hnd;
    WriteFile(handle, msg, len, &bytes_written, nullptr); // No Asynchronous execution for now
    return (uint32_t)bytes_written;
}

// TODO :: Make it signed and implement Peeking Named Pipe 
uint32_t ReadMessage(void *hnd, uint8_t *msg, uint32_t max_allowed_read)
{
    HANDLE handle     = (HANDLE)hnd;
    DWORD  bytes_read = 0;
    if (ReadFile(handle, msg, max_allowed_read, &bytes_read, nullptr))
        return bytes_read;

    // Since no asynchronous requests are pending, we can safely assume that the call to the readfile failed
    // Reason of failing
    DumpWin32ErrorMessage("ReadMessage() failed ");
    return 0;
}

Handle WaitForConnection(void* hnd) {
	HANDLE handle = (HANDLE) hnd; 
	ConnectNamedPipe(handle, NULL); 
	return hnd; 
}

void CreateDaemonProcess(int argc, char* arg[]) { 
	char self_name[512] = {}; 
	for (uint32_t i = 0; i < argc; ++i) {
		fprintf(stderr, "%d -> %s\n",i,arg[i]); 
	}
	assert(GetModuleFileName(NULL, self_name, 512)!=512);
        fprintf(stderr, "Module loaded : %s.",self_name); 
	
	if (argc != 1 || strcmp(arg[0], "--daemon")) {
		PROCESS_INFORMATION process_info = {}; 
		STARTUPINFO startup_info = {}; 
		startup_info.cb = sizeof(startup_info); 
	
		char arguments[512] = "--daemon"; 
		bool process_created = CreateProcess(
			self_name, 
			arguments,
			NULL, // security context 
			NULL, 
			FALSE, 
			DETACHED_PROCESS, // Specify detached mode 
			NULL, 
			NULL, 
			&startup_info, 
			&process_info); 
		if (!process_created) {
			DumpWin32ErrorMessage("CreateProcess() failed : "); 
			exit(-5); 
		}
		fprintf(stderr, "Daemon launched successfully"); 
		exit(EXIT_SUCCESS); 
	}
}

Handle LaunchAsDaemon(int argc, char* argv[]) {
//	CreateDaemonProcess(argc, argv); 

	PipeDesc pipe_desc = {}; 
	pipe_desc.in_buffer_size = 1024; 
	pipe_desc.out_buffer_size = 1024; 
	pipe_desc.mode = PipeMode::Duplex; 
	return InitProcessCommunication(pipe_desc); 
}

// Integrate with the filesystem to get information about the tracked files and folders 
int32_t ReadNonBlocking(Handle hnd, uint8_t* msg, uint32_t max_read_allowed) {
	DWORD bytes_available = 0;
	HANDLE handle = (HANDLE) hnd; 
	if (!PeekNamedPipe(hnd, nullptr, 0,nullptr, &bytes_available, nullptr))
	{
	       fprintf(stderr, "Failed to connect with pipe"); 
	       return -1; 
	}

	if (!bytes_available) {
		return 0; 
	}

	uint32_t to_read = max_read_allowed < bytes_available ? max_read_allowed : bytes_available; 
 	return	ReadMessage(hnd, msg, to_read); 
}

