//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Insert this file into all projects using the memory system
// It will cause that project to use the shader memory allocator
//
// $NoKeywords: $
//=============================================================================//


#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

#undef PROTECTED_THINGS_ENABLE   // allow use of _vsnprintf



#include "tier0/dbg.h"
#include "tier0/memalloc.h"
#include <string.h>
#include <stdio.h>
#include "memdbgoff.h"


#if POSIX
#define __cdecl
#endif

#define MakeModuleFileName() NULL
inline void *AllocUnattributed( size_t nSize )
{
	return g_pMemAlloc->Alloc(nSize);
}

inline void *ReallocUnattributed( void *pMem, size_t nSize )
{
	return g_pMemAlloc->Realloc(pMem, nSize);
}

//-----------------------------------------------------------------------------
// Standard functions in the CRT that we're going to override to call our allocator
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Prevents us from using an inappropriate new or delete method,
// ensures they are here even when linking against debug or release static libs
//-----------------------------------------------------------------------------
#ifndef NO_MEMOVERRIDE_NEW_DELETE
#if defined (OSX) || defined (ANDROID)
void *__cdecl operator new( size_t nSize ) throw (std::bad_alloc)
#else
void *__cdecl operator new( size_t nSize )
#endif
{
	return AllocUnattributed( nSize );
}

void *__cdecl operator new( size_t nSize, int nBlockUse, const char *pFileName, int nLine )
{
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

#if defined (OSX) || defined (ANDROID)
void __cdecl operator delete( void *pMem ) throw()
#else
void __cdecl operator delete( void *pMem )
#endif
{
	g_pMemAlloc->Free( pMem );
}
#if defined (OSX) || defined (ANDROID)
void operator delete(void*pMem, std::size_t)
#else
void operator delete(void*pMem, std::size_t) throw()
#endif
{
	g_pMemAlloc->Free( pMem );
}

#if defined (OSX) || defined (ANDROID)
void *__cdecl operator new[]( size_t nSize ) throw (std::bad_alloc)
#else
void *__cdecl operator new[]( size_t nSize )
#endif
{
	return AllocUnattributed( nSize );
}

void *__cdecl operator new[] ( size_t nSize, int nBlockUse, const char *pFileName, int nLine )
{
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

#if defined (OSX) || defined (ANDROID)
void __cdecl operator delete[]( void *pMem ) throw()
#else
void __cdecl operator delete[]( void *pMem )
#endif
{
	g_pMemAlloc->Free( pMem );
}
#endif


//-----------------------------------------------------------------------------
// Override some debugging allocation methods in MSVC
// NOTE: These have to be here for release + debug builds in case we
// link to a debug static lib!!!
//-----------------------------------------------------------------------------
#ifndef _STATIC_LINKED
#if   defined(POSIX)
#define AttribIfCrt()
#endif // _WIN32


extern "C"
{
	
void *__cdecl _nh_malloc_dbg( size_t nSize, int nFlag, int nBlockUse,
								const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

void *__cdecl _malloc_dbg( size_t nSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

#if defined( _X360 )
void *__cdecl _calloc_dbg_impl( size_t nNum, size_t nSize, int nBlockUse, 
								const char * szFileName, int nLine, int * errno_tmp )
{
	return _calloc_dbg( nNum, nSize, nBlockUse, szFileName, nLine );
}
#endif

void *__cdecl _calloc_dbg( size_t nNum, size_t nSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	void *pMem = g_pMemAlloc->Alloc(nSize * nNum, pFileName, nLine);
	memset(pMem, 0, nSize * nNum);
	return pMem;
}

void *__cdecl _calloc_dbg_impl( size_t nNum, size_t nSize, int nBlockUse, 
	const char * szFileName, int nLine, int * errno_tmp )
{
	return _calloc_dbg( nNum, nSize, nBlockUse, szFileName, nLine );
}

void *__cdecl _realloc_dbg( void *pMem, size_t nNewSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Realloc(pMem, nNewSize, pFileName, nLine);
}

void *__cdecl _expand_dbg( void *pMem, size_t nNewSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	Assert( 0 );
	return NULL;
}

void __cdecl _free_dbg( void *pMem, int nBlockUse )
{
	AttribIfCrt();
	g_pMemAlloc->Free(pMem);
}

size_t __cdecl _msize_dbg( void *pMem, int nBlockUse )
{
#if   POSIX
	Assert( "_msize_dbg unsupported" );
	return 0;
#endif
}



} // end extern "C"


//-----------------------------------------------------------------------------
// Override some the _CRT debugging allocation methods in MSVC
//-----------------------------------------------------------------------------

// Most files include this file, so when it's used it adds an extra .ValveDbg section,
// to help identify debug binaries.

#endif

// Extras added prevent dbgheap.obj from being included - DAL

#endif // _WIN32
