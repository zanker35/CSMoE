

add_library(vinterface STATIC
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/interface.cpp"
)

target_link_libraries(vinterface PUBLIC platform_config)

add_library(tier0 STATIC
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/assert_dialog.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/commandline.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/cpu.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/cpumonitoring.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/cpu_usage.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/dbg.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/dynfunction.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/fasttimer.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/mem.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/mem_helpers.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/memdbg.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/memstd.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/memvalidate.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/minidump.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/pch_tier0.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/progressbar.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/security.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/systeminformation.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/stacktools.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/thread.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/threadtools.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/tier0_strtools.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/tslist.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/vprof.cpp"
)
    target_sources(tier0 PUBLIC "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/cpu_posix.cpp")
    target_sources(tier0 PUBLIC "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/platform_posix.cpp")
    target_sources(tier0 PUBLIC "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/pme_posix.cpp")
    target_sources(tier0 PUBLIC "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier0/vcrmode_posix.cpp")


target_link_libraries(tier0 PUBLIC platform_config video_config)
target_link_libraries(tier0 PUBLIC 3rdparty-sdl2)

add_library(tier1 STATIC
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/bitbuf.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/byteswap.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/characterset.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/checksum_crc.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/checksum_md5.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/convar.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/datamanager.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/diff.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/generichash.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/KeyValues.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/mempool.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/memstack.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/NetAdr.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/processor_detect.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/rangecheckedvar.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/stringpool.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/strtools.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/strtools_unicode.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/tier1.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/utlbuffer.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/utlstring.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/utlsymbol.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/tier1/ilocalize.cpp"
        )

target_link_libraries(tier1 PUBLIC platform_config)
find_package(Iconv REQUIRED)
target_link_libraries(tier1 PRIVATE Iconv::Iconv)

add_library(mathlib STATIC
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/color_conversion.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/halton.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/lightdesc.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/mathlib_base.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/powsse.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/sparse_convolution_noise.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/sseconst.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/sse.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/ssenoise.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/anorms.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/bumpvects.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/IceKey.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/imagequant.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/polyhedron.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/quantize.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/randsse.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/spherical.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/simdvectormatrix.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/vector.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/vmatrix.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/mathlib/almostequal.cpp"
        )
    target_compile_options(mathlib PRIVATE -Wno-c++11-narrowing)

target_link_libraries(mathlib PUBLIC platform_config)

target_link_libraries(tier1 PUBLIC vinterface tier0 mathlib)

add_library(vstdlib STATIC
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/coroutine.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/cvar.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/jobthread.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/KeyValuesSystem.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/osversion.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/random.cpp"
        "${CMAKE_SOURCE_DIR}/vendor/source_sdk/vstdlib/vcover.cpp"
        )
target_link_libraries(vstdlib PRIVATE tier0 tier1)
target_compile_definitions(vstdlib PRIVATE -DVSTDLIB_DLL_EXPORT=1)
    # fix for Gestalt
    target_link_libraries(vstdlib PUBLIC "-framework CoreServices")

add_library(SourceSDK INTERFACE)
