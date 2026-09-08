//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#if !defined( CVAR_H )
#define CVAR_H

#include "source_sdk/public/vstdlib/vstdlib.h"
#include "source_sdk/public/icvar.h"


//-----------------------------------------------------------------------------
// Returns a CVar dictionary for tool usage
//-----------------------------------------------------------------------------
VSTDLIB_INTERFACE CreateInterfaceFn VStdLib_GetICVarFactory();


#endif // CVAR_H
