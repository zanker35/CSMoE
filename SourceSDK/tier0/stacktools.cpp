//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//===========================================================================//

#include "pch_tier0.h"
#include "tier0/stacktools.h"
#include "tier0/threadtools.h"
#include "tier0/icommandline.h"

#include "tier0/valve_off.h"

#if defined( PLATFORM_WINDOWS_PC )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#endif

#if defined( PLATFORM_X360 )
#include <xbdm.h>
#include "xbox/xbox_console.h"
#include "xbox/xbox_vxconsole.h"
#include <map>
#include <set>
#endif

#if defined( LINUX ) && !defined( ANDROID )
#include <execinfo.h>
#endif

#include "tier0/valve_on.h"

#include "tier0/memdbgon.h"


#if !defined( ENABLE_RUNTIME_STACK_TRANSLATION ) //disable the whole toolset

#if defined( LINUX ) && !defined( ANDROID )

int GetCallStack( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	return backtrace( pReturnAddressesOut, iArrayCount );
}

int GetCallStack_Fast( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	return backtrace( pReturnAddressesOut, iArrayCount );
}

#else

int GetCallStack( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	return 0;
}

int GetCallStack_Fast( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	return 0;
}

#endif

//where we'll find our PDB's for win32. Translation will not work until this has been called once (even if with NULL)
void SetStackTranslationSymbolSearchPath( const char *szSemicolonSeparatedList )
{
}

void StackToolsNotify_LoadedLibrary( const char *szLibName )
{
}

int TranslateStackInfo( const void * const *pCallStack, int iCallStackCount, tchar *szOutput, int iOutBufferSize, const tchar *szEntrySeparator, TranslateStackInfo_StyleFlags_t style )
{
	if( iOutBufferSize > 0 )
		*szOutput = '\0';

	return 0;
}

void PreloadStackInformation( const void **pAddresses, int iAddressCount )
{
}

bool GetFileAndLineFromAddress( const void *pAddress, tchar *pFileNameOut, int iMaxFileNameLength, uint32 &iLineNumberOut, uint32 *pDisplacementOut )
{
	if( iMaxFileNameLength > 0 )
		*pFileNameOut = '\0';

	return false;
}

bool GetSymbolNameFromAddress( const void *pAddress, tchar *pSymbolNameOut, int iMaxSymbolNameLength, uint64 *pDisplacementOut )
{
	if( iMaxSymbolNameLength > 0 )
		*pSymbolNameOut = '\0';

	return false;
}

bool GetModuleNameFromAddress( const void *pAddress, tchar *pModuleNameOut, int iMaxModuleNameLength )
{
	if( iMaxModuleNameLength > 0 )
		*pModuleNameOut = '\0';

	return false;
}

#else //#if !defined( ENABLE_RUNTIME_STACK_TRANSLATION )

//===============================================================================================================
// Shared Windows/X360 code
//===============================================================================================================

CTHREADLOCALPTR( CStackTop_Base ) g_StackTop;

class CStackTop_FriendFuncs : public CStackTop_Base
{
public:
	friend int AppendParentStackTrace( void **pReturnAddressesOut, int iArrayCount, int iAlreadyFilled );
	friend int GetCallStack_Fast( void **pReturnAddressesOut, int iArrayCount, int iSkipCount );
};


inline int AppendParentStackTrace( void **pReturnAddressesOut, int iArrayCount, int iAlreadyFilled )
{
	CStackTop_FriendFuncs *pTop = (CStackTop_FriendFuncs *)(CStackTop_Base *)g_StackTop;
	if( pTop != NULL )
	{
		if( pTop->m_pReplaceAddress != NULL )
		{
			for( int i = iAlreadyFilled; --i >= 0; )
			{
				if( pReturnAddressesOut[i] == pTop->m_pReplaceAddress )
				{
					iAlreadyFilled = i;
					break;
				}
			}
		}

		if( pTop->m_iParentStackTraceLength != 0 )
		{
			int iCopy = MIN( iArrayCount - iAlreadyFilled, pTop->m_iParentStackTraceLength );
			memcpy( pReturnAddressesOut + iAlreadyFilled, pTop->m_pParentStackTrace, iCopy * sizeof( void * ) );
			iAlreadyFilled += iCopy;
		}
	}

	return iAlreadyFilled;
}

