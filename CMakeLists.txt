cmake_minimum_required(VERSION 3.12)
project(Relic)

set(CMAKE_CXX_STANDARD 17)
set(ENV{VULKAN_SDK} /Users/mikag/Documents/vulkan-sdk/macOS)
set(ENV{DYLD_LIBRARY_PATH} $ENV{VULKAN_SDK}/lib)
set(ENV{PATH} $ENV{VULKAN_SDK}/bin:ENV{PATH})
set(ENV{VK_ICD_FILENAMES} $ENV{VULKAN_SDK}/etc/vulkan/icd.d/MoltenVK_icd.json)
set(ENV{VK_LAYER_PATH} $ENV{VULKAN_SDK}/etc/vulkan/explicit_layer.d)

include_directories(.)
include_directories(/usr/local/include)
include_directories(~/Documents/vulkan-sdk/macOS/include)
link_directories(/usr/local/lib)
link_directories(~/Documents/vulkan-sdk/macOS/lib)

add_executable(Relic main.cpp)

add_subdirectory(Concurrency)
add_subdirectory(Core)
add_subdirectory(Debugging)
add_subdirectory(MemoryManager)
add_subdirectory(ResourceManager)
add_subdirectory(Graphics)

message("SDK: " $ENV{VULKAN_SDK})
message("Layers: " $ENV{VK_LAYER_PATH})
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(Relic Vulkan glfw)