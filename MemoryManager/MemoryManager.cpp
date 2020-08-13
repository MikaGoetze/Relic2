//
// Created by Mika Goetze on 2020-08-13.
//

#include "MemoryManager.h"

MemoryManager *MemoryManager::instance;

MemoryManager *MemoryManager::GetInstance()
{
    return instance;
}

MemoryManager::MemoryManager()
{
    frontStack = new StackAllocator(STACK_SIZE);
    backStack = new StackAllocator(STACK_SIZE);
}

MemoryManager::~MemoryManager()
{
    delete frontStack;
    delete backStack;
}

template<typename T>
T *MemoryManager::Allocate()
{
    return frontStack->Allocate(sizeof(T));
}

void MemoryManager::CycleStacks()
{
    StackAllocator *newStack = backStack;
    backStack = frontStack;
    frontStack = newStack;
    frontStack->Clear();
}

