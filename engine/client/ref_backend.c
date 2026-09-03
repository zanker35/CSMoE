/*
ref_backend.c - renderer selection and shared renderer configuration
*/

#ifndef XASH_DEDICATED

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "ref_backend.h"

convar_t *r_renderer;
convar_t *r_shadow_quality;
convar_t *r_shadow_distance;
convar_t *r_contact_shadows;
convar_t *r_ssao;
convar_t *r_bloom;
convar_t *r_tonemapper;
convar_t *r_exposure_compensation;
convar_t *r_sun_angles;
convar_t *r_sun_color;
convar_t *r_sun_lux;

static renderer_backend_kind_t r_backend_kind = RENDERER_BACKEND_OPENGL;
static const renderer_backend_t *r_backend_api;

#ifdef XASH_FILAMENT
extern const renderer_backend_t *FilamentBackend_GetAPI( void );
extern void FilamentBackend_TextureUpload( int texture, int width, int height,
	int pixelFormat, const void *pixels, qboolean srgb );
extern void FilamentBackend_TextureFree( int texture );
extern void FilamentBackend_SkyTextureUpload( int side, int texture,
	int width, int height, int pixelFormat, const void *pixels );
extern void FilamentBackend_SkyTextureClear( void );
extern void FilamentBackend_R2DSetColor( unsigned char r, unsigned char g,
	unsigned char b, unsigned char a );
extern void FilamentBackend_R2DSetRenderMode( int mode );
extern void FilamentBackend_R2DDrawQuad( float x, float y, float width,
	float height, float s1, float t1, float s2, float t2, int texture );
extern void FilamentBackend_R2DBegin( int primitive );
extern void FilamentBackend_R2DEnd( void );
extern void FilamentBackend_R2DSetTexture( int texture );
extern void FilamentBackend_R2DTexCoord( float u, float v );
extern void FilamentBackend_R2DVertex( float x, float y );
extern void FilamentBackend_StudioSubmit( int texture, int renderMode,
	int faceFlags, const float *positions, const float *texcoords,
	const unsigned char *colors, int firstVertex, int vertexCount,
	const unsigned short *indices, int indexCount, qboolean viewModel );
extern void FilamentBackend_SpriteSubmit( int texture, int renderMode,
	int textureFormat, unsigned char r, unsigned char g, unsigned char b,
	unsigned char a, const float *positions );
extern qboolean FilamentBackend_DrawableSize( int *width, int *height );
extern void FilamentBackend_PrintInfo( void );
#endif

void R_BackendSelectFromCommandLine( void )
{
	char requested[32];

	r_backend_kind = RENDERER_BACKEND_OPENGL;
	r_backend_api = NULL;

	if( !Sys_GetParmFromCmdLine( "-renderer", requested ))
		return;

	if( !Q_stricmp( requested, "gl" ) || !Q_stricmp( requested, "opengl" ))
		return;

	if( Q_stricmp( requested, "filament" ))
	{
		MsgDev( D_WARN, "Unknown renderer '%s'; using OpenGL\n", requested );
		return;
	}

#ifdef XASH_FILAMENT
	r_backend_api = FilamentBackend_GetAPI();
	if( r_backend_api )
	{
		r_backend_kind = RENDERER_BACKEND_FILAMENT;
		return;
	}
	MsgDev( D_ERROR, "Filament renderer is compiled in but unavailable; using OpenGL\n" );
#else
	MsgDev( D_ERROR, "Filament renderer was requested but this build does not include it; using OpenGL\n" );
#endif
}

void R_BackendFallbackToOpenGL( const char *reason )
{
	if( reason && reason[0] )
		MsgDev( D_WARN, "Filament: %s; rebuilding the video system with OpenGL\n", reason );

	r_backend_kind = RENDERER_BACKEND_OPENGL;
	r_backend_api = NULL;

	if( r_renderer )
		Cvar_FullSet( "r_renderer", "gl", CVAR_READ_ONLY );
}

