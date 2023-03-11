#include "./pipe.hpp"
#include <unistd.h>
#include <cstdio>

int main(int argc, char **argv)
{
    Handle pipe = ConnectAsClient();
    sleep(2);
    char message[512] = "1Hellothisismes";
    WriteMessage(pipe, (uint8_t *)message, 512);
    sleep(1);
    char read_buffer[512]  = {}; 
    int bytes_read = ReadMessage(pipe, (uint8_t*) read_buffer, 512);
    if (bytes_read) {
	    fprintf(stderr, "Message received : %s.\n",read_buffer); 
    }
    else {
	    fprintf(stderr, "Message receiving failed"); 
    }

    char newmsg[512] = "55jsdf";
    WriteMessage(pipe, (uint8_t *)newmsg, 512);
    sleep(2);
    return 0;
}
