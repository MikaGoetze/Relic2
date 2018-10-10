add_subdirectory(Compression)
target_sources(Relic_2_0 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.h"
        )