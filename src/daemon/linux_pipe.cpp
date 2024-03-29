#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "./pipe.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

// Linux : Use of Domain Socket for IPC

Handle InitProcessCommunication(PipeDesc &desc)
{
    int pipe = socket(AF_UNIX, SOCK_STREAM, 0);
    if (pipe == -1) {
        perror("Failed to open pipe");
        exit(-2);
    }

    struct sockaddr_un sock_addr = {};
    sock_addr.sun_family = AF_UNIX;

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    char *path_to_pipe = new char[strlen(homedir) + 14];
    memset(path_to_pipe, 0, strlen(homedir) + 14);
    strcpy(path_to_pipe, homedir);
    strcat(path_to_pipe, "/GUTHI_PIPE");

    printf("%s\n", path_to_pipe);
    strncpy(sock_addr.sun_path,path_to_pipe,sizeof(sock_addr.sun_path));

    fprintf(stderr, "Full name of domain socket : %s\n",sock_addr.sun_path);
    unlink(sock_addr.sun_path);
    if (bind(pipe,(const struct sockaddr*)&sock_addr, sizeof(sockaddr_un)) == -1) {
        perror("Failed to bind pipe to the domain socket");
        exit(-3);
    }

    return pipe;
}

Handle WaitForConnection(Handle sock) {
    if (listen(sock,10) == -1) {
        perror("Failed to listen to the socket");
        exit(-4);
    }
    // Accept the client connection
    sockaddr_un sock_addr = {};
    socklen_t sock_len = sizeof(sock_addr);
    int client_socket = accept(sock,(struct sockaddr*)&sock_addr,&sock_len);
    if (client_socket == -1)
    {
        perror("Failed to accept for the connection");
        exit(-55);
    }
    return client_socket;
}

Handle ConnectAsClient() // could be null for win32
{
    sockaddr_un sock_desc = {.sun_family = AF_UNIX};
    strncpy(sock_desc.sun_path,GUTHI_PIPE_NAME,sizeof(sock_desc.sun_path));
    int in_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (in_socket == -1) {
        perror("Failed to create a socket");
        exit(-10);
    }
    if (connect(in_socket,(const struct sockaddr*)&sock_desc, sizeof(sock_desc)) == -1) {
        perror("Failed to connect to server as client");
        exit(-11);
    }
    return in_socket;
}

uint32_t WriteMessage(Handle fd, const uint8_t *msg, uint32_t len) {
    int  bytes_written = 0;
    bytes_written = write(fd,(void*)msg,len);
    return (uint32_t)bytes_written;
}

uint32_t ReadMessage(Handle fd, uint8_t *msg, uint32_t max_allowed_read)
{
    int  bytes_read = 0;
    bytes_read = read(fd,(void*)msg, max_allowed_read);
    if (bytes_read != -1)
        return bytes_read;

    // Since no asynchronous requests are pending, we can safely assume that the call to the readfile failed
    // Reason of failing
    perror("Call to read() failed : ");
    return 0;
}

Handle LaunchAsDaemon(int argc, char* argv[]) // Parameters ignored
{
    // Initialize the demon
    // daemon(1,1);
    PipeDesc desc        = {};
    desc.in_buffer_size  = 1024;
    desc.out_buffer_size = 1024;
    desc.mode            = PipeMode::Duplex;
    return InitProcessCommunication(desc);
}


int32_t ReadNonBlocking(Handle handle, uint8_t* buffer, uint32_t max_allowed_read) {
    int bytes_read = 0;
    if (ioctl(handle,FIONREAD,&bytes_read) < 0) {
        perror("ioctl() failed");
        return -1;
    }

    if (!bytes_read)
        return bytes_read;

    return ReadMessage(handle,buffer,max_allowed_read);
}
