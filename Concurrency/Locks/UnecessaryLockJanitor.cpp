//
// Created by mikag on 9/29/2018.
//

#include "UnecessaryLockJanitor.h"

Relic::Concurrency::Locks::UnecessaryLockJanitor::UnecessaryLockJanitor(Relic::Concurrency::Locks::UnecessaryLock &lock) : lock(&lock)
{
    this->lock->Acquire();
}

Relic::Concurrency::Locks::UnecessaryLockJanitor::~UnecessaryLockJanitor()
{
    lock->Release();
}
