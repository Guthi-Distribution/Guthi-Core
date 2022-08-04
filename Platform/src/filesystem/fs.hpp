#pragma once

#include <chrono>
#include <concepts>
#include <cstdio>
#include <filesystem>
#include <unordered_set>
#include <variant>

// Abstraction to deal with local file changes/writing/opening

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

struct OSFile;
struct RawFile;

template <typename T> struct FileBufWriter {
    T *underlying_file;

    FileBufWriter(T *file) : underlying_file{file} {
    }

    bool flush(); // submit accumulated write to the original file
    void printf();
};

template <typename T> struct FileBufReader {
    T *underlying_file;

    FileBufReader(T *file) : underlying_file{file} {
    }

    void read();
    void skip();
    void unread();
    void scanf();
};

template <typename T> struct GenericFile {};

// So I guess, this idea of file should be made a little broader and inherit from base file type
// It won't be OOP way of inheritance, but of templated way.

// This struct would be re-named to OSFile
struct OSFile {
    FILE                           *file_ptr  = nullptr;
    size_t                          file_size = 0; // size of file in bytes
    std::string                     file_name;
    FileMode                        file_mode;
    FileStatus                      file_status;
    std::filesystem::file_time_type last_write_time;

    OSFile() = default;

    static OSFile                   OpenFile(const std::string &file, FileMode mode);
    static void                     CloseFile(OSFile &file);

    FileBufReader<OSFile>           ReadBuffered();
    FileBufWriter<OSFile>           WriteBuffered();

    std::filesystem::file_time_type QueryLastWriteTime() const;
};

struct RawFile {
    size_t      file_size = 0;
    std::string file_name;
    // type -> sock_stream, udp_stream or raw file, could be adjusted for them later on
    size_t                 pos             = 0;
    size_t                 len             = 0;
    size_t                 buffer_capacity = 0;
    uint8_t               *data            = nullptr;

    OSFile                 ConvertToOSFile() const;

    FileBufReader<RawFile> ReadBuffered();
    FileBufWriter<RawFile> WriteBuffered();
};

void FetchFile(const std::string &file);

struct FileCache {
    std::string local_cache_directory;

    // maybe a multimap needed here

    // std::unordered_map<std::string, std::string>
    //     files; // map from file name to its stored location in the temporary memory
    //  unordered_set could be used here, if file name contains its location string too like instead of "parrot.png", if
    //  used "./birds/parrot.png"
    std::unordered_set<std::string> cached_files;

    FileCache(const std::string &cache_directory) : local_cache_directory{cache_directory} {
    }

    // File name is sufficient for these functions to work
    bool IsFileCached(std::string const &file) const;
    bool RemoveFromCache(std::string const &file);

    // Adding to cache requires the file to be created inside the cache directory
    void              AddToCache(GenericFile<OSFile> const &file);
    void              UpdateCache(GenericFile<OSFile> const &file);

    static FileCache &GetLocalFileCache();
};

} // namespace FileSystem