inline bool ValidStackAddress( void *pAddress, const void *pNoLessThan, const void *pNoGreaterThan )
{
	if( (uintp)pAddress & 3 )
		return false;
	if( pAddress < pNoLessThan ) //frame pointer traversal should always increase the pointer
		return false;
	if( pAddress > pNoGreaterThan ) //never traverse outside the stack (Oh 0xCCCCCCCC, how I hate you)
		return false;


	return true;
}

#pragma auto_inline( off )
int GetCallStack_Fast( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	//Only tested in windows. This function won't work with frame pointer omission enabled. "vpc /nofpo" all projects
#if (defined( TIER0_FPO_DISABLED ) || defined( _DEBUG )) &&\
	(defined( WIN32 ) && !defined( _X360 ) && defined(_M_IX86))
	void *pStackCrawlEBP;
	__asm
	{
		mov [pStackCrawlEBP], ebp;
	}

	/*
	With frame pointer omission disabled, this should be the pattern all the way up the stack
	[ebp+00]   Old ebp value
	[ebp+04]   Return address
	*/

	void *pNoLessThan = pStackCrawlEBP; //impossible for a valid stack to traverse before this address
	int i;

	CStackTop_FriendFuncs *pTop = (CStackTop_FriendFuncs *)(CStackTop_Base *)g_StackTop;
	if( pTop != NULL ) //we can do fewer error checks if we have a valid reference point for the top of the stack
	{		
		void *pNoGreaterThan = pTop->m_pStackBase;

		//skips
		for( i = 0; i != iSkipCount; ++i )
		{
			if( (pStackCrawlEBP < pNoLessThan) || (pStackCrawlEBP > pNoGreaterThan) )
				return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, 0 );

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		//store
		for( i = 0; i != iArrayCount; ++i )
		{
			if( (pStackCrawlEBP < pNoLessThan) || (pStackCrawlEBP > pNoGreaterThan) )
				break;

			pReturnAddressesOut[i] = *((void **)pStackCrawlEBP + 1);

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, i );
	}
	else
	{
		void *pNoGreaterThan = ((unsigned char *)pNoLessThan) + (1024 * 1024); //standard stack is 1MB. TODO: Get actual stack end address if available since this check isn't foolproof	

		//skips
		for( i = 0; i != iSkipCount; ++i )
		{
			if( !ValidStackAddress( pStackCrawlEBP, pNoLessThan, pNoGreaterThan ) )
				return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, 0 );

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		//store
		for( i = 0; i != iArrayCount; ++i )
		{
			if( !ValidStackAddress( pStackCrawlEBP, pNoLessThan, pNoGreaterThan ) )
				break;

			pReturnAddressesOut[i] = *((void **)pStackCrawlEBP + 1);

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, i );
	}
#endif

	return 0;
}
#pragma auto_inline( on )







//===============================================================================================================
// X360 version of the toolset
//===============================================================================================================

class C360StackTranslationHelper
{
public:
	C360StackTranslationHelper( void )
	{
		m_bInitialized = true;
	}

	~C360StackTranslationHelper( void )
	{
		StringSet_t::const_iterator iter;
		
		//module names
		{
			iter = m_ModuleNameSet.begin();
			while(iter != m_ModuleNameSet.end())
			{
				char *pModuleName = (char*)(*iter);
				delete []pModuleName;
				iter++;
			}
			m_ModuleNameSet.clear();
		}

		//file names
		{
			iter = m_FileNameSet.begin();
			while(iter != m_FileNameSet.end())
			{
				char *pFileName = (char*)(*iter);
				delete []pFileName;
				iter++;
			}
			m_FileNameSet.clear();
		}

		//symbol names
		{
			iter = m_SymbolNameSet.begin();
			while(iter != m_SymbolNameSet.end())
			{
				char *pSymbolName = (char*)(*iter);
				delete []pSymbolName;
				iter++;
			}
			m_SymbolNameSet.clear();
		}		

		m_bInitialized = false;
	}

private:
	struct StackAddressInfo_t;
public:

