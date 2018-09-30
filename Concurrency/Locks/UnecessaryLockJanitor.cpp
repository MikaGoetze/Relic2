//
// Created by mikag on 9/29/2018.
//

#include "UnecessaryLockJanitor.h"

Relic::UnecessaryLockJanitor::UnecessaryLockJanitor(Relic::UnecessaryLock &lock) : lock(&lock)
{
    this->lock->Acquire();
}

Relic::UnecessaryLockJanitor::~UnecessaryLockJanitor()
{
    lock->Release();
}
