//
// Created by mikag on 9/29/2018.
//

#ifndef RELIC_2_0_REENTRANTLOCK_H
#define RELIC_2_0_REENTRANTLOCK_H

#include <atomic>
#include <cstdint>

namespace Relic
{
    namespace Concurrency
    {
        namespace Locks
        {
            class ReentrantLock32
            {
            private:
                std::atomic<std::size_t>  owner;
                std::int32_t ref_count;

            public:
                ///Creates a reentrant lock (32 bit)
                ReentrantLock32();

                ///Acquires the lock if not already acquired by the same thread. [Blocking]
                void Acquire();

                ///Release the lock. [Non Blocking]
                void Release();

                /// Attemps to acquire the lock. [Non Blocking]
                /// \return Returns whether or not the lock was successfully acquired
                bool TryAcquire();
            };

        }
    }
}


#endif //RELIC_2_0_REENTRANTLOCK_H
