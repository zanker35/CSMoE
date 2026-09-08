//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any
#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI
#include "engine_api/types/const.h"
#include "engine_api/protocol/entity_state.h"
#include "engine_api/types/cl_entity.h"
#include "engine_api/interfaces/triangleapi.h"
#include "game/client/effects/rain.h"

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
	gHUD.m_Spectator.DrawOverview();
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
	ProcessFXObjects();
	ProcessRain();
	DrawRain();
	DrawFXObjects();
}
