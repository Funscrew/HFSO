cmake_minimum_required(VERSION 4.2)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_GENERATOR_TOOLSET "v143" CACHE STRING "Platform Toolset" FORCE)
endif()

project(HFSO LANGUAGES CXX)
project(HFSO VERSION 0.0.1)

if(MSVC AND NOT CMAKE_VS_PLATFORM_TOOLSET STREQUAL "v143")
    message(FATAL_ERROR "This project requires MSVC platform toolset v143.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HFSO_SOURCES
    src/CClient.h
    src/CClient.cpp
)

add_library(HFSO_static STATIC
    ${HFSO_SOURCES}
)

add_library(HFSO_shared SHARED
    ${HFSO_SOURCES}
)

target_include_directories(HFSO_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(HFSO_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


add_executable(example-app
    src/example-app/main.cpp
    src/example-app/main.h
)

target_link_libraries(example-app PRIVATE
    HFSO_static
)
# target_include_directories(example-app PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )
