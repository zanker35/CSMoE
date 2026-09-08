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

cmake_minimum_required(VERSION 3.10.2)

project (server)
set(CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -stdlib=libc++")

set (SERVER_LIBRARY server)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(SERVER_INSTALL_DIR "dlls" CACHE STRING "Where put server dll")
	set(SERVER_LIBRARY_NAME "cs" CACHE STRING "Library name for Linux/MacOS")

#-----------------
# MAIN BUILD CODE \
###################\

set(HLDLL_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/ammo.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/animating.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/animation.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/bmodels.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/buttons.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/cbase.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/client.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/combat.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/debug.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/doors.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/effects.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/explode.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/func_break.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/game.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/gamerules.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/ggrenade.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/globals.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/h_ai.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/h_cycler.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/h_export.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/csdm_randomspawn.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/hintmessage.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_msg.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_spawnpoint.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/data/player_model.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_zombie.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_spawnprotection.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_knockback.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/items.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/lights.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/maprules.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/mpstubb.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/multiplay_gamerules.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/observer.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/pathcorner.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/plats.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/skill.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/sound.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/soundent.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/spectator.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/subs.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/triggers.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/util.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/vehicle.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/weapons.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/weapons_buy.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/world.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_chatter.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_event.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_init.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_learn.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_listen.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_manager.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_nav.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_pathfind.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_radio.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_statemachine.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_update.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_vision.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_weapon.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_gamestate.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_attack.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_buy.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_defuse_bomb.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_escape_from_bomb.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_fetch_bomb.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_follow.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_hide.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_hunt.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_idle.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_investigate_noise.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_move_to.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_plant_bomb.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/states/cs_bot_use_entity.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/hostage.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/hostage_improv.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/hostage_localnav.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/states/hostage_animate.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/states/hostage_escape.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/states/hostage_follow.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/states/hostage_idle.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/hostage/states/hostage_retreat.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mods.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mod_base.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mod_none.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mod_tdm.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mod_dm.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mod_zb1.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/strings/unicode_strtools.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/mod_zb2.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb2/supplybox.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/cs_bot/cs_bot_bhopjump.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/weapons_data.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/weapons_msg.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_mod_strategy.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/players/player_account.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/base/cbase_memory.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb2/zb2_skill_human.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb2/zb2_skill_zombie.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb2/zb2_zclass.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb2/zb2_zclass_tank.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb2/zb2_zclass_speed.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/combat/weapons_precache.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/match/modes/zb1/zb1_countdown.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/base/cbase_entity_factory.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/entities/base/cbase_physint.cpp"
	)

set (HLWPN_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_ak47.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_aug.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_awp.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_c4.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_deagle.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_elite.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_famas.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_fiveseven.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_flashbang.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_g3sg1.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_galil.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_glock18.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_hegrenade.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_knife.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m249.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m3.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m4a1.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_mac10.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_mp5navy.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_p228.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_p90.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_scout.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_sg550.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_sg552.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_smokegrenade.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_tmp.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_ump45.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_usp.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_xm1014.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/knife_zombi.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_ak47l.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_mp7a1d.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_deagled.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_infinity.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_k1a.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m1887.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_scarh.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_scarl.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_xm8c.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_xm8s.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_mg3.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_mp7a1c.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_mp7a1p.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_infinitys.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m14ebr.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_infinityex1.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_infinityex2.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_anaconda.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m134.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_m4a1dragon.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_ak47dragon.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/weapons/wpn_knifedragon.cpp"
	)

set(PM_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/shared/movement/pm_shared.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/movement/pm_debug.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/shared/movement/pm_math.cpp"
    )

set(GAME_SHARED_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/shared_util.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/bot.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/bot_manager.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/bot_profile.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/bot_util.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/nav_area.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/nav_file.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/nav_node.cpp"
	"${CMAKE_SOURCE_DIR}/src/game/server/ai/navigation/nav_path.cpp"
    )

set(PUBLIC_SRCS
	"${CMAKE_SOURCE_DIR}/src/game/server/runtime/memory/MemPool.cpp"
    )

set (SERVER_SOURCES
	${HLWPN_SRCS}
	${HLDLL_SRCS}
	${PM_SRCS}
	${GAME_SHARED_SRCS}
	${PUBLIC_SRCS}
)



	add_library (${SERVER_LIBRARY} STATIC ${SERVER_SOURCES})

	add_definitions(
		-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf
		-D_alloca=alloca -D_strdup=strdup -D_write=write -DNO_MALLOC_OVERRIDE
		-D_close=close -D_write=write
	)




add_definitions(
	-DCLIENT_WEAPONS
	-DQUIVER -DQUAKE2 -DVALVE_DLL -DSERVER_DLL
	)

target_link_libraries( ${SERVER_LIBRARY} ${CMAKE_DL_LIBS} )

	set_target_properties(${SERVER_LIBRARY} PROPERTIES
			PREFIX "" OUTPUT_NAME ${SERVER_LIBRARY_NAME})


if(XASH_PCH)
	target_precompile_headers(${SERVER_LIBRARY} PRIVATE "${CMAKE_SOURCE_DIR}/src/game/server/runtime/stdafx.h")
endif()

if(XASH_UNITY_BUILD)
	set_target_properties(${SERVER_LIBRARY} PROPERTIES UNITY_BUILD ON)
endif()
