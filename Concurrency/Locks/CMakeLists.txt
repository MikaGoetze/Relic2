target_sources(Relic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ReentrantLock32.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ReentrantLock32.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ScopedLock.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/SpinLock.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SpinLock.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/UnecessaryLock.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/UnecessaryLock.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/UnecessaryLockJanitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/UnecessaryLockJanitor.h"
        )