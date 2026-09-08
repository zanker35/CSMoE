//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: return the cpu usage as a float value
//
// On win32 this is 0.0 to 1.0 indicating the amount of CPU time used
// On posix its the load avg from the last minute
// 
// On win32 you need to call this once in a while.  Every few seconds.
// First call returns zero
//=============================================================================//

#include "pch_tier0.h"
#include "tier0/platform.h"


#ifdef POSIX
#include <stdlib.h>

float GetCPUUsage() 
{
	double loadavg[3];

	getloadavg( loadavg, 3 );
	return loadavg[0];
}
#endif //POSIX
