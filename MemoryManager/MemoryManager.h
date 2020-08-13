//
// Created by Mika Goetze on 2020-08-13.
//

#ifndef RELIC_MEMORYMANAGER_H
#define RELIC_MEMORYMANAGER_H

#include <cstddef>
#include "StackAllocator.h"

class MemoryManager
{
private:
    static MemoryManager *instance;

    StackAllocator *frontStack, *backStack;

    //256MB stack buffers for the frames.
    const size_t STACK_SIZE = 268435456;

public:
    static MemoryManager *GetInstance();

    template<typename T>
    T *Allocate();

    void CycleStacks();

    MemoryManager();

    ~MemoryManager();
};

//Some handy macros

#define REL_ALLOC(type) MemoryManager::GetInstance()->Allocate<type>()

#endif //RELIC_MEMORYMANAGER_H
