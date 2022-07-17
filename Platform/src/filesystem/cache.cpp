#include "./fs.hpp"

// should it be a mapping from string to string? or from string to file?
// TODO :: Maintain an active list of file that are currently opened but not yet submitted, if any node attemps to read
// that file, submit the change and forward file to requester

// It could be extended to first consider if there's sufficient memory to store file in RAM before needing to cache it. 

namespace FileSystem
{
static FileCache g_file_cache = FileCache("./tmp/");

bool FileCache::IsFileCached(std::string const &file) const
{
    return cached_files.find(file) != cached_files.end();
}

// Adding only the name of file to cache is no fun, take File and save it 

void FileCache::AddToCache(const GenericFile<OSFile>& file)
{
    // cached_files.insert(file.file_name);

}

bool FileCache::RemoveFromCache(std::string const& file_name)
{
    if (cached_files.contains(file_name))
        cached_files.erase(file_name);

	return false;
}

FileCache &FileCache::GetLocalFileCache()
{
    return g_file_cache;
}
}
