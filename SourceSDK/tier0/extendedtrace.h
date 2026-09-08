//========= Copyright Valve Corporation, All rights reserved. ============//
//////////////////////////////////////////////////////////////////////////////////////
//
// Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
// The source is free, but if you want to use it, mention my name and e-mail address
//
// History:
//    1.0      Initial version                  Zoltan Csizmadia
//
//////////////////////////////////////////////////////////////////////////////////////
//
// ExtendedTrace.h
//

#ifndef EXTENDEDTRACE_H_INCLUDED
#define EXTENDEDTRACE_H_INCLUDED


#define EXTENDEDTRACEINITIALIZE( IniSymbolPath )   ((void)0)
#define EXTENDEDTRACEUNINITIALIZE()			         ((void)0)
#define TRACEF									            ((void)0)
#define SRCLINKTRACECUSTOM( Msg, File, Line)	      ((void)0)
#define SRCLINKTRACE( Msg )						      ((void)0)
#define FNPARAMTRACE()							         ((void)0)
#define STACKTRACEMSG( Msg )					         ((void)0)
#define STACKTRACE()						         	   ((void)0)
#define THREADSTACKTRACEMSG( hThread, Msg )		   ((void)0)
#define THREADSTACKTRACE( hThread )				      ((void)0)


#endif