	inline StackAddressInfo_t *CreateEntry( const FullStackInfo_t &info )
	{
		std::pair<AddressInfoMapIter_t, bool> retval = m_AddressInfoMap.insert( AddressInfoMapEntry_t( info.pAddress, StackAddressInfo_t() ) );
		if( retval.first->second.szModule != NULL )
			return &retval.first->second; //already initialized

		retval.first->second.iLine = info.iLine;
		

		//share strings

		//module
		{
			const char *pModuleName;
			StringSet_t::const_iterator iter = m_ModuleNameSet.find( info.szModuleName );
			if ( iter == m_ModuleNameSet.end() )
			{
				int nLen = strlen(info.szModuleName) + 1;
				pModuleName = new char [nLen];
				memcpy( (char *)pModuleName, info.szModuleName, nLen );
				m_ModuleNameSet.insert( pModuleName );
			}
			else
			{
				pModuleName = (char *)(*iter);
			}

			retval.first->second.szModule = pModuleName;
		}

		//file
		{
			const char *pFileName;
			StringSet_t::const_iterator iter = m_FileNameSet.find( info.szFileName );
			if ( iter == m_FileNameSet.end() )
			{
				int nLen = strlen(info.szFileName) + 1;
				pFileName = new char [nLen];
				memcpy( (char *)pFileName, info.szFileName, nLen );
				m_FileNameSet.insert( pFileName );
			}
			else
			{
				pFileName = (char *)(*iter);
			}

			retval.first->second.szFileName = pFileName;
		}

		//symbol
		{
			const char *pSymbolName;
			StringSet_t::const_iterator iter = m_SymbolNameSet.find( info.szSymbol );
			if ( iter == m_SymbolNameSet.end() )
			{
				int nLen = strlen(info.szSymbol) + 1;
				pSymbolName = new char [nLen];
				memcpy( (char *)pSymbolName, info.szSymbol, nLen );
				m_SymbolNameSet.insert( pSymbolName );
			}
			else
			{
				pSymbolName = (char *)(*iter);
			}

			retval.first->second.szSymbol = pSymbolName;
		}

		return &retval.first->second;
	}

	inline StackAddressInfo_t *FindInfoEntry( const void *pAddress )
	{
		AddressInfoMapIter_t Iter = m_AddressInfoMap.find( pAddress );
		if( Iter != m_AddressInfoMap.end() )
			return &Iter->second;

		return NULL;
	}

	inline int RetrieveStackInfo(  const void * const *pAddresses, FullStackInfo_t *pReturnedStructs, int iAddressCount )
	{
		int ReturnedTranslatedCount = -1;
		//construct the message
		//				Header	Finished Count(out)		Input Count					Input Array			Returned data write address
		int iMessageSize = 2 + sizeof( int * ) + sizeof( uint32 ) + (sizeof( void * ) * iAddressCount) + sizeof( FullStackInfo_t * );
		uint8 *pMessage = (uint8 *)stackalloc( iMessageSize );
		uint8 *pMessageWrite = pMessage;
		pMessageWrite[0] = XBX_DBG_BNH_STACKTRANSLATOR; //have this message handled by stack translator handler
		pMessageWrite[1] = ST_BHC_GETTRANSLATIONINFO;
		pMessageWrite += 2;
		*(int **)pMessageWrite = (int *)BigDWord( (DWORD)&ReturnedTranslatedCount );
		pMessageWrite += sizeof( int * );
		*(uint32 *)pMessageWrite = (uint32)BigDWord( (DWORD)iAddressCount );
		pMessageWrite += sizeof( uint32 );
		memcpy( pMessageWrite, pAddresses, iAddressCount * sizeof( void * ) );
		pMessageWrite += iAddressCount * sizeof( void * );
		*(FullStackInfo_t **)pMessageWrite = (FullStackInfo_t *)BigDWord( (DWORD)pReturnedStructs );
		bool bSuccess = XBX_SendBinaryData( pMessage, iMessageSize, false, 30000 );
		ReturnedTranslatedCount = BigDWord( ReturnedTranslatedCount );

		if( bSuccess && (ReturnedTranslatedCount > 0) )
		{
			return ReturnedTranslatedCount;
		}
		return 0;
	}

