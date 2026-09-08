cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "Mac MVP architecture")
project(csmoe LANGUAGES C CXX)
if(NOT APPLE OR CMAKE_SYSTEM_NAME STREQUAL "iOS" OR NOT CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    message(FATAL_ERROR "This MVP targets Apple Silicon macOS only")
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(XASH_SDL ON)
set(XASH_STATIC_GAMELIB ON)
set(XASH_VGUI2 ON)
set(XASH_64BIT ON)
set(XASH_LIBDL ON)
set(XASH_USE_SELECT ON)
option(XASH_PCH "Enable precompiled headers" OFF)
option(XASH_UNITY_BUILD "Enable unity build" OFF)
if(XASH_PCH)
    add_definitions(-DXASH_PCH=1)
endif()
if(XASH_UNITY_BUILD)
    add_definitions(-DXASH_UNITY_BUILD=1)
endif()
add_definitions(-DXASH_STATIC_GAMELIB=1 -DHAVE_STDINT_H)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-error=incompatible-function-pointer-types>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=int-conversion>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=implicit-function-declaration>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=incompatible-pointer-types>
    -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-invalid-offsetof
    -Wno-unused-private-field)
# Sources use qualified paths from these two roots. No legacy include search buckets.
set(CSMOE_SOURCE_SDK_DIR "${CMAKE_SOURCE_DIR}/vendor/source_sdk")
add_subdirectory(vendor)
include(cmake/cso_platform.cmake)
add_subdirectory(vendor/source_sdk)
add_subdirectory(src/ui/vgui2)
add_subdirectory(src/game/client)
add_subdirectory(src/game/server)
add_subdirectory(src/engine)
# Keep the documented app output stable while the source lives under src/app.
add_subdirectory(src/app game_launch)
foreach(target IN ITEMS vinterface tier0 tier1 mathlib vstdlib vguimatsurface
        vgui_controls cso_controls GameUI vgui2 vgui_surfacelib dead_wrapper
        vgui2_support client server xash game_launch)
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/vendor")
endforeach()
target_include_directories(vgui2_interfaces INTERFACE
    "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/vendor")
