add_subdirectory(lz4)

target_sources(Relic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/CompressionManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CompressionManager.h"
        )