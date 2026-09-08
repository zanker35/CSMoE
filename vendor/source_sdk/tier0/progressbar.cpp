//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "source_sdk/tier0/pch_tier0.h"

#include <assert.h>
#include "source_sdk/public/tier0/platform.h"
#include "source_sdk/public/tier0/progressbar.h"

#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
#include "source_sdk/public/tier0/memalloc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "source_sdk/public/tier0/memdbgon.h"
#endif


static ProgressReportHandler_t pReportHandlerFN;

PLATFORM_INTERFACE void ReportProgress(char const *job_name, int total_units_to_do, int n_units_completed)
{
	if ( pReportHandlerFN )
		(*pReportHandlerFN)( job_name, total_units_to_do, n_units_completed );
}

PLATFORM_INTERFACE ProgressReportHandler_t InstallProgressReportHandler( ProgressReportHandler_t pfn)
{
	ProgressReportHandler_t old = pReportHandlerFN;
	pReportHandlerFN = pfn;
	return old;
}

