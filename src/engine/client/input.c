/*
input.c - win32 input devices
Copyright (C) 2007 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "engine/runtime/common.h"
#include "engine/client/input.h"
#include "engine/client/client.h"
#include "engine/client/vgui/vgui_draw.h"
#include "engine_api/types/wrect.h"

#include <SDL.h>



Xash_Cursor*	in_mousecursor;
qboolean	in_mouseactive;				// false when not focus app
qboolean	in_mouseinitialized;
qboolean	in_mouse_suspended;
int	in_mouse_oldbuttonstate;
int	in_mouse_buttons;
static struct inputstate_s
{
	float lastpitch, lastyaw;
} inputstate;

extern convar_t *vid_fullscreen;

static byte scan_to_key[128] =
{
	0,27,'1','2','3','4','5','6','7','8','9','0','-','=',K_BACKSPACE,9,
	'q','w','e','r','t','y','u','i','o','p','[',']', 13 , K_CTRL,
	'a','s','d','f','g','h','j','k','l',';','\'','`',
	K_SHIFT,'\\','z','x','c','v','b','n','m',',','.','/',K_SHIFT,
	'*',K_ALT,' ',K_CAPSLOCK,
	K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,K_F8,K_F9,K_F10,
	K_PAUSE,0,K_HOME,K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,
	K_RIGHTARROW,K_KP_PLUS,K_END,K_DOWNARROW,K_PGDN,K_INS,K_DEL,
	0,0,0,K_F11,K_F12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

convar_t *m_enginemouse;
convar_t *m_pitch;
convar_t *m_yaw;

convar_t *m_enginesens;
convar_t *m_ignore;
convar_t *cl_forwardspeed;
convar_t *cl_sidespeed;
convar_t *cl_backspeed;
convar_t *look_filter;
/*
=======
Host_MapKey

Map from windows to engine keynums
=======
*/
static int Host_MapKey( int key )
{
	int	result, modified;
	qboolean	is_extended = false;

	modified = ( key >> 16 ) & 255;
	if( modified > 127 ) return 0;

	if( key & ( 1U << 24 ))
		is_extended = true;

	result = scan_to_key[modified];

	if( !is_extended )
	{
		switch( result )
		{
		case K_HOME: return K_KP_HOME;
		case K_UPARROW: return K_KP_UPARROW;
		case K_PGUP: return K_KP_PGUP;
		case K_LEFTARROW: return K_KP_LEFTARROW;
		case K_RIGHTARROW: return K_KP_RIGHTARROW;
		case K_END: return K_KP_END;
		case K_DOWNARROW: return K_KP_DOWNARROW;
		case K_PGDN: return K_KP_PGDN;
		case K_INS: return K_KP_INS;
		case K_DEL: return K_KP_DEL;
		default: return result;
		}
	}
	else
	{
		switch( result )
		{
		case K_PAUSE: return K_KP_NUMLOCK;
		case 0x0D: return K_KP_ENTER;
		case 0x2F: return K_KP_SLASH;
		case 0xAF: return K_KP_PLUS;
		}
		return result;
	}
}

/*
===========
IN_StartupMouse
===========
*/


void IN_StartupMouse( void )
{
	{  }

	m_ignore = Cvar_Get( "m_ignore", DEFAULT_M_IGNORE, CVAR_ARCHIVE , "ignore mouse events" );

	m_enginemouse = Cvar_Get( "m_enginemouse", "0", CVAR_ARCHIVE, "read mouse events in engine instead of client" );
	m_enginesens = Cvar_Get( "m_enginesens", "0.3", CVAR_ARCHIVE, "mouse sensitivity, when m_enginemouse enabled" );
	m_pitch = Cvar_Get( "m_pitch", "0.022", CVAR_ARCHIVE, "mouse pitch value" );
	m_yaw = Cvar_Get( "m_yaw", "0.022", CVAR_ARCHIVE, "mouse yaw value" );
	look_filter = Cvar_Get( "look_filter", "0", CVAR_ARCHIVE, "filter look events making it smoother" );

	// You can use -nomouse argument to prevent using mouse from client
	// -noenginemouse will disable all mouse input
	if( Sys_CheckParm( "-noenginemouse" )) return;

	in_mouse_buttons = 8;
	in_mouseinitialized = true;
}

static void IN_ActivateCursor( void )
{
	if( cls.key_dest == key_menu )
	{
		SDL_SetCursor( in_mousecursor );
	}
}

void IN_SetCursor( Xash_Cursor* hCursor )
{
	in_mousecursor = hCursor;

	IN_ActivateCursor();
}

