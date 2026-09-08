//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( HUD_IFACEH )
#define HUD_IFACEH

#include "game/shared/interfaces/exportdef.h"

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);
#include "engine_api/types/wrect.h"
#include "engine_api/interfaces/cdll_int.h"
extern cl_enginefunc_t gEngfuncs;

#endif
