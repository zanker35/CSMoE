/*
stdafx.h - Pre-compile header
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/players/player.h"
#include "game/shared/weapons/weapons.h"

#ifndef CLIENT_DLL
#include "game/shared/entities/monsters.h"
#include "game/server/match/game.h"
#include "game/server/players/client.h"
#include "game/shared/interfaces/enginecallback.h"
#include "game/server/runtime/globals.h"
#include "game/server/entities/trains.h"
#include "game/server/entities/bmodels.h"
#include "game/server/ai/bot_include.h"

#include "game/server/match/modes/mods.h"

#include "game/shared/types/u_functor.hpp"
#include "game/shared/types/u_iterator.hpp"
#include "game/shared/types/u_range.hpp"
#include "game/shared/time/u_time.hpp"
#include "game/shared/math/u_vector.hpp"
#endif

#include "base/std_include.h"
