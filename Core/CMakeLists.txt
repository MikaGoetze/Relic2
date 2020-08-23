target_sources(Relic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Relic.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Relic.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Util.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RelicStruct.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Time.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Time.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/World.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/World.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ISystem.h"
)

add_subdirectory("Components")
add_subdirectory("Systems")
