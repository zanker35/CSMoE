#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H

#include "game/shared/data/player_const.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum PlayerClassShowState
{
	SHOW_DAMAGE = 1 << 0,
	SHOW_SPEED = 1 << 1,
	SHOW_ACCSHOOT_FIRST = 1 << 2,
	SHOW_TRACE_LOW_HEALTH = 1 << 3,
	SHOW_LAST = 1 << 4,
};

const char *Client_ApperanceToModel(int iApperance);

void PlayerModel_Precache();
void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax);

class CPlayerClassManager
{
public:
	struct ClassData
	{
		int ClassID = MODEL_UNASSIGNED;
		const char *model_name = nullptr;
		TeamName team = UNASSIGNED;
		bool isFemale = false;
		int m_iBitsShowState = 0;
		int HandTexid[6] = {};
	};

	void PlayerModel_Precache();
	const char *PlayerClass_GetModelName(int classId) const;
	bool PlayerClass_IsFemale(int classId) const;
	const ClassData &PlayerClass_GetInfo(int classId) const;
	void Client_ApperanceToModel(char *buffer, int classId) const;
	int Client_ModelToApperance(const char *modelName) const;
	int PlayerClass_GetNumClass() const;
	int PlayerClass_GetNumCT() const;
	int PlayerClass_GetNumTR() const;
	ModelName PlayerClass_GetRandomClass() const;
	ModelName PlayerClass_FromTeamSlot(TeamName team, int slot) const;
	int PlayerClass_GetTeamSlot(int classId) const;

	void SetPlayerClass(int index, const char *name);
	ClassData &GetPlayerClass(int index);

	ClassData *m_PlayerClass[33] = {};
	ClassData m_NullClass = {};
};

CPlayerClassManager &PlayerClassManager();

}

#endif
