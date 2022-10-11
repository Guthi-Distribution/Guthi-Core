#pragma once

#include "./network_fs.hpp"

// #include <bitsery/bitsery.h>
#include <vector>
#include <iostream>

#include "../../include/serializer/bitsery/bitsery.h"
#include "../../include/serializer/bitsery/adapter/buffer.h"
#include "../../include/serializer/bitsery/traits/vector.h"
#include "../../include/serializer/bitsery/traits/string.h"
#include "../../include/serializer/bitsery/ext/pointer.h"
#include "../../include/serializer/bitsery/ext/std_smart_ptr.h"
namespace FileSystem
{
template <typename S> void serialize(S &s, FileSystem::FileContent &content)
{
    s.value2b(content.type);
    s.value2b(content.network_node.ip_version);
    s.value2b(content.network_node.port_addr);

    for (auto &x : content.network_node.ip_addr)
        s.value1b(x);
    s.container1b(content.name, 64);

    // Serialize the smart pointers
    s.container(content.contents, FileSystem::FileContent::MAX_FILE_CAPACITY,
                [](S &s, std::shared_ptr<FileSystem::FileContent> &it) { s.ext(it, bitsery::ext::StdSmartPtr{}); });

}
} // namespace FileSystem