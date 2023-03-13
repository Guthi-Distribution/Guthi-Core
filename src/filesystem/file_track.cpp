// lol
// TODO :: Modify CMakeLists.txt to accomodate for this

#if defined(_WIN32)

#include "./file_track.hpp"
#include <chrono>
#include <thread>

void FileTracker::TrackFolder(FileSystem::FileContent &folder, TrackFor track_option, bool track_recursively)
{
    assert(folder.type == FileSystem::FileType::Directory); // win32 can do this check for us, if we wanted to

    std::string dir    = folder.name;

    DWORD       filter = 0;
    if (TestEnum(track_option, NameChange))
        filter = filter | FILE_NOTIFY_CHANGE_FILE_NAME;
    if (TestEnum(track_option, WriteChange))
        filter = filter | FILE_NOTIFY_CHANGE_LAST_WRITE;
    if (TestEnum(track_option, SizeChange))
        filter = filter | FILE_NOTIFY_CHANGE_SIZE;
    if (TestEnum(track_option, AttrChange))
        filter = filter | FILE_NOTIFY_CHANGE_ATTRIBUTES;

    HANDLE dir_handle = CreateFileA(dir.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (dir_handle == INVALID_HANDLE_VALUE)
    {
        DWORD error_val = GetLastError();
        fprintf(stdout, "Cannot open specified directory : %d.", error_val);
        char buffer[512] = {};
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_val, 0, buffer, 512, nullptr);
        fprintf(stdout, "%s\n", buffer);
        exit(-2);
    }
    tracked_directory.insert(std::pair{dir_handle, DirectoryTrackInfo{true, true, dir}});
}

void FormatWin32SystemError(DWORD error_code)
{
    char buffer[512] = {};
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, buffer, 512, nullptr);
    fprintf(stdout, "%s\n", buffer);
}

void FileTracker::TrackFile(FileSystem::FileContent &file, TrackFor track_option)
{
    std::string path(512, ' ');

    DWORD       len = GetFullPathNameA(file.name.c_str(), 512, path.data(), nullptr);
    path.resize(len);

    fprintf(stdout, "\nFSTracLog : Obtained full path %d : %s.", len, path.c_str());

    auto        last_pos = path.find_last_of('\\');
    std::string dir;

    if (last_pos != std::string::npos)
    {
        dir.resize(last_pos);
        std::copy(path.begin(), path.begin() + last_pos, dir.begin());
    }
    else
    {
        fprintf(stderr, "Invalid directory/file handle");
        exit(-1);
    }

    fprintf(stderr, "\nFSTracLog : Path extracted : %s.", dir.c_str());
    // TODO :: Complete the filter criteria list
    DWORD filter = 0;
    if (TestEnum(track_option, NameChange))
        filter = filter | FILE_NOTIFY_CHANGE_FILE_NAME;
    if (TestEnum(track_option, WriteChange))
        filter = filter | FILE_NOTIFY_CHANGE_LAST_WRITE;
    if (TestEnum(track_option, SizeChange))
        filter = filter | FILE_NOTIFY_CHANGE_SIZE;
    if (TestEnum(track_option, AttrChange))
        filter = filter | FILE_NOTIFY_CHANGE_ATTRIBUTES;

    std::string file_name;
    std::copy(path.begin() + last_pos + 1, path.end(), std::back_inserter(file_name));

    auto it = name_to_handle.find(dir);

    // TODO :: Later update the directory info, if it mismatched

    if (it != name_to_handle.end())
    {
        std::unique_lock l(tracker_lock);
        // Directory has already been tracked
        if (auto new_iter = files_tracked_within_directory.find((*it).second);
            new_iter != files_tracked_within_directory.end())
        {
            (*new_iter).second.push_back(file_name);
        }
        else
        {
            files_tracked_within_directory.insert(std::pair{(*it).second, std::vector{file_name}});
        }
        reset_tracking = true;
        return;
    }

    HANDLE dir_handle = CreateFileA(dir.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

    if (dir_handle == INVALID_HANDLE_VALUE)
    {
        DWORD error_val = GetLastError();
        fprintf(stdout, "Cannot open specified directory : %d.", error_val);
        char buffer[512] = {};
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_val, 0, buffer, 512, nullptr);
        fprintf(stdout, "%s\n", buffer);
        exit(-2);
    }

    std::unique_lock l(tracker_lock);
    // Directory has been tracked but file hasn't been added to it,so add it now
    files_tracked_within_directory.insert({dir_handle, std::vector{file_name}});
    tracked_directory.insert(std::pair{dir_handle, DirectoryTrackInfo(true, false, dir)});
    name_to_handle.insert(std::pair{dir, dir_handle});
    reset_tracking = true;
}

