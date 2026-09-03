/*
ref_backend.h - renderer backend boundary

This interface deliberately uses a C ABI.  The legacy renderer is written in C,
while the Filament implementation is isolated in C++20/Objective-C++.
*/

#pragma once

#ifndef XASH_DEDICATED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct model_s model_t;
typedef struct ref_params_s ref_params_t;

typedef enum renderer_backend_kind_e
{
	RENDERER_BACKEND_OPENGL = 0,
	RENDERER_BACKEND_FILAMENT
} renderer_backend_kind_t;

typedef struct renderer_init_s
{
	void *window;
	int width;
	int height;
	qboolean fullscreen;
} renderer_init_t;

typedef struct renderer_readback_s
{
	void *pixels;
	int width;
	int height;
	int stride;
} renderer_readback_t;

typedef struct renderer_backend_s
{
	const char *name;
	qboolean (*Init)( const renderer_init_t *init );
	void (*Shutdown)( void );
	void (*Resize)( int width, int height );
	void (*BeginFrame)( const ref_params_t *refdef );
	void (*RenderScene)( const ref_params_t *refdef, qboolean drawWorld );
	void (*RenderOverlay)( void );
	void (*EndFrame)( void );
	void (*OnModelLoaded)( model_t *model );
	void (*OnModelUnloaded)( model_t *model );
	void (*OnMapLoaded)( model_t *world );
	void (*OnMapUnloaded)( void );
	qboolean (*ReadPixels)( renderer_readback_t *request );
} renderer_backend_t;

void R_BackendSelectFromCommandLine( void );
void R_BackendFallbackToOpenGL( const char *reason );
void R_BackendRegisterCvars( void );
renderer_backend_kind_t R_BackendKind( void );
qboolean R_BackendIsFilament( void );
const char *R_BackendName( void );
const renderer_backend_t *R_BackendAPI( void );
void R_BackendTextureUpload( int texture, int width, int height,
	int pixelFormat, const void *pixels, qboolean srgb );
void R_BackendTextureFree( int texture );
void R_BackendSkyTextureUpload( int side, int texture, int width, int height,
	int pixelFormat, const void *pixels );
void R_BackendSkyTextureClear( void );
void R_BackendR2DSetColor( unsigned char r, unsigned char g,
	unsigned char b, unsigned char a );
void R_BackendR2DSetRenderMode( int mode );
void R_BackendR2DDrawQuad( float x, float y, float width, float height,
	float s1, float t1, float s2, float t2, int texture );
void R_BackendR2DBegin( int primitive );
void R_BackendR2DEnd( void );
void R_BackendR2DSetTexture( int texture );
void R_BackendR2DTexCoord( float u, float v );
void R_BackendR2DVertex( float x, float y );
void R_BackendStudioSubmit( int texture, int renderMode, int faceFlags,
	const float *positions, const float *texcoords, const unsigned char *colors,
	int firstVertex, int vertexCount, const unsigned short *indices,
	int indexCount, qboolean viewModel );
void R_BackendSpriteSubmit( int texture, int renderMode, int textureFormat,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a,
	const float *positions );
int R_BackendLightmapTexture( int page );
int R_BackendLightmapBlockSize( void );
int R_BackendLightmapSampleSize( void );
qboolean R_BackendDrawableSize( int *width, int *height );
void R_BackendPrintInfo( void );

extern convar_t *r_renderer;
extern convar_t *r_shadow_quality;
extern convar_t *r_shadow_distance;
extern convar_t *r_contact_shadows;
extern convar_t *r_ssao;
extern convar_t *r_bloom;
extern convar_t *r_tonemapper;
extern convar_t *r_exposure_compensation;
extern convar_t *r_sun_angles;
extern convar_t *r_sun_color;
extern convar_t *r_sun_lux;

#ifdef __cplusplus
}
#endif

#endif /* XASH_DEDICATED */
