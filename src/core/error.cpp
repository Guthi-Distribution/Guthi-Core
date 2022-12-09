#include "error.hpp"

#ifdef _WIN32
void print_error(const char* message) {
    char *error_message_buff;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&error_message_buff,
        0, NULL);

    // Display the error message and exit the process
    printf("%s: %s\n", message, error_message_buff);

    LocalFree(error_message_buff);
}
#endif // _MSC_VER
