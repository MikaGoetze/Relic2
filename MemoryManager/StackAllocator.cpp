//
// Created by mikag on 9/30/2018.
//

#include "StackAllocator.h"

StackAllocator::StackAllocator(uint32_t stack_size_bytes)
{
    stack_base = (Marker) new char[stack_size_bytes];
    stack_top = stack_base + stack_size_bytes;
    current_marker = stack_base;
}

void *StackAllocator::Allocate(uint32_t size_bytes)
{
    //Make sure we have enough memory to service the request
    if(stack_top - current_marker < size_bytes)
    {
        Logger::Log("[StackAllocator] Cannot service memory request.");
        return nullptr;
    }

    Marker allocated_base = current_marker;

    current_marker += size_bytes;

    return reinterpret_cast<void *>(allocated_base);
}

StackAllocator::Marker StackAllocator::GetMarker()
{
    return current_marker;
}

void StackAllocator::FreeToMarker(StackAllocator::Marker marker)
{
    //We just put the marker back and don't bother actually clearing anything.
    current_marker = marker;
}

void StackAllocator::Clear()
{
    current_marker = stack_base;
}

uint32_t StackAllocator::Size()
{
    //Return the remaining size
    return (uint32_t) (stack_top - current_marker);
}

uint32_t StackAllocator::Capacity()
{
    return (uint32_t) (stack_top - stack_base);
}