/*
===========
IN_ToggleClientMouse

Called when key_dest is changed
===========
*/
void IN_ToggleClientMouse( int newstate, int oldstate )
{
	if( newstate == oldstate ) return;

	if( oldstate == key_game )
	{
		if( cls.initialized )
			clgame.dllFuncs.IN_DeactivateMouse();
	}
	else if( newstate == key_game )
	{
		// reset mouse pos, so cancel effect in game
		{
			SDL_WarpMouseInWindow( host.hWnd, host.window_center_x, host.window_center_y );
			SDL_SetWindowGrab( host.hWnd, SDL_TRUE );
			if( clgame.dllFuncs.pfnMouseLook )
				SDL_SetRelativeMouseMode( SDL_TRUE );
		}
		if( cls.initialized )
			clgame.dllFuncs.IN_ActivateMouse();
	}

	if(((newstate == key_menu) || (newstate == key_console) || (newstate == key_message)) && (!(CL_IsBackgroundMap())))
	{
		SDL_SetWindowGrab(host.hWnd, SDL_FALSE);
		if( clgame.dllFuncs.pfnMouseLook )
			SDL_SetRelativeMouseMode( SDL_FALSE );
	}
	else
	{
	}
}

/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse( qboolean force )
{
	static qboolean	oldstate;

	if( !in_mouseinitialized )
		return;

	if( CL_Active() && host.mouse_visible && !force )
		return;	// VGUI controls

	if( cls.key_dest == key_menu && vid_fullscreen && !vid_fullscreen->integer)
	{
		// check for mouse leave-entering
		if( !in_mouse_suspended && !UI_MouseInRect( ))
			in_mouse_suspended = true;

		if( oldstate != in_mouse_suspended )
		{
			if( in_mouse_suspended )
			{
				SDL_ShowCursor( false );
				UI_ShowCursor( false );
			}
		}

		oldstate = in_mouse_suspended;

		if( in_mouse_suspended )
		{
			in_mouse_suspended = false;
			in_mouseactive = false; // re-initialize mouse
			UI_ShowCursor( true );
		}
	}

	if( in_mouseactive ) return;
	in_mouseactive = true;

	if( UI_IsVisible( )) return;

	if( cls.key_dest == key_game )
	{
		clgame.dllFuncs.IN_ActivateMouse();
		SDL_GetRelativeMouseState( 0, 0 ); // Reset mouse position
	}

}

/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse( void )
{
	if( !in_mouseinitialized || !in_mouseactive )
		return;

	if( cls.key_dest == key_game && cls.initialized )
	{
		clgame.dllFuncs.IN_DeactivateMouse();
	}
	in_mouseactive = false;
	SDL_SetWindowGrab( host.hWnd, SDL_FALSE );
}

