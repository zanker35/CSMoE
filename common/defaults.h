/*
defaults.h - set up default configuration
Copyright (C) 2016 Mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "backends.h"
#define XASH_VIDEO VIDEO_SDL
#define XASH_TIMER TIMER_SDL
#define XASH_INPUT INPUT_SDL
#define XASH_SOUND SOUND_SDL
#ifndef XASH_CRASHHANDLER
#ifdef CRASHHANDLER
#define XASH_CRASHHANDLER CRASHHANDLER_UCONTEXT
#else
#define XASH_CRASHHANDLER CRASHHANDLER_NULL
#endif
#endif
#define DEFAULT_M_IGNORE "0"
#define XASH_INTERNAL_GAMELIBS
#define DEFAULT_SV_FORCESIMULATING "0"
#ifndef DEFAULT_DEV
#define DEFAULT_DEV 0
#endif
#ifndef DEFAULT_FULLSCREEN
#define DEFAULT_FULLSCREEN 1
#endif
#define DEFAULT_CON_MAXFRAC "1"
#endif // DEFAULTS_H
