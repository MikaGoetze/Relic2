//
// Created by mikag on 9/29/2018.
//

#include "UnecessaryLock.h"

void Relic::Concurrency::Locks::UnecessaryLock::Acquire()
{
    assert(!is_locked);

    is_locked = true;
}

void Relic::Concurrency::Locks::UnecessaryLock::Release()
{
    assert(is_locked);

    is_locked = false;
}


