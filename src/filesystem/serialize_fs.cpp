#include "./serialize_fs.hpp"

std::vector<uint8_t> FileSystem::NetworkFS::SerializeLocalFS() const
{

    using Buffer        = std::vector<uint8_t>;
    using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
    using InputAdapter  = bitsery::InputBufferAdapter<Buffer>;

    using TContext      = bitsery::ext::PointerLinkingContext;

    using Serializer    = bitsery::Serializer<OutputAdapter, TContext>;
    using Deserializer  = bitsery::Deserializer<InputAdapter, TContext>;

    Buffer     buffer(512);

    TContext   ctx{};
    Serializer ser(ctx, buffer);
    ser.object(this->local_fs);
    ser.adapter().flush();

    uint32_t written = ser.adapter().writtenBytesCount();
    buffer.resize(written);

    std::cout << "Written count : " << buffer.size() << " " << written << std::endl;
    return buffer;
}

bool FileSystem::NetworkFS::DeserializeToFileContent(std::vector<uint8_t> &data, FileContent& content)
{
    using Buffer        = std::vector<uint8_t>;
    using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
    using InputAdapter  = bitsery::InputBufferAdapter<Buffer>;

    using TContext      = bitsery::ext::PointerLinkingContext;

    using Serializer    = bitsery::Serializer<OutputAdapter, TContext>;
    using Deserializer  = bitsery::Deserializer<InputAdapter, TContext>;

    TContext ctx; 
    Deserializer            des(ctx, data.begin(), data.size());
    des.object(content); 
    
    return des.adapter().error() == bitsery::ReaderError::NoError && des.adapter().isCompletedSuccessfully();
}