	inline StackAddressInfo_t *CreateEntry( const void *pAddress )
	{
		//ask VXConsole for information about the addresses we're clueless about

		FullStackInfo_t ReturnedData;
		ReturnedData.pAddress = pAddress;
		ReturnedData.szFileName[0] = '\0'; //strncpy( ReturnedData.szFileName, "FileUninitialized", sizeof( ReturnedData.szFileName ) );
		ReturnedData.szModuleName[0] = '\0'; //strncpy( ReturnedData.szModuleName, "ModuleUninitialized", sizeof( ReturnedData.szModuleName ) );
		ReturnedData.szSymbol[0] = '\0'; //strncpy( ReturnedData.szSymbol, "SymbolUninitialized", sizeof( ReturnedData.szSymbol ) );
		ReturnedData.iLine = 0;
		ReturnedData.iSymbolOffset = 0;

		int iTranslated = RetrieveStackInfo( &pAddress, &ReturnedData, 1 );

		if( iTranslated == 1 )
		{
			//store
			return CreateEntry( ReturnedData );
		}

		return FindInfoEntry( pAddress ); //probably won't work, but last ditch.
	}

	inline StackAddressInfo_t *FindOrCreateEntry( const void *pAddress )
	{
		StackAddressInfo_t *pReturn = FindInfoEntry( pAddress );
		if( pReturn == NULL )
		{
			pReturn = CreateEntry( pAddress );
		}

		return pReturn;
	}

	inline void LoadStackInformation( void * const *pAddresses, int iAddressCount )
	{
		Assert( (iAddressCount > 0) && (pAddresses != NULL) );
		
		int iNeedLoading = 0;
		void **pNeedLoading = (void **)stackalloc( sizeof( const void * ) * iAddressCount ); //addresses we need to ask VXConsole about		
		
		for( int i = 0; i != iAddressCount; ++i )
		{
			if( FindInfoEntry( pAddresses[i] ) == NULL )
			{
				//need to load this address
				pNeedLoading[iNeedLoading] = pAddresses[i];
				++iNeedLoading;
			}
		}

		if( iNeedLoading != 0 )
		{
			//ask VXConsole for information about the addresses we're clueless about			
			FullStackInfo_t *pReturnedStructs = (FullStackInfo_t *)stackalloc( sizeof( FullStackInfo_t ) * iNeedLoading );
			
			for( int i = 0; i < iNeedLoading; ++i )
			{				
				pReturnedStructs[i].pAddress = 0;
				pReturnedStructs[i].szFileName[0] = '\0'; //strncpy( pReturnedStructs[i].szFileName, "FileUninitialized", sizeof( pReturnedStructs[i].szFileName ) );
				pReturnedStructs[i].szModuleName[0] = '\0'; //strncpy( pReturnedStructs[i].szModuleName, "ModuleUninitialized", sizeof( pReturnedStructs[i].szModuleName ) );
				pReturnedStructs[i].szSymbol[0] = '\0'; //strncpy( pReturnedStructs[i].szSymbol, "SymbolUninitialized", sizeof( pReturnedStructs[i].szSymbol ) );
				pReturnedStructs[i].iLine = 0;
				pReturnedStructs[i].iSymbolOffset = 0;
			}

			int iTranslated = RetrieveStackInfo( pNeedLoading, pReturnedStructs, iNeedLoading );

			if( iTranslated == iNeedLoading )
			{				
				//store
				for( int i = 0; i < iTranslated; ++i )
				{
					CreateEntry( pReturnedStructs[i] );
				}
			}
		}
	}

	inline bool GetFileAndLineFromAddress( const void *pAddress, tchar *pFileNameOut, int iMaxFileNameLength, uint32 &iLineNumberOut, uint32 *pDisplacementOut )
	{
		StackAddressInfo_t *pInfo = FindOrCreateEntry( pAddress );
		if( pInfo && (pInfo->szFileName[0] != '\0') )
		{
			strncpy( pFileNameOut, pInfo->szFileName, iMaxFileNameLength );
			iLineNumberOut = pInfo->iLine;

			if( pDisplacementOut )
				*pDisplacementOut = 0; //can't get line displacement on 360

			return true;
		}

		return false;
	}

