#if defined(__linux__)

#include "./file_track.hpp"

#include <unistd.h>
#include <sys/inotify.h>

FileTracker::FileTracker()
{
    inotify_handle = inotify_init();
}

void FileTracker::TrackFolder(FileSystem::FileContent &folder, TrackFor track_option, bool track_recursively)
{
    assert(folder.type == FileSystem::FileType::Directory);

    std::string dir         = folder.name;

    int         filter_mask = 0;

    // Anything except write change aren't properly handled for linux
    if (TestEnum(track_option, NameChange))
        filter_mask = filter_mask | IN_ATTRIB;
    if (TestEnum(track_option, WriteChange))
        filter_mask = filter_mask | IN_MODIFY;
    if (TestEnum(track_option, SizeChange))
        filter_mask = filter_mask | IN_ATTRIB;
    if (TestEnum(track_option, AttrChange))
        filter_mask = filter_mask | IN_ATTRIB;

    // Get full path of the directory first
    char full_path[128] = {};
    realpath(dir.c_str(), full_path);

    int result = inotify_add_watch(inotify_handle, full_path, filter_mask);
    assert(result >= 0);

    tracked_directory.insert(std::pair{result, DirectoryTrackInfo{true, true, dir}});
    name_to_dir_handle.insert(std::pair{dir, result});
}

void FileTracker::TrackFile(FileSystem::FileContent &file, TrackFor track_option)
{
    char full_path[128] = {};

    realpath(file.name.c_str(), full_path);
    fprintf(stderr, "\nFull path obtained as : %s.", full_path);

    std::string dir;
    std::string path     = std::string(full_path);

    auto        last_pos = path.find_last_of('/');

    if (last_pos != std::string::npos)
    {
        dir.resize(last_pos);
        std::copy(path.begin(), path.begin() + last_pos, dir.begin());
    }
    else
    {
        fprintf(stderr, "\nInvalid directory or file handle\n");
        exit(-1);
    }

    fprintf(stderr, "\nPath extracted : %s.", dir.c_str());

    int filter_mask = 0;

    // Anything except write change aren't properly handled for linux
    if (TestEnum(track_option, NameChange))
        filter_mask = filter_mask | IN_ATTRIB;
    if (TestEnum(track_option, WriteChange))
        filter_mask = filter_mask | IN_MODIFY;
    if (TestEnum(track_option, SizeChange))
        filter_mask = filter_mask | IN_ATTRIB;
    if (TestEnum(track_option, AttrChange))
        filter_mask = filter_mask | IN_ATTRIB;

    std::string file_name;
    std::copy(path.begin() + last_pos + 1, path.end(), std::back_inserter(file_name));

    fprintf(stderr, "\n");
    // Since file can be individually tracked, we have

    // Skip the check to determine if the file's parent directory is being tracked
    // for or not auto it = name_to_handle.find(dir);

    // In linux, each file can be seperately tracked irrespective of the directory
    // so assosciate each handle to it, and use accordingly
    int change_handle = inotify_add_watch(inotify_handle, full_path, filter_mask);
    assert(change_handle >= 0);

    name_to_file_handle.insert(std::pair{std::string(file.name.c_str()), change_handle});
    handle_to_file_name.insert(std::pair{change_handle, std::string(file.name.c_str())});
}

// timeout shamelessly ignored
void FileTracker::ListenForChanges(uint32_t timeout)
{
    struct Buffer
    {
        char buffer[1024] = {};
    };

    // A simple call to read is enough for blocking file tracking in linux

    Buffer storage;

    while (!stop_listening.load(std::memory_order_relaxed))
    {

        int bytes_read = read(inotify_handle, storage.buffer, 1024);
        assert(bytes_read < 1024);

        int pos = 0;

        // Its a pain to work with linux editor from the perspective of a developer.

        while (pos < bytes_read)
        {
            // Deliberately made blocking
            inotify_event *event = reinterpret_cast<inotify_event *>(
                storage.buffer + pos); // This could be an underfined behaviour too though, but not quite sure

            if (!event->len)
            {
                // Check if it is the director or file that's been tracked actively

                // Look for file first and then directory
                auto file_iter = handle_to_file_name.find(event->wd);
                if (file_iter != handle_to_file_name.end())
                {

                    fprintf(stderr, "[File] -> %s.\n", (*file_iter).second.c_str());
                    auto file_name = (*file_iter).second;
                    // Re-register it, since that file would have been replaced by another one
                    char full_path[128] = {};
                    realpath((*file_iter).second.c_str(), full_path);
                    // TODO :: Remove the bidirectional link
                    int change_handle = inotify_add_watch(inotify_handle, full_path, IN_MODIFY);
                    assert(change_handle >= 0);

                    {
                        ChangeInfo info;
                        info.file      = file_name;
                        info.directory = {};
                        info.result    = TrackFor::WriteChange;
                        std::unique_lock l(queue_lock);
                        change_info.push(std::move(info));
                    }

                    handle_to_file_name.erase(file_iter);
                    name_to_file_handle.insert(std::pair{std::string(file_name.c_str()), change_handle});
                    handle_to_file_name.insert(std::pair{change_handle, std::string(file_name.c_str())});
                }
                else
                {
                    auto dir_iter = tracked_directory.find(event->wd);
                    if (dir_iter != tracked_directory.end())
                    {
                        fprintf(stderr, "[Folder] -> %s.\n", (*dir_iter).second.dir_name.c_str());

                        {
                            ChangeInfo info;
                            info.file      = {};
                            info.directory = (*dir_iter).second.dir_name;
                            info.result    = TrackFor::WriteChange; // Default option
                            std::unique_lock l(queue_lock);
                            change_info.push(std::move(info));
                        }
                    }
                }
            }
            else
            {
                fprintf(stdout, "Name obtained : %s.\n", event->name);
            }

            pos = sizeof(*event) + event->len;
        }
    }
}

#endif
