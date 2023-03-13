#pragma once

#include "./fs.hpp"
#include "./network_fs.hpp"
#include <string>
#include <algorithm>
#include <atomic>
#include <unordered_map>
#include <queue>
#include <mutex> 

#ifdef _WIN32
#include <Windows.h>
#else
// #error "File tracking not implemented for OS other than windows"
#endif

// TODO :: Use overlapped event to convert this code to asynchronous calls
// For now just track the single folder

// Keep track of the files and generate an event if file changes

namespace fs = FileSystem;

enum class TrackFor : uint8_t
{
    NameChange  = 1,
    AttrChange  = 2,
    SizeChange  = 4,
    WriteChange = 8,
};

#if defined(_WIN32)
using TrackHandle = HANDLE;
#elif defined(__linux__)
using TrackHandle = int;
#endif

#define TestEnum(var, val) (uint8_t) var &(uint8_t) decltype(var)::val
// This code is strictly for win32

struct FileTracker
{
    using DirectoryPath = std::string;
    struct DirectoryTrackInfo
    {
        bool recursive;
        bool whole_dir_tracked = false; // false represents individual files are tracked for instead of the whole
                                        // directory, mapping from handle to vector is used then
        DirectoryPath dir_name;
    };

    // Unorderd map is quite slow, but meh
    // Optionally include tracking filter for respective directory, but we omit that for now
    // Map from name of handle of the directory to its information
    std::unordered_map<TrackHandle, DirectoryTrackInfo> tracked_directory;
#if defined(_WIN32)
    FileTracker() = default;
    // A mapping from directory name to its handle
    std::unordered_map<DirectoryPath, TrackHandle> name_to_handle;
    // I guess, we require a map that maps from directory to all files that are
    // tracked within else, every changes will be reported
    std::unordered_map<TrackHandle, std::vector<DirectoryPath>>
        files_tracked_within_directory; // map from handle to the directory
#elif defined(__linux__)
    FileTracker();
    std::unordered_map<DirectoryPath, TrackHandle> name_to_dir_handle;
    std::unordered_map<std::string, TrackHandle>   name_to_file_handle;
    std::unordered_map<TrackHandle, std::string>   handle_to_file_name;
    // if this much information should be okish to track file
#endif

    struct ChangeInfo
    {
        std::string   file;
        DirectoryPath directory;
        TrackFor      result;
    };

    std::mutex             queue_lock;
    std::queue<ChangeInfo> change_info;
    std::mutex             tracker_lock;// Lock for adding new file 
    std::atomic_bool       reset_tracking; 

    void                   TrackFile(FileSystem::FileContent &file, TrackFor track_option);
    void                   TrackFolder(FileSystem::FileContent &folder, TrackFor track_option, bool track_recursively);
    void                   ListenForChanges(uint32_t timeout_ms);
    void                   StopListening()
    {
        stop_listening = true;
    }
    bool TryPopChangeInfo(ChangeInfo &info)
    {
        std::unique_lock  l(queue_lock); 
        if (!change_info.empty())
        {
            info = change_info.front(); 
            change_info.pop(); 
            fprintf(stderr, "\n\nChange info propagated\n\n");
            return true; 
        }
        return false; 
    }
  private:
    // signal to stop tracking files
    std::atomic<bool> stop_listening = false;

#if defined(__linux__)
    int inotify_handle;
#endif
};
