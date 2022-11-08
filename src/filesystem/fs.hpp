#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <chrono>
#include <concepts>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <algorithm>
#include <string.h>

#ifdef _MSC_VER
#define safe_memcpy(dest, dest_size, src, src_size) memcpy_s(dest, dest_size, src, src_size)
#else
#define safe_memcpy(dest, dest_size, src, src_size) memcpy(dest, src, src_size)
#endif

// Abstraction to deal with local file changes/writing/opening

namespace chrono = std::chrono;
namespace FileSystem
{

enum class FileMode
{
    In     = 0,
    Out    = 1,
    InOut  = 2,
    Append = 3, // Don't use append mode, its non-intuitive
    None
};

enum class FileStatus
{
    Opened,
    Modified,
    OutOfSync,
    Closed
};

// Specifies whether the file is in the form of raw data (as obtained from network) or behind the OS
// To handle things either way, requires some nice API

struct Buffer
{
    constexpr static int DEFAULT_BUFFER_SIZE = 4096; // 512 bytes
    uint8_t             *data                = nullptr;
    uint32_t             pos;
    uint8_t              read_ptr; // read ptr
    uint32_t             capacity;
};

// TODO :: Implement reallocations
template <typename T> struct FileBufWriter
{
    T     *underlying_file;

    Buffer buffer = {};

    FileBufWriter(T *file) : underlying_file{file}
    {
        // initialize the capacity of the buffer

        buffer.data     = malloc(sizeof(*buffer.data) * decltype(buffer)::DEFAULT_BUFFER_SIZE);
        buffer.pos      = 0;
        buffer.capacity = decltype(buffer)::DEFAULT_BUFFER_SIZE;
    }

    bool Flush() // submit accumulated write back to the original file
    {
        underlying_file->WriteBlockWithSize(buffer.data, buffer.pos);
        buffer.pos = 0;
        return true;
    }

    void Printf(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

#ifdef _DEBUG
        // Check for overflow and such
        va_list args_copy;
        va_copy(args_copy, args);
        uint32_t req_len = vsnprintf(nullptr, 0, fmt, args_copy);
        va_end(args_copy);
        assert(buffer.pos + req_len < buffer.capacity);
#endif

        uint32_t written = vsprintf((char *const)(buffer.data + buffer.pos), fmt, args);
        buffer.pos       = buffer.pos + written;
        va_end(args);
    }

    void Write(const void *buffer_, size_t size)
    {
        assert(size < (buffer.capacity - buffer.pos));
        std::memcpy(buffer.data + buffer.pos, buffer_, size);
    }

    ~FileBufWriter()
    {
        free(buffer.data);
    }
};

// Let's complete the buffered reader first
template <typename T>
requires requires(T &x) { x.FetchBlockWithSize(nullptr, 0); }
struct FileBufReader
{
    T     *underlying_file;

    Buffer buffer = {};

    FileBufReader(T *file) : underlying_file{file}
    {
        buffer.data     = (uint8_t *)malloc(sizeof(*buffer.data) * Buffer::DEFAULT_BUFFER_SIZE);
        buffer.pos      = 0;
        buffer.capacity = decltype(buffer)::DEFAULT_BUFFER_SIZE;

        buffer.pos      = underlying_file->FetchBlockWithSize(buffer.data, Buffer::DEFAULT_BUFFER_SIZE);
    }

    uint32_t Read(char *buffer_, size_t size)
    {
        assert(size < decltype(buffer)::DEFAULT_BUFFER_SIZE);
        if (size >= (buffer.pos - buffer.read_ptr))
        {
            if (buffer.pos != buffer.capacity)
                return 0;
            // else there's more data to fetch
            // first move the remaining bytes to the start of the buffer
            std::memmove(buffer.data, buffer.read_ptr + buffer.data, buffer.pos - buffer.read_ptr);
            uint32_t fetched =
                underlying_file->FetchBlockWithSize(buffer.data + buffer.pos - buffer.read_ptr,
                                                    Buffer::DEFAULT_BUFFER_SIZE - (buffer.pos - buffer.read_ptr));

            buffer.pos      = buffer.pos - buffer.read_ptr + fetched;
            buffer.read_ptr = 0;
        }

        assert(size < (buffer.pos - buffer.read_ptr));
        // start from read ptr
        std::memcpy(buffer_, buffer.data + buffer.read_ptr, size);
    }

    void Skip(uint32_t no_of_bytes) // Skip n characters in the forward direction
    {
        if (buffer.read_ptr + no_of_bytes < buffer.pos)
        {
            buffer.read_ptr = buffer.read_ptr + no_of_bytes;
        }
    }

    void Unread(uint32_t no_of_bytes)
    {
        if (buffer.read_ptr < no_of_bytes)
            buffer.read_ptr = 0;
        else
            buffer.read_ptr = buffer.read_ptr - no_of_bytes;
    }

