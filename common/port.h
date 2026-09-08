/*
port.h -- Portability Layer for Windows types
Copyright (C) 2015 Alibek Omarov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once
#ifndef PORT_H
#define PORT_H

#if defined(__LP64__) || defined(__LLP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
  #define XASH_64BIT
#endif

#ifdef XASH_64BIT
#define ARCH_SUFFIX "64"
#else
#define ARCH_SUFFIX
#endif

#if defined(__ANDROID__) || TARGET_OS_IOS || defined(__SAILFISH__)
#define XASH_MOBILE_PLATFORM
#endif

#define PATH_SPLITTER "/"

	#include <limits.h>
	#include <dlfcn.h>
	#include <stdlib.h>
	#include <unistd.h>

		#include <sys/syslimits.h>
		#define OS_LIB_EXT "dylib"
        #define OPEN_COMMAND "open"
		#include "TargetConditionals.h"



	#if   defined(__HAIKU__)
		#define POSTFIX   "-haiku"
 		#define MENUDLL   "libmenu"                       "." OS_LIB_EXT
 		#define CLIENTDLL "libclient" POSTFIX ARCH_SUFFIX "." OS_LIB_EXT
 		#define SERVERDLL "libserver" POSTFIX ARCH_SUFFIX "." OS_LIB_EXT
 		#define PACKAGE   "/Xash3D"
	#else
		#define MENUDLL   "libxashmenu" ARCH_SUFFIX "." OS_LIB_EXT
		#define CLIENTDLL "client"      ARCH_SUFFIX "." OS_LIB_EXT
	#endif

	#define VGUI_SUPPORT_DLL "libvgui_support." OS_LIB_EXT

	// Windows-specific
#ifndef __HAIKU__
	#define __cdecl
#endif
	#define _inline	static inline
	#define O_BINARY 0 // O_BINARY is Windows extension
	#define O_TEXT 0 // O_TEXT is Windows extension

	// Windows functions to Linux equivalent
	#define _mkdir( x )					mkdir( x, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH )
	#define LoadLibrary( x )			dlopen( x, RTLD_NOW )
	#define GetProcAddress( x, y )		dlsym( x, y )
	#define SetCurrentDirectory( x )	(!chdir( x ))
	#define FreeLibrary( x )			dlclose( x )
	//#define MAKEWORD( a, b )			((short int)(((unsigned char)(a))|(((short int)((unsigned char)(b)))<<8)))
#ifndef __cplusplus
	#define max( a, b )                 (((a) > (b)) ? (a) : (b))
	#define min( a, b )                 (((a) < (b)) ? (a) : (b))
#endif
	#define tell( a )					lseek(a, 0, SEEK_CUR)

	typedef unsigned char	BYTE;
	typedef unsigned short WORD;
	typedef unsigned int    DWORD;
	typedef int	    LONG;
#if defined(XASH_VGUI2) && defined(__cplusplus)
	typedef unsigned long ULONG;
#else
	typedef unsigned int   ULONG;
#endif
	typedef int			WPARAM;
	typedef unsigned int    LPARAM;

	typedef void* HANDLE;
	typedef void* HMODULE;
	typedef void* HINSTANCE;

	typedef char* LPSTR;

	typedef struct tagPOINT
	{
		int x, y;
	} POINT;

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef USHRT_MAX
#define USHRT_MAX 65535
#endif

#endif // PORT_H