	inline bool GetSymbolNameFromAddress( const void *pAddress, tchar *pSymbolNameOut, int iMaxSymbolNameLength, uint64 *pDisplacementOut )
	{
		StackAddressInfo_t *pInfo = FindOrCreateEntry( pAddress );
		if( pInfo && (pInfo->szSymbol[0] != '\0') )
		{
			strncpy( pSymbolNameOut, pInfo->szSymbol, iMaxSymbolNameLength );

			if( pDisplacementOut )
				*pDisplacementOut = pInfo->iSymbolOffset;

			return true;
		}

		return false;
	}

	inline bool GetModuleNameFromAddress( const void *pAddress, tchar *pModuleNameOut, int iMaxModuleNameLength )
	{
		StackAddressInfo_t *pInfo = FindOrCreateEntry( pAddress );
		if( pInfo && (pInfo->szModule[0] != '\0') )
		{
			strncpy( pModuleNameOut, pInfo->szModule, iMaxModuleNameLength );
			return true;
		}

		return false;
	}


	CThreadFastMutex m_hMutex;

private:

#pragma pack(push)
#pragma pack(1)
	struct StackAddressInfo_t
	{
		StackAddressInfo_t( void ) : szModule(NULL), szFileName(NULL), szSymbol(NULL), iLine(0), iSymbolOffset(0) {}
		const char *szModule;
		const char *szFileName;
		const char *szSymbol;
		uint32 iLine;
		uint32 iSymbolOffset;
	};
#pragma pack(pop)

	typedef std::map< const void *, StackAddressInfo_t, std::less<const void *>> AddressInfoMap_t;
	typedef AddressInfoMap_t::iterator AddressInfoMapIter_t;
	typedef AddressInfoMap_t::value_type AddressInfoMapEntry_t;

	class CStringLess
	{
	public:
		bool operator()(const char *pszLeft, const char *pszRight ) const 
		{
			return ( stricmp( pszLeft, pszRight ) < 0 );
		}
	};

	typedef std::set<const char *, CStringLess> StringSet_t;

	AddressInfoMap_t	m_AddressInfoMap; //TODO: retire old entries?
	StringSet_t			m_ModuleNameSet;
	StringSet_t			m_FileNameSet;
	StringSet_t			m_SymbolNameSet;
	bool				m_bInitialized;

};
static C360StackTranslationHelper s_360StackTranslator;


int GetCallStack( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	++iSkipCount; //skip this function

	//DmCaptureStackBackTrace() has no skip functionality, so we need to grab everything and skip within that list
	void **pAllResults = (void **)stackalloc( sizeof( void * ) * (iArrayCount + iSkipCount) );
	if( DmCaptureStackBackTrace( iArrayCount + iSkipCount, pAllResults ) == XBDM_NOERR )
	{
		for( int i = 0; i != iSkipCount; ++i )
		{
			if( *pAllResults == NULL ) //DmCaptureStackBackTrace() NULL terminates the list instead of telling us how many were returned
				return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, 0 );

			++pAllResults; //move the pointer forward so the second loop indices match up
		}

		for( int i = 0; i != iArrayCount; ++i )
		{
			if( pAllResults[i] == NULL ) //DmCaptureStackBackTrace() NULL terminates the list instead of telling us how many were returned
				return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, i );

			pReturnAddressesOut[i] = pAllResults[i];
		}

		return iArrayCount; //no room to append parent
	}

	return GetCallStack_Fast( pReturnAddressesOut, iArrayCount, iSkipCount );
}


void SetStackTranslationSymbolSearchPath( const char *szSemicolonSeparatedList )
{
	//nop on 360
}


void StackToolsNotify_LoadedLibrary( const char *szLibName )
{
	//send off the notice to VXConsole
	uint8 message[2];
	message[0] = XBX_DBG_BNH_STACKTRANSLATOR; //have this message handled by stack translator handler
	message[1] = ST_BHC_LOADEDLIBARY; //loaded a library notification
	XBX_SendBinaryData( message, 2 );
}