void FileTracker::ListenForChanges(uint32_t timeout)
{
    // We might spawn a lot of thread to listen for those changes
    // or use asynchronous listening provided by win32
    struct Buffer
    {
        char buffer[1024] = {};
    };

    HANDLE            *events       = nullptr;
    constexpr uint32_t buffer_size  = 1024;

    OVERLAPPED        *overlap      = nullptr;
    Buffer            *read_buffer  = nullptr;
    bool              *completed    = nullptr;

    reset_tracking                  = true;

    uint32_t tracked_directory_size = 0;

    while (!stop_listening.load(std::memory_order_relaxed))
    {
        if (reset_tracking.load())
        {
            // Cleanup phase
            delete[] events;
            delete[] overlap;
            delete[] completed;
            delete[] read_buffer;

            // Reallocate
            uint32_t dir_count = 0;
            {
                std::unique_lock l(tracker_lock);
                dir_count = tracked_directory.size();
            }

            fprintf(stderr, "FSTrackLog : Started retracking : count -> %d", dir_count);
            if (dir_count > 0)
            {
                events    = new HANDLE[dir_count];
                overlap   = new OVERLAPPED[dir_count];
                completed = new bool[dir_count];

                for (uint32_t j = 0; j < dir_count; ++j)
                {
                    overlap[j]        = {};
                    events[j]         = CreateEvent(NULL, TRUE, FALSE, NULL);
                    overlap[j].hEvent = events[j];
                    completed[j]      = true;
                }
                read_buffer = new Buffer[dir_count];
            }
            reset_tracking         = false;
            tracked_directory_size = dir_count;
        }

        uint32_t c = 0;
        for (auto const &[handle, dir_info] : tracked_directory)
        {
            // TODO :: Use completion routine
            if (completed[c])
            {
                bool result = ReadDirectoryChangesW(handle, &read_buffer[c].buffer, 1024, TRUE,
                                                    FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr, &overlap[c], nullptr);
                if (!result)
                {
                    DWORD error = GetLastError();
                    fprintf(stdout, "Queuing failed %d.\n", error);
                    FormatWin32SystemError(error);
                    exit(-1);
                }
            }
            c = c + 1;
        }

        DWORD bytes_read = 0;

        while (!stop_listening.load(std::memory_order_relaxed))
        {
            uint32_t         j        = 0;
            bool             relisten = false;
            std::unique_lock l(tracker_lock);
            if (reset_tracking.load())
                break;  
            for (auto const &[handle, handle_info] : tracked_directory)
            {
                if (GetOverlappedResult(handle, &overlap[j], &bytes_read, FALSE))
                {
                    fprintf(stdout, "Bytes read : %d.", bytes_read);
                    uint32_t bytes_consumed = 0;
                    while (true)
                    {
                        _FILE_NOTIFY_INFORMATION *changes = reinterpret_cast<_FILE_NOTIFY_INFORMATION *>(
                            (uint8_t *)&read_buffer[j].buffer + bytes_consumed);

                        fprintf(stdout, "NextEntryOffset : %d.\n", changes->NextEntryOffset);
                        fprintf(stdout, "Action          : %d.\n", changes->Action);
                        fprintf(stdout, "FileNameLength  : %d.\n", changes->FileNameLength);

                        char    file_name[512] = {};
                        int32_t len =
                            WideCharToMultiByte(CP_ACP, NULL, (LPCWCH)changes->FileName,
                                                changes->FileNameLength / sizeof(WCHAR), file_name, 512, NULL, NULL);

                        printf("File Name -> Length : %d : %s.", len, file_name);

                        bytes_consumed += changes->NextEntryOffset;

                        // TODO :: Refactor it and make linux friendly
                        // Update the maintained mapping between handles, file_name and their directories

                        // Get the handle and see if the directory contains the file name
                        if (handle_info.whole_dir_tracked)
                        {
                            ChangeInfo info;
                            info.file      = file_name;
                            info.directory = handle_info.dir_name;
                            info.result    = TrackFor::WriteChange; // Fixed for now
                            std::unique_lock l(queue_lock);
                            change_info.push(std::move(info));
                        }
                        else
                        {
                            if (files_tracked_within_directory.contains(handle))
                            {
                                auto iter = files_tracked_within_directory.find(handle);
                                assert(iter != files_tracked_within_directory.end()); //  This check isn't required
                                auto const &vec       = (*iter).second;

                                auto        file_iter = std::find(vec.begin(), vec.end(), file_name);
                                if (file_iter != vec.end())
                                {
                                    ChangeInfo info;
                                    info.file      = file_name;
                                    info.directory = handle_info.dir_name;
                                    info.result    = TrackFor::WriteChange; // Fixed for now
                                    std::unique_lock l(queue_lock);
                                    change_info.push(std::move(info));
                                }
                                // else the file isn't tracked
                            }
                        }

                        if (changes->NextEntryOffset == 0)
                            break;
                    }

                    // Reset the overlapped structure
                    completed[j] = true; // Reset the overlapped structure, so that it would start listening again
                    overlap[j]   = {};
                    ResetEvent(events[j]);
                    overlap[j].hEvent = events[j];
                    relisten          = true;
                }
                j = j + 1;
            }

            std::this_thread::yield();

            if (relisten)
            {
                relisten = false;
                break;
            }
        }
    }
    // Clean up manually
    delete[] events;
    delete[] overlap;
    delete[] completed;
    delete[] read_buffer;
}

#endif
