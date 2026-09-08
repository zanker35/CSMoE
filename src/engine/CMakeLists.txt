#
# Copyright (c) 2016 Alibek Omarov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
cmake_minimum_required(VERSION  3.6.0)
project(XASH_ENGINE C CXX)

set(CMAKE_CXX_STANDARD 17)

# fix C++17 register
	add_compile_options(-Wno-register)

set(XASH_ENGINE xash) # Lib name

option(XASH_NO_ASYNC_NS_RESOLVE "Disable asynchronous domain name resolving." OFF)

option(XASH_VECTORIZE_SINCOS "Try to use vectorized versions of sin(), cos() and sincos()" ON)

# Desktop or dedicated version options

set(XASH_ENGINE_SOURCES
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/avikit.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/build.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/base_cmd.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/cfgscript.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/cmd.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/common.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/con_utils.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/crclib.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/crtlib.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/cvar.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/filesystem.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/host.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/hpak.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/infostring.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/identification.c"
	"${CMAKE_SOURCE_DIR}/src/engine/math/mathlib.c"
	"${CMAKE_SOURCE_DIR}/src/engine/math/matrixlib.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/mod_studio.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/mod_extend_seq.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/model.c"
	"${CMAKE_SOURCE_DIR}/src/engine/network/net_buffer.c"
	"${CMAKE_SOURCE_DIR}/src/engine/network/net_chan.c"
	"${CMAKE_SOURCE_DIR}/src/engine/network/net_encode.c"
	"${CMAKE_SOURCE_DIR}/src/engine/network/net_huff.c"
	"${CMAKE_SOURCE_DIR}/src/engine/network/network.c"
	"${CMAKE_SOURCE_DIR}/src/engine/physics/pm_surface.c"
	"${CMAKE_SOURCE_DIR}/src/engine/physics/pm_trace.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/random.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/sys_con.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/system.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/titles.c"
	"${CMAKE_SOURCE_DIR}/src/engine/physics/world.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/zone.c"
	"${CMAKE_SOURCE_DIR}/src/engine/runtime/crashhandler.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_bmp.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_main.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_quant.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_tga.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_utils.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_wad.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/imagelib/img_dds.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/snd_main.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/snd_mp3.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/snd_utils.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/snd_wav.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/dct36.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/dct64.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/format.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/frame.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/index.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/layer3.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/libmpg.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/mpg123.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/parse.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/reader.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/synth.c"
	"${CMAKE_SOURCE_DIR}/src/engine/audio/soundlib/libmpg/tabinit.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/Sequence.c"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/IceKey.cpp"
	"${CMAKE_SOURCE_DIR}/src/engine/assets/mod_decryptor.cpp"


	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_client.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_cmds.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_custom.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_frame.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_filter.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_game.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_init.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_main.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_log.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_move.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_phys.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_pmove.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_save.c"
	"${CMAKE_SOURCE_DIR}/src/engine/server/sv_world.c"

)

	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} "${CMAKE_SOURCE_DIR}/src/engine/runtime/library_static.c")



set(XASH_ENGINE_APPLE_SOURCES
	"${CMAKE_SOURCE_DIR}/src/engine/platform/macos/vid_macos.mm"
	"${CMAKE_SOURCE_DIR}/src/engine/platform/ime_stub.cpp"
)

set(XASH_ENGINE_CLIENT_SOURCES
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_cmds.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_events.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_frame.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_game.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_main.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_menu.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_parse.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_pmove.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_remap.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_scrn.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_tent.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_video.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_view.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/cl_netgraph.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_backend.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_beams.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_cull.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_decals.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_draw.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_image.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_mirror.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_refrag.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rlight.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rmain.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rmath.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rmisc.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rpart.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rsurf.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_rstrobe.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_sprite.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_studio.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/vid_common.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gl_warp.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/input.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/keys.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/console.c"
   "${CMAKE_SOURCE_DIR}/src/engine/renderer/gamma.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_dsp.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_load.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_main.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_mix.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_mouth.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_stream.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_utils.c"
   "${CMAKE_SOURCE_DIR}/src/engine/audio/s_vox.c"
   "${CMAKE_SOURCE_DIR}/src/engine/client/vgui/vgui_draw.c"
)



set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES}
    "${CMAKE_SOURCE_DIR}/src/engine/platform/sdl/events.c" "${CMAKE_SOURCE_DIR}/src/engine/platform/sdl/gl_sdl.c"
    "${CMAKE_SOURCE_DIR}/src/engine/platform/sdl/snd_sdl.c" "${CMAKE_SOURCE_DIR}/src/engine/platform/sdl/vid_sdl.c")
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_APPLE_SOURCES})
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_CLIENT_SOURCES})





add_definitions(-DXASH_FORCEINLINE)

# ----- Conditions -----

if(XASH_VECTORIZE_SINCOS)
	add_definitions(-DVECTORIZE_SINCOS)
endif()

	add_definitions(-DSINGLE_BINARY)
		add_definitions(-DXASH_SDLMAIN)
	add_library(${XASH_ENGINE} STATIC ${XASH_ENGINE_SOURCES})

		target_link_libraries(${XASH_ENGINE} client server vgui2_support vgui2 GameUI)
		target_compile_definitions(${XASH_ENGINE} PRIVATE XASH_VGUI2=1)




		add_definitions(-DUSE_SELECT)
	add_definitions(-DCOLORIZE_CONSOLE)






	set(SDL2_BUILDING_EXECUTABLE ${XASH_SINGLE_BINARY}) # link sdl2main, if needed
	add_definitions(-DXASH_SDL)

	target_link_libraries(${XASH_ENGINE} sdl2)





	target_link_libraries(${XASH_ENGINE} ${CMAKE_DL_LIBS})

	target_link_libraries(${XASH_ENGINE} "-framework AppKit -framework AudioToolBox")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
	SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-D_DEBUG)
endif()

if(XASH_RELEASE)
	add_definitions(-DXASH_RELEASE)
else()
	execute_process(COMMAND "git" "rev-parse" "--short" "HEAD"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		OUTPUT_VARIABLE SHORT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE)

if(SHORT_HASH)
		message(STATUS "git hash: ${SHORT_HASH}")
		add_definitions(-DXASH_BUILD_COMMIT="${SHORT_HASH}")
else()
		message(STATUS "git hash: not set")
endif()
endif()

if(XASH_NO_ASYNC_NS_RESOLVE)
	add_definitions(-DXASH_NO_ASYNC_NS_RESOLVE)
endif()

if(XASH_USE_STB_SPRINTF)
	add_definitions(-DXASH_USE_STB_SPRINTF)
endif()

		target_link_libraries(${XASH_ENGINE} -lm -lpthread)
	# For single binary builds we change name to game_launch's executable name
	# For single binary builds we change name to game_launch's executable name

# engine doesnt support unity build now
set_target_properties(${XASH_ENGINE} PROPERTIES UNITY_BUILD OFF)
