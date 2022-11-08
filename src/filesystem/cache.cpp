#include "./fs.hpp"
#include "./runtime/sys_info.hpp"

// Multi Level Cache - Comparison using timestamp
//      i) in RAM
//      ii) in Disk

namespace FileSystem
{

bool FileCache::IsFileCached(std::string_view file, uint8_t ip_addr[4]) const
{
    // Unnecessary conversion just for comparison, meh
    auto count = this->caches.OfflineCache.count(std::string(file));
    if (!count)
        return false;

    auto iter = this->caches.OfflineCache.lower_bound(std::string(file));

    for (; iter != this->caches.OfflineCache.end(); iter++)
    {
        if (memcmp(ip_addr, iter->second.ip_addr, 4 * sizeof(uint8_t)))
        {
            return true;
        }
    }
    return false;
}

// Adding only the name of file to cache is no fun, take File and save it

void FileCache::AddFileToCache(RawFile &file, uint8_t ip_addr[4])
{
    // If enough RAM available, cache on the RAM (or rely on the OS virtual paging ??) else cache on the disk at
    // temporary location
    // TODO :: Will it be required? Since virtual paging can take care of the most of the RAM usage -> Thrashing and
    // system load is still the problem though

    // TODO :: Implement LFU cache
    auto           memory_info = Runtime::GetSysMemoryInfo();
    FileCacheEntry entry;
    entry.file_name = file.file_name;
    entry.size      = file.file_size;

    for (uint32_t i = 0; i < 4; ++i)
        entry.ip_addr[i] = ip_addr[i];

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
        // this->caches.OfflineCache[entry.file_name] = std::move(entry);
        this->caches.OfflineCache.insert({entry.file_name, std::move(entry)});
        return;
    }

    //  Else store  on the RAM and take ownership of the included data
    //    entry.data = std::unique_ptr<uint8_t[]>(new uint8_t[entry.size]);
    //
    // #if defined(_MSC_VER)
    //    memcpy_s(entry.data.get(), sizeof(uint8_t) * entry.size, file.data, sizeof(uint8_t) * entry.size);
    // #else
    //    memcpy(entry.data.get(), file.data, sizeof(uint8_t) * entry.size);
    // #endif
    entry.data = std::unique_ptr<uint8_t[]>(file.data);
    file.data  = nullptr;
    this->caches.OfflineCache.insert({entry.file_name, std::move(entry)});

    file.DestroyFile();
}

bool FileCache::RemoveFromCache(std::string_view file_name,uint8_t ip_addr[4])
{
    auto count = this->caches.OfflineCache.count(std::string(file_name));
    if (!count)
        return false;

    auto iter = this->caches.OfflineCache.lower_bound(std::string(file_name));

    for (; iter != this->caches.OfflineCache.end(); iter++)
    {
        if (!memcmp(ip_addr, iter->second.ip_addr, 4 * sizeof(uint8_t)))
        {
            this->caches.OfflineCache.erase(iter);
            return true;
        }
    }
    return false;
}

FileCache::FileCacheEntry *FileCache::GetCachedEntry(std::string_view file, uint8_t ip_addr[4])
{
    /*
    auto it = this->caches.OfflineCache.find(std::string(file));
    if (it != this->caches.OfflineCache.end())
    {
        return &(*it).second;
    }
    return nullptr;
    */

    auto count = this->caches.OfflineCache.count(std::string(file));
    if (!count)
        return nullptr;

    auto iter = this->caches.OfflineCache.lower_bound(std::string(file));

    for (; iter != this->caches.OfflineCache.end(); iter++)
    {
        if (memcmp(ip_addr, iter->second.ip_addr, 4 * sizeof(uint8_t)))
        {
            return &(*iter).second;
        }
    }
    return nullptr;
}

// User should check for the validity of the obtained file using RawFile.data == nullptr;

RawFile FileCache::GetCachedFile(std::string_view file, uint8_t ip_addr[4])
{
    auto cache_entry = this->GetCachedEntry(file, ip_addr);

    // TODO :: Connect with the network to get latest timestamps from other node and compare accordingly

    RawFile r_file = {};
    if (cache_entry)
    {
        r_file.file_name = file;
        r_file.file_size = cache_entry->size;

        if (cache_entry->location == CacheLocation::OnRAM)
        {
            r_file.data = new uint8_t[r_file.file_size];
            safe_memcpy(r_file.data, r_file.file_size, cache_entry->data.get(), cache_entry->size);
        }
        else
        {
            FILE *fp = fopen(r_file.file_name.c_str(), "rb");
            if (!fp)
            {
                fprintf(stderr, "Failed to open file : %.*s.\n", (uint32_t)file.size(), file.data());
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

// FileCache &FileCache::GetLocalFileCache()
//{
//     return g_file_cache;
// }

} // namespace FileSystem
