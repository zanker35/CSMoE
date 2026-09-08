
set(CMAKE_CXX_STANDARD 20)

add_library(vgui2_interfaces INTERFACE)

target_link_libraries(vgui2_interfaces INTERFACE vinterface mathlib)

add_library(vguimatsurface STATIC
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vguimatsurface/Clip2D.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vguimatsurface/FontTextureCache.cpp"
		)

target_link_libraries(vguimatsurface PRIVATE vgui_surfacelib)

add_subdirectory(vgui_controls)
add_subdirectory(cso_controls)
add_subdirectory(../gameui ${CMAKE_BINARY_DIR}/src/ui/gameui)

add_library(vgui2 STATIC
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/Bitmap.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/Border.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/input.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/LocalizedStringTable.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/MemoryBitmap.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/Scheme.cpp"
		"${CMAKE_SOURCE_DIR}/vendor/source_sdk/public/UnicodeFileHelpers.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/vgui.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/VGuiKeyValues.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/vgui_key_translation.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/VPanel.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/VPanelWrapper.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui_controls/controls.cpp"
		)
	target_sources(vgui2 PRIVATE
			"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/src/system_posix.cpp"
			)
target_compile_definitions(vgui2 PRIVATE -DDONT_PROTECT_FILEIO_FUNCTIONS=1)

target_link_libraries(vgui2 PRIVATE platform_config tier0 tier1 vstdlib 3rdparty-sdl2)

add_library(vgui_surfacelib STATIC
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/vgui_surfacelib/FontAmalgam.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/vgui_surfacelib/FontManager.cpp"
		"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/vgui_surfacelib/FontEffects.cpp"
		)
	target_sources(vgui_surfacelib PRIVATE
			# MoeMod : We dont use Carbon API any more...
			"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2/vgui_surfacelib/linuxfont.cpp"
			)
	target_link_libraries(vgui_surfacelib PUBLIC 3rdparty-freetype)

target_link_libraries(vgui_surfacelib PUBLIC platform_config video_config)

add_library(dead_wrapper STATIC
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/dead_wrapper/deadfilesystem.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/dead_wrapper/deadinput.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/dead_wrapper/deadipanel.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/dead_wrapper/deadscheme.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/dead_wrapper/deadsurface.cpp"
	"${CMAKE_SOURCE_DIR}/vendor/source_sdk/public/filesystem_helpers.cpp"
)
target_link_libraries(dead_wrapper PUBLIC vgui2_interfaces)
target_link_libraries(dead_wrapper PRIVATE vguimatsurface)
target_link_libraries(dead_wrapper PRIVATE tier0)

add_library(vgui2_support STATIC
	"${CMAKE_SOURCE_DIR}/src/ui/vgui2/BaseUISurface.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/CBaseUI.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2_int.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/CGameUIFuncs.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui_int.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/udll_int.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/interface_static.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/filesystem_xash.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/vgui2_surface.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/vgui2/ui_test.cpp"
)

target_link_libraries(vgui2_support PUBLIC vgui2_interfaces)
target_link_libraries(vgui2_support PRIVATE platform_config)
target_link_libraries(vgui2_support PRIVATE video_config)
target_link_libraries(vgui2_support PRIVATE vgui_controls)
target_link_libraries(vgui2_support PRIVATE cso_controls)
target_link_libraries(vgui2_support PRIVATE vgui_surfacelib)
target_link_libraries(vgui2_support PRIVATE vguimatsurface)
target_link_libraries(vgui2_support PRIVATE dead_wrapper)
target_link_libraries(vgui2_support PRIVATE tier0 tier1 vstdlib mathlib)