int TranslateStackInfo( const void * const *pCallStack, int iCallStackCount, tchar *szOutput, int iOutBufferSize, const tchar *szEntrySeparator, TranslateStackInfo_StyleFlags_t style )
{
	if( iCallStackCount == 0 )
	{
		if( iOutBufferSize > 1 )
			*szOutput = '\0';

		return 0;
	}

	if( szEntrySeparator == NULL )
		szEntrySeparator = "";

	int iSeparatorLength = strlen( szEntrySeparator ) + 1;
	int iDataSize = (sizeof( void * ) * iCallStackCount) + (iSeparatorLength * sizeof( tchar )) + 1; //1 for style flags

	//360 is incapable of translation on it's own. Encode the stack for translation in VXConsole
	//Encoded section is as such ":CSDECODE[encoded binary]"
	int iEncodedSize = -EncodeBinaryToString( NULL, iDataSize, NULL, 0 ); //get needed buffer size
	static const tchar cControlPrefix[] = XBX_CALLSTACKDECODEPREFIX;
	const size_t cControlLength = (sizeof( cControlPrefix )/sizeof(tchar)) - 1; //-1 to remove null terminator

	if( iOutBufferSize > (iEncodedSize + (int)cControlLength + 2) ) //+2 for ']' and null term
	{
		COMPILE_TIME_ASSERT( TSISTYLEFLAG_LAST < (1<<8) ); //need to update the encoder/decoder to use more than a byte for style flags

		uint8 *pData = (uint8 *)stackalloc( iDataSize );		
		pData[0] = (uint8)style;
		memcpy( pData + 1, szEntrySeparator, iSeparatorLength * sizeof( tchar ) );
		memcpy( pData + 1 + (iSeparatorLength * sizeof( tchar )), pCallStack, iCallStackCount * sizeof( void * ) );

		memcpy( szOutput, XBX_CALLSTACKDECODEPREFIX, cControlLength * sizeof( tchar ) );
		int iLength = cControlLength + EncodeBinaryToString( pData, iDataSize, &szOutput[cControlLength], (iOutBufferSize - cControlLength) );

		szOutput[iLength] = ']';
		szOutput[iLength + 1] = '\0';
	}

	return iCallStackCount;
}

void PreloadStackInformation( void * const *pAddresses, int iAddressCount )
{
	AUTO_LOCK( s_360StackTranslator.m_hMutex );
	s_360StackTranslator.LoadStackInformation( pAddresses, iAddressCount );
}

bool GetFileAndLineFromAddress( const void *pAddress, tchar *pFileNameOut, int iMaxFileNameLength, uint32 &iLineNumberOut, uint32 *pDisplacementOut )
{
	AUTO_LOCK( s_360StackTranslator.m_hMutex );
	return s_360StackTranslator.GetFileAndLineFromAddress( pAddress, pFileNameOut, iMaxFileNameLength, iLineNumberOut, pDisplacementOut );
}

bool GetSymbolNameFromAddress( const void *pAddress, tchar *pSymbolNameOut, int iMaxSymbolNameLength, uint64 *pDisplacementOut )
{
	AUTO_LOCK( s_360StackTranslator.m_hMutex );
	return s_360StackTranslator.GetSymbolNameFromAddress( pAddress, pSymbolNameOut, iMaxSymbolNameLength, pDisplacementOut );
}

bool GetModuleNameFromAddress( const void *pAddress, tchar *pModuleNameOut, int iMaxModuleNameLength )
{
	AUTO_LOCK( s_360StackTranslator.m_hMutex );
	return s_360StackTranslator.GetModuleNameFromAddress( pAddress, pModuleNameOut, iMaxModuleNameLength );
}


#endif //#if !defined( ENABLE_RUNTIME_STACK_TRANSLATION )




CCallStackStorage::CCallStackStorage( FN_GetCallStack GetStackFunction, uint32 iSkipCalls )
{
	iValidEntries = GetStackFunction( pStack, ARRAYSIZE( pStack ), iSkipCalls + 1 );
}



