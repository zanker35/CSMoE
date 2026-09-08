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

#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"

#include "engine_api/interfaces/render_api.h"
#include "game/client/hud/vgui_parser.h"

#include "engine_api/types/const.h"
#include "engine_api/protocol/entity_state.h"
#include "engine_api/protocol/usercmd.h"
#include "engine_api/types/ref_params.h"
#include "engine_api/types/cl_entity.h"
#include "engine_api/types/cdll_exp.h"
#include "game/client/hud/draw_util.h"
#include "engine_api/interfaces/triangleapi.h"
#include "engine_api/types/entity_types.h"
#include "engine_api/protocol/studio_event.h" // def. of mstudioevent_t
#include "engine_api/interfaces/r_efx.h"
#include "engine_api/interfaces/event_api.h"
#include "game/client/effects/eventscripts.h"
#include "game/client/view/camera.h"
#include "game/client/input/kbutton.h"
#include "engine_api/types/cvardef.h"
#include "game/client/input/in_defs.h"
#include "game/client/prediction/com_weapons.h"
#include "game/client/effects/rain.h"
#include "game/client/view/studio/studio_util.h"

#include "engine_api/types/pm_movevars.h"
#include "game/shared/movement/pm_shared.h"
#include "engine_api/types/pm_defs.h"
#include "game/shared/movement/pm_debug.h"
#include "engine_api/types/pmtrace.h"

#include "engine_api/types/screenfade.h"
#include "engine_api/types/shake.h"
#include "engine_api/protocol/hltv.h"
#include "engine_api/interfaces/r_studioint.h"
#include "game/client/input/input.h"

#include "game/client/hud/ammohistory.h"
#include "game/shared/interfaces/cdll_dll.h"
#include "game/client/hud/hud_sub_impl.h"
#include "game/shared/data/mods_const.h"

#include "game/client/effects/events.h"
#include "game/shared/interfaces/exportdef.h"

#include "base/std_include.h"
