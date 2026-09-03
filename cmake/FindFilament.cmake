# Locate the pinned Filament SDK used by the experimental CSMoE renderer.
#
# Input:
#   CSMOE_FILAMENT_ROOT
#
# Output:
#   Filament::Filament
#   Filament_MATC_EXECUTABLE
#   Filament_VERSION

include(FindPackageHandleStandardArgs)

set(_filament_root_hints)
if(CSMOE_FILAMENT_ROOT)
	list(APPEND _filament_root_hints "${CSMOE_FILAMENT_ROOT}")
endif()
if(DEFINED ENV{FILAMENT_ROOT})
	list(APPEND _filament_root_hints "$ENV{FILAMENT_ROOT}")
endif()

find_path(Filament_ROOT_DIR
	NAMES include/filament/Engine.h
	HINTS ${_filament_root_hints}
	NO_DEFAULT_PATH)

if(NOT Filament_ROOT_DIR)
	find_path(Filament_ROOT_DIR NAMES include/filament/Engine.h)
endif()

find_program(Filament_MATC_EXECUTABLE
	NAMES matc
	HINTS "${Filament_ROOT_DIR}/bin"
	NO_DEFAULT_PATH)

set(Filament_VERSION "")
if(Filament_MATC_EXECUTABLE)
	execute_process(
		COMMAND "${Filament_MATC_EXECUTABLE}" --version
		OUTPUT_VARIABLE _filament_matc_version
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET)
	if(_filament_matc_version STREQUAL "74")
		set(Filament_VERSION "1.74.0")
	endif()
endif()

if(APPLE)
	if(CMAKE_OSX_ARCHITECTURES)
		list(GET CMAKE_OSX_ARCHITECTURES 0 _filament_arch)
	else()
		set(_filament_arch "${CMAKE_SYSTEM_PROCESSOR}")
	endif()
	if(_filament_arch STREQUAL "aarch64")
		set(_filament_arch "arm64")
	endif()
else()
	set(_filament_arch "${CMAKE_SYSTEM_PROCESSOR}")
	if(_filament_arch STREQUAL "AMD64")
		set(_filament_arch "x86_64")
	endif()
endif()

set(Filament_LIBRARY_DIR "${Filament_ROOT_DIR}/lib/${_filament_arch}")
set(_filament_required_libraries
	filament
	backend
	bluegl
	bluevk
	filabridge
	filaflat
	utils
	geometry
	smol-v
	ibl
	abseil
	zstd)

set(Filament_LIBRARIES)
set(_filament_all_libraries_found TRUE)
foreach(_filament_library IN LISTS _filament_required_libraries)
	set(_filament_library_path
		"${Filament_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${_filament_library}${CMAKE_STATIC_LIBRARY_SUFFIX}")
	if(EXISTS "${_filament_library_path}")
		list(APPEND Filament_LIBRARIES "${_filament_library_path}")
	else()
		set(_filament_all_libraries_found FALSE)
	endif()
endforeach()

find_package_handle_standard_args(Filament
	REQUIRED_VARS
		Filament_ROOT_DIR
		Filament_MATC_EXECUTABLE
		Filament_LIBRARY_DIR
		_filament_all_libraries_found
	VERSION_VAR Filament_VERSION)

if(Filament_FOUND AND NOT TARGET Filament::Filament)
	add_library(Filament::Filament INTERFACE IMPORTED)
	set_target_properties(Filament::Filament PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${Filament_ROOT_DIR}/include"
		INTERFACE_LINK_LIBRARIES "${Filament_LIBRARIES}")
	if(APPLE)
		set_property(TARGET Filament::Filament APPEND PROPERTY
			INTERFACE_LINK_LIBRARIES
			"-framework AppKit"
			"-framework Cocoa"
			"-framework CoreVideo"
			"-framework Metal"
			"-framework QuartzCore")
	endif()
endif()

mark_as_advanced(
	Filament_ROOT_DIR
	Filament_LIBRARY_DIR
	Filament_MATC_EXECUTABLE)
