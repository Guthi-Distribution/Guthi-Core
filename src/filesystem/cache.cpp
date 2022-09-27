#include "./fs.hpp"

// Multi Level Cache - Comparison using timestamp
//      i) in RAM
//      ii) in Disk

namespace FileSystem
{
static FileCache g_file_cache = FileCache("./tmp/");

bool             FileCache::IsFileCached(std::string_view file) const
{
    auto it = std::find_if(this->caches.OnRAMCached.begin(), this->caches.OnRAMCached.end(),
                           [&](const auto &c) { return file == c.second.file_name; });

    if (it != this->caches.OnRAMCached.end())
        return true;

    // Unnecessary conversion just for comparison, meh
    return this->caches.OnDiskCached.find(std::string{file}) != this->caches.OnDiskCached.end();
}

// Adding only the name of file to cache is no fun, take File and save it

void FileCache::AddFileToCache(RawFile const &file)
{
    // If enough RAM available, cache on the RAM (or rely on the OS virtual paging ??) else cache on the disk at
    // temporary location

    FileCacheEntry entry;
    entry.file_name = file.file_name;
    entry.location  = FileCache::CacheLocation::InRAM;
    entry.size      = file.file_size;
    entry.timestamp = chrono::system_clock::now();
    entry.data      = std::unique_ptr<uint8_t[]>(new uint8_t[entry.size]);
    #if defined(_MSC_VER)
    memcpy_s(entry.data.get(), sizeof(uint8_t) * entry.size, file.data,sizeof(uint8_t) * entry.size); 
    #else 
    memcpy(entry.data.get(), file.data, sizeof(uint8_t) * entry.size); 
    #endif

    this->caches.OnRAMCached[entry.file_name] = std::move(entry); 
}

bool FileCache::RemoveFromCache(std::string_view file_name)
{
    if (this->IsFileCached(file_name))
        return false;

    auto it = this->caches.OnRAMCached.find(std::string(file_name));
    if (it != this->caches.OnRAMCached.end())
        this->caches.OnRAMCached.erase(it); 

    auto nit = this->caches.OnDiskCached.find(std::string(file_name));
    if (nit != this->caches.OnDiskCached.end())
        this->caches.OnDiskCached.erase(it); 
    return true; 
}

FileCache::FileCacheEntry *FileCache::GetCachedFile(std::string_view file)
{
    auto it = this->caches.OnRAMCached.find(std::string(file));
    if (it != this->caches.OnRAMCached.end())
        return &(*it).second;

    auto nit = this->caches.OnDiskCached.find(std::string(file));
    if (nit != this->caches.OnDiskCached.end())
        return &(*it).second;

    return nullptr;
}

FileCache &FileCache::GetLocalFileCache()
{
    return g_file_cache;
}
} // namespace FileSystem
