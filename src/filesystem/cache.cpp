#include "./fs.hpp"
#include "./runtime/sys_info.hpp"

// Multi Level Cache - Comparison using timestamp
//      i) in RAM
//      ii) in Disk

namespace FileSystem
{
// static FileCache g_file_cache = FileCache("./tmp/");

bool FileCache::IsFileCached(std::string_view file) const
{
    // Unnecessary conversion just for comparison, meh
    return this->caches.OfflineCache.find(std::string{file}) != this->caches.OfflineCache.end();
}

// Adding only the name of file to cache is no fun, take File and save it

void FileCache::AddFileToCache(RawFile const &file)
{
    // If enough RAM available, cache on the RAM (or rely on the OS virtual paging ??) else cache on the disk at
    // temporary location
    // TODO :: Will it be required? Since virtual paging can take care of the most of the RAM usage -> Thrashing and
    // system load is still the problem though

    auto           memory_info = Runtime::GetSysMemoryInfo();
    FileCacheEntry entry;
    entry.file_name = file.file_name;
    entry.size      = file.file_size;
    entry.timestamp = chrono::system_clock::now();
    entry.location  = FileCache::CacheLocation::OnRAM;

    // TODO :: Implement dynamic cache load management
    if (memory_info.memory_load > 0.75f)
    {
        entry.location = FileCache::CacheLocation::OnDisk;
        FILE *fp       = fopen(entry.file_name.c_str(), "wb+");

        if (!fp)
        {
            exit(-1);
        }
        fwrite(file.data, sizeof(*file.data), file.file_size, fp);
        fclose(fp);
        this->caches.OfflineCache[entry.file_name] = std::move(entry);
        return;
    }

    //  Else store  on the RAM and take ownership of the included data
    entry.data = std::unique_ptr<uint8_t[]>(new uint8_t[entry.size]);

#if defined(_MSC_VER)
    memcpy_s(entry.data.get(), sizeof(uint8_t) * entry.size, file.data, sizeof(uint8_t) * entry.size);
#else
    memcpy(entry.data.get(), file.data, sizeof(uint8_t) * entry.size);
#endif
    this->caches.OfflineCache[entry.file_name] = std::move(entry);
}

bool FileCache::RemoveFromCache(std::string_view file_name)
{
    if (this->IsFileCached(file_name))
        return false;

    auto it = this->caches.OfflineCache.find(std::string(file_name));
    if (it != this->caches.OfflineCache.end())
        this->caches.OfflineCache.erase(it);

    return true;
}

FileCache::FileCacheEntry *FileCache::GetCachedEntry(std::string_view file)
{
    auto it = this->caches.OfflineCache.find(std::string(file));
    if (it != this->caches.OfflineCache.end())
    {
        return &(*it).second;
    }

    return nullptr;
}

// User should check for the validity of the obtained file using RawFile.data == nullptr;

RawFile FileCache::GetCachedFile(std::string_view file)
{
    auto cache_entry = this->GetCachedEntry(file);

    // TODO :: Connect with the network to get latest timestamps from other node and compare accordingly

    RawFile r_file = {};
    if (cache_entry)
    {
        r_file.file_name = file;
        r_file.file_size = cache_entry->size;

        if (cache_entry->location == CacheLocation::OnRAM)
        {
            r_file.data = new uint8_t[r_file.file_size];
            memcpy_s(r_file.data, r_file.file_size, cache_entry->data.get(), cache_entry->size);
        }
        else
        {
            FILE *fp = fopen(r_file.file_name.c_str(), "rb");
            if (!fp)
            {
                fprintf(stderr, "Failed to open file : %.*s.\n", file.size(), file.data());
            }
            else
            {
                r_file.data = new uint8_t[r_file.file_size];
                fread(r_file.data, sizeof(uint8_t), r_file.file_size, fp);
                fclose(fp);
            }
        }
    }
    return r_file;
}

//FileCache &FileCache::GetLocalFileCache()
//{
//    return g_file_cache;
//}

} // namespace FileSystem