CStackTop_CopyParentStack::CStackTop_CopyParentStack( void * const *pParentStackTrace, int iParentStackTraceLength )
{
#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
	//miniature version of GetCallStack_Fast()
#if (defined( TIER0_FPO_DISABLED ) || defined( _DEBUG )) &&\
	(defined( WIN32 ) && !defined( _X360 ) && defined(_M_IX86))
	void *pStackCrawlEBP;
	__asm
	{
		mov [pStackCrawlEBP], ebp;
	}
	pStackCrawlEBP = *(void **)pStackCrawlEBP;
	m_pReplaceAddress = *((void **)pStackCrawlEBP + 1);
	m_pStackBase = (void *)((void **)pStackCrawlEBP + 1);
#else
	m_pReplaceAddress = NULL;
	m_pStackBase = this;
#endif

	m_pParentStackTrace = NULL;

	if( (pParentStackTrace != NULL) && (iParentStackTraceLength > 0) )
	{
		while( (iParentStackTraceLength > 0) && (pParentStackTrace[iParentStackTraceLength - 1] == NULL) )
		{
			--iParentStackTraceLength;
		}

		if( iParentStackTraceLength > 0 )
		{
			m_pParentStackTrace = new void * [iParentStackTraceLength];
			memcpy( (void **)m_pParentStackTrace, pParentStackTrace, sizeof( void * ) * iParentStackTraceLength );
		}
	}	

	m_iParentStackTraceLength = iParentStackTraceLength;

	m_pPrevTop = g_StackTop;
	g_StackTop = this;
	Assert( (CStackTop_Base *)g_StackTop == this );
#endif //#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
}

CStackTop_CopyParentStack::~CStackTop_CopyParentStack( void )
{
#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
	Assert( (CStackTop_Base *)g_StackTop == this );
	g_StackTop = m_pPrevTop;

	if( m_pParentStackTrace != NULL )
	{
		delete []m_pParentStackTrace;
	}
#endif
}



CStackTop_ReferenceParentStack::CStackTop_ReferenceParentStack( void * const *pParentStackTrace, int iParentStackTraceLength )
{
#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
	//miniature version of GetCallStack_Fast()
#if (defined( TIER0_FPO_DISABLED ) || defined( _DEBUG )) &&\
	(defined( WIN32 ) && !defined( _X360 ) && defined(_M_IX86))
	void *pStackCrawlEBP;
	__asm
	{
		mov [pStackCrawlEBP], ebp;
	}
	pStackCrawlEBP = *(void **)pStackCrawlEBP;
	m_pReplaceAddress = *((void **)pStackCrawlEBP + 1);
	m_pStackBase = (void *)((void **)pStackCrawlEBP + 1);
#else
	m_pReplaceAddress = NULL;
	m_pStackBase = this;
#endif

	m_pParentStackTrace = pParentStackTrace;

	if( (pParentStackTrace != NULL) && (iParentStackTraceLength > 0) )
	{
		while( (iParentStackTraceLength > 0) && (pParentStackTrace[iParentStackTraceLength - 1] == NULL) )
		{
			--iParentStackTraceLength;
		}
	}	

	m_iParentStackTraceLength = iParentStackTraceLength;

	m_pPrevTop = g_StackTop;
	g_StackTop = this;
	Assert( (CStackTop_Base *)g_StackTop == this );
#endif //#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
}

CStackTop_ReferenceParentStack::~CStackTop_ReferenceParentStack( void )
{
#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
	Assert( (CStackTop_Base *)g_StackTop == this );
	g_StackTop = m_pPrevTop;

	ReleaseParentStackReferences();
#endif
}

void CStackTop_ReferenceParentStack::ReleaseParentStackReferences( void )
{
#if defined( ENABLE_RUNTIME_STACK_TRANSLATION )
	m_pParentStackTrace = NULL;
	m_iParentStackTraceLength = 0;
#endif
}




