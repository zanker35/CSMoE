//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Includes all the headers/declarations necessary to access the
//			engine interface
//
// $NoKeywords: $
//=============================================================================

#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H


// these stupid set of includes are required to use the cdll_int interface
#include <assert.h>
#include "source_sdk/public/tier0/platform.h"
#include "engine_api/types/xash3d_types.h"

#include "engine_api/interfaces/vgui_api.h"
#include "engine_api/interfaces/cdll_int.h"
#include "engine_api/types/cvardef.h"

// engine interface singleton accessor
extern cl_enginefunc_t *engine;
extern class IGameUIFuncs *gameuifuncs;

#endif // ENGINEINTERFACE_H
