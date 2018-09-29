//
// Created by mikag on 9/29/2018.
//

#include "SpinLock.h"

using namespace Relic::Concurrency::Locks;

SpinLock::SpinLock() : lock_status(false)
{}

bool SpinLock::TryAcquire()
{
    bool already_locked = lock_status.test_and_set(std::memory_order::memory_order_acquire);
    return !already_locked;
}

void SpinLock::Acquire()
{
    while (!TryAcquire())
    {
        //Ask the cpu to pause for a couple of cycles (saves power)
        __asm__("pause;");
    }
}

void SpinLock::Release()
{
    lock_status.clear(std::memory_order::memory_order_release);
}
