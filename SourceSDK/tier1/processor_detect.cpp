//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: win32 dependant ASM code for CPU capability detection
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if defined( _X360 ) || defined( _M_X64 ) || defined(_M_ARM) || defined(_M_ARM64)

bool CheckMMXTechnology(void) { return false; }
bool CheckSSETechnology(void) { return false; }
bool CheckSSE2Technology(void) { return false; }
bool Check3DNowTechnology(void) { return false; }

#endif // _WIN32
