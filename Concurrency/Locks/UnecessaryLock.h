//
// Created by mikag on 9/29/2018.
//

#ifndef RELIC_2_0_UNECESSARYLOCK_H
#define RELIC_2_0_UNECESSARYLOCK_H

#include <assert.h>

namespace Relic
{

    class UnecessaryLock
    {
    private:
        volatile bool is_locked;

    public:
        void Acquire();

        void Release();

    };
}

#if ASSERTIONS_ENABLED
#define BEGIN_ASSERT_LOCK_NOT_NECESSARY(L) (L).Acquire()
#define END_ASSERT_LOCK_NOT_NECESSARY(L) (L).Release()
#else
#define BEGIN_ASSERT_LOCK_NOT_NECESSARY(L)
#define END_ASSERT_LOCK_NOT_NECESSARY(L)
#endif

#endif //RELIC_2_0_UNECESSARYLOCK_H
