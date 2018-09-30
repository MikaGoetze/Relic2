//
// Created by mikag on 9/30/2018.
//

#ifndef RELIC_2_0_STACKALLOCATOR_H
#define RELIC_2_0_STACKALLOCATOR_H

#include <stdint.h>
#include <Debugging/Logger.h>

namespace Relic
{

    class StackAllocator
    {
    public:
        ///Marker that identifies a memory location within the stack (including fall back locations).
        typedef uintptr_t Marker;

        /// Creates a stack of the desired size.
        /// \param stack_size_bytes - The number of bytes to allocate.
        explicit StackAllocator(uint32_t stack_size_bytes);

        /// Allocate a desired number of bytes from the stack.
        /// \param size_bytes - Number of bytes to allocate.
        /// \return Pointer to the allocated memory.
        void* Allocate(uint32_t size_bytes);

        /// The size of the stack.
        /// \return The total allocatable space left in the stack (in bytes).
        uint32_t Size();

        /// The total capacity of the stack.
        /// \return The total capacity of the stack (in bytes).
        uint32_t Capacity();

        /// Get the marker to the current top of the stack.
        /// \return Marker that is the current top of the stack - can be used to roll back.
        Marker GetMarker();

        /// Frees the stack back to the specified marker
        /// \param marker - The point to free the stack back to.
        void FreeToMarker(Marker marker);

        ///Free the entire stack.
        void Clear();

    private:
        //Base address of the stack.
        Marker stack_base;

        //Top address of the stack.
        Marker stack_top;

        //Marker to the currently allocated portion of the stack.
        Marker current_marker;
    };

}

#endif //RELIC_2_0_STACKALLOCATOR_H
