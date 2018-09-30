//
// Created by mikag on 9/29/2018.
//

#ifndef RELIC_2_0_SPINLOCK_H
#define RELIC_2_0_SPINLOCK_H


#include <atomic>

namespace Relic
{
    class SpinLock
    {
    private:
        std::atomic_flag lock_status;

    public:
        ///Constructs a new SpinLock object.
        SpinLock();

        /// Attempts to acquire the lock. [Non Blocking]
        /// \return Returns true if acquisition was successful, false otherwise.
        bool TryAcquire();

        ///Acquires the lock. [Blocking]
        void Acquire();

        ///Releases the lock. [Non Blocking]
        void Release();

    };
}

#endif //RELIC_2_0_SPINLOCK_H
