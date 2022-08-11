#include "./fs.hpp"
#include <vector>

namespace FileSystem
{
enum class FileType
{
    Directory,
    File
};

// Helpers
std::ostream &operator<<(std::ostream &os, FileType &file_type)
{
    switch (file_type)
    {
    case FileType::Directory:
        return os << "Directory";
    case FileType::File:
        return os << "File";
    default:
        break;
    }
    return os;
}

// Represent the file system locally first
struct FileContent
{
    // NodeDetails node; // Node details from which its fetched
    std::string                name;
    FileType                   type;

    std::vector<FileContent *> contents;
};

// Pretty print the FileContent

void FileFormatter(std::ostream &os, const FileContent &file, const uint32_t depth)
{
    // TODO :: Upgrade it for multiple console supports
    // For now printable ascii characters

    // The objective is to render tree structure of the directory
    // Dir1
    // |____ Images
    // |    |_____ img1.png
    // |    |_____ img2.png
    // |
    // |___ Videos
    //     |____ video1.mp4 and so on

    constexpr uint32_t sub_folder_spacing = 5;

    for (int32_t i = 0; i < depth - 1; ++i)
    {
        os << '|';
        for (int32_t j = 0; j < sub_folder_spacing; ++j)
            os << ' ';
    }

    os << '|';
    for (int32_t j = 0; j < sub_folder_spacing; ++j)
        os << '_';

    if (file.type == FileType::File)
    {
        os << "File [" << file.name << ']' << '\n';
        return;
    }
    else
        os << "Folder [" << file.name << ']' << '\n';

    for (auto const &dir : file.contents)
        FileFormatter(os, *dir, depth + 1);
}

std::ostream &operator<<(std::ostream &os, const FileContent &file)
{
    FileFormatter(os, file, 1);
    return os;
}

// AddFile/Directory to the current content
bool AddFile(FileContent &file_dir, FileContent *file)
{
    file_dir.contents.push_back(file);
    return true;
}

// Remove file from the current directory
bool RemoveFile(FileContent &file_dir, FileContent *file)
{
    //// Shall just name be compared to mark for deletion instead of while directory recursively?
    //// auto iter = std::find(file_dir.contents.begin(), file_dir.contents.end(), file);
    auto iter = std::find_if(file_dir.contents.begin(), file_dir.contents.end(),
                             [&](const auto &f1) { return f1->name == file->name; });
    if (iter == file_dir.contents.end())
        return false;
    file_dir.contents.erase(iter);
    return true;
}

// file1 and file2 are mergeable files with same level and same directory label
// file1 is modified to incorporate changes from file2

void MergeSingleFileContent(FileContent *file1, FileContent *file2)
{
    for (auto const &x : file2->contents)
    {
        auto iter = std::find_if(file1->contents.begin(), file1->contents.end(),
                                 [&](const auto &f1) { return f1->name == x->name; });

        if (iter == file1->contents.end() || x->type == FileType::File)
        {
            // TODO :: Implement collision resolution here
            // currently just add with multiple same names

            file1->contents.push_back(x);
        }
        else
        {
            MergeSingleFileContent(*iter, x);
        }
    }
}

// The most important function on the whole network file system
FileContent *MergeTotalFileContent(std::vector<FileContent *> const &files)
{
    // Start at the top hirarchy and move downward, at each step merging file at the same hierarchy
    // If the folders aren't same at that level, create seperate directory
    auto dfs        = new FileContent();
    dfs->name       = "Guthi File System"; // Top root level directory content
    dfs->type       = FileSystem::FileType::Directory;

    auto files_copy = files;

    // Run down over all the file hierarchy simultaneously
    for (auto const &dir : files)
    {
        auto iter = std::find_if(dfs->contents.begin(), dfs->contents.end(),
                                 [&](const auto &f1) { return f1->name == dir->name; });

        if (iter == dfs->contents.end())
        {
            dfs->contents.push_back(dir);
        }
        else
        {
            MergeSingleFileContent(*iter, dir);
        }
    }
    return dfs;
}

struct NetworkFS
{
    // Only the file structure need to be transferred across to form the view of complete filesystem
    // It involves :
    //          -- Merging of folders with same name
    //          -- Querying for file independent of the nodes in which its present
    // Learn to merge FileContent from multiple nodes to provide coherent view
};
} // namespace FileSystem