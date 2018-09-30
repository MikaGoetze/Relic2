//
// Created by mikag on 9/29/2018.
//

#include "SpinLock.h"
#include <Core/Util.h>

Relic::SpinLock::SpinLock() : lock_status()
{
    lock_status.clear();
}

bool Relic::SpinLock::TryAcquire()
{
    bool already_locked = lock_status.test_and_set(std::memory_order::memory_order_acquire);
    return !already_locked;
}

void Relic::SpinLock::Acquire()
{
    while (!TryAcquire())
    {
        //Ask the cpu to pause for a couple of cycles (saves power)
        PAUSE();
    }
}

void Relic::SpinLock::Release()
{
    lock_status.clear(std::memory_order::memory_order_release);
}

