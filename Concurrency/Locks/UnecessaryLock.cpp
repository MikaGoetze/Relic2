//
// Created by mikag on 9/29/2018.
//

#include "UnecessaryLock.h"

void Relic::UnecessaryLock::Acquire()
{
    assert(!is_locked);

    is_locked = true;
}

void Relic::UnecessaryLock::Release()
{
    assert(is_locked);

    is_locked = false;
}


