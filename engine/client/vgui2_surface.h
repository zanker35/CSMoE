/* C interface between the master engine and the CSO VGUI2 runtime. */
#ifndef VGUI2_SURFACE_H
#define VGUI2_SURFACE_H

#include "xash3d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cl_enginefuncs_s;
int VGui2_Initialize( struct cl_enginefuncs_s *engine_funcs );
void VGui2_Startup( void );
int VGui2_VidInit( void );
void VGui2_Paint( void );
int VGui2_Shutdown( void );
void VGui2_LoadingFinished( const char *map_name );

int VGUI2_Surface_GetCharWidth( int ch );
int VGUI2_Surface_GetCharHeight( void );
int VGUI2_Surface_DrawConsoleString( int x, int y, const char *text, byte r, byte g, byte b, byte a );
void VGUI2_Surface_DrawStringLen( const char *text, int *width, int *height );
int VGUI2_Surface_DrawChar( int x, int y, int ch, byte r, byte g, byte b, byte a );

void VGuiWrap2_HideConsole( void );
void VGuiWrap2_ToggleConsole( void );
int VGuiWrap2_IsConsoleVisible( void );
void VGuiWrap2_ClearConsole( void );
void VGuiWrap2_ConPrintf( const char *text );
void VGuiWrap2_ConDPrintf( const char *text );

#ifdef __cplusplus
}
#endif
#endif
