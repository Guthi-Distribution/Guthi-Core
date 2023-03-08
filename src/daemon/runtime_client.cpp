#include "./pipe.hpp"

#include <Windows.h> // Included for Waiting and Sleeping

using Handle = void *;

int main(int argc, char **argv)
{
    Handle pipe = ConnectAsClient();
    Sleep(2000);
    char message[512] = "1Hellothisismes";
    WriteMessage(pipe, (uint8_t *)message, 512);
    Sleep(1000);
    char newmsg[512] = "55jsdf";
    WriteMessage(pipe, (uint8_t *)newmsg, 512);
    Sleep(2500);
    return 0;
}