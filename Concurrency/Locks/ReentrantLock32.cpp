//
// Created by mikag on 9/29/2018.
//

#include <thread>
#include <assert.h>
#include <Core/Util.h>
#include "ReentrantLock32.h"

using namespace Relic;

ReentrantLock32::ReentrantLock32() : owner(0), ref_count(0)
{
}

void ReentrantLock32::Acquire()
{
    std::hash<std::thread::id> hasher;
    std::size_t tid = hasher(std::this_thread::get_id());

    //Attempt to acquire it if the current thread doesn't already hold it.
    if(owner.load(std::memory_order_relaxed) != tid)
    {
        //Atomically attempt to set the owner to the current thread's id, if it's not currently owned.
        std::size_t unlock_value = 0;
        while(!owner.compare_exchange_weak(unlock_value, tid, std::memory_order_relaxed, std::memory_order_relaxed))
        {
            unlock_value = 0;
            PAUSE();
        }

        ++ref_count;

        //Make sure all further reads are valid.
        std::atomic_thread_fence(std::memory_order_acquire);
    }
}

void ReentrantLock32::Release()
{
    //Make sure all writes have gone through before we release.
    std::atomic_thread_fence(std::memory_order_release);

    std::hash<std::thread::id> hasher;
    std::size_t tid = hasher(std::this_thread::get_id());
    std::size_t current = owner.load(std::memory_order_relaxed);
    assert(current == tid);

    --ref_count;

    if(ref_count == 0)
    {
        //Release the lock.
        owner.store(0, std::memory_order_relaxed);
    }
}

bool ReentrantLock32::TryAcquire()
{
    std::hash<std::thread::id> hasher;
    std::size_t tid = hasher(std::this_thread::get_id());

    bool acquired = false;

    if(owner.load(std::memory_order_relaxed) == tid)
    {
        acquired = true;
    }
    else
    {
        std::size_t unlock_value = 0;
        acquired = owner.compare_exchange_strong(unlock_value, tid, std::memory_order_relaxed, std::memory_order_relaxed);
    }

    if(acquired)
    {
        ++ref_count;
        std::atomic_thread_fence(std::memory_order_acquire);
    }

    return acquired;
}
