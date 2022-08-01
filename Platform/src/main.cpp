
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
    //FileSystem::FileBufReader reader(&file);

    ////char                      buffer[512] = {};
    ////reader.Read(buffer, 10); 
    ////std::cout << buffer << std::endl; 

    //int   a;
    //float f; 
    //char  str[32] = {}; 
    //reader.Scanf("%d %f %s", &a, &f,str); 
    //std::cout << "a -> " << a << " and f -> " << f << " and str -> " << str << std::endl; 

    // Pretty printing testing 

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
    return 0;
}
