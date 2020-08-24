//
// Created by mikag on 9/30/2018.
//

#include "DoubleHeadedStackAllocator.h"

DoubleHeadedStackAllocator::DoubleHeadedStackAllocator(uint32_t stack_size_bytes)
{
    stack_base = (Marker) new char[stack_size_bytes];
    stack_top = stack_base + stack_size_bytes;
    current_marker_bottom = stack_base;
    current_marker_top = stack_top;
}

void *DoubleHeadedStackAllocator::Allocate(uint32_t size_bytes, bool top)
{
    //Make sure we have enough memory
    if(current_marker_top - current_marker_bottom < size_bytes)
    {
        Logger::Log("[DoubleHeadedStackAllocator] Failed to allocate memory for object.");
        return nullptr;
    }

    Marker allocated_maker = top ? current_marker_top : current_marker_bottom;

    if(top)
        current_marker_top -= size_bytes;
    else
        current_marker_bottom += size_bytes;

    return reinterpret_cast<void *>(allocated_maker);
}


DoubleHeadedStackAllocator::Marker DoubleHeadedStackAllocator::GetMarker(bool top)
{
    return top ? current_marker_top : current_marker_bottom;
}

void DoubleHeadedStackAllocator::FreeToMarker(DoubleHeadedStackAllocator::Marker marker, bool top)
{
    //Check that the marker we're resetting to makes sense
    if( (top && marker <= current_marker_bottom) || (!top && marker >= current_marker_top))
    {
        Logger::Log( "[DoubleHeadedStackAllocator] Resetting to marker attempted to reset head into other head's space. Not rolling back...");
        return;
    }

    (top ? current_marker_top : current_marker_bottom) = marker;
}

void DoubleHeadedStackAllocator::Clear()
{
    current_marker_top = stack_top;
    current_marker_bottom = stack_base;
}

uint32_t DoubleHeadedStackAllocator::Size()
{
    return (uint32_t) (current_marker_top - current_marker_bottom);
}

uint32_t DoubleHeadedStackAllocator::Capacity()
{
    return (uint32_t) (stack_top - stack_base);
}