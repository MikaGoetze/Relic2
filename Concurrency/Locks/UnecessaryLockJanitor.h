//
// Created by mikag on 9/29/2018.
//

#ifndef RELIC_2_0_UNECESSARYLOCKJANITOR_H
#define RELIC_2_0_UNECESSARYLOCKJANITOR_H

#include "UnecessaryLock.h"


namespace Relic
{
    class UnecessaryLockJanitor
    {
    private:
        UnecessaryLock *lock;

    public:
        explicit UnecessaryLockJanitor(UnecessaryLock &lock);

        ~UnecessaryLockJanitor();
    };
}

#if ASSERTIONS_ENABLED
#define ASSERT_LOCK_NOT_NECESSARY(J, L) \
    UnnecessaryLockJanitor J(L)
#else
#define ASSERT_LOCK_NOT_NECESSARY(J, L)
#endif

#endif //RELIC_2_0_UNECESSARYLOCKJANITOR_H
