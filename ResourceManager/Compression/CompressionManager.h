//
// Created by mikag on 10/10/2018.
//

#ifndef RELIC_2_0_COMPRESSIONMANAGER_H
#define RELIC_2_0_COMPRESSIONMANAGER_H

#include <ResourceManager/Compression/lz4/lz4hc.h>
#include <cstdio>


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

    FILE* current_file;
    struct FileTable* fileTable;

    //Read and write functions.
    void WriteInt(FILE* fp, int i);
    void WriteBin(FILE* fp, const void* bytes, size_t size);
    void ReadInt(FILE* fp, int* i);
    void ReadBin(FILE* fp, void* bytes, size_t size);
    void SeekBin(FILE* fp, long offset, int origin);

    //Read and write metadata from files.
    struct Metadata* ReadMetadata(FILE* fp);
    void WriteMetadata(FILE* fp, const struct Metadata* metadata);

    void WriteFileTable(FILE* fp);
    void ReadFileTable(FILE* fp);

public:

    void Test();
};


#endif //RELIC_2_0_COMPRESSIONMANAGER_H
