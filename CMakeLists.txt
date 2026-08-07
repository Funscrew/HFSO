cmake_minimum_required(VERSION 4.2)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_GENERATOR_TOOLSET "v143" CACHE STRING "Platform Toolset" FORCE)
endif()

project(HFPO LANGUAGES CXX)
project(HFPO VERSION 0.0.1)

if(MSVC AND NOT CMAKE_VS_PLATFORM_TOOLSET STREQUAL "v143")
    message(FATAL_ERROR "This project requires MSVC platform toolset v143.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(HFPO SHARED
    src/CClient.h
    src/CClient.cpp
#     ../ggpo/src/lib/CReplayFile.h
#     ../ggpo/src/lib/CReplayFile.cpp
#     ../ggpo/src/lib/game_input.h
#     ../src/burner/EZStream.cpp
#     NetcodeInterop.cpp
#     NetcodeInterop.h
)

target_include_directories(HFPO PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)