void R_BackendRegisterCvars( void )
{
	r_renderer = Cvar_Get( "r_renderer", R_BackendName(), CVAR_READ_ONLY, "active renderer backend" );
	r_shadow_quality = Cvar_Get( "r_shadow_quality", "2", CVAR_ARCHIVE, "Filament shadow quality (0-3)" );
	r_shadow_distance = Cvar_Get( "r_shadow_distance", "2048", CVAR_ARCHIVE, "Filament directional shadow distance" );
	r_contact_shadows = Cvar_Get( "r_contact_shadows", "1", CVAR_ARCHIVE, "Filament contact shadows" );
	r_ssao = Cvar_Get( "r_ssao", "1", CVAR_ARCHIVE, "Filament screen-space ambient occlusion" );
	r_bloom = Cvar_Get( "r_bloom", "1", CVAR_ARCHIVE, "Filament HDR bloom" );
	r_tonemapper = Cvar_Get( "r_tonemapper", "agx", CVAR_ARCHIVE, "Filament tone mapper: agx, aces or linear" );
	r_exposure_compensation = Cvar_Get( "r_exposure_compensation", "0", CVAR_ARCHIVE, "Filament exposure compensation in EV" );
	r_sun_angles = Cvar_Get( "r_sun_angles", "45 -45 0", CVAR_ARCHIVE, "fallback Filament sun angles" );
	r_sun_color = Cvar_Get( "r_sun_color", "255 244 226", CVAR_ARCHIVE, "fallback Filament sun color" );
	r_sun_lux = Cvar_Get( "r_sun_lux", "50000", CVAR_ARCHIVE, "fallback Filament sun illuminance" );
}

renderer_backend_kind_t R_BackendKind( void )
{
	return r_backend_kind;
}

qboolean R_BackendIsFilament( void )
{
	return r_backend_kind == RENDERER_BACKEND_FILAMENT;
}

const char *R_BackendName( void )
{
	return R_BackendIsFilament() ? "filament" : "gl";
}

const renderer_backend_t *R_BackendAPI( void )
{
	return r_backend_api;
}

void R_BackendTextureUpload( int texture, int width, int height,
	int pixelFormat, const void *pixels, qboolean srgb )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_TextureUpload( texture, width, height, pixelFormat, pixels, srgb );
#endif
}

void R_BackendTextureFree( int texture )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_TextureFree( texture );
#endif
}

void R_BackendSkyTextureUpload( int side, int texture, int width, int height,
	int pixelFormat, const void *pixels )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_SkyTextureUpload( side, texture, width, height,
			pixelFormat, pixels );
#endif
}

void R_BackendSkyTextureClear( void )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_SkyTextureClear();
#endif
}

void R_BackendR2DSetColor( unsigned char r, unsigned char g,
	unsigned char b, unsigned char a )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DSetColor( r, g, b, a );
#endif
}

void R_BackendR2DSetRenderMode( int mode )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DSetRenderMode( mode );
#endif
}

void R_BackendR2DDrawQuad( float x, float y, float width, float height,
	float s1, float t1, float s2, float t2, int texture )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DDrawQuad( x, y, width, height,
			s1, t1, s2, t2, texture );
#endif
}

void R_BackendR2DBegin( int primitive )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DBegin( primitive );
#endif
}

void R_BackendR2DEnd( void )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DEnd();
#endif
}

void R_BackendR2DSetTexture( int texture )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DSetTexture( texture );
#endif
}

void R_BackendR2DTexCoord( float u, float v )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DTexCoord( u, v );
#endif
}

void R_BackendR2DVertex( float x, float y )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_R2DVertex( x, y );
#endif
}

void R_BackendStudioSubmit( int texture, int renderMode, int faceFlags,
	const float *positions, const float *texcoords, const unsigned char *colors,
	int firstVertex, int vertexCount, const unsigned short *indices,
	int indexCount, qboolean viewModel )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_StudioSubmit( texture, renderMode, faceFlags,
			positions, texcoords, colors, firstVertex, vertexCount,
			indices, indexCount, viewModel );
#endif
}

void R_BackendSpriteSubmit( int texture, int renderMode, int textureFormat,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a,
	const float *positions )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_SpriteSubmit( texture, renderMode, textureFormat,
			r, g, b, a, positions );
#endif
}

int R_BackendLightmapTexture( int page )
{
	if( page < 0 || page >= MAX_LIGHTMAPS )
		return 0;
	return tr.lightmapTextures[page];
}

int R_BackendLightmapBlockSize( void )
{
	return BLOCK_SIZE;
}

int R_BackendLightmapSampleSize( void )
{
	return LM_SAMPLE_SIZE;
}

qboolean R_BackendDrawableSize( int *width, int *height )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		return FilamentBackend_DrawableSize( width, height );
#endif
	return false;
}

void R_BackendPrintInfo( void )
{
#ifdef XASH_FILAMENT
	if( R_BackendIsFilament() )
		FilamentBackend_PrintInfo();
#endif
}

#endif /* XASH_DEDICATED */
