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

set(HFPO_SOURCES
    src/CClient.h
    src/CClient.cpp
)

add_library(HFPO_static STATIC
    ${HFPO_SOURCES}
)

add_library(HFPO_shared SHARED
    ${HFPO_SOURCES}
)

target_include_directories(HFPO_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(HFPO_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


add_executable(example-app
    src/example-app/main.cpp
    src/example-app/main.h
)

target_link_libraries(example-app PRIVATE
    HFPO_static
)
# target_include_directories(example-app PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )
