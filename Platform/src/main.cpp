
// #include <format>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "./filesystem/fs.hpp"
#include "./filesystem/network_fs.hpp"

// Lets get started with file sharing then

int main(int argc, char *argv[])
{
    printf("Hello from Guthi : A framework for distributed application development\n");

    // File cache
    auto &cache = FileSystem::FileCache::GetLocalFileCache();

    auto  file  = FileSystem::OSFile::OpenFile("../../../../Platform/src/test.txt", FileSystem::FileMode::In);
    FileSystem::FileBufReader reader(&file);

    int   a;
    float f; 
    char  str[32] = {}; 
    reader.Scanf("%d %f %s", &a, &f,str); 
    std::cout << "a -> " << a << " and f -> " << f << " and str -> " << str << std::endl; 

    // Pretty printing testing 
    // TODO :: Implement unit testing framework for automated testing approach  
    FileSystem::FileContent content; 
    content.name = "Directory1"; 
    content.type = FileSystem::FileType::Directory; 

    FileSystem::FileContent images; 
    images.name = "Images"; 
    images.type = FileSystem::FileType::Directory; 

    FileSystem::FileContent gfile; 
    gfile.name = "img1.png"; 
    gfile.type = FileSystem::FileType::File; 

    // Temporary address assignment 
    images.contents.push_back(&gfile);
    images.contents.push_back(&gfile);
    images.contents.push_back(&gfile);
    images.contents.push_back(&gfile);

    content.contents.push_back(&images); 

    std::cout << content << std::endl; 

    std::cout << "\nFileSystem2\n";
    FileSystem::FileContent content2;
    content2.name = "Directory1";
    content2.type = FileSystem::FileType::Directory;

    FileSystem::FileContent images2;
    images2.name = "Images";
    images2.type = FileSystem::FileType::Directory;

    FileSystem::FileContent gfile2;
    gfile2.name = "img2.jpeg";
    gfile2.type = FileSystem::FileType::File;

    // Temporary address assignment
    images2.contents.push_back(&gfile2);
    images2.contents.push_back(&gfile2);
    images2.contents.push_back(&gfile2);
    images2.contents.push_back(&gfile2);

    content2.contents.push_back(&images2); 

    auto v      = std::vector{&content, &content2};
    auto merged = FileSystem::MergeTotalFileContent(v);

    std::cout << content2 << std::endl; 
    std::cout << "Merged File Contents : " << std::endl; 
    std::cout << *merged << std::endl; 
    return 0;
}
