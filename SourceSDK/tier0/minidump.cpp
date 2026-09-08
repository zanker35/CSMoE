//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "pch_tier0.h"

#include "tier0/minidump.h"
#include "tier0/platform.h"

#if   defined(_X360 )
PLATFORM_INTERFACE void WriteMiniDump( const char *pszFilenameSuffix )
{
	DmCrashDump(false);
}

#else // !_WIN32
#include "tier0/minidump.h"

PLATFORM_INTERFACE void WriteMiniDump( const char *pszFilenameSuffix )
{
}

PLATFORM_INTERFACE void CatchAndWriteMiniDump( FnWMain pfn, int argc, tchar *argv[] )
{
	pfn( argc, argv );
}

#endif 

// User minidump stream info comment strings.
//
// Single header string of 512 bytes set via MinidumpUserStreamInfoSetHeader.
static char g_UserStreamInfoHeader[ 512 ];
// Array of 32 round robin 128 byte strings set via MinidumpUserStreamInfoAppend.
static char g_UserStreamInfo[ 64 ][ 128 ];
static int g_UserStreamInfoIndex = 0;

// Set the single g_UserStreamInfoHeader string.
void MinidumpUserStreamInfoSetHeader( const char *pFormat, ... )
{
	va_list marker;

	va_start( marker, pFormat );
	_vsnprintf( g_UserStreamInfoHeader, ARRAYSIZE( g_UserStreamInfoHeader ), pFormat, marker );
	g_UserStreamInfoHeader[ ARRAYSIZE( g_UserStreamInfoHeader ) - 1 ] = 0;
	va_end( marker );
}

// Set the next comment in the g_UserStreamInfo array.
void MinidumpUserStreamInfoAppend( const char *pFormat, ... )
{
	va_list marker;
	char *pData = g_UserStreamInfo[ g_UserStreamInfoIndex ];
	const int DataSize = ARRAYSIZE( g_UserStreamInfo[ g_UserStreamInfoIndex ] );

	// Add tick count just so we have a general idea of when this event happened.
	_snprintf( pData, DataSize, "[%x]", Plat_MSTime() );
	pData[ DataSize - 1 ] = 0;
	size_t HeaderLen = strlen( pData );

	va_start( marker, pFormat );
	_vsnprintf( pData + HeaderLen, DataSize - HeaderLen, pFormat, marker );
	pData[ DataSize - 1 ] = 0;
	va_end( marker );

	// Bump up index, and go back to 0 if we've hit the end.
	g_UserStreamInfoIndex++;
	if( g_UserStreamInfoIndex >= ARRAYSIZE( g_UserStreamInfo ) )
	{
		g_UserStreamInfoIndex = 0;
	}
}

// Retrieve the string given the Index.
//	Index 0: header string
//	Index 1+: comment string
//	Returns NULL when you've reached the end of the comment string array
//  Empty strings ("\0") can be returned if comment hasn't been set
const char *MinidumpUserStreamInfoGet( int Index )
{
	if( ( Index < 0 ) || ( Index >= (ARRAYSIZE( g_UserStreamInfo ) + 1) ) ) //+1 because we map 0 to the header
		return NULL;

	if( Index == 0 )
		return g_UserStreamInfoHeader;

	Index = ( (Index + (ARRAYSIZE( g_UserStreamInfo ) - 1)) + //subtract 1 in a way that circularly wraps. Since 0 maps to the header, the comment indices are 1 based
		g_UserStreamInfoIndex ) //start with our oldest comment
		% ARRAYSIZE( g_UserStreamInfo ); //circular buffer wrapping

	return g_UserStreamInfo[ Index ];
}