/*
================
IN_Mouse
================
*/
void IN_MouseMove( void )
{
	POINT	current_pos = {0, 0};

	if( !in_mouseinitialized || !in_mouseactive || m_ignore->integer )
		return;

	// find mouse movement
	SDL_GetMouseState( &current_pos.x, &current_pos.y );
	if( host.hWnd )
	{
		int width, height;
		SDL_GetWindowSize( host.hWnd, &width, &height );
		// UI painting uses drawable pixels, SDL mouse positions use window points.
		if( width > 0 ) current_pos.x = (int)((float)current_pos.x * scr_width->value / width);
		if( height > 0 ) current_pos.y = (int)((float)current_pos.y * scr_height->value / height);
	}


	VGui_MouseMove( current_pos.x, current_pos.y );

	if( !UI_IsVisible() )
		return;

	// Show cursor in UI
	if( UI_IsVisible() )
		SDL_ShowCursor( SDL_TRUE );

	// if the menu is visible, move the menu cursor
	UI_MouseMove( current_pos.x, current_pos.y );

	IN_ActivateCursor();
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent( int mstate )
{
	int	i;

	if( !in_mouseinitialized || !in_mouseactive )
		return;

	if( m_ignore->integer )
		return;

	if( cls.key_dest == key_game )
	{
		static qboolean ignore; // igonre mouse warp event
		int x, y;
		SDL_GetMouseState(&x, &y);
		if( host.mouse_visible )
			SDL_ShowCursor( SDL_TRUE );
		else
			SDL_ShowCursor( SDL_FALSE );

		int division = 32; // 2
		if( x < host.window_center_x - host.window_center_x / division ||
			y < host.window_center_y - host.window_center_y / division ||
			x > host.window_center_x + host.window_center_x / division ||
			y > host.window_center_y + host.window_center_y / division)
		{
			SDL_WarpMouseInWindow(host.hWnd, host.window_center_x, host.window_center_y);
			ignore = 1; // next mouse event will be mouse warp
			return;
		}

		if ( !ignore )
		{
			if( !m_enginemouse->integer )
				clgame.dllFuncs.IN_MouseEvent( mstate );
		}
		else
		{
			SDL_GetRelativeMouseState( 0, 0 ); // reset relative state
			ignore = 0;
		}
		return;
	}
	else
	{
		SDL_SetRelativeMouseMode( SDL_FALSE );
		SDL_ShowCursor( SDL_TRUE );
		IN_MouseMove();
	}

	// perform button actions
	for( i = 0; i < in_mouse_buttons; i++ )
	{
		if(( mstate & ( 1U << i )) && !( in_mouse_oldbuttonstate & ( 1U << i )))
		{
			Key_Event( K_MOUSE1 + i, true );
		}

		if(!( mstate & ( 1U << i )) && ( in_mouse_oldbuttonstate & ( 1U << i )))
		{
			Key_Event( K_MOUSE1 + i, false );
		}
	}

	in_mouse_oldbuttonstate = mstate;
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown( void )
{
	IN_DeactivateMouse( );

}


/*
===========
IN_Init
===========
*/
void IN_Init( void )
{
	IN_StartupMouse( );

	cl_forwardspeed	= Cvar_Get( "cl_forwardspeed", "400", CVAR_ARCHIVE | CVAR_CLIENTDLL, "Default forward move speed" );
	cl_backspeed	= Cvar_Get( "cl_backspeed", "400", CVAR_ARCHIVE | CVAR_CLIENTDLL, "Default back move speed"  );
	cl_sidespeed	= Cvar_Get( "cl_sidespeed", "400", CVAR_ARCHIVE | CVAR_CLIENTDLL, "Default side move speed"  );


}

/*
================
IN_JoyMove

Common function for engine joystick movement

	-1 < forwardmove < 1,	-1 < sidemove < 1

================
*/

#define F (1U << 0)	// Forward
#define B (1U << 1)	// Back
#define L (1U << 2)	// Left
#define R (1U << 3)	// Right
#define T (1U << 4)	// Forward stop
#define S (1U << 5)	// Side stop


/*
================
IN_EngineAppendMove

Called from cl_main.c after generating command in client
================
*/
void IN_EngineAppendMove( float frametime, usercmd_t *cmd, qboolean active )
{
	if (clgame.dllFuncs.pfnMouseLook)
		return;
	float forward, side, dpitch, dyaw;


	if( cls.key_dest != key_game || cl.refdef.paused || cl.refdef.intermission )
		return;

	forward = side = dpitch = dyaw = 0;

	if(active)
	{
		float sensitivity = ( (float)cl.refdef.fov_x / (float)90.0f );
#if XASH_INPUT == INPUT_SDL
		if( m_enginemouse->integer && !m_ignore->integer )
		{
			int mouse_x, mouse_y;
			SDL_GetRelativeMouseState( &mouse_x, &mouse_y );
			cl.refdef.cl_viewangles[PITCH] += mouse_y * m_pitch->value * sensitivity;
			cl.refdef.cl_viewangles[YAW] -= mouse_x * m_yaw->value * sensitivity;
		}
#endif
		if( look_filter->integer )
		{
			dpitch = ( inputstate.lastpitch + dpitch ) / 2;
			dyaw = ( inputstate.lastyaw + dyaw ) / 2;
			inputstate.lastpitch = dpitch;
			inputstate.lastyaw = dyaw;
		}

		cl.refdef.cl_viewangles[YAW] += dyaw * sensitivity;
		cl.refdef.cl_viewangles[PITCH] += dpitch * sensitivity;
		cl.refdef.cl_viewangles[PITCH] = bound( -90, cl.refdef.cl_viewangles[PITCH], 90 );
	}
}
/*
==================
Host_InputFrame

Called every frame, even if not generating commands
==================
*/
void Host_InputFrame( void )
{
	qboolean	shutdownMouse = false;
	float forward = 0, side = 0, pitch = 0, yaw = 0;

	if( clgame.dllFuncs.pfnMouseLook )
	{
		int dx, dy;

#if XASH_INPUT == INPUT_SDL
		if( in_mouseinitialized && !m_ignore->integer )
		{
			SDL_GetRelativeMouseState( &dx, &dy );
			pitch += dy * m_pitch->value, yaw -= dx * m_yaw->value; //mouse speed
		}
#endif


		if( look_filter->integer )
		{
			pitch = ( inputstate.lastpitch + pitch ) / 2;
			yaw = ( inputstate.lastyaw + yaw ) / 2;
			inputstate.lastpitch = pitch;
			inputstate.lastyaw = yaw;
		}

		if( cls.key_dest == key_game )
		{
			clgame.dllFuncs.pfnMouseLook( yaw, pitch );
		}
	}
	Cbuf_Execute ();

	if( host.state == HOST_RESTART )
		host.state = HOST_FRAME; // restart is finished

	if( !in_mouseinitialized )
		return;

	if( host.state != HOST_FRAME )
	{
		IN_DeactivateMouse();
		return;
	}

	if( cl.refdef.paused && cls.key_dest == key_game )
		shutdownMouse = true; // release mouse during pause or console typeing

	if( shutdownMouse && !vid_fullscreen->integer )
	{
		IN_DeactivateMouse();
		return;
	}

	IN_ActivateMouse( false );

	IN_MouseMove();
}
