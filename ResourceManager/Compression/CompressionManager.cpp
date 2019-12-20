//
// Created by mikag on 10/10/2018.
//

#include <Debugging/Logger.h>
#include <cstring>
#include "CompressionManager.h"

void CompressionManager::WriteInt(int i)
{
    size_t written = fwrite(&i, sizeof(i), 1, currentFile);
    if (written != 1) Logger::Log("[CompressionManager] [ERR} Failed to write int.");
}

void CompressionManager::WriteBin(const void *bytes, size_t size)
{
    size_t written = fwrite(bytes, 1, size, currentFile);
    if (written != size) Logger::Log("[CompressionManager] [ERR} Failed to write bytes.");
}

void CompressionManager::ReadInt(int *i)
{
    size_t read = fread(i, sizeof(*i), 1, currentFile);
    if (read != 1) Logger::Log("[CompressionManager] [ERR] Failed to read int.");
}

void CompressionManager::ReadBin(void *bytes, size_t size)
{
    size_t read = fread(bytes, 1, size, currentFile);
    if (read != size) Logger::Log("[CompressionManager] [ERR] Failed to read bytes.");
}

void CompressionManager::SeekBin(long offset, int origin)
{
    if (fseek(currentFile, offset, origin)) Logger::Log("[CompressionManager] [ERR} Failed to seek file.");
}

CompressionManager::Metadata *CompressionManager::ReadMetadata()
{
    //Make sure that the file pointer is reset.
    SeekBin(0, SEEK_SET);

    //Create a new metadata object
    auto *metadata = new Metadata();

    //Now we read a fixed size number of bytes, and build metadata.
    ReadBin(static_cast<void *>(metadata), sizeof(*metadata));

    return metadata;
}

void CompressionManager::WriteMetadata(const Metadata *metadata)
{
    //Go back to the start of the file and write out the metadata.
    fseek(currentFile, 0, SEEK_SET);
    WriteBin(metadata, sizeof(*metadata));
}

void CompressionManager::WriteFileTable()
{
    Metadata meta{};
    meta.version_number = version_number;
    meta.lut_size = sizeof(Pair) * fileTable->lut_size;
    meta.filetable_size = sizeof(*fileTable);

    //Write the metadata to file
    WriteMetadata(&meta);

    //Write the file table to file.
    WriteBin(fileTable, meta.filetable_size);
    WriteBin(fileTable->lut, meta.lut_size);
}

void CompressionManager::ReadFileTable()
{
    //Read the metadata
    Metadata *meta = ReadMetadata();
    if (meta->version_number != version_number)
    {
        Logger::Log("[CompressionManager] [ERR} RPACK is of the wrong version. Cannot read.");
        return;
    }

    //Now we read the file table
    fileTable = new FileTable(meta->lut_size / sizeof(Pair));

    ReadBin(fileTable, meta->filetable_size);
    ReadBin(fileTable->lut, meta->lut_size);
}

void CompressionManager::Test()
{
    int a = 4;
    Pair b{10, 10};
    float c = 12.4;

    SetRPACK("test.rpack");

    AddResource(&a, sizeof(a), 0);
    AddResource(&b, sizeof(b), 1);
    AddResource(&c, sizeof(c), 2);

    WriteRPACK();

    printf("%i\n", *LoadResource<int>(0));
    Pair *p = LoadResource<Pair>(1);
    printf("%i, %i\n", static_cast<int>(p->offset), static_cast<int>(p->guid));
    printf("%f\n", *LoadResource<float>(2));
}

void CompressionManager::SetRPACK(std::string target)
{
    //If there was a previously loaded RPACK and it needs a write, let's write it now.
    if (!currentRPACK.empty() && ::strcmp(target.c_str(), currentRPACK.c_str()) != 0 && needsWrite)
    {
        WriteRPACK();
    }

    currentRPACK = target;

    //Clear and reset current resources.
    for (auto &currentResource : *currentResources)
    {
        delete currentResource;
    }
    currentResources->clear();
}

