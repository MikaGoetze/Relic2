//
// Created by mikag on 10/10/2018.
//

#ifndef RELIC_2_0_COMPRESSIONMANAGER_H
#define RELIC_2_0_COMPRESSIONMANAGER_H

#include <ResourceManager/Compression/lz4/lz4hc.h>
#include <cstdio>
#include <string>
#include <vector>
#include <Debugging/Logger.h>


class CompressionManager
{
private:
    //Structs that will be used for compression.
    struct Metadata
    {
        size_t filetable_size;
        size_t lut_size;
        uint_fast8_t version_number;
    };

    struct Pair
    {
        Pair();

        Pair(uint_fast32_t guid, size_t offset);

        size_t offset;
        uint_fast32_t guid;
    };

    struct Resource
    {
        void* data;
        size_t dataSize;
        uint_fast32_t guid;
    };

    struct FileTable
    {
        size_t lut_size;
        Pair* lut;

        explicit FileTable(size_t number_of_elements)
        {
            lut = new Pair[number_of_elements];
            lut_size = number_of_elements;
        }

        ~FileTable()
        {
            delete[] lut;
        }
    };


    //Keep track of the version used to encode it (in case of changes)
    const uint_fast8_t version_number = 1;
    const uint_fast32_t block_bytes = 1024;

    FILE* currentFile;
    FileTable* fileTable;
    std::string currentRPACK;
    std::vector<Resource*> *currentResources;
    bool needsWrite = false;

    //Read and write functions.
    void WriteInt(int i);
    void WriteBin(const void* bytes, size_t size);
    void ReadInt(int* i);
    void ReadBin(void* bytes, size_t size);
    void SeekBin(long offset, int origin);

    //Read and write metadata from files.
    Metadata* ReadMetadata();
    void WriteMetadata(const Metadata* metadata);

    void WriteFileTable();
    void ReadFileTable();

    //Search through file table
    Pair* SearchForGUID(uint_fast32_t guid);

public:

    CompressionManager();
    ~CompressionManager();
    void SetRPACK(std::string target);
    void AddResource(void* data, size_t dataSize, uint_fast32_t guid);
    void WriteRPACK();

    template <typename T>
    T* LoadResource(uint_fast32_t guid);

    void Test();
};

//Template definitions
template<typename T>
T* CompressionManager::LoadResource(uint_fast32_t guid)
{
    currentFile = fopen(currentRPACK.c_str(), "rb+");
    if(fileTable == nullptr)
        ReadFileTable();

    //Now we look up the requested resource
    Pair* pair = SearchForGUID(guid);

    //If we didn't find it, then it's not in this RPACK
    if(pair == nullptr)
    {
        Logger::Log("[CompressionManager] [WRN] Could not find file...");
        return nullptr;
    }

    T* t = new T();

    size_t ftOffset = sizeof(Pair) * fileTable->lut_size + sizeof(*fileTable) + sizeof(Metadata);
    SeekBin(pair->offset + (ftOffset), SEEK_SET);
    ReadBin(t, sizeof(*t));
    return t;
}

#endif //RELIC_2_0_COMPRESSIONMANAGER_H
