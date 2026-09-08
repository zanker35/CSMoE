//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef MINIDUMP_H
#define MINIDUMP_H

#include "source_sdk/public/tier0/platform.h"

// Set prefix to use for minidump files.  If you don't set one, it is defaulted for you,
// using the current module name
PLATFORM_INTERFACE void SetMinidumpFilenamePrefix( const char *pszPrefix );

// Set comment to put into minidump file upon next call of WriteMiniDump.  (Most common use is the assert text.)
PLATFORM_INTERFACE void SetMinidumpComment( const char *pszComment );

// writes out a minidump of the current stack trace with a unique filename
PLATFORM_INTERFACE void WriteMiniDump( const char *pszFilenameSuffix = NULL );

typedef void (*FnWMain)( int , tchar *[] );
typedef void (*FnVoidPtrFn)( void * );


//
// Minidump User Stream Info Comments.
//
// There currently is a single header string, and an array of 64 comment strings.
//	MinidumpUserStreamInfoSetHeader() will set the single header string.
//	MinidumpUserStreamInfoAppend() will round robin through and array and set the comment strings, overwriting old.
PLATFORM_INTERFACE void MinidumpUserStreamInfoSetHeader( const char *pFormat, ... );
PLATFORM_INTERFACE void MinidumpUserStreamInfoAppend( const char *pFormat, ... );

// Retrieve the StreamInfo strings.
//  Index 0: header string
//  Index 1..: comment string
//  Returns NULL when you've reached the end of the comment string array
//  Empty strings ("\0") can be returned if comment hasn't been set
PLATFORM_INTERFACE const char *MinidumpUserStreamInfoGet( int Index );

#endif // MINIDUMP_H

