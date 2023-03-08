#include "./pipe.hpp"
#include <Windows.h>
#include <cstdio>

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
        fprintf(stderr, "Cannot open named pipe as a client \n");
        exit(-5);
    }
    return pipe;
}

uint32_t WriteMessage(void *hnd, uint8_t *msg, uint32_t len)
{
    DWORD  bytes_written = 0;
    HANDLE handle        = (HANDLE)hnd;
    WriteFile(handle, msg, len, &bytes_written, nullptr); // No Asynchronous execution for now
    return (uint32_t)bytes_written;
}

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
