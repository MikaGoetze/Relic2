//
// Created by mikag on 9/29/2018.
//

#ifndef RELIC_2_0_SCOPEDLOCK_H
#define RELIC_2_0_SCOPEDLOCK_H

namespace Relic
{

    template<class LOCK>
    class ScopedLock
    {
    private:
        LOCK *target_lock;

    public:
        /// Creates a new scoped lock.
        /// \param lock - The lock to use.
        explicit ScopedLock(LOCK &lock);

        ~ScopedLock();
    };

    template<class LOCK>
    ScopedLock<LOCK>::ScopedLock(LOCK &lock) : target_lock(&lock)
    {
        target_lock->Acquire();
    }

    template<class LOCK>
    ScopedLock<LOCK>::~ScopedLock()
    {
        target_lock->Release();
    }

}


#endif //RELIC_2_0_SCOPEDLOCK_H
