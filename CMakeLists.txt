cmake_minimum_required(VERSION 3.12)
project(Relic_2_0)

set(CMAKE_CXX_STANDARD 14)

add_executable(Relic_2_0 main.cpp Concurrency/Locks/SpinLock.cpp Concurrency/Locks/SpinLock.h  Concurrency/Locks/ScopedLock.h Concurrency/Locks/ReentrantLock.cpp Concurrency/Locks/ReentrantLock.h)