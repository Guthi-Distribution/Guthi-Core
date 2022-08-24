#include "./fs.hpp"

namespace fs     = std::filesystem;
namespace chrono = std::chrono;

namespace FileSystem
{
OSFile OSFile::OpenFile(const std::string &file, FileMode mode)
{
    OSFile                os_file; 
    const constexpr char *file_modes[] = {"r", "w", "r+", "a+"};
    os_file.file_name                          = file;
    os_file.file_mode                          = mode;
    os_file.file_status                        = FileStatus::Opened;

    if (!(os_file.file_ptr = fopen(file.c_str(), file_modes[static_cast<size_t>(mode)])))
    {
        fprintf(stdout, "Failed to open the file %s.", file.c_str()); 
        return os_file;  // Its error but not handled :D
    }

    fseek(os_file.file_ptr, 0, SEEK_END);
    os_file.file_size = ftell(os_file.file_ptr);
    rewind(os_file.file_ptr);
    return os_file;
}

void OSFile::CloseFile(OSFile& file)
{
    file.file_status = FileStatus::Closed;
    fclose(file.file_ptr);
}

fs::file_time_type OSFile::QueryLastWriteTime() const
{
    return fs::last_write_time(file_name);
}

uint32_t OSFile::FetchBlockWithSize(uint8_t *data, uint32_t size)
{
    return (uint32_t)std::fread(data, sizeof(*data), size, file_ptr);
}

uint32_t OSFile::WriteBlockWithSize(uint8_t* data, uint32_t size)
{
    return (uint32_t)std::fwrite(data, sizeof(*data), size, file_ptr);
}

void FetchFile(const std::string &file)
{
    // How to do the verification though?
    // Check the timestamp? -> I guess that should be the most reasonable way to do it.
    // Checksum ?
    // if it is already available in the cache, read from it.
    // else report above layer to retrieve the file and then save it
}

} // namespace FileSystem
