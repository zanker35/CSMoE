//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Thread management routines
//
// $NoKeywords: $
//=============================================================================//

#include "pch_tier0.h"

#include "tier0/valve_off.h"

#include "tier0/platform.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"

unsigned long Plat_GetCurrentThreadID()
{
	return ThreadGetCurrentId();
}



void Plat_SetHardwareDataBreakpoint( const void *pAddress, int nWatchBytes, bool bBreakOnRead )
{
	// no impl on this platform yet
}

void Plat_ApplyHardwareDataBreakpointsToNewThread( unsigned long dwThreadID )
{
	// no impl on this platform yet
}
