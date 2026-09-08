#include "game/shared/movement/pm_shared.h"
#include "base/utllinkedlist.h"

// CSBOT and Nav
#include "game/server/match/GameEvent.h"		// Game event enum used by career mode, tutor system, and bots
#include "game/server/ai/navigation/bot_util.h"
#include "game/server/ai/navigation/simple_state_machine.h"

#include "game/server/runtime/steam_util.h"

#include "game/server/ai/navigation/bot_manager.h"
#include "game/server/ai/navigation/bot_constants.h"
#include "game/server/ai/navigation/bot.h"

#include "game/shared/strings/shared_util.h"
#include "game/server/ai/navigation/bot_profile.h"

#include "game/server/ai/navigation/nav.h"
#include "game/server/ai/navigation/improv.h"
#include "game/server/ai/navigation/nav_node.h"
#include "game/server/ai/navigation/nav_area.h"
#include "game/server/ai/navigation/nav_file.h"
#include "game/server/ai/navigation/nav_path.h"

#include "game/server/ai/h_ai.h"
#include "game/server/entities/h_cycler.h"

// Hostage
#include "game/server/ai/hostage/hostage.h"
#include "game/server/ai/hostage/hostage_localnav.h"
#include "game/server/ai/hostage/hostage_improv.h"

#include "game/server/ai/cs_bot/cs_bot.h"

// Tutor

#include "game/shared/entities/gamerules.h"
#include "game/server/match/maprules.h"
