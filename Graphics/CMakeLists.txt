target_sources(Relic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Window.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VulkanUtils.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Model.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/VulkanModelExtensions.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Texture.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/MaterialUtil.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/MaterialUtil.cpp"
        )

add_subdirectory("OpenFBX")
add_subdirectory("Systems")
add_subdirectory("Components")
