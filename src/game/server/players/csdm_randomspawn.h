#ifndef CSDM_RANDOMSPAWN_H
#define CSDM_RANDOMSPAWN_H

#include <utility>
#include "game/shared/entities/vector.h"

namespace sv {

class CBaseEntity;
void CSDM_LoadSpawnPoints();

bool CSDM_DoRandomSpawn(CBaseEntity *pEntity);

}

#endif
