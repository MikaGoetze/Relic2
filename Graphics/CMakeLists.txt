target_sources(Relic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Window.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Renderer.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VulkanRenderer.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/VulkanRenderer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VulkanUtils.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Model.h")

add_subdirectory("OpenFBX")