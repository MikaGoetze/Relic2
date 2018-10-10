cmake_minimum_required(VERSION 3.12)
project(Relic_2_0)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Relic_2_0 main.cpp)

add_subdirectory(Concurrency)
add_subdirectory(Core)
add_subdirectory(Debugging)
add_subdirectory(MemoryManager)
add_subdirectory(ResourceManager)

