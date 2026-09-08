# Build the citrus UI libraries against the original game and engine.
add_library(platform_config INTERFACE)
add_library(video_config INTERFACE)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_64BIT ON)
    target_compile_definitions(platform_config INTERFACE PLATFORM_64BITS=1)
endif()
    set(OSX ON)
    set(POSIX ON)
    set(PLATFORM_LIBRARY_SHARED_EXT ".dylib")
    target_compile_definitions(platform_config INTERFACE OSX _OSX)
    target_compile_definitions(platform_config INTERFACE POSIX _POSIX GNUC
        NO_HOOK_MALLOC NO_MALLOC_OVERRIDE)
target_compile_definitions(platform_config INTERFACE STATIC_TIER0 STATIC_VSTDLIB
    "_DLL_EXT=${PLATFORM_LIBRARY_SHARED_EXT}")
target_compile_definitions(platform_config INTERFACE
    $<$<CONFIG:Debug>:DEBUG=1> $<$<CONFIG:Debug>:_DEBUG=1>)
    target_compile_options(platform_config INTERFACE -Wno-register
        -Wno-dynamic-exception-spec -Wno-implicit-float-conversion
        -Wno-deprecated-volatile -Wno-inconsistent-missing-override
        -Wno-invalid-offsetof)
    target_compile_definitions(video_config INTERFACE USE_SDL XASH_SDL)
