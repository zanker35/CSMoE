#include "extdll.h"
#include "enginecallback.h"
#include "player_model.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

static const char *sPlayerModelFiles[] =
{
	"models/player.mdl",
	"models/player/leet/leet.mdl",
	"models/player/gign/gign.mdl",
	"models/player/vip/vip.mdl",
	"models/player/gsg9/gsg9.mdl",
	"models/player/guerilla/guerilla.mdl",
	"models/player/arctic/arctic.mdl",
	"models/player/sas/sas.mdl",
	"models/player/terror/terror.mdl",
	"models/player/urban/urban.mdl",
	"models/player/spetsnaz/spetsnaz.mdl",	// CZ
	"models/player/militia/militia.mdl"	// CZ
};

const char *Client_ApperanceToModel(int iApperance)
{
	if (iApperance < 0 || iApperance >= int(sizeof(sPlayerModelFiles) / sizeof(sPlayerModelFiles[0])))
		return sPlayerModelFiles[0];
	return sPlayerModelFiles[iApperance];
}

void PlayerModel_Precache()
{
	for(auto psz : sPlayerModelFiles)
		PRECACHE_MODEL(const_cast<char *>(psz));
	PlayerClassManager().PlayerModel_Precache();
}

void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax)
{
	for (auto psz : sPlayerModelFiles)
		ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&vMin, (float *)&vMax, psz);
}

// The shared class order is the citrus wire/menu order. Only models supplied by
// the local CSO resource pack are selectable; buffclass IDs remain reserved.
static CPlayerClassManager::ClassData gPlayerClass[] =
{
	{ MODEL_UNASSIGNED, nullptr, UNASSIGNED },
	{ MODEL_YURI, "yuri", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_SAF, "saf", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_PIRATEBOY, "pirateboy", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_CHOIJIYOON, "choijiyoon", CT, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_MARINEBOY, "marineboy", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_FERNANDO, "fernando", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_PIRATEGIRL, "pirategirl", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_707, "707", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_RB, "rb", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_SOZO, "sozo", CT, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_JPNGIRL01, "jpngirl01", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_MAGUI, "magui", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_RITSUKA, "ritsuka", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_NATASHA, "natasha", CT, true, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_TERROR, "terror", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_URBAN, "urban", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_LEET, "leet", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_GSG9, "gsg9", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_ARCTIC, "arctic", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_SAS, "sas", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_GUERILLA, "guerilla", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_GIGN, "gign", CT, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_MILITIA, "militia", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE },
	{ MODEL_SPETSNAZ, "spetsnaz", CT, false, SHOW_SPEED | SHOW_DAMAGE },
};

constexpr int NUM_PLAYER_CLASSES = sizeof(gPlayerClass) / sizeof(gPlayerClass[0]);

CPlayerClassManager &PlayerClassManager()
{
	static CPlayerClassManager manager;
	return manager;
}

void CPlayerClassManager::PlayerModel_Precache()
{
	char path[128];
	for (int i = 1; i < NUM_PLAYER_CLASSES; ++i)
	{
		Client_ApperanceToModel(path, i);
		PRECACHE_MODEL(path);
	}
}

const CPlayerClassManager::ClassData &CPlayerClassManager::PlayerClass_GetInfo(int classId) const
{
	return gPlayerClass[classId > 0 && classId < NUM_PLAYER_CLASSES ? classId : 0];
}

const char *CPlayerClassManager::PlayerClass_GetModelName(int classId) const
{
	if (classId == MODEL_VIP)
		return "vip";
	const auto &info = PlayerClass_GetInfo(classId);
	return info.model_name ? info.model_name : "urban";
}

bool CPlayerClassManager::PlayerClass_IsFemale(int classId) const
{
	return PlayerClass_GetInfo(classId).isFemale;
}

void CPlayerClassManager::Client_ApperanceToModel(char *buffer, int classId) const
{
	const char *model = PlayerClass_GetModelName(classId);
	Q_sprintf(buffer, "models/player/%s/%s.mdl", model, model);
}

int CPlayerClassManager::Client_ModelToApperance(const char *modelName) const
{
	if (!modelName)
		return MODEL_UNASSIGNED;

	char path[128];
	for (int i = 1; i < NUM_PLAYER_CLASSES; ++i)
	{
		Client_ApperanceToModel(path, i);
		if (!Q_stricmp(modelName, path) || !Q_stricmp(modelName, gPlayerClass[i].model_name))
			return i;
	}
	return MODEL_UNASSIGNED;
}

int CPlayerClassManager::PlayerClass_GetNumClass() const
{
	return NUM_PLAYER_CLASSES;
}

int CPlayerClassManager::PlayerClass_GetNumCT() const
{
	return (NUM_PLAYER_CLASSES - 1) / 2;
}

int CPlayerClassManager::PlayerClass_GetNumTR() const
{
	return (NUM_PLAYER_CLASSES - 1) / 2;
}

ModelName CPlayerClassManager::PlayerClass_GetRandomClass() const
{
	return static_cast<ModelName>(RANDOM_LONG(1, NUM_PLAYER_CLASSES - 1));
}

ModelName CPlayerClassManager::PlayerClass_FromTeamSlot(TeamName team, int slot) const
{
	if ((team != CT && team != TERRORIST) || slot < 1 || slot > PlayerClass_GetNumCT())
		return MODEL_UNASSIGNED;
	return static_cast<ModelName>(slot * 2 - (team == TERRORIST ? 1 : 0));
}

int CPlayerClassManager::PlayerClass_GetTeamSlot(int classId) const
{
	return PlayerClass_GetInfo(classId).model_name ? (classId + 1) / 2 : 0;
}

void CPlayerClassManager::SetPlayerClass(int index, const char *name)
{
	if (index < 0 || index >= int(sizeof(m_PlayerClass) / sizeof(m_PlayerClass[0])))
		return;
	m_PlayerClass[index] = &gPlayerClass[Client_ModelToApperance(name)];
}

CPlayerClassManager::ClassData &CPlayerClassManager::GetPlayerClass(int index)
{
	if (index < 0 || index >= int(sizeof(m_PlayerClass) / sizeof(m_PlayerClass[0])) || !m_PlayerClass[index])
		return m_NullClass;
	return *m_PlayerClass[index];
}

}
