//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Thread management routines
//
// $NoKeywords: $
//=============================================================================//

#include "source_sdk/tier0/pch_tier0.h"

#include "source_sdk/public/tier0/valve_off.h"

#include "source_sdk/public/tier0/platform.h"
#include "source_sdk/public/tier0/dbg.h"
#include "source_sdk/public/tier0/threadtools.h"

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
