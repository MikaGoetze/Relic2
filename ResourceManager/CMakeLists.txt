add_subdirectory(Compression)
target_sources(Relic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.h"
        )