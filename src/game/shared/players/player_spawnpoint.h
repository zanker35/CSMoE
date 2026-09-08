#ifndef PLAYER_SPAWNPOINT_H
#define PLAYER_SPAWNPOINT_H

class CBaseEntity;
BOOL IsSpawnPointValid(CBaseEntity *pPlayer, CBaseEntity *pSpot);
edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer);

#endif
