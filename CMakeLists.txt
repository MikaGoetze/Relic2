cmake_minimum_required(VERSION 3.12)
project(Relic)

set(CMAKE_CXX_STANDARD 17)
#set(ENV{VULKAN_SDK} /Users/mikag/Documents/vulkan-sdk/macOS)
#set(ENV{DYLD_LIBRARY_PATH} $ENV{VULKAN_SDK}/lib)
#set(ENV{PATH} $ENV{VULKAN_SDK}/bin:ENV{PATH})
#set(ENV{VK_ICD_FILENAMES} $ENV{VULKAN_SDK}/etc/vulkan/icd.d/MoltenVK_icd.json)
#set(ENV{VK_LAYER_PATH} $ENV{VULKAN_SDK}/etc/vulkan/explicit_layer.d)


add_executable(Relic main.cpp)

add_subdirectory(Concurrency)
add_subdirectory(Core)
add_subdirectory(Debugging)
add_subdirectory(MemoryManager)
add_subdirectory(ResourceManager)
add_subdirectory(Graphics)
add_subdirectory(Importers)
add_subdirectory(Libraries)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
include_directories(.)

include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIR})
include_directories($ENV{GLM_PATH})
target_link_libraries(Relic ${Vulkan_LIBRARIES} ${GLFW_LIBRARY} )

