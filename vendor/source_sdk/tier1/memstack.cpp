//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#if   defined( _X360 )
#define VA_COMMIT_FLAGS (MEM_COMMIT|MEM_NOZERO|MEM_LARGE_PAGES)
#define VA_RESERVE_FLAGS (MEM_RESERVE|MEM_LARGE_PAGES)
#endif

#include "source_sdk/public/tier0/dbg.h"
#include "source_sdk/public/tier1/memstack.h"
#include "source_sdk/public/tier1/utlmap.h"
#include "source_sdk/public/tier0/memdbgon.h"


//-----------------------------------------------------------------------------

MEMALLOC_DEFINE_EXTERNAL_TRACKING(CMemoryStack);

//-----------------------------------------------------------------------------

CMemoryStack::CMemoryStack()
 : 	m_pBase( NULL ),
	m_pNextAlloc( NULL ),
	m_pAllocLimit( NULL ),
	m_pCommitLimit( NULL ),
	m_alignment( 16 ),
 	m_maxSize( 0 )
{
}
	
//-------------------------------------

CMemoryStack::~CMemoryStack()
{
	if ( m_pBase )
		Term();
}

//-------------------------------------

bool CMemoryStack::Init( unsigned maxSize, unsigned commitSize, unsigned initialCommit, unsigned alignment )
{
	Assert( !m_pBase );

#ifdef _X360
	m_bPhysical = false;
#endif

	m_maxSize = maxSize;
	m_alignment = AlignValue( alignment, 4 );

	Assert( m_alignment == alignment );
	Assert( m_maxSize > 0 );

	m_pBase = (byte *)MemAlloc_AllocAligned( m_maxSize, alignment ? alignment : 1 );
	m_pNextAlloc = m_pBase;
	m_pCommitLimit = m_pBase + m_maxSize;

	m_pAllocLimit = m_pBase + m_maxSize;

	return ( m_pBase != NULL );
}

//-------------------------------------

#ifdef _X360
bool CMemoryStack::InitPhysical( unsigned size, unsigned alignment )
{
	m_bPhysical = true;

	m_maxSize = m_commitSize = size;
	m_alignment = AlignValue( alignment, 4 );

	int flags = PAGE_READWRITE;
	if ( size >= 16*1024*1024 )
	{
		flags |= MEM_16MB_PAGES;
	}
	else
	{
		flags |= MEM_LARGE_PAGES;
	}
	m_pBase = (unsigned char *)XPhysicalAlloc( m_maxSize, MAXULONG_PTR, 4096, flags );
	Assert( m_pBase );
	m_pNextAlloc = m_pBase;
	m_pCommitLimit = m_pBase + m_maxSize;
	m_pAllocLimit = m_pBase + m_maxSize;

	MemAlloc_RegisterExternalAllocation( CMemoryStack, GetBase(), GetSize() );
	return ( m_pBase != NULL );
}
#endif

//-------------------------------------

void CMemoryStack::Term()
{
	FreeAll();
	if ( m_pBase )
	{
		MemAlloc_FreeAligned( m_pBase );
		m_pBase = NULL;
	}
}

//-------------------------------------

int CMemoryStack::GetSize()
{ 
	return m_maxSize;
}


//-------------------------------------

bool CMemoryStack::CommitTo( byte *pNextAlloc ) RESTRICT
{
#ifdef _X360
	if ( m_bPhysical )
	{
		return NULL;
	}
#endif
	Assert( 0 );
	return false;
}

//-------------------------------------

void CMemoryStack::FreeToAllocPoint( MemoryStackMark_t mark, bool bDecommit )
{
	void *pAllocPoint = m_pBase + mark;
	Assert( pAllocPoint >= m_pBase && pAllocPoint <= m_pNextAlloc );
	
	if ( pAllocPoint >= m_pBase && pAllocPoint < m_pNextAlloc )
	{
		if ( bDecommit )
		{
		}
		m_pNextAlloc = (unsigned char *)pAllocPoint;
	}
}

//-------------------------------------

void CMemoryStack::FreeAll( bool bDecommit )
{
	if ( m_pBase && m_pCommitLimit - m_pBase > 0 )
	{
		if ( bDecommit )
		{
		}
		m_pNextAlloc = m_pBase;
	}
}

//-------------------------------------

void CMemoryStack::Access( void **ppRegion, unsigned *pBytes )
{
	*ppRegion = m_pBase;
	*pBytes = ( m_pNextAlloc - m_pBase);
}

//-------------------------------------

void CMemoryStack::PrintContents()
{
	Msg( "Total used memory:      %d\n", GetUsed() );
	Msg( "Total committed memory: %d\n", GetSize() );
}

//-----------------------------------------------------------------------------
