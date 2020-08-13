//
// Created by mikag on 9/30/2018.
//

#ifndef RELIC_2_0_DOUBLEHEADEDSTACKALLOCATOR_H
#define RELIC_2_0_DOUBLEHEADEDSTACKALLOCATOR_H


#include <cstdint>
#include <Debugging/Logger.h>

class DoubleHeadedStackAllocator
{
public:
    ///Marker that identifies a memory location within the stack (including fall back locations).
    typedef uintptr_t Marker;

    /// Creates a stack of the desired size.
    /// \param stack_size_bytes - The number of bytes to allocate.
    explicit DoubleHeadedStackAllocator(uint32_t stack_size_bytes);


    /// Allocate the specified amount of memory.
    /// \param size_bytes The amount of bytes to allocate.
    /// \param top Whether or not to allocate from the stack (true), or to allocate from the bottom of the stack (false).
    /// \return Returns a pointer to the allocated memory.
    void *Allocate(uint32_t size_bytes, bool top = false);

    /// The size of the stack.
    /// \return The total allocatable space left in the stack (in bytes).
    uint32_t Size();

    /// The total capacity of the stack.
    /// \return The total capacity of the stack (in bytes).
    uint32_t Capacity();

    /// Get the marker to the current top of the stack.
    /// \param top Whether or not to get the current marker from the top head (true), or the bottom head (false).
    /// \return Marker that is the current top of the stack - can be used to roll back.
    Marker GetMarker(bool top = false);

    /// Frees the stack back to the specified marker
    /// \param marker - The point to free the stack back to.
    /// \param top Whether or not to free from the top head (true), or the bottom head (false).
    void FreeToMarker(Marker marker, bool top = false);

    ///Free the entire stack.
    void Clear();

private:
    //Base address of the stack.
    Marker stack_base;

    //Top address of the stack.
    Marker stack_top;

    //Markers to the currently allocated portion of the stack.
    Marker current_marker_top;
    Marker current_marker_bottom;
};

#endif //RELIC_2_0_DOUBLEHEADEDSTACKALLOCATOR_H
