#include "hud.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input.h"
#include "in_defs.h"
#include "view.h"

using namespace cl;

cvar_t	*sensitivity;


float rel_yaw;
float rel_pitch;
bool bMouseInUse = false;

extern Vector dead_viewangles;

inline namespace input_xash3d {
constexpr auto F = 1U<<0;	// Forward
constexpr auto B = 1U<<1;	// Back
constexpr auto L = 1U<<2;	// Left
constexpr auto R = 1U<<3;	// Right
constexpr auto T = 1U<<4;	// Forward stop
constexpr auto S = 1U<<5;	// Side stop

#define BUTTON_DOWN		1
#define IMPULSE_DOWN	2
#define IMPULSE_UP		4
}

bool CL_IsDead();





void IN_MouseLook( float relyaw, float relpitch )
{
	rel_yaw += relyaw;
	rel_pitch += relpitch;
}

// Rotate camera and add move values to usercmd
void IN_Move( float frametime, usercmd_t *cmd )
{
	Vector viewangles;

	if( gHUD.m_iIntermission )
		return; // we can't move during intermission



	if( in_mlook.state & 1 )
	{
		V_StopPitchDrift();
	}

	if( CL_IsDead( ) )
	{
		viewangles = dead_viewangles; // HACKHACK: see below
	}
	else
	{
		gEngfuncs.GetViewAngles( viewangles );
	}

	if( gHUD.GetSensitivity() != 0 )
	{
		rel_yaw *= gHUD.GetSensitivity();
		rel_pitch *= gHUD.GetSensitivity();
	}
	else
	{
		rel_yaw *= sensitivity->value;
		rel_pitch *= sensitivity->value;
	}
	if(gHUD.m_MOTD.cl_hide_motd->value == 0.0f && gHUD.m_MOTD.m_bShow)
	{
		gHUD.m_MOTD.scroll += rel_pitch;
	}
	else
	{
		viewangles[PITCH] += rel_pitch;
		viewangles[YAW] += rel_yaw;

	}
	if (viewangles[PITCH] > cl_pitchdown->value)
		viewangles[PITCH] = cl_pitchdown->value;
	if (viewangles[PITCH] < -cl_pitchup->value)
		viewangles[PITCH] = -cl_pitchup->value;


	if( !CL_IsDead( ) )
	{
		gEngfuncs.SetViewAngles( viewangles );
	}

	dead_viewangles = viewangles;
	rel_pitch = rel_yaw = 0;

	
	
}

void DLLEXPORT IN_MouseEvent_CL( int mstate )
{
	static int mouse_oldbuttonstate;
	// perform button actions
	for( int i = 0; i < 5; i++ )
	{
		if(( mstate & (1 << i)) && !( mouse_oldbuttonstate & (1 << i)))
		{
			gEngfuncs.Key_Event( K_MOUSE1 + i, 1 );
		}

		if( !( mstate & (1 << i)) && ( mouse_oldbuttonstate & (1 << i)))
		{
			gEngfuncs.Key_Event( K_MOUSE1 + i, 0 );
		}
	}	
	
	mouse_oldbuttonstate = mstate;
	bMouseInUse = true;
}

// Stubs

void DLLEXPORT IN_ClearStates ( void )
{
	//gEngfuncs.Con_Printf("IN_ClearStates\n");
}

void DLLEXPORT  IN_ActivateMouse_CL ( void )
{
	//gEngfuncs.Con_Printf("IN_ActivateMouse\n");
}

void DLLEXPORT  IN_DeactivateMouse_CL ( void )
{
	//gEngfuncs.Con_Printf("IN_DeactivateMouse\n");
}

void DLLEXPORT IN_Accumulate ( void )
{
	//gEngfuncs.Con_Printf("IN_Accumulate\n");
}

void IN_Commands ( void )
{
	//gEngfuncs.Con_Printf("IN_Commands\n");
}

void IN_Shutdown ( void )
{
}
// Register cvars and reset data
void IN_Init( void )
{
	sensitivity = gEngfuncs.pfnRegisterVariable ( "sensitivity", "3", FCVAR_ARCHIVE );
	
	

}
