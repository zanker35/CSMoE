#
# Copyright (c) 2015 Pavlo Lavrenenko
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

cmake_minimum_required(VERSION 3.10.2)

project (client)

set(CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -stdlib=libc++")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ")

#find_package(SDL2 REQUIRED)

set (CLIENT_LIBRARY client)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(CLIENT_INSTALL_DIR "cl_dlls" CACHE STRING "Where put client dll")
set(CLIENT_LIBRARY_NAME "client" CACHE STRING "Library name for client")

#-----------------
# MAIN BUILD CODE \
###################\

file (GLOB WEAPONEVENT_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/client/effects/ev_common.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/effects/events/ev_cs16.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/effects/events/hl_events.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/effects/events/event_*.cpp"
)

set (CLWEAPON_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/client/prediction/com_weapons.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/prediction/cs_baseentity.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/prediction/cs_weapons.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/prediction/bte_weapons.cpp"
)

set (HUD_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/ammo.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/ammo_secondary.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/ammohistory.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/battery.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/geiger.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/health.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/hud.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/hud_msg.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/hud_redraw.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/hud_spectator.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/hud_update.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/sniperscope.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/nvg.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/hud_menu.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/message.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/status_icons.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/statusbar.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/text_message.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/train.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/scoreboard.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/MOTD.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/radio.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/radar.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/timer.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/money.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/flashlight.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/death.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/saytext.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/spectator_gui.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/followicon.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/headname.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/retina.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/scenariostatus.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/zb2/zb2.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/zb2/zb2_skill.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/legacy/hud_radar_legacy.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/legacy/hud_scoreboard_legacy.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/modern/hud_radar_modern.cpp"

)

set (STUDIORENDER_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/client/view/studio/GameStudioModelRenderer.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/view/studio/StudioModelRenderer.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/view/studio/studio_util.cpp"

)

set(INPUT_SRCS "${CMAKE_SOURCE_DIR}/src/game/client/input/input_xash3d.cpp")

set (CLIENT_SOURCES
	"${CMAKE_SOURCE_DIR}/src/game/client/view/r_texture.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/newhud/NewHud.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/showwin.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/runtime/cdll_int.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/client/runtime/entity.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/client/view/in_camera.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/client/input/input.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/effects/rain.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/client/view/tri.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/runtime/util.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/view/view.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/client/hud/vgui_parser.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/hud/draw_util.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/client/view/calcscreen.cpp"

	${INPUT_SRCS}
	${CSMOE_GAME_SHARED_SOURCES}
	${HUD_SRCS}
	${WEAPONEVENT_SRCS}
	${CLWEAPON_SRCS}

	${STUDIORENDER_SRCS}
)

	list(APPEND CLIENT_SOURCES
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/CBackGroundPanel.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/CBaseViewport.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/CClientMOTD.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/CClientVGUI.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/CHudViewPort.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/CGameUITestPanel.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/game_controls/buymenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/game_controls/buysubmenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/game_controls/teammenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/game_controls/classmenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/CSBackGroundPanel.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/vgui_int.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/cstriketeammenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/cstrikeclassmenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/BuyMenu/buypresetbutton.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/BuyMenu/cstrikebuymenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/BuyMenu/cstrikebuymouseoverpanel.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/BuyMenu/cstrikebuymouseoverpanelbutton.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/BuyMenu/cstrikebuysubmenu.cpp"
		"${CMAKE_SOURCE_DIR}/src/game/client/menus/csmoe/BuyMenu/WeaponImagePanel.cpp")
	add_definitions(-DXASH_VGUI2=1)



	add_library (${CLIENT_LIBRARY} STATIC ${CLIENT_SOURCES} "${CMAKE_SOURCE_DIR}/src/game/client/effects/events/eventdef.h")

	add_definitions(
			-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
	)




add_definitions( -D_CS16CLIENT_ENABLE_GSRC_SUPPORT
	-DCLIENT_WEAPONS -DCLIENT_DLL
	-D_CS16CLIENT_ALLOW_SPECIAL_SCRIPTING
	 )

target_link_libraries( ${CLIENT_LIBRARY} ${CMAKE_DL_LIBS} )

	target_link_libraries(${CLIENT_LIBRARY} vgui_controls cso_controls)




	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			PREFIX "" OUTPUT_NAME ${CLIENT_LIBRARY_NAME})

	get_target_property(CLIENT_LIBRARY_NAME ${CLIENT_LIBRARY} OUTPUT_NAME)
if(NOT CLIENT_LIBRARY_NAME)
		set(CLIENT_LIBRARY_NAME ${CLIENT_LIBRARY})
endif()
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			OUTPUT_NAME ${CLIENT_LIBRARY_NAME}64)


if(XASH_PCH)
	target_precompile_headers(${CLIENT_LIBRARY} PRIVATE "${CMAKE_SOURCE_DIR}/src/game/client/runtime/stdafx.h")
endif()

if(XASH_UNITY_BUILD)
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES UNITY_BUILD ON)
endif()