    void Scanf(const char *fmt, ...)
    {
        // TODO :: Replace with safer version
        if constexpr (false)
        {
            va_list arg;
            va_start(arg, fmt);

            // There's no straight way of getting consumed input length using sscanf family of functions
            vsscanf((const char *const)(buffer.data + buffer.read_ptr), fmt, arg);
            // TODO :: increment the read pointer
            va_end(arg);
        }
        else
        {
            uint32_t pos     = 0;
            uint32_t fmt_len = strlen(fmt);
            // Custom scanf implementation
            va_list arg;
            va_start(arg, fmt);
            // Format support for %d, %f and %s for now.
            // optional read_count used by %32s like formatting
            uint32_t read_count = 0;

            uint32_t in_pos     = 0;

            // TODO :: Implement it properly
            while (pos < fmt_len - 1)
            {
                // skip whitespace
                while (fmt[pos] == ' ' || fmt[pos] == '\t' || fmt[pos] == '\n')
                    pos = pos + 1;

                if (fmt[pos] == '%')
                {
                    switch (fmt[pos + 1])
                    {
                    case 'd':
                    {
                        // prepare to read integer from the input stream
                        char    *end_ptr = NULL;
                        uint32_t val     = strtol((const char *)(buffer.read_ptr + buffer.data + in_pos), &end_ptr, 0);

                        if ((uint8_t *)end_ptr == (buffer.read_ptr + buffer.data + in_pos))
                            assert(!"Error reading the integer literal ");

                        in_pos        = in_pos + (uint8_t *)end_ptr - (buffer.read_ptr + buffer.data + in_pos);

                        uint32_t *var = va_arg(arg, uint32_t *);
                        *var          = val;
                        break;
                    }
                    case 'f':
                    {
                        // read floating point literal from the input
                        char *end_ptr = NULL;
                        float val     = strtof((const char *)(buffer.read_ptr + buffer.data + in_pos), &end_ptr);

                        if ((uint8_t *)end_ptr == (buffer.read_ptr + buffer.data + in_pos))
                            assert(!"Error reading the integer literal ");

                        in_pos     = in_pos + (uint8_t *)end_ptr - (buffer.read_ptr + buffer.data + in_pos);

                        float *var = va_arg(arg, float *);
                        *var       = val;
                        break;
                    }
                    case 'c':
                    {
                        // consume single character
                    }
                    case 's':
                    {
                        char *ptr = va_arg(arg, char *);
                        sscanf((const char *const)buffer.data + buffer.read_ptr + in_pos, "%s", ptr);
                        break;
                    }
                    }
                    pos = pos + 2;
                }
            }
            buffer.read_ptr += in_pos;
            va_end(arg);
        }
    }

    ~FileBufReader()
    {
        free(buffer.data);
    }
};

template <typename T> struct GenericFile
{
};

// So I guess, this idea of file should be made a little broader and inherit from base file type
// It won't be OOP way of inheritance, but of templated way.

// This struct would be re-named to OSFile
struct OSFile
{
    FILE                           *file_ptr  = nullptr;
    size_t                          file_size = 0; // size of file in bytes
    std::string                     file_name;
    FileMode                        file_mode;
    FileStatus                      file_status;
    std::filesystem::file_time_type last_write_time;

    OSFile() = default;

    static OSFile                   OpenFile(const std::string &file, FileMode mode);
    static void                     CloseFile(OSFile &file);
    uint32_t                        FetchBlockWithSize(uint8_t *data, uint32_t size);
    uint32_t                        WriteBlockWithSize(uint8_t *data, uint32_t size);

    std::filesystem::file_time_type QueryLastWriteTime() const;
    ~OSFile()
    {
    }
};

struct RawFile
{
    size_t      file_size = 0;
    std::string file_name;

    // type -> sock_stream, udp_stream or raw file, could be adjusted for them later on
    size_t   pos             = 0;
    size_t   len             = 0;
    size_t   buffer_capacity = 0;
    uint8_t *data            = nullptr;

    OSFile   ConvertToOSFile() const;

    // FileBufReader<RawFile> ReadBuffered();
    // FileBufWriter<RawFile> WriteBuffered();
    void DestroyFile()
    {
        delete[] data;
        file_size = 0;
    }
};

void FetchFile(const std::string &file);

struct FileCache
{

    enum class CacheLocation
    {
        OnRAM,
        OnDisk,
        UnCached
    };

    struct FileCacheEntry
    {
        CacheLocation                            location;
        size_t                                   size;       // in bytes
        uint8_t                                  ip_addr[4]; // location of node in the network
        std::string                              file_name;
        std::unique_ptr<uint8_t[]>               data = nullptr;
        chrono::time_point<chrono::system_clock> timestamp; // last updated timestamp of the cache
    };

    std::string local_cache_directory;

    // maybe a multimap needed here
    // std::unordered_map<std::string, std::string>
    //     files; // map from file name to its stored location in the temporary memory
    //  unordered_set could be used here, if file name contains its location string too like instead of "parrot.png", if
    //  used "./birds/parrot.png"

    struct Caches
    {
        // TODO :: Try merging them, and take care about std::string
        std::multimap<std::string, FileCacheEntry> OfflineCache;
    } caches;

    FileCache(std::string_view cache_directory) : local_cache_directory{cache_directory}
    {
    }

    // File name is sufficient for these functions to work
    bool            IsFileCached(std::string_view file, uint8_t ip_addr[4]) const;
    bool            RemoveFromCache(std::string_view file, uint8_t ip_addr[4]);
    FileCacheEntry *GetCachedEntry(std::string_view file, uint8_t ip_addr[4]);
    RawFile         GetCachedFile(std::string_view file, uint8_t ip_addr[4]);

    // Adding to cache requires the file to be created inside the cache directory
    void AddFileToCache(RawFile &file, uint8_t ip_addr[4]);
    // void              UpdateCache(GenericFile<OSFile> const& file);

    static FileCache &GetLocalFileCache();
};

} // namespace FileSystem