void CompressionManager::AddResource(void *data, size_t dataSize, uint_fast32_t guid)
{
    if (currentRPACK.empty())
    {
        Logger::Log("[CompressionManager] {ERR} Trying to add file before RPACK is selected.");
        return;
    }
    auto *resource = new Resource();
    resource->data = data;
    resource->dataSize = dataSize;
    resource->guid = guid;

    currentResources->push_back(resource);
    needsWrite = true;
}

void CompressionManager::WriteRPACK()
{
    if (!needsWrite) return;
    currentFile = fopen(currentRPACK.c_str(), "wb");

    //Delete the old file table, if there is one.
    delete fileTable;

    fileTable = new FileTable(currentResources->size() + 1);

    //Next we compress all of our resources, ready to write the file table
    for (auto resource : *currentResources)
    {
        resource->compressedSize = Compress(resource->data, resource->compressedData, resource->dataSize);
    }

    //Go through each loaded resource and add it to the file table.
    uint_fast32_t offset = 0;
    for (unsigned long i = 0; i < currentResources->size(); i++)
    {
        fileTable->lut[i].offset = offset;
        fileTable->lut[i].guid = currentResources->at(i)->guid;
        offset += currentResources->at(i)->compressedSize;
    }


    fileTable->lut[currentResources->size()].offset = offset;
    fileTable->lut[currentResources->size()].guid = UINT32_MAX;

    //Write out the file table.
    WriteFileTable();

    //Now we write out the actual files
    for (auto &currentResource : *currentResources)
    {
        WriteBin(currentResource->compressedData, currentResource->compressedSize);
        //Delete the compressed data since we're done with it
        delete static_cast<char *>(currentResource->compressedData);
        currentResource->compressedSize = 0;
    }

    fclose(currentFile);
    currentFile = nullptr;

    needsWrite = false;
}

CompressionManager::~CompressionManager()
{
    delete fileTable;

    for (auto &currentResource : *currentResources)
    {
        delete currentResource;
    }

    delete currentResources;
}

CompressionManager::CompressionManager()
{
    currentRPACK = "";
    currentResources = new std::vector<Resource *>();
    currentFile = nullptr;
    fileTable = nullptr;
}

CompressionManager::Pair *CompressionManager::SearchForGUID(uint_fast32_t guid, size_t *index)
{
    for (int i = 0; i < fileTable->lut_size; i++)
    {
        if (fileTable->lut[i].guid == guid)
        {
            if (index != nullptr) *index = static_cast<size_t>(i);
            return &fileTable->lut[i];
        }
    }
    return nullptr;
}


size_t CompressionManager::Compress(const void *bytes, void *&compressedBytes, size_t size)
{
    char *compressed = new char[size];
    int compressedSize = LZ4_compress_default(static_cast<const char *>(bytes), compressed, static_cast<int>(size),
                                              static_cast<int>(size));

    if (compressedSize != 0)
    {
        //Now we init the compressedBytes field to the correct size, and copy the data
        compressedBytes = new char[compressedSize];
        memcpy(compressedBytes, compressed, static_cast<size_t>(compressedSize));
    } else
    {
        //If compression fails, we just copy the uncompressed buffer.
        compressedBytes = new char[size];
        memcpy(compressedBytes, bytes, size);
    }
    delete[] compressed;

    return compressedSize == 0 ? size : compressedSize;
}

void
CompressionManager::Decompress(void *bytes, void *decompressedBytes, size_t compressedSize, size_t decompressedSize)
{
    int size = LZ4_decompress_safe(static_cast<const char *>(bytes), static_cast<char *>(decompressedBytes),
                                   static_cast<int>(compressedSize), static_cast<int>(decompressedSize));
    if (size < 0 || size != decompressedSize)
    {
        Logger::Log(
                "[CompressionManager] [ERR] Size of compressed resource did not match expected. Type mismatch is likely.");
    }
}

CompressionManager::Pair::Pair(uint_fast32_t guid, size_t offset)
{
    this->guid = guid;
    this->offset = offset;
}

CompressionManager::Pair::Pair()
{
    guid = 0;
    offset = 0;
}
