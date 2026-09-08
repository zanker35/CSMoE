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
	common/avikit.c
	common/build.c
	common/base_cmd.c
	common/cfgscript.c
	common/cmd.c
	common/common.c
	common/con_utils.c
	common/crclib.c
	common/crtlib.c
	common/cvar.c
	common/filesystem.c
	common/host.c
	common/hpak.c
	common/infostring.c
	common/identification.c
	common/mathlib.c
	common/matrixlib.c
	common/mod_studio.c
	common/mod_extend_seq.c
	common/model.c
	common/net_buffer.c
	common/net_chan.c
	common/net_encode.c
	common/net_huff.c
	common/network.c
	common/pm_surface.c
	common/pm_trace.c
	common/random.c
	common/sys_con.c
	common/system.c
	common/titles.c
	common/world.c
	common/zone.c
	common/crashhandler.c
	common/imagelib/img_bmp.c
	common/imagelib/img_main.c
	common/imagelib/img_quant.c
	common/imagelib/img_tga.c
	common/imagelib/img_utils.c
	common/imagelib/img_wad.c
	common/imagelib/img_dds.c
	common/soundlib/snd_main.c
	common/soundlib/snd_mp3.c
	common/soundlib/snd_utils.c
	common/soundlib/snd_wav.c
	common/soundlib/libmpg/dct36.c
	common/soundlib/libmpg/dct64.c
	common/soundlib/libmpg/format.c
	common/soundlib/libmpg/frame.c
	common/soundlib/libmpg/index.c
	common/soundlib/libmpg/layer3.c
	common/soundlib/libmpg/libmpg.c
	common/soundlib/libmpg/mpg123.c
	common/soundlib/libmpg/parse.c
	common/soundlib/libmpg/reader.c
	common/soundlib/libmpg/synth.c
	common/soundlib/libmpg/tabinit.c
	common/Sequence.c
	common/IceKey.cpp
	common/mod_decryptor.cpp 


	server/sv_client.c
	server/sv_cmds.c
	server/sv_custom.c
	server/sv_frame.c
	server/sv_filter.c
	server/sv_game.c
	server/sv_init.c
	server/sv_main.c
	server/sv_log.c
	server/sv_move.c
	server/sv_phys.c
	server/sv_pmove.c
	server/sv_save.c
	server/sv_world.c

)

	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} common/library_static.c)



set(XASH_ENGINE_APPLE_SOURCES
	platform/macos/vid_macos.mm
	platform/ime_stub.cpp
)

set(XASH_ENGINE_CLIENT_SOURCES
   client/cl_cmds.c
   client/cl_demo.c
   client/cl_events.c
   client/cl_frame.c
   client/cl_game.c
   client/cl_main.c
   client/cl_menu.c
   client/cl_parse.c
   client/cl_pmove.c
   client/cl_remap.c
   client/cl_scrn.c
   client/cl_tent.c
   client/cl_video.c
   client/cl_view.c
   client/cl_netgraph.c
   client/gl_backend.c
   client/gl_beams.c
   client/gl_cull.c
   client/gl_decals.c
   client/gl_draw.c
   client/gl_image.c
   client/gl_mirror.c
   client/gl_refrag.c
   client/gl_rlight.c
   client/gl_rmain.c
   client/gl_rmath.c
   client/gl_rmisc.c
   client/gl_rpart.c
   client/gl_rsurf.c
   client/gl_rstrobe.c
   client/gl_sprite.c
   client/gl_studio.c
   client/vid_common.c
   client/gl_warp.c
   client/input.c
   client/keys.c
   client/console.c
   client/gamma.c
   client/s_dsp.c
   client/s_load.c
   client/s_main.c
   client/s_mix.c
   client/s_mouth.c
   client/s_stream.c
   client/s_utils.c
   client/s_vox.c
   client/vgui/vgui_draw.c
)



set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES}
    platform/sdl/events.c platform/sdl/gl_sdl.c
    platform/sdl/snd_sdl.c platform/sdl/vid_sdl.c)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_APPLE_SOURCES})
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_CLIENT_SOURCES})


include_directories(
	.
	common/
	common/imagelib/
	common/soundlib/
	client/
	client/vgui/
	server/
	../common
	../pm_shared
)


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
