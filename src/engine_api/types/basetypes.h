//========= Copyright  1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if defined(XASH_VGUI2) && defined(__cplusplus)
#include "base/platform/port.h"
#include "engine_api/types/xash3d_types.h"
#include "source_sdk/public/tier0/platform.h"
#else
#ifndef BASETYPES_H
#define BASETYPES_H

#include "base/platform/port.h"
#include "engine_api/types/xash3d_types.h"
#include "base/math/minmax.h"
#include "base/protected_things.h"


// For backward compatibilty only...
//#include "tier0/platform.h"

// stdio.h
#ifndef NULL
#define NULL 0
#endif

// Pad a number so it lies on an N byte boundary.
// So PAD_NUMBER(0,4) is 0 and PAD_NUMBER(1,4) is 4
#define PAD_NUMBER(number, boundary) \
	( ((number) + ((boundary)-1)) / (boundary) ) * (boundary)

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

typedef int BOOL;
typedef int qboolean;
typedef unsigned char BYTE;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int uint;
typedef unsigned int uint32; //!!!

typedef float vec_t;

#ifndef UNUSED
#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings
#endif

#ifdef __cplusplus

struct vrect_t
{
	int				x, y, width, height;
	vrect_t			*pnext;
};

#endif


#endif // BASETYPES_H
#endif // XASH_VGUI2
