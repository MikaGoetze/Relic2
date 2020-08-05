cmake_minimum_required(VERSION 2.8)

file(GLOB IMGUI_SRC
        "*.h"
        "*.cpp"
        )

target_sources(Relic PRIVATE ${IMGUI_SRC})