//Encodes data so that every byte's most significant bit is a 1. Ensuring no null terminators.
//This puts the encoded data in the 128-255 value range. Leaving all standard ascii characters for control.
//Returns string length (not including the written null terminator as is standard). 
//Or if the buffer is too small. Returns negative of necessary buffer size (including room needed for null terminator)
int EncodeBinaryToString( const void *pToEncode, int iDataLength, char *pEncodeOut, int iEncodeBufferSize )
{
	int iEncodedSize = iDataLength;
	iEncodedSize += (iEncodedSize + 6) / 7; //Have 1 control byte for every 7 actual bytes
	iEncodedSize += sizeof( uint32 ) + 1; //data size at the beginning of the blob and null terminator at the end

	if( (iEncodedSize > iEncodeBufferSize) || (pEncodeOut == NULL) || (pToEncode == NULL) )
		return -iEncodedSize; //not enough room

	uint8 *pEncodeWrite = (uint8 *)pEncodeOut;	

	//first encode the data size. Encodes lowest 28 bits and discards the high 4
	pEncodeWrite[0] = ((iDataLength >> 21) & 0xFF) | 0x80;
	pEncodeWrite[1] = ((iDataLength >> 14) & 0xFF) | 0x80;
	pEncodeWrite[2] = ((iDataLength >> 7) & 0xFF) | 0x80;
	pEncodeWrite[3] = ((iDataLength >> 0) & 0xFF) | 0x80;
	pEncodeWrite += 4;

	const uint8 *pEncodeRead = (const uint8 *)pToEncode;
	const uint8 *pEncodeStop = pEncodeRead + iDataLength;
	uint8 *pEncodeWriteLastControlByte = pEncodeWrite;
	int iControl = 0;

	//Encode the data
	while( pEncodeRead < pEncodeStop )
	{
		if( iControl == 0 )
		{
			pEncodeWriteLastControlByte = pEncodeWrite;
			*pEncodeWriteLastControlByte = 0x80;
		}
		else
		{
			*pEncodeWrite = *pEncodeRead | 0x80; //encoded data always has the MSB bit set (cheap avoidance of null terminators)
			*pEncodeWriteLastControlByte |= (((*pEncodeRead) & 0x80) ^ 0x80) >> iControl; //We use the control byte to XOR the MSB back to original values on decode
			++pEncodeRead;
		}

		++pEncodeWrite;
		++iControl;
		iControl &= 7; //8->0
	}
	*pEncodeWrite = '\0';

	return iEncodedSize - 1;
}

//Decodes a string produced by EncodeBinaryToString(). Safe to decode in place if you don't mind trashing your string, binary byte count always less than string byte count.
//Returns:
//	>= 0 is the decoded data size
//	INT_MIN (most negative value possible) indicates an improperly formatted string (not our data)
//	all other negative values are the negative of how much dest buffer size is necessary.
int DecodeBinaryFromString( const char *pString, void *pDestBuffer, int iDestBufferSize, char **ppParseFinishOut )
{
	const uint8 *pDecodeRead = (const uint8 *)pString;

	if( (pDecodeRead[0] < 0x80) || (pDecodeRead[1] < 0x80) || (pDecodeRead[2] < 0x80) || (pDecodeRead[3] < 0x80) )
	{
		if( ppParseFinishOut != NULL )
			*ppParseFinishOut = (char *)pString;

		return INT_MIN; //Don't know what the string is, but it's not our format
	}

	int iDecodedSize = 0;
	iDecodedSize |= (pDecodeRead[0] & 0x7F) << 21;
	iDecodedSize |= (pDecodeRead[1] & 0x7F) << 14;
	iDecodedSize |= (pDecodeRead[2] & 0x7F) << 7;
	iDecodedSize |= (pDecodeRead[3] & 0x7F) << 0;
	pDecodeRead += 4;

	int iTextLength = iDecodedSize;
	iTextLength += (iTextLength + 6) / 7; //Have 1 control byte for every 7 actual bytes

	//make sure it's formatted properly
	for( int i = 0; i != iTextLength; ++i )
	{
		if( pDecodeRead[i] < 0x80 ) //encoded data always has MSB set
		{
			if( ppParseFinishOut != NULL )
				*ppParseFinishOut = (char *)pString;

			return INT_MIN; //either not our data, or part of the string is missing
		}
	}

	if( iDestBufferSize < iDecodedSize )
	{
		if( ppParseFinishOut != NULL )
			*ppParseFinishOut = (char *)pDecodeRead;

		return -iDecodedSize; //dest buffer not big enough to hold the data
	}

	const uint8 *pStopDecoding = pDecodeRead + iTextLength;		
	uint8 *pDecodeWrite = (uint8 *)pDestBuffer;
	int iControl = 0;
	int iLSBXOR = 0;

	while( pDecodeRead < pStopDecoding )
	{
		if( iControl == 0 )
		{
			iLSBXOR = *pDecodeRead;
		}
		else
		{
			*pDecodeWrite = *pDecodeRead ^ ((iLSBXOR << iControl) & 0x80);
			++pDecodeWrite;
		}

		++pDecodeRead;
		++iControl;
		iControl &= 7; //8->0
	}

	if( ppParseFinishOut != NULL )
		*ppParseFinishOut = (char *)pDecodeRead;

	return iDecodedSize;	
}


