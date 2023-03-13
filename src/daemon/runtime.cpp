#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <thread>

#include "./pipe.hpp"
#include "../filesystem/file_track.hpp"
#include "./daemon_protocol.h"

// Message format
// Every message should start with these 5 magic bytes 0x48 0x55 0x54 0x49 0x4A

constexpr static uint8_t magic_bytes[]  = {0x48, 0x55, 0x54, 0x49, 0x4A};
constexpr static uint8_t magic_constant = 5;
static FileTracker       tracker        = {};

uint16_t                 PrepareMessage(uint8_t *msg, EMessage, const uint8_t *, uint16_t);

bool                     MessageVerified(uint8_t *msg, uint32_t length)
{
    if (length <= 5)
        return false;
    if (memcmp(magic_bytes, msg, 5))
        return false;
    return true;
}

// TODO :: Add logs in a seperate log fiel
void DispatchMessage(Handle client, uint8_t *msg, uint32_t valid_length)
{
    if (!MessageVerified(msg, valid_length))
    {
        fprintf(stderr, "Failed to verify the message");
        return;
    }

    valid_length = valid_length - magic_constant;
    msg          = msg + magic_constant;

    if (valid_length < 3)
    {
        fprintf(stderr, "Incomplete request");
        return;
    }

    // Read the first byte of the message
    EMessage msg_type = static_cast<EMessage>(
        msg[0]); // We are just consuming the first byte of the message followed immediately by length of the message
                 // These two files are mandatory, even if no further bytes are consumed
    // Read the length of the message in little endian order

    uint16_t msg_length = static_cast<uint16_t>(msg[1]) + static_cast<uint16_t>(msg[2]) << 8;
    // if (msg_length <= valid_length) // Received the complete message
    uint8_t *msg_start        = msg + 3; // Place where message actually start
    uint32_t to_read_length   = valid_length - 3;
    uint8_t  send_buffer[512] = {};
    switch (msg_type)
    {
    case EchoMessage:
        fprintf(stderr, "Obtained echo message\n");
        // WriteMessage(client, msg + 3, valid_length - 3);
        {
            uint16_t content_length = PrepareMessage(send_buffer, EMessage::EchoMessage, msg_start, to_read_length);
            WriteMessage(client, send_buffer, content_length);
        }
        break;
    case TrackThisFile:
    {
        fprintf(stderr, "File tracking request obtained : ");
        // Get the name of the file to be tracked
        char buffer[512] = {};
        memcpy(buffer, msg_start, to_read_length);
        // buffer[to_read_length] = 0; // is implicit
        fprintf(stderr, "%s\n", buffer);
        FileSystem::FileContent content = {};
        content.name                    = std::string(buffer, to_read_length);
        tracker.TrackFile(content, TrackFor::WriteChange);
        break;
    }
    case GetFile:
        fprintf(stderr, "GetFile Request obtained"); // WriteMessage(client,buffer,sizeof(buffer));
        break;
    default:
        fprintf(stderr, "Unknown Request obtained");
        exit(-2);
    }
}

#define MagicBytes "\x48\x55\x54\x49\x4A"

// Prepare the message for sending and returns the total length to be send off
uint16_t PrepareMessage(uint8_t *buf, EMessage msg_type, /* Additional data */ const uint8_t *data,
                        /* Length of additional data */ uint16_t length)
{
    // Copy the magic bytes
    memcpy(buf, MagicBytes, 5);
    buf    = buf + 5;
    *buf   = static_cast<uint8_t>(msg_type);
    buf[1] = length & 0xFF;
    buf[2] = length >> 8;
    buf    = buf + 3;
    memcpy(buf, data, length);
    return length + 8;
}

int main(int argc, char **argv)
{
    Handle server = LaunchAsDaemon(argc, argv);
    fprintf(stderr, "Daemon Launched\n"); // TODO :: Launch seperately using CreateProcess() in a detached mode
    uint8_t msg[512] = {};
    // Wait for other end of the named pipe
    fprintf(stderr, "Waiting for the message\n");
    Handle client = WaitForConnection(server);

    // Initialize file tracking
    //    FileSystem::FileContent file;
    //    file.name = "./ch_test.txt";
    //    tracker.TrackFile(file, TrackFor::WriteChange);

    // Launch the tracking thread
    std::thread             tracking_thread(&FileTracker::ListenForChanges, &tracker, 0);

    FileTracker::ChangeInfo changed_info = {};

    while (true)
    {
        int32_t bytes_read = ReadNonBlocking(client, msg, 512);

        if (bytes_read == -1)
            break;

        if (bytes_read)
        {
            DispatchMessage(client, msg, bytes_read);
        }
        // Stop the heart bleed
        memset(msg, 0, sizeof(msg));

        // Listen if there's tracking info
        if (tracker.TryPopChangeInfo(changed_info))
        {
            // propage the change info to the network client
            // Prepare the magic bytes
            const char *changed_content = changed_info.file.c_str();
            uint16_t    msg_length = PrepareMessage(msg, EMessage::TrackedFileChanged, (const uint8_t *)changed_content,
                                                    strlen(changed_content));
            WriteMessage(client, msg, msg_length);
            fprintf(stderr, "Track file changed detected and broadcasted");
        }
    }

    fprintf(stderr, "Exited the main loop\n");
    tracker.StopListening();
    if (tracking_thread.joinable())
    {
        tracking_thread.join();
    }
    return 0;
}
