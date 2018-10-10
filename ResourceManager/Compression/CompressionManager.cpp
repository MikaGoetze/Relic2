//
// Created by mikag on 10/10/2018.
//

#include <Debugging/Logger.h>
#include "CompressionManager.h"

void CompressionManager::WriteInt(FILE *fp, int i)
{
    size_t written = fwrite(&i, sizeof(i), 1, fp);
    if(written != 1) Logger::Log("[CompressionManager] [ERR} Failed to write int.");
}

void CompressionManager::WriteBin(FILE *fp, const void *bytes, size_t size)
{
    size_t written = fwrite(bytes, 1, size, fp);
    if(written != size) Logger::Log("[CompressionManager] [ERR} Failed to write bytes.");
}

void CompressionManager::ReadInt(FILE *fp, int *i)
{
    size_t read = fread(i, sizeof(*i), 1, fp);
    if(read != 1) Logger::Log("[CompressionManager] [ERR] Failed to read int.");
}

void CompressionManager::ReadBin(FILE *fp, void *bytes, size_t size)
{
    size_t read = fread(bytes, 1, size, fp);
    if(read != size) Logger::Log("[CompressionManager] [ERR] Failed to read bytes.");
}

void CompressionManager::SeekBin(FILE *fp, long offset, int origin)
{
    if(fseek(fp, offset, origin)) Logger::Log("[CompressionManager] [ERR} Failed to seek file.");
}

CompressionManager::Metadata *CompressionManager::ReadMetadata(FILE *fp)
{
    //Make sure that the file pointer is reset.
    SeekBin(fp, 0, SEEK_SET);

    //Create a new metadata object
    auto * metadata = new Metadata();

    //Now we read a fixed size number of bytes, and build metadata.
    ReadBin(fp, static_cast<void*>(metadata), sizeof(*metadata));

    return metadata;
}

void CompressionManager::WriteMetadata(FILE *fp, const struct Metadata *metadata)
{
    //Go back to the start of the file and write out the metadata.
    fseek(fp, 0, SEEK_SET);
    WriteBin(fp, metadata, sizeof(*metadata));
}

void CompressionManager::WriteFileTable(FILE *fp)
{
    struct Metadata meta{};
    meta.version_number = version_number;
    meta.lut_size = sizeof(struct Pair) * fileTable->lut_size;
    meta.filetable_size = sizeof(*fileTable);

    //Write the metadata to file
    WriteMetadata(fp, &meta);

    //Write the file table to file.
    WriteBin(fp, fileTable, meta.filetable_size);
    WriteBin(fp, fileTable->lut, meta.lut_size);
}

void CompressionManager::ReadFileTable(FILE *fp)
{
    //Read the metadata
    struct Metadata* meta = ReadMetadata(fp);

    //Now we read the file table
    fileTable = new struct FileTable(meta->lut_size / sizeof(struct Pair));

    ReadBin(fp, fileTable, meta->filetable_size);
    ReadBin(fp, fileTable->lut, meta->lut_size);
}

void CompressionManager::Test()
{
    FILE* fp = fopen("test.bin", "wb+");
    fileTable = new FileTable(3);

    fileTable->lut[0] = Pair(0, 5);
    fileTable->lut[1] = Pair(1, 10);
    fileTable->lut[2] = Pair(2, 15);

    WriteFileTable(fp);

    //Now we clear the filetable, and read
    fclose(fp);

    delete fileTable;
    fileTable = nullptr;

    fp = fopen("test.bin", "rb");
    ReadFileTable(fp);

    int size = static_cast<int>(fileTable->lut_size);
    for(int i = 0; i < size; i++)
    {
        int val = static_cast<int>(fileTable->lut[i].offset);
        printf("%i: %i\n", i, val);
    }

    fclose(fp);
    delete fileTable;
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
