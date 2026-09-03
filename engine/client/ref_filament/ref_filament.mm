/*
ref_filament.mm - Filament/Metal renderer backend bootstrap
*/

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "ref_backend.h"
#include "sprite.h"
#include "studio.h"
#include "triangleapi.h"

#import <AppKit/NSColorSpace.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#import <QuartzCore/CAMetalLayer.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include <backend/DriverEnums.h>
#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/Fence.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/SwapChain.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/ToneMapper.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <utils/EntityManager.h>

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;

namespace
{

static const uint8_t kWorldBakedMaterial[] = {
#include "world_baked.inc"
};

static const uint8_t kWorldTransparentMaterial[] = {
#include "world_transparent.inc"
};

static const uint8_t kWorldShadowReceiverMaterial[] = {
#include "world_shadow_receiver.inc"
};

static const uint8_t kSkyMaterial[] = {
#include "sky.inc"
};

static const uint8_t kR2DMaterial[] = {
#include "r2d.inc"
};

static const uint8_t kStudioOpaqueMaterial[] = {
#include "studio_opaque.inc"
};

static const uint8_t kStudioTransparentMaterial[] = {
#include "studio_transparent.inc"
};

static const uint8_t kSpriteMaskedMaterial[] = {
#include "sprite_masked.inc"
};

static const uint8_t kSpriteTransparentMaterial[] = {
#include "sprite_transparent.inc"
};

static const uint8_t kSpriteAdditiveMaterial[] = {
#include "sprite_additive.inc"
};

constexpr size_t kMaxR2DQuads = 16384;
constexpr size_t kMaxR2DBatches = 256;
constexpr size_t kMaxStudioVertices = 200000;
constexpr size_t kMaxStudioIndices = 400000;
constexpr size_t kMaxStudioBatches = 512;
constexpr size_t kStudioLayerCount = 2;
constexpr size_t kMaxSpriteQuads = 8192;
constexpr size_t kMaxSpriteBatches = 256;
constexpr size_t kSpriteMaterialCount = 3;

struct WorldVertex
{
	float3 position;
	float4 tangents;
	float2 uv0;
	float2 uv1;
};

struct TextureSlot
{
	Texture *texture = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	bool srgb = false;
};

struct SkyFace
{
	std::vector<uint8_t> rgba;
	uint32_t width = 0;
	uint32_t height = 0;
	int textureId = -1;
};

struct R2DVertex
{
	float2 position;
	float2 uv;
	uint32_t color;
};

struct R2DBatch
{
	int texture = 0;
	int renderMode = kRenderTransTexture;
	uint32_t indexOffset = 0;
	uint32_t indexCount = 0;
};

struct StudioVertex
{
	float3 position;
	float4 tangents;
	float2 uv;
	uint32_t color;
};

struct StudioBatch
{
	int texture = 0;
	bool transparent = false;
	bool viewModel = false;
	uint32_t indexOffset = 0;
	uint32_t indexCount = 0;
};

enum SpriteMaterialKind : uint8_t
{
	SPRITE_MATERIAL_MASKED = 0,
	SPRITE_MATERIAL_TRANSPARENT,
	SPRITE_MATERIAL_ADDITIVE
};

struct SpriteVertex
{
	float3 position;
	float2 uv;
	uint32_t color;
};

struct SpriteBatch
{
	int texture = 0;
	SpriteMaterialKind materialKind = SPRITE_MATERIAL_TRANSPARENT;
	uint32_t indexOffset = 0;
	uint32_t indexCount = 0;
};

struct WorldBatch
{
	MaterialInstance *material = nullptr;
	int baseTexture = 0;
	int lightmapTexture = 0;
};

struct BrushPrimitive
{
	int baseTexture = 0;
	int lightmapTexture = 0;
	uint32_t offset = 0;
	uint32_t count = 0;
};

struct BrushMesh
{
	model_t *model = nullptr;
	VertexBuffer *vertexBuffer = nullptr;
	IndexBuffer *indexBuffer = nullptr;
	std::vector<BrushPrimitive> primitives;
	Box bounds = {};
};

struct BrushInstance
{
	cl_entity_t *source = nullptr;
	model_t *model = nullptr;
	Entity entity;
	std::vector<WorldBatch> batches;
	uint64_t seenFrame = 0;
	bool inScene = false;
	bool transparent = false;
};

struct FilamentState
{
	Engine *engine = nullptr;
	SwapChain *swapChain = nullptr;
	Renderer *renderer = nullptr;
	Scene *scene = nullptr;
	Scene *overlayScene = nullptr;
	View *view = nullptr;
	View *viewModelView = nullptr;
	View *overlayView = nullptr;
	Camera *camera = nullptr;
	Camera *viewModelCamera = nullptr;
	Camera *overlayCamera = nullptr;
	ColorGrading *colorGrading = nullptr;
	Skybox *skybox = nullptr;
	Texture *skyboxTexture = nullptr;
	VertexBuffer *skyVertexBuffer = nullptr;
	IndexBuffer *skyIndexBuffer = nullptr;
	Texture *skyFaceTextures[6] = {};
	VertexBuffer *r2dVertexBuffer = nullptr;
	IndexBuffer *r2dIndexBuffer = nullptr;
	VertexBuffer *studioVertexBuffer = nullptr;
	IndexBuffer *studioIndexBuffer = nullptr;
	VertexBuffer *spriteVertexBuffer = nullptr;
	IndexBuffer *spriteIndexBuffer = nullptr;
	Material *worldMaterial = nullptr;
	Material *worldTransparentMaterial = nullptr;
	Material *worldShadowReceiverMaterial = nullptr;
	MaterialInstance *worldShadowReceiverInstance = nullptr;
	Material *skyMaterial = nullptr;
	Material *r2dMaterial = nullptr;
	Material *r2dAdditiveMaterial = nullptr;
	MaterialInstance *r2dMaterialInstances[kMaxR2DBatches] = {};
	MaterialInstance *r2dAdditiveInstances[kMaxR2DBatches] = {};
	Material *studioOpaqueMaterial = nullptr;
	Material *studioTransparentMaterial = nullptr;
	MaterialInstance *studioOpaqueInstances[kStudioLayerCount][kMaxStudioBatches] = {};
	MaterialInstance *studioTransparentInstances[kStudioLayerCount][kMaxStudioBatches] = {};
	Material *spriteMaterials[kSpriteMaterialCount] = {};
	MaterialInstance *spriteInstances[kSpriteMaterialCount][kMaxSpriteBatches] = {};
	Texture *whiteTexture = nullptr;
	VertexBuffer *worldVertexBuffer = nullptr;
	IndexBuffer *worldIndexBuffer = nullptr;
	Entity worldEntity;
	Entity worldShadowReceiverEntity;
	Entity skyEntities[6];
	Entity sunEntity;
	std::vector<WorldBatch> worldBatches;
	std::vector<WorldBatch> skyBatches;
	std::map<model_t *, BrushMesh> brushMeshes;
	std::map<cl_entity_t *, BrushInstance> brushInstances;
	TextureSlot textures[MAX_TEXTURES];
	SkyFace skyFaces[6];
	Entity cameraEntity;
	Entity viewModelCameraEntity;
	Entity overlayCameraEntity;
	Entity r2dEntity;
	Entity studioEntities[kStudioLayerCount];
	Entity spriteEntity;
	Entity dynamicLights[MAX_DLIGHTS];
	bool dynamicLightInScene[MAX_DLIGHTS] = {};
	uint32_t activeDynamicLights = 0;
	std::vector<R2DVertex> r2dVertices;
	std::vector<uint32_t> r2dIndices;
	std::vector<R2DBatch> r2dBatches;
	std::vector<R2DVertex> r2dImmediateVertices;
	std::vector<StudioVertex> studioVertices;
	std::vector<uint32_t> studioIndices;
	std::vector<StudioBatch> studioBatches;
	std::vector<SpriteVertex> spriteVertices;
	std::vector<uint32_t> spriteIndices;
	std::vector<SpriteBatch> spriteBatches;
	uint32_t r2dColor = 0xffffffffu;
	int r2dRenderMode = kRenderTransTexture;
	int r2dImmediateMode = TRI_TRIANGLES;
	int r2dTexture = 0;
	float2 r2dTexCoord = {0.0f, 0.0f};
	bool r2dImmediateActive = false;
	size_t lastR2DBatchCount = 0;
	size_t lastStudioBatchCount[kStudioLayerCount] = {};
	size_t lastSpriteBatchCount = 0;
	NSView *nativeView = nil;
	CAMetalLayer *metalLayer = nil;
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t logicalWidth = 1;
	uint32_t logicalHeight = 1;
	bool frameStarted = false;
	bool renderedThisFrame = false;
	bool scenePrepared = false;
	bool r2dFlushed = false;
	uint64_t entityFrame = 0;
	uint64_t begunFrames = 0;
	uint64_t skippedFrames = 0;
	uint64_t renderedFrames = 0;
	int appliedSsao = -1;
	int appliedBloom = -1;
	int appliedShadowQuality = -1;
	int appliedStudioShadows = -1;
	int appliedContactShadows = -1;
	float appliedShadowDistance = -1.0f;
	float appliedExposure = std::numeric_limits<float>::quiet_NaN();
	std::string appliedToneMapper;
};

FilamentState g_filament;

void FinishRenderedFrameBeforeResourceMutation()
{
	FilamentState &state = g_filament;
	if( state.frameStarted && state.renderedThisFrame && state.renderer )
	{
		state.renderer->endFrame();
		state.frameStarted = false;
		state.renderedThisFrame = false;
	}
}

float3 GoldSrcToFilament( const vec3_t value )
{
	return {-value[1], value[2], -value[0]};
}

void GoldSrcAngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up )
{
	constexpr float radians = 3.14159265358979323846f / 180.0f;
	const float pitch = angles[0] * radians;
	const float yaw = angles[1] * radians;
	const float roll = angles[2] * radians;
	const float sp = std::sin( pitch ), cp = std::cos( pitch );
	const float sy = std::sin( yaw ), cy = std::cos( yaw );
	const float sr = std::sin( roll ), cr = std::cos( roll );

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
	right[0] = -sr * sp * cy + cr * sy;
	right[1] = -sr * sp * sy - cr * cy;
	right[2] = -sr * cp;
	up[0] = cr * sp * cy + sr * sy;
	up[1] = cr * sp * sy - sr * cy;
	up[2] = cr * cp;
}

float3 ParseColor( const char *text, const float3 &fallback )
{
	float r, g, b;
	if( !text || std::sscanf( text, "%f %f %f", &r, &g, &b ) < 3 )
		return fallback;
	return {
		std::pow( std::clamp( r / 255.0f, 0.0f, 1.0f ), 2.2f ),
		std::pow( std::clamp( g / 255.0f, 0.0f, 1.0f ), 2.2f ),
		std::pow( std::clamp( b / 255.0f, 0.0f, 1.0f ), 2.2f )
	};
}

void ParseAngles( const char *text, vec3_t angles )
{
	if( !text || std::sscanf( text, "%f %f %f",
		&angles[0], &angles[1], &angles[2] ) < 2 )
	{
		angles[0] = 45.0f;
		angles[1] = -45.0f;
		angles[2] = 0.0f;
	}
}

struct SunDefinition
{
	vec3_t angles = {45.0f, -45.0f, 0.0f};
	float3 color = {1.0f, 0.91f, 0.77f};
	float lux = 50000.0f;
	bool fromMap = false;
};

SunDefinition FindSunDefinition( model_t *worldModel )
{
	SunDefinition result;
	if( r_sun_angles )
		ParseAngles( r_sun_angles->string, result.angles );
	if( r_sun_color )
		result.color = ParseColor( r_sun_color->string, result.color );
	if( r_sun_lux && r_sun_lux->value > 0.0f )
		result.lux = r_sun_lux->value;

	if( !worldModel || !worldModel->entities )
		return result;

	char *cursor = worldModel->entities;
	char token[2048];
	while(( cursor = COM_ParseFile( cursor, token )) != nullptr )
	{
		if( std::strcmp( token, "{" ))
			continue;

		std::string classname;
		std::string angles;
		std::string pitch;
		std::string light;
		while(( cursor = COM_ParseFile( cursor, token )) != nullptr )
		{
			if( !std::strcmp( token, "}" ))
				break;
			const std::string key = token;
			cursor = COM_ParseFile( cursor, token );
			if( !cursor )
				break;
			if( key == "classname" )
				classname = token;
			else if( key == "angles" )
				angles = token;
			else if( key == "pitch" )
				pitch = token;
			else if( key == "_light" || key == "light" )
				light = token;
		}

		if( classname != "light_environment" )
			continue;

		if( !angles.empty() )
			ParseAngles( angles.c_str(), result.angles );
		if( !pitch.empty() )
			result.angles[0] = std::strtof( pitch.c_str(), nullptr );
		if( !light.empty() )
		{
			float r = 255.0f, g = 255.0f, b = 255.0f, brightness = 200.0f;
			const int fields = std::sscanf( light.c_str(), "%f %f %f %f",
				&r, &g, &b, &brightness );
			char colorText[96];
			std::snprintf( colorText, sizeof( colorText ), "%g %g %g", r, g, b );
			result.color = ParseColor( colorText, result.color );
			if( fields >= 4 && brightness > 0.0f )
				result.lux = brightness * 250.0f;
		}
		result.fromMap = true;
		return result;
	}
	return result;
}

void DestroySun()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.sunEntity )
		return;
	if( state.scene )
		state.scene->remove( state.sunEntity );
	state.engine->destroy( state.sunEntity );
	EntityManager::get().destroy( state.sunEntity );
	state.sunEntity.clear();
}

LightManager::ShadowOptions CurrentShadowOptions()
{
	LightManager::ShadowOptions options;
	const int quality = r_shadow_quality ?
		std::clamp( r_shadow_quality->integer, 0, 3 ) : 2;
	options.mapSize = quality >= 3 ? 4096u : quality >= 2 ? 2048u : 1024u;
	options.shadowCascades = quality >= 3 ? 4 : quality >= 2 ? 3 : 1;
	if( options.shadowCascades > 1 )
		LightManager::ShadowCascades::computePracticalSplits(
			options.cascadeSplitPositions, options.shadowCascades,
			4.0f, r_shadow_distance ?
				std::max( 64.0f, r_shadow_distance->value ) : 2048.0f,
			0.35f );
	options.shadowFar = r_shadow_distance ?
		std::max( 64.0f, r_shadow_distance->value ) : 2048.0f;
	options.shadowNearHint = 4.0f;
	options.shadowFarHint = options.shadowFar;
	options.screenSpaceContactShadows =
		r_contact_shadows && r_contact_shadows->integer != 0;
	options.maxShadowDistance = 32.0f;
	options.stepCount = 8;
	options.stable = true;
	return options;
}

void CreateSun( model_t *worldModel )
{
	FilamentState &state = g_filament;
	DestroySun();
	if( !state.engine || !state.scene )
		return;

	const SunDefinition sun = FindSunDefinition( worldModel );
	vec3_t goldForward, goldRight, goldUp;
	GoldSrcAngleVectors( sun.angles, goldForward, goldRight, goldUp );
	const float3 direction = normalize( GoldSrcToFilament( goldForward ));
	const int shadowQuality = r_shadow_quality ?
		std::clamp( r_shadow_quality->integer, 0, 3 ) : 2;

	state.sunEntity = EntityManager::get().create();
	LightManager::Builder( LightManager::Type::SUN )
		.direction( direction )
		.color( sun.color )
		.intensity( sun.lux )
		.sunAngularRadius( 0.545f )
		.castShadows( shadowQuality > 0 )
		.shadowOptions( CurrentShadowOptions() )
		.build( *state.engine, state.sunEntity );
	state.scene->addEntity( state.sunEntity );

	MsgDev( D_INFO,
		"Filament: sun %s, angles %.1f %.1f %.1f, %.0f lux\n",
		sun.fromMap ? "from light_environment" : "from r_sun_* fallback",
		sun.angles[0], sun.angles[1], sun.angles[2], sun.lux );
}

void DestroyDynamicLights()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;
	for( int index = 0; index < MAX_DLIGHTS; ++index )
	{
		Entity &entity = state.dynamicLights[index];
		if( state.scene && state.dynamicLightInScene[index] && entity )
			state.scene->remove( entity );
		if( entity )
		{
			state.engine->destroy( entity );
			EntityManager::get().destroy( entity );
			entity.clear();
		}
		state.dynamicLightInScene[index] = false;
	}
	state.activeDynamicLights = 0;
}

bool EnsureDynamicLight( int index )
{
	FilamentState &state = g_filament;
	if( index < 0 || index >= MAX_DLIGHTS || !state.engine )
		return false;
	if( state.dynamicLights[index] )
		return true;
	state.dynamicLights[index] = EntityManager::get().create();
	LightManager::Builder( LightManager::Type::POINT )
		.position({0.0f, 0.0f, 0.0f})
		.color({1.0f, 1.0f, 1.0f})
		.intensity( 1000.0f )
		.falloff( 128.0f )
		.castShadows( false )
		.build( *state.engine, state.dynamicLights[index] );
	return bool( state.engine->getLightManager().getInstance(
		state.dynamicLights[index] ));
}

void UpdateDynamicLights()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.scene )
		return;
	const bool enabled = !r_dynamic || r_dynamic->integer != 0;
	LightManager &lights = state.engine->getLightManager();
	state.activeDynamicLights = 0;

	for( int index = 0; index < MAX_DLIGHTS; ++index )
	{
		const dlight_t &source = cl_dlights[index];
		const bool active = enabled && source.radius > 0.0f &&
			source.die >= cl.time;
		if( !active )
		{
			if( state.dynamicLightInScene[index] && state.dynamicLights[index] )
			{
				state.scene->remove( state.dynamicLights[index] );
				state.dynamicLightInScene[index] = false;
			}
			continue;
		}
		if( !EnsureDynamicLight( index ))
			continue;

		const LightManager::Instance light =
			lights.getInstance( state.dynamicLights[index] );
		if( !light )
			continue;
		const auto linearChannel = []( uint8_t value ) {
			return std::pow( float( value ) / 255.0f, 2.2f );
		};
		lights.setPosition( light, GoldSrcToFilament( source.origin ));
		lights.setColor( light, {
			linearChannel( source.color.r ),
			linearChannel( source.color.g ),
			linearChannel( source.color.b )
		});
		const float lumens =
			std::clamp( source.radius * 20.0f, 200.0f, 20000.0f );
		lights.setIntensity( light, source.dark ? -lumens : lumens );
		lights.setFalloff( light, std::max( source.radius, 1.0f ));
		if( !state.dynamicLightInScene[index] )
		{
			state.scene->addEntity( state.dynamicLights[index] );
			state.dynamicLightInScene[index] = true;
		}
		++state.activeDynamicLights;
	}
}

ColorGrading *BuildColorGrading( Engine &engine, const char *name, float exposure )
{
	ColorGrading::Builder builder;
	builder.quality( ColorGrading::QualityLevel::HIGH ).exposure( exposure );
	if( name && !Q_stricmp( name, "linear" ))
	{
		LinearToneMapper toneMapper;
		return builder.toneMapper( &toneMapper ).build( engine );
	}
	if( name && !Q_stricmp( name, "aces" ))
	{
		ACESToneMapper toneMapper;
		return builder.toneMapper( &toneMapper ).build( engine );
	}
	AgxToneMapper toneMapper( AgxToneMapper::AgxLook::NONE );
	return builder.toneMapper( &toneMapper ).build( engine );
}

void ApplyQualitySettings()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.view )
		return;

	const int ssao = r_ssao && r_ssao->integer != 0;
	if( ssao != state.appliedSsao )
	{
		AmbientOcclusionOptions options;
		options.enabled = ssao != 0;
		options.aoType = AmbientOcclusionOptions::AmbientOcclusionType::GTAO;
		options.radius = 24.0f;
		options.power = 1.15f;
		options.intensity = 0.85f;
		options.quality = QualityLevel::MEDIUM;
		options.lowPassFilter = QualityLevel::MEDIUM;
		options.upsampling = QualityLevel::MEDIUM;
		state.view->setAmbientOcclusionOptions( options );
		state.appliedSsao = ssao;
	}

	const int bloom = r_bloom && r_bloom->integer != 0;
	if( bloom != state.appliedBloom )
	{
		BloomOptions options;
		options.enabled = bloom != 0;
		options.strength = 0.08f;
		options.threshold = true;
		options.resolution = 512;
		options.levels = 6;
		options.quality = QualityLevel::MEDIUM;
		state.view->setBloomOptions( options );
		state.appliedBloom = bloom;
	}

	const char *toneName = r_tonemapper && r_tonemapper->string ?
		r_tonemapper->string : "agx";
	const float exposure = r_exposure_compensation ?
		r_exposure_compensation->value : 0.0f;
	if( state.appliedToneMapper != toneName || state.appliedExposure != exposure )
	{
		ColorGrading *replacement =
			BuildColorGrading( *state.engine, toneName, exposure );
		if( replacement )
		{
			state.view->setColorGrading( replacement );
			if( state.colorGrading )
				state.engine->destroy( state.colorGrading );
			state.colorGrading = replacement;
			state.appliedToneMapper = toneName;
			state.appliedExposure = exposure;
		}
	}

	TemporalAntiAliasingOptions taa;
	taa.enabled = true;
	taa.feedback = 0.12f;
	taa.sharpness = 0.15f;
	taa.hdr = true;
	state.view->setAntiAliasing( AntiAliasing::NONE );
	state.view->setTemporalAntiAliasingOptions( taa );

	RenderQuality renderQuality;
	renderQuality.hdrColorBuffer = QualityLevel::HIGH;
	state.view->setRenderQuality( renderQuality );
	state.view->setShadowType( ShadowType::DPCF );
	state.view->setShadowingEnabled( true );

	const int shadowQuality = r_shadow_quality ?
		std::clamp( r_shadow_quality->integer, 0, 3 ) : 2;
	const int studioShadows = shadowQuality > 0 &&
		Cvar_VariableInteger( "cl_shadows" ) != 0;
	if( studioShadows != state.appliedStudioShadows &&
		state.studioEntities[0] )
	{
		RenderableManager &renderables = state.engine->getRenderableManager();
		const RenderableManager::Instance studio =
			renderables.getInstance( state.studioEntities[0] );
		if( studio )
			renderables.setCastShadows( studio, studioShadows != 0 );
		state.appliedStudioShadows = studioShadows;
	}
	const int contactShadows =
		r_contact_shadows && r_contact_shadows->integer != 0;
	const float shadowDistance = r_shadow_distance ?
		r_shadow_distance->value : 2048.0f;
	if( state.sunEntity && ( shadowQuality != state.appliedShadowQuality ||
		contactShadows != state.appliedContactShadows ||
		shadowDistance != state.appliedShadowDistance ))
	{
		LightManager &lights = state.engine->getLightManager();
		const LightManager::Instance sun = lights.getInstance( state.sunEntity );
		if( sun )
		{
			lights.setShadowCaster( sun, shadowQuality > 0 );
			lights.setShadowOptions( sun, CurrentShadowOptions() );
		}
		state.appliedShadowQuality = shadowQuality;
		state.appliedContactShadows = contactShadows;
		state.appliedShadowDistance = shadowDistance;
	}
}

NSView *GetContentView( SDL_Window *window )
{
	SDL_SysWMinfo info;
	SDL_VERSION( &info.version );
	if( !SDL_GetWindowWMInfo( window, &info ))
		return nil;
	NSWindow *nativeWindow = info.info.cocoa.window;
	return nativeWindow ? [nativeWindow contentView] : nil;
}

CAMetalLayer *SetUpMetalLayer( NSView *view )
{
	[view.window setColorSpace:[NSColorSpace sRGBColorSpace]];
	[view setWantsLayer:YES];

	CAMetalLayer *layer = [CAMetalLayer layer];
	layer.bounds = view.bounds;
	layer.drawableSize = [view convertSizeToBacking:view.bounds.size];
	layer.contentsScale = view.window.backingScaleFactor;
	layer.opaque = YES;
	[view setLayer:layer];
	return layer;
}

void ResizeMetalLayer( FilamentState &state )
{
	if( !state.nativeView || !state.metalLayer )
		return;

	state.metalLayer.bounds = state.nativeView.bounds;
	state.metalLayer.drawableSize =
		[state.nativeView convertSizeToBacking:state.nativeView.bounds.size];
	state.metalLayer.contentsScale = state.nativeView.window.backingScaleFactor;
}

void UpdateDrawableSize( FilamentState &state )
{
	ResizeMetalLayer( state );
	if( !state.metalLayer )
		return;
	const CGSize drawableSize = state.metalLayer.drawableSize;
	state.width = drawableSize.width > 0.0 ?
		static_cast<uint32_t>( std::lround( drawableSize.width )) : 1u;
	state.height = drawableSize.height > 0.0 ?
		static_cast<uint32_t>( std::lround( drawableSize.height )) : 1u;
}

void ReleaseUploadBuffer( void *buffer, size_t, void * )
{
	std::free( buffer );
}

TextureSampler BaseTextureSampler()
{
	return TextureSampler( TextureSampler::MinFilter::LINEAR,
		TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT );
}

TextureSampler LightmapTextureSampler()
{
	return TextureSampler( TextureSampler::MinFilter::LINEAR,
		TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::CLAMP_TO_EDGE );
}

void ResetR2DCommands()
{
	FilamentState &state = g_filament;
	state.r2dVertices.clear();
	state.r2dIndices.clear();
	state.r2dBatches.clear();
	state.r2dImmediateVertices.clear();
	state.r2dColor = 0xffffffffu;
	state.r2dRenderMode = kRenderTransTexture;
	state.r2dImmediateActive = false;
	state.r2dFlushed = false;
}

void SetR2DColor( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	g_filament.r2dColor = uint32_t( r ) | ( uint32_t( g ) << 8u ) |
		( uint32_t( b ) << 16u ) | ( uint32_t( a ) << 24u );
}

void SetR2DRenderMode( int mode )
{
	g_filament.r2dRenderMode = mode;
}

R2DBatch *GetR2DBatch( int texture )
{
	FilamentState &state = g_filament;
	R2DBatch *batch = state.r2dBatches.empty() ? nullptr :
		&state.r2dBatches.back();
	if( !batch || batch->texture != texture ||
		batch->renderMode != state.r2dRenderMode )
	{
		if( state.r2dBatches.size() >= kMaxR2DBatches )
			return nullptr;
		state.r2dBatches.push_back({
			texture, state.r2dRenderMode,
			uint32_t( state.r2dIndices.size() ), 0
		});
		batch = &state.r2dBatches.back();
	}
	return batch;
}

void AppendR2DTriangle( const R2DVertex &a, const R2DVertex &b,
	const R2DVertex &c, int texture )
{
	FilamentState &state = g_filament;
	if( state.r2dVertices.size() + 3 > kMaxR2DQuads * 4 )
		return;
	R2DBatch *batch = GetR2DBatch( texture );
	if( !batch )
		return;
	const uint32_t base = uint32_t( state.r2dVertices.size() );
	state.r2dVertices.insert( state.r2dVertices.end(), {a, b, c});
	state.r2dIndices.insert( state.r2dIndices.end(), {
		base, base + 1, base + 2
	});
	batch->indexCount += 3;
}

void DrawR2DQuad( float x, float y, float width, float height,
	float s1, float t1, float s2, float t2, int texture )
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.r2dMaterial ||
		state.r2dVertices.size() / 4 >= kMaxR2DQuads )
		return;

	R2DBatch *batch = GetR2DBatch( texture );
	if( !batch )
		return;

	const uint32_t base = uint32_t( state.r2dVertices.size() );
	const uint32_t color = state.r2dColor;
	const float top = 1.0f - t1;
	const float bottom = 1.0f - t2;
	state.r2dVertices.insert( state.r2dVertices.end(), {
		{{x, y}, {s1, top}, color},
		{{x + width, y}, {s2, top}, color},
		{{x + width, y + height}, {s2, bottom}, color},
		{{x, y + height}, {s1, bottom}, color}
	});
	state.r2dIndices.insert( state.r2dIndices.end(), {
		base, base + 1, base + 2, base, base + 2, base + 3
	});
	batch->indexCount += 6;
}

void BeginR2DImmediate( int primitive )
{
	FilamentState &state = g_filament;
	state.r2dImmediateMode = primitive;
	state.r2dImmediateVertices.clear();
	state.r2dImmediateActive = true;
}

void SetR2DTexture( int texture )
{
	g_filament.r2dTexture = texture;
}

void SetR2DTexCoord( float u, float v )
{
	g_filament.r2dTexCoord = {u, 1.0f - v};
}

void AddR2DImmediateVertex( float x, float y )
{
	FilamentState &state = g_filament;
	if( !state.r2dImmediateActive ||
		state.r2dImmediateVertices.size() >= kMaxR2DQuads * 4 )
		return;
	state.r2dImmediateVertices.push_back({
		{x, y}, state.r2dTexCoord, state.r2dColor
	});
}

void EndR2DImmediate()
{
	FilamentState &state = g_filament;
	if( !state.r2dImmediateActive )
		return;
	state.r2dImmediateActive = false;
	const std::vector<R2DVertex> &v = state.r2dImmediateVertices;
	const size_t count = v.size();
	switch( state.r2dImmediateMode )
	{
	case TRI_TRIANGLES:
		for( size_t i = 0; i + 2 < count; i += 3 )
			AppendR2DTriangle( v[i], v[i + 1], v[i + 2], state.r2dTexture );
		break;
	case TRI_QUADS:
		for( size_t i = 0; i + 3 < count; i += 4 )
		{
			AppendR2DTriangle( v[i], v[i + 1], v[i + 2], state.r2dTexture );
			AppendR2DTriangle( v[i], v[i + 2], v[i + 3], state.r2dTexture );
		}
		break;
	case TRI_TRIANGLE_STRIP:
		for( size_t i = 2; i < count; ++i )
		{
			if( i & 1 )
				AppendR2DTriangle( v[i - 1], v[i - 2], v[i], state.r2dTexture );
			else
				AppendR2DTriangle( v[i - 2], v[i - 1], v[i], state.r2dTexture );
		}
		break;
	case TRI_QUAD_STRIP:
		for( size_t i = 3; i < count; i += 2 )
		{
			AppendR2DTriangle( v[i - 3], v[i - 2], v[i], state.r2dTexture );
			AppendR2DTriangle( v[i - 3], v[i], v[i - 1], state.r2dTexture );
		}
		break;
	case TRI_TRIANGLE_FAN:
	case TRI_POLYGON:
		for( size_t i = 2; i < count; ++i )
			AppendR2DTriangle( v[0], v[i - 1], v[i], state.r2dTexture );
		break;
	case TRI_LINES:
		for( size_t i = 0; i + 1 < count; i += 2 )
		{
			const float2 delta = v[i + 1].position - v[i].position;
			const float length = std::max( std::sqrt(
				delta.x * delta.x + delta.y * delta.y ), 0.001f );
			const float2 normal = {-delta.y / length * 0.5f,
				delta.x / length * 0.5f};
			R2DVertex a = v[i], b = v[i], c = v[i + 1], d = v[i + 1];
			a.position += normal;
			b.position -= normal;
			c.position -= normal;
			d.position += normal;
			AppendR2DTriangle( a, b, c, state.r2dTexture );
			AppendR2DTriangle( a, c, d, state.r2dTexture );
		}
		break;
	case TRI_POINTS:
		for( const R2DVertex &point : v )
		{
			R2DVertex a = point, b = point, c = point, d = point;
			b.position.x += 1.0f;
			c.position += float2{1.0f, 1.0f};
			d.position.y += 1.0f;
			AppendR2DTriangle( a, b, c, state.r2dTexture );
			AppendR2DTriangle( a, c, d, state.r2dTexture );
		}
		break;
	default:
		break;
	}
	state.r2dImmediateVertices.clear();
}

void UpdateOverlayCamera()
{
	FilamentState &state = g_filament;
	if( !state.overlayCamera || !state.overlayView )
		return;
	state.overlayView->setViewport({0, 0, state.width, state.height});
	state.overlayCamera->setProjection( Camera::Projection::ORTHO,
		0.0, double( std::max( state.logicalWidth, 1u )),
		double( std::max( state.logicalHeight, 1u )), 0.0, 0.0, 2.0 );
	state.overlayCamera->lookAt({0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0});
}

bool CreateR2DResources()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.overlayScene || !state.overlayView ||
		!state.overlayCamera || !state.whiteTexture )
		return false;

	state.r2dMaterial = Material::Builder()
		.package( kR2DMaterial, sizeof( kR2DMaterial ))
		.build( *state.engine );
	state.r2dAdditiveMaterial = Material::Builder()
		.package( kSpriteAdditiveMaterial, sizeof( kSpriteAdditiveMaterial ))
		.build( *state.engine );
	state.r2dVertexBuffer = VertexBuffer::Builder()
		.vertexCount( uint32_t( kMaxR2DQuads * 4 ))
		.bufferCount( 1 )
		.attribute( VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( R2DVertex, position ), sizeof( R2DVertex ))
		.attribute( VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( R2DVertex, uv ), sizeof( R2DVertex ))
		.attribute( VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4,
			offsetof( R2DVertex, color ), sizeof( R2DVertex ))
		.normalized( VertexAttribute::COLOR )
		.build( *state.engine );
	state.r2dIndexBuffer = IndexBuffer::Builder()
		.indexCount( uint32_t( kMaxR2DQuads * 6 ))
		.bufferType( IndexBuffer::IndexType::UINT )
		.build( *state.engine );
	if( !state.r2dMaterial || !state.r2dAdditiveMaterial ||
		!state.r2dVertexBuffer || !state.r2dIndexBuffer )
		return false;

	state.r2dEntity = EntityManager::get().create();
	RenderableManager::Builder builder( kMaxR2DBatches );
	builder.boundingBox({{0.0f, 0.0f, 0.0f}, {100000.0f, 100000.0f, 1.0f}})
		.culling( false )
		.castShadows( false )
		.receiveShadows( false )
		.priority( 7 );
	for( size_t primitive = 0; primitive < kMaxR2DBatches; ++primitive )
	{
		MaterialInstance *instance = state.r2dMaterial->createInstance();
		MaterialInstance *additive =
			state.r2dAdditiveMaterial->createInstance();
		instance->setParameter( "colorTexture",
			state.whiteTexture, BaseTextureSampler() );
		additive->setParameter( "colorTexture",
			state.whiteTexture, BaseTextureSampler() );
		state.r2dMaterialInstances[primitive] = instance;
		state.r2dAdditiveInstances[primitive] = additive;
		builder.material( primitive, instance )
			.geometry( primitive, RenderableManager::PrimitiveType::TRIANGLES,
				state.r2dVertexBuffer, state.r2dIndexBuffer, 0, 0 )
			.blendOrder( primitive, uint16_t( primitive ))
			.globalBlendOrderEnabled( primitive, true );
	}
	builder.build( *state.engine, state.r2dEntity );
	state.overlayScene->addEntity( state.r2dEntity );
	state.r2dVertices.reserve( kMaxR2DQuads * 4 );
	state.r2dIndices.reserve( kMaxR2DQuads * 6 );
	state.r2dBatches.reserve( kMaxR2DBatches );
	state.r2dImmediateVertices.reserve( 256 );
	UpdateOverlayCamera();
	return true;
}

void DestroyR2DResources()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;
	if( state.overlayScene && state.r2dEntity )
		state.overlayScene->remove( state.r2dEntity );
	if( state.r2dEntity )
	{
		state.engine->destroy( state.r2dEntity );
		EntityManager::get().destroy( state.r2dEntity );
		state.r2dEntity.clear();
	}
	for( MaterialInstance *&instance : state.r2dMaterialInstances )
	{
		if( instance )
			state.engine->destroy( instance );
		instance = nullptr;
	}
	for( MaterialInstance *&instance : state.r2dAdditiveInstances )
	{
		if( instance )
			state.engine->destroy( instance );
		instance = nullptr;
	}
	if( state.r2dMaterial )
	{
		state.engine->destroy( state.r2dMaterial );
		state.r2dMaterial = nullptr;
	}
	if( state.r2dAdditiveMaterial )
	{
		state.engine->destroy( state.r2dAdditiveMaterial );
		state.r2dAdditiveMaterial = nullptr;
	}
	if( state.r2dVertexBuffer )
	{
		state.engine->destroy( state.r2dVertexBuffer );
		state.r2dVertexBuffer = nullptr;
	}
	if( state.r2dIndexBuffer )
	{
		state.engine->destroy( state.r2dIndexBuffer );
		state.r2dIndexBuffer = nullptr;
	}
	ResetR2DCommands();
	state.lastR2DBatchCount = 0;
}

void PrepareR2D()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.overlayView || !state.r2dEntity ||
		!state.r2dVertexBuffer || !state.r2dIndexBuffer ||
		state.r2dFlushed )
		return;
	RenderableManager &renderables = state.engine->getRenderableManager();
	const RenderableManager::Instance renderable =
		renderables.getInstance( state.r2dEntity );
	if( !renderable )
		return;

	if( !state.r2dVertices.empty() && !state.r2dIndices.empty() )
	{
		const size_t vertexBytes = state.r2dVertices.size() * sizeof( R2DVertex );
		const size_t indexBytes = state.r2dIndices.size() * sizeof( uint32_t );
		void *vertexData = std::malloc( vertexBytes );
		void *indexData = std::malloc( indexBytes );
		if( !vertexData || !indexData )
		{
			std::free( vertexData );
			std::free( indexData );
			return;
		}
		std::memcpy( vertexData, state.r2dVertices.data(), vertexBytes );
		std::memcpy( indexData, state.r2dIndices.data(), indexBytes );
		state.r2dVertexBuffer->setBufferAt( *state.engine, 0,
			VertexBuffer::BufferDescriptor(
				vertexData, vertexBytes, ReleaseUploadBuffer ));
		state.r2dIndexBuffer->setBuffer( *state.engine,
			IndexBuffer::BufferDescriptor(
				indexData, indexBytes, ReleaseUploadBuffer ));
	}

	for( size_t batchIndex = 0; batchIndex < state.r2dBatches.size(); ++batchIndex )
	{
		const R2DBatch &batch = state.r2dBatches[batchIndex];
		Texture *texture = batch.texture >= 0 && batch.texture < MAX_TEXTURES ?
			state.textures[batch.texture].texture : nullptr;
		const bool additive = batch.renderMode == kRenderTransAdd ||
			batch.renderMode == kRenderGlow;
		MaterialInstance *material = additive ?
			state.r2dAdditiveInstances[batchIndex] :
			state.r2dMaterialInstances[batchIndex];
		material->setParameter( "colorTexture",
			texture ? texture : state.whiteTexture, BaseTextureSampler() );
		renderables.setMaterialInstanceAt( renderable, batchIndex, material );
		renderables.setGeometryAt( renderable, batchIndex,
			RenderableManager::PrimitiveType::TRIANGLES,
			state.r2dVertexBuffer, state.r2dIndexBuffer,
			batch.indexOffset, batch.indexCount );
		renderables.setBlendOrderAt( renderable, batchIndex,
			uint16_t( batchIndex ));
	}
	for( size_t batchIndex = state.r2dBatches.size();
		batchIndex < state.lastR2DBatchCount; ++batchIndex )
	{
		renderables.setGeometryAt( renderable, batchIndex,
			RenderableManager::PrimitiveType::TRIANGLES,
			state.r2dVertexBuffer, state.r2dIndexBuffer, 0, 0 );
	}
	state.lastR2DBatchCount = state.r2dBatches.size();
	state.r2dFlushed = true;
}

void ResetStudioCommands()
{
	FilamentState &state = g_filament;
	state.studioVertices.clear();
	state.studioIndices.clear();
	state.studioBatches.clear();
}

bool IsStudioTransparent( int renderMode, int faceFlags )
{
	return renderMode != kRenderNormal ||
		( faceFlags & ( STUDIO_NF_TRANSPARENT |
			STUDIO_NF_ADDITIVE | STUDIO_NF_ALPHA )) != 0;
}

StudioBatch *GetStudioBatch( int texture, bool transparent, bool viewModel )
{
	FilamentState &state = g_filament;
	StudioBatch *batch = state.studioBatches.empty() ? nullptr :
		&state.studioBatches.back();
	if( !batch || batch->texture != texture ||
		batch->transparent != transparent ||
		batch->viewModel != viewModel )
	{
		if( state.studioBatches.size() >= kMaxStudioBatches )
			return nullptr;
		state.studioBatches.push_back({
			texture, transparent, viewModel,
			uint32_t( state.studioIndices.size() ), 0
		});
		batch = &state.studioBatches.back();
	}
	return batch;
}

void SubmitStudio( int texture, int renderMode, int faceFlags,
	const float *positions, const float *texcoords, const uint8_t *colors,
	int firstVertex, int vertexCount, const uint16_t *indices, int indexCount,
	bool viewModel )
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.studioVertexBuffer || !state.studioIndexBuffer ||
		!positions || !texcoords || !colors || !indices ||
		firstVertex < 0 || vertexCount <= 0 || indexCount < 3 )
		return;
	if( state.studioVertices.size() + size_t( vertexCount ) >
			kMaxStudioVertices ||
		state.studioIndices.size() + size_t( indexCount ) >
			kMaxStudioIndices )
		return;

	StudioBatch *batch =
		GetStudioBatch( texture, IsStudioTransparent( renderMode, faceFlags ),
			viewModel );
	if( !batch )
		return;

	const uint32_t baseVertex = uint32_t( state.studioVertices.size() );
	for( int vertex = 0; vertex < vertexCount; ++vertex )
	{
		const int source = firstVertex + vertex;
		const vec3_t goldPosition = {
			positions[source * 3 + 0],
			positions[source * 3 + 1],
			positions[source * 3 + 2]
		};
		uint32_t color;
		std::memcpy( &color, colors + source * 4, sizeof( color ));
		state.studioVertices.push_back({
			GoldSrcToFilament( goldPosition ),
			{0.0f, 0.0f, 0.0f, 1.0f},
			{texcoords[source * 2 + 0], 1.0f - texcoords[source * 2 + 1]},
			color
		});
	}

	const int lastVertex = firstVertex + vertexCount;
	const size_t firstIndex = state.studioIndices.size();
	for( int index = 0; index + 2 < indexCount; index += 3 )
	{
		const int a = indices[index + 0];
		const int b = indices[index + 1];
		const int c = indices[index + 2];
		if( a < firstVertex || a >= lastVertex ||
			b < firstVertex || b >= lastVertex ||
			c < firstVertex || c >= lastVertex )
			continue;
		state.studioIndices.insert( state.studioIndices.end(), {
			baseVertex + uint32_t( a - firstVertex ),
			baseVertex + uint32_t( b - firstVertex ),
			baseVertex + uint32_t( c - firstVertex )
		});
	}
	batch->indexCount +=
		uint32_t( state.studioIndices.size() - firstIndex );
}

bool CreateStudioResources()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.scene || !state.whiteTexture )
		return false;

	state.studioOpaqueMaterial = Material::Builder()
		.package( kStudioOpaqueMaterial, sizeof( kStudioOpaqueMaterial ))
		.build( *state.engine );
	state.studioTransparentMaterial = Material::Builder()
		.package( kStudioTransparentMaterial, sizeof( kStudioTransparentMaterial ))
		.build( *state.engine );
	state.studioVertexBuffer = VertexBuffer::Builder()
		.vertexCount( uint32_t( kMaxStudioVertices ))
		.bufferCount( 1 )
		.attribute( VertexAttribute::POSITION, 0,
			VertexBuffer::AttributeType::FLOAT3,
			offsetof( StudioVertex, position ), sizeof( StudioVertex ))
		.attribute( VertexAttribute::TANGENTS, 0,
			VertexBuffer::AttributeType::FLOAT4,
			offsetof( StudioVertex, tangents ), sizeof( StudioVertex ))
		.attribute( VertexAttribute::UV0, 0,
			VertexBuffer::AttributeType::FLOAT2,
			offsetof( StudioVertex, uv ), sizeof( StudioVertex ))
		.attribute( VertexAttribute::COLOR, 0,
			VertexBuffer::AttributeType::UBYTE4,
			offsetof( StudioVertex, color ), sizeof( StudioVertex ))
		.normalized( VertexAttribute::COLOR )
		.build( *state.engine );
	state.studioIndexBuffer = IndexBuffer::Builder()
		.indexCount( uint32_t( kMaxStudioIndices ))
		.bufferType( IndexBuffer::IndexType::UINT )
		.build( *state.engine );
	if( !state.studioOpaqueMaterial || !state.studioTransparentMaterial ||
		!state.studioVertexBuffer || !state.studioIndexBuffer )
		return false;

	for( size_t layer = 0; layer < kStudioLayerCount; ++layer )
	{
		state.studioEntities[layer] = EntityManager::get().create();
		RenderableManager::Builder builder( kMaxStudioBatches );
		builder.boundingBox({
				{0.0f, 0.0f, 0.0f},
				{100000.0f, 100000.0f, 100000.0f}
			})
			.culling( false )
			.castShadows( layer == 0 )
			.receiveShadows( layer == 0 )
			.layerMask( 0xff, layer == 0 ? 0x01 : 0x02 )
			.priority( layer == 0 ? 4 : 0 );
		for( size_t primitive = 0; primitive < kMaxStudioBatches; ++primitive )
		{
			MaterialInstance *opaque =
				state.studioOpaqueMaterial->createInstance();
			MaterialInstance *transparent =
				state.studioTransparentMaterial->createInstance();
			if( !opaque || !transparent )
				return false;
			opaque->setParameter( "colorTexture",
				state.whiteTexture, BaseTextureSampler() );
			transparent->setParameter( "colorTexture",
				state.whiteTexture, BaseTextureSampler() );
			state.studioOpaqueInstances[layer][primitive] = opaque;
			state.studioTransparentInstances[layer][primitive] = transparent;
			builder.material( primitive, opaque )
				.geometry( primitive, RenderableManager::PrimitiveType::TRIANGLES,
					state.studioVertexBuffer, state.studioIndexBuffer, 0, 0 )
				.blendOrder( primitive, uint16_t( primitive ))
				.globalBlendOrderEnabled( primitive, true );
		}
		builder.build( *state.engine, state.studioEntities[layer] );
		state.scene->addEntity( state.studioEntities[layer] );
	}
	state.studioVertices.reserve( kMaxStudioVertices );
	state.studioIndices.reserve( kMaxStudioIndices );
	state.studioBatches.reserve( kMaxStudioBatches );
	return true;
}

void DestroyStudioResources()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;
	for( Entity &entity : state.studioEntities )
	{
		if( state.scene && entity )
			state.scene->remove( entity );
		if( entity )
		{
			state.engine->destroy( entity );
			EntityManager::get().destroy( entity );
			entity.clear();
		}
	}
	for( auto &layer : state.studioOpaqueInstances )
	{
		for( MaterialInstance *&instance : layer )
		{
			if( instance )
				state.engine->destroy( instance );
			instance = nullptr;
		}
	}
	for( auto &layer : state.studioTransparentInstances )
	{
		for( MaterialInstance *&instance : layer )
		{
			if( instance )
				state.engine->destroy( instance );
			instance = nullptr;
		}
	}
	if( state.studioOpaqueMaterial )
	{
		state.engine->destroy( state.studioOpaqueMaterial );
		state.studioOpaqueMaterial = nullptr;
	}
	if( state.studioTransparentMaterial )
	{
		state.engine->destroy( state.studioTransparentMaterial );
		state.studioTransparentMaterial = nullptr;
	}
	if( state.studioVertexBuffer )
	{
		state.engine->destroy( state.studioVertexBuffer );
		state.studioVertexBuffer = nullptr;
	}
	if( state.studioIndexBuffer )
	{
		state.engine->destroy( state.studioIndexBuffer );
		state.studioIndexBuffer = nullptr;
	}
	ResetStudioCommands();
	std::fill( std::begin( state.lastStudioBatchCount ),
		std::end( state.lastStudioBatchCount ), 0 );
}

void FlushStudio()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.studioEntities[0] || !state.studioEntities[1] ||
		!state.studioVertexBuffer || !state.studioIndexBuffer )
		return;
	RenderableManager &renderables = state.engine->getRenderableManager();

	if( !state.studioVertices.empty() && !state.studioIndices.empty() )
	{
		const size_t vertexBytes =
			state.studioVertices.size() * sizeof( StudioVertex );
		const size_t indexBytes =
			state.studioIndices.size() * sizeof( uint32_t );
		void *vertexData = std::malloc( vertexBytes );
		void *indexData = std::malloc( indexBytes );
		if( !vertexData || !indexData )
		{
			std::free( vertexData );
			std::free( indexData );
			return;
		}
		std::memcpy( vertexData, state.studioVertices.data(), vertexBytes );
		std::memcpy( indexData, state.studioIndices.data(), indexBytes );
		state.studioVertexBuffer->setBufferAt( *state.engine, 0,
			VertexBuffer::BufferDescriptor(
				vertexData, vertexBytes, ReleaseUploadBuffer ));
		state.studioIndexBuffer->setBuffer( *state.engine,
			IndexBuffer::BufferDescriptor(
				indexData, indexBytes, ReleaseUploadBuffer ));
	}

	size_t layerBatchCounts[kStudioLayerCount] = {};
	for( const StudioBatch &batch : state.studioBatches )
	{
		const size_t layer = batch.viewModel ? 1 : 0;
		const size_t primitive = layerBatchCounts[layer]++;
		if( primitive >= kMaxStudioBatches )
			continue;
		const RenderableManager::Instance renderable =
			renderables.getInstance( state.studioEntities[layer] );
		if( !renderable )
			continue;
		Texture *texture = batch.texture >= 0 && batch.texture < MAX_TEXTURES ?
			state.textures[batch.texture].texture : nullptr;
		MaterialInstance *material = batch.transparent ?
			state.studioTransparentInstances[layer][primitive] :
			state.studioOpaqueInstances[layer][primitive];
		material->setParameter( "colorTexture",
			texture ? texture : state.whiteTexture, BaseTextureSampler() );
		renderables.setMaterialInstanceAt( renderable, primitive, material );
		renderables.setGeometryAt( renderable, primitive,
			RenderableManager::PrimitiveType::TRIANGLES,
			state.studioVertexBuffer, state.studioIndexBuffer,
			batch.indexOffset, batch.indexCount );
		renderables.setBlendOrderAt( renderable, primitive,
			uint16_t( primitive ));
	}
	for( size_t layer = 0; layer < kStudioLayerCount; ++layer )
	{
		const RenderableManager::Instance renderable =
			renderables.getInstance( state.studioEntities[layer] );
		if( !renderable )
			continue;
		for( size_t primitive = layerBatchCounts[layer];
			primitive < state.lastStudioBatchCount[layer]; ++primitive )
		{
			renderables.setGeometryAt( renderable, primitive,
				RenderableManager::PrimitiveType::TRIANGLES,
				state.studioVertexBuffer, state.studioIndexBuffer, 0, 0 );
		}
		state.lastStudioBatchCount[layer] = layerBatchCounts[layer];
	}
}

void ResetSpriteCommands()
{
	FilamentState &state = g_filament;
	state.spriteVertices.clear();
	state.spriteIndices.clear();
	state.spriteBatches.clear();
}

SpriteMaterialKind SelectSpriteMaterial( int renderMode, int textureFormat )
{
	if( renderMode == kRenderTransAdd || renderMode == kRenderGlow ||
		renderMode == kRenderWorldGlow )
		return SPRITE_MATERIAL_ADDITIVE;
	if( textureFormat == SPR_ALPHTEST &&
		( renderMode == kRenderNormal || renderMode == kRenderTransAlpha ))
		return SPRITE_MATERIAL_MASKED;
	return SPRITE_MATERIAL_TRANSPARENT;
}

SpriteBatch *GetSpriteBatch( int texture, SpriteMaterialKind materialKind )
{
	FilamentState &state = g_filament;
	SpriteBatch *batch = state.spriteBatches.empty() ? nullptr :
		&state.spriteBatches.back();
	if( !batch || batch->texture != texture ||
		batch->materialKind != materialKind )
	{
		if( state.spriteBatches.size() >= kMaxSpriteBatches )
			return nullptr;
		state.spriteBatches.push_back({
			texture, materialKind,
			uint32_t( state.spriteIndices.size() ), 0
		});
		batch = &state.spriteBatches.back();
	}
	return batch;
}

void SubmitSprite( int texture, int renderMode, int textureFormat,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a, const float *positions )
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.spriteVertexBuffer ||
		!state.spriteIndexBuffer || !positions ||
		state.spriteVertices.size() + 4 > kMaxSpriteQuads * 4 ||
		state.spriteIndices.size() + 6 > kMaxSpriteQuads * 6 )
		return;
	const SpriteMaterialKind materialKind =
		SelectSpriteMaterial( renderMode, textureFormat );
	SpriteBatch *batch = GetSpriteBatch( texture, materialKind );
	if( !batch )
		return;

	const uint32_t color = uint32_t( r ) | ( uint32_t( g ) << 8u ) |
		( uint32_t( b ) << 16u ) | ( uint32_t( a ) << 24u );
	static constexpr float2 kUvs[4] = {
		{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}
	};
	const uint32_t baseVertex = uint32_t( state.spriteVertices.size() );
	for( uint32_t vertex = 0; vertex < 4; ++vertex )
	{
		const vec3_t goldPosition = {
			positions[vertex * 3 + 0],
			positions[vertex * 3 + 1],
			positions[vertex * 3 + 2]
		};
		state.spriteVertices.push_back({
			GoldSrcToFilament( goldPosition ), kUvs[vertex], color
		});
	}
	state.spriteIndices.insert( state.spriteIndices.end(), {
		baseVertex, baseVertex + 1, baseVertex + 2,
		baseVertex, baseVertex + 2, baseVertex + 3
	});
	batch->indexCount += 6;
}

bool CreateSpriteResources()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.scene || !state.whiteTexture )
		return false;
	state.spriteMaterials[SPRITE_MATERIAL_MASKED] = Material::Builder()
		.package( kSpriteMaskedMaterial, sizeof( kSpriteMaskedMaterial ))
		.build( *state.engine );
	state.spriteMaterials[SPRITE_MATERIAL_TRANSPARENT] = Material::Builder()
		.package( kSpriteTransparentMaterial, sizeof( kSpriteTransparentMaterial ))
		.build( *state.engine );
	state.spriteMaterials[SPRITE_MATERIAL_ADDITIVE] = Material::Builder()
		.package( kSpriteAdditiveMaterial, sizeof( kSpriteAdditiveMaterial ))
		.build( *state.engine );
	state.spriteVertexBuffer = VertexBuffer::Builder()
		.vertexCount( uint32_t( kMaxSpriteQuads * 4 ))
		.bufferCount( 1 )
		.attribute( VertexAttribute::POSITION, 0,
			VertexBuffer::AttributeType::FLOAT3,
			offsetof( SpriteVertex, position ), sizeof( SpriteVertex ))
		.attribute( VertexAttribute::UV0, 0,
			VertexBuffer::AttributeType::FLOAT2,
			offsetof( SpriteVertex, uv ), sizeof( SpriteVertex ))
		.attribute( VertexAttribute::COLOR, 0,
			VertexBuffer::AttributeType::UBYTE4,
			offsetof( SpriteVertex, color ), sizeof( SpriteVertex ))
		.normalized( VertexAttribute::COLOR )
		.build( *state.engine );
	state.spriteIndexBuffer = IndexBuffer::Builder()
		.indexCount( uint32_t( kMaxSpriteQuads * 6 ))
		.bufferType( IndexBuffer::IndexType::UINT )
		.build( *state.engine );
	if( !state.spriteMaterials[0] || !state.spriteMaterials[1] ||
		!state.spriteMaterials[2] || !state.spriteVertexBuffer ||
		!state.spriteIndexBuffer )
		return false;

	state.spriteEntity = EntityManager::get().create();
	RenderableManager::Builder builder( kMaxSpriteBatches );
	builder.boundingBox({
			{0.0f, 0.0f, 0.0f},
			{100000.0f, 100000.0f, 100000.0f}
		})
		.culling( false )
		.castShadows( false )
		.receiveShadows( false )
		.priority( 5 );
	for( size_t primitive = 0; primitive < kMaxSpriteBatches; ++primitive )
	{
		for( size_t kind = 0; kind < kSpriteMaterialCount; ++kind )
		{
			MaterialInstance *instance =
				state.spriteMaterials[kind]->createInstance();
			if( !instance )
				return false;
			instance->setParameter( "colorTexture",
				state.whiteTexture, BaseTextureSampler() );
			state.spriteInstances[kind][primitive] = instance;
		}
		builder.material( primitive,
				state.spriteInstances[SPRITE_MATERIAL_TRANSPARENT][primitive] )
			.geometry( primitive, RenderableManager::PrimitiveType::TRIANGLES,
				state.spriteVertexBuffer, state.spriteIndexBuffer, 0, 0 )
			.blendOrder( primitive, uint16_t( primitive ))
			.globalBlendOrderEnabled( primitive, true );
	}
	builder.build( *state.engine, state.spriteEntity );
	state.scene->addEntity( state.spriteEntity );
	state.spriteVertices.reserve( kMaxSpriteQuads * 4 );
	state.spriteIndices.reserve( kMaxSpriteQuads * 6 );
	state.spriteBatches.reserve( kMaxSpriteBatches );
	return true;
}

void DestroySpriteResources()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;
	if( state.scene && state.spriteEntity )
		state.scene->remove( state.spriteEntity );
	if( state.spriteEntity )
	{
		state.engine->destroy( state.spriteEntity );
		EntityManager::get().destroy( state.spriteEntity );
		state.spriteEntity.clear();
	}
	for( size_t kind = 0; kind < kSpriteMaterialCount; ++kind )
	{
		for( MaterialInstance *&instance : state.spriteInstances[kind] )
		{
			if( instance )
				state.engine->destroy( instance );
			instance = nullptr;
		}
		if( state.spriteMaterials[kind] )
		{
			state.engine->destroy( state.spriteMaterials[kind] );
			state.spriteMaterials[kind] = nullptr;
		}
	}
	if( state.spriteVertexBuffer )
	{
		state.engine->destroy( state.spriteVertexBuffer );
		state.spriteVertexBuffer = nullptr;
	}
	if( state.spriteIndexBuffer )
	{
		state.engine->destroy( state.spriteIndexBuffer );
		state.spriteIndexBuffer = nullptr;
	}
	ResetSpriteCommands();
	state.lastSpriteBatchCount = 0;
}

void FlushSprites()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.spriteEntity ||
		!state.spriteVertexBuffer || !state.spriteIndexBuffer )
		return;
	RenderableManager &renderables = state.engine->getRenderableManager();
	const RenderableManager::Instance renderable =
		renderables.getInstance( state.spriteEntity );
	if( !renderable )
		return;

	if( !state.spriteVertices.empty() && !state.spriteIndices.empty() )
	{
		const size_t vertexBytes =
			state.spriteVertices.size() * sizeof( SpriteVertex );
		const size_t indexBytes =
			state.spriteIndices.size() * sizeof( uint32_t );
		void *vertexData = std::malloc( vertexBytes );
		void *indexData = std::malloc( indexBytes );
		if( !vertexData || !indexData )
		{
			std::free( vertexData );
			std::free( indexData );
			return;
		}
		std::memcpy( vertexData, state.spriteVertices.data(), vertexBytes );
		std::memcpy( indexData, state.spriteIndices.data(), indexBytes );
		state.spriteVertexBuffer->setBufferAt( *state.engine, 0,
			VertexBuffer::BufferDescriptor(
				vertexData, vertexBytes, ReleaseUploadBuffer ));
		state.spriteIndexBuffer->setBuffer( *state.engine,
			IndexBuffer::BufferDescriptor(
				indexData, indexBytes, ReleaseUploadBuffer ));
	}

	for( size_t batchIndex = 0;
		batchIndex < state.spriteBatches.size(); ++batchIndex )
	{
		const SpriteBatch &batch = state.spriteBatches[batchIndex];
		Texture *texture = batch.texture >= 0 && batch.texture < MAX_TEXTURES ?
			state.textures[batch.texture].texture : nullptr;
		MaterialInstance *material =
			state.spriteInstances[batch.materialKind][batchIndex];
		material->setParameter( "colorTexture",
			texture ? texture : state.whiteTexture, BaseTextureSampler() );
		renderables.setMaterialInstanceAt( renderable, batchIndex, material );
		renderables.setGeometryAt( renderable, batchIndex,
			RenderableManager::PrimitiveType::TRIANGLES,
			state.spriteVertexBuffer, state.spriteIndexBuffer,
			batch.indexOffset, batch.indexCount );
		renderables.setBlendOrderAt( renderable, batchIndex,
			uint16_t( batchIndex ));
	}
	for( size_t batchIndex = state.spriteBatches.size();
		batchIndex < state.lastSpriteBatchCount; ++batchIndex )
	{
		renderables.setGeometryAt( renderable, batchIndex,
			RenderableManager::PrimitiveType::TRIANGLES,
			state.spriteVertexBuffer, state.spriteIndexBuffer, 0, 0 );
	}
	state.lastSpriteBatchCount = state.spriteBatches.size();
}

void RebindTexture( int textureId, Texture *texture )
{
	FilamentState &state = g_filament;
	for( WorldBatch &batch : state.worldBatches )
	{
		if( batch.baseTexture == textureId )
			batch.material->setParameter( "baseColor",
				texture ? texture : state.whiteTexture, BaseTextureSampler() );
		if( batch.lightmapTexture == textureId )
			batch.material->setParameter( "lightmap",
				texture ? texture : state.whiteTexture, LightmapTextureSampler() );
	}
	for( WorldBatch &batch : state.skyBatches )
	{
		if( batch.baseTexture == textureId )
			batch.material->setParameter( "baseColor",
				texture ? texture : state.whiteTexture, BaseTextureSampler() );
	}
	for( auto &entry : state.brushInstances )
	{
		for( WorldBatch &batch : entry.second.batches )
		{
			if( batch.baseTexture == textureId )
				batch.material->setParameter( "baseColor",
					texture ? texture : state.whiteTexture, BaseTextureSampler() );
			if( batch.lightmapTexture == textureId )
				batch.material->setParameter( "lightmap",
					texture ? texture : state.whiteTexture, LightmapTextureSampler() );
		}
	}
}

uint8_t *ConvertPixelsToRgba( int width, int height, int pixelFormat,
	const void *pixels )
{
	if( width <= 0 || height <= 0 || !pixels )
		return nullptr;

	const size_t pixelCount = size_t( width ) * size_t( height );
	uint8_t *rgba = static_cast<uint8_t *>( std::malloc( pixelCount * 4 ));
	if( !rgba )
		return nullptr;

	const uint8_t *source = static_cast<const uint8_t *>( pixels );
	for( size_t i = 0; i < pixelCount; ++i )
	{
		switch( pixelFormat )
		{
		case GL_RGBA:
			std::memcpy( rgba + i * 4, source + i * 4, 4 );
			break;
		case GL_BGRA:
			rgba[i * 4 + 0] = source[i * 4 + 2];
			rgba[i * 4 + 1] = source[i * 4 + 1];
			rgba[i * 4 + 2] = source[i * 4 + 0];
			rgba[i * 4 + 3] = source[i * 4 + 3];
			break;
		case GL_RGB:
			rgba[i * 4 + 0] = source[i * 3 + 0];
			rgba[i * 4 + 1] = source[i * 3 + 1];
			rgba[i * 4 + 2] = source[i * 3 + 2];
			rgba[i * 4 + 3] = 255;
			break;
		case GL_BGR:
			rgba[i * 4 + 0] = source[i * 3 + 2];
			rgba[i * 4 + 1] = source[i * 3 + 1];
			rgba[i * 4 + 2] = source[i * 3 + 0];
			rgba[i * 4 + 3] = 255;
			break;
		case GL_LUMINANCE_ALPHA:
			rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = source[i * 2];
			rgba[i * 4 + 3] = source[i * 2 + 1];
			break;
		case 0x1906: // GL_ALPHA is omitted by the engine's core-profile shim.
			rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = 255;
			rgba[i * 4 + 3] = source[i];
			break;
		case GL_LUMINANCE:
		default:
			rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = source[i];
			rgba[i * 4 + 3] = 255;
			break;
		}
	}
	return rgba;
}

void UploadTexture( int textureId, int width, int height, int pixelFormat,
	const void *pixels, bool srgb )
{
	FilamentState &state = g_filament;
	if( !state.engine || textureId < 0 || textureId >= MAX_TEXTURES ||
		width <= 0 || height <= 0 || !pixels )
		return;
	FinishRenderedFrameBeforeResourceMutation();

	const size_t pixelCount = size_t( width ) * size_t( height );
	uint8_t *rgba = ConvertPixelsToRgba( width, height, pixelFormat, pixels );
	if( !rgba )
		return;

	TextureSlot &slot = state.textures[textureId];
	if( !slot.texture || slot.width != uint32_t( width ) ||
		slot.height != uint32_t( height ) || slot.srgb != srgb )
	{
		Texture *replacement = Texture::Builder()
			.width( uint32_t( width ))
			.height( uint32_t( height ))
			.levels( 1 )
			.sampler( Texture::Sampler::SAMPLER_2D )
			.format( srgb ? Texture::InternalFormat::SRGB8_A8 :
				Texture::InternalFormat::RGBA8 )
			.build( *state.engine );
		Texture *oldTexture = slot.texture;
		slot.texture = replacement;
		slot.width = uint32_t( width );
		slot.height = uint32_t( height );
		slot.srgb = srgb;
		RebindTexture( textureId, replacement );
		if( oldTexture )
			state.engine->destroy( oldTexture );
	}

	if( slot.texture )
	{
		slot.texture->setImage( *state.engine, 0,
			Texture::PixelBufferDescriptor( rgba, pixelCount * 4,
				Texture::Format::RGBA, Texture::Type::UBYTE,
				ReleaseUploadBuffer ));
	}
	else
	{
		std::free( rgba );
	}
}

void FreeTexture( int textureId )
{
	FilamentState &state = g_filament;
	if( !state.engine || textureId < 0 || textureId >= MAX_TEXTURES )
		return;
	FinishRenderedFrameBeforeResourceMutation();

	TextureSlot &slot = state.textures[textureId];
	if( !slot.texture )
		return;
	RebindTexture( textureId, nullptr );
	state.engine->destroy( slot.texture );
	slot = {};
}

void DestroySkyGeometry()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;
	for( Entity &entity : state.skyEntities )
	{
		if( state.scene && entity )
			state.scene->remove( entity );
		if( entity )
		{
			state.engine->destroy( entity );
			EntityManager::get().destroy( entity );
			entity.clear();
		}
	}
	for( WorldBatch &batch : state.skyBatches )
	{
		if( batch.material )
			state.engine->destroy( batch.material );
	}
	state.skyBatches.clear();
	for( Texture *&texture : state.skyFaceTextures )
	{
		if( texture )
			state.engine->destroy( texture );
		texture = nullptr;
	}
	if( state.skyVertexBuffer )
	{
		state.engine->destroy( state.skyVertexBuffer );
		state.skyVertexBuffer = nullptr;
	}
	if( state.skyIndexBuffer )
	{
		state.engine->destroy( state.skyIndexBuffer );
		state.skyIndexBuffer = nullptr;
	}
}

float3 GoldSrcSkyPosition( float s, float t, int axis )
{
	static constexpr int kStToVec[6][3] = {
		{ 3,-1, 2 }, {-3, 1, 2 }, { 1, 3, 2 },
		{-1,-3, 2 }, {-2,-1, 3 }, { 2,-1,-3 }
	};
	const float b[3] = {s, t, 1.0f};
	vec3_t goldPosition;
	for( int component = 0; component < 3; ++component )
	{
		const int source = kStToVec[axis][component];
		goldPosition[component] =
			source < 0 ? -b[-source - 1] : b[source - 1];
	}
	return GoldSrcToFilament( goldPosition );
}

void BuildSkyGeometry()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.scene || !state.skyMaterial || !state.whiteTexture )
		return;

	static constexpr int kGoldSrcTextureOrder[6] = { 0, 2, 1, 3, 4, 5 };
	static constexpr float kCorners[4][2] = {
		{-1.0f,-1.0f}, {-1.0f, 1.0f},
		{ 1.0f, 1.0f}, { 1.0f,-1.0f}
	};
	std::vector<WorldVertex> vertices;
	std::vector<uint32_t> indices;
	vertices.reserve( 24 );
	indices.reserve( 36 );
	for( int axis = 0; axis < 6; ++axis )
	{
		for( const auto &corner : kCorners )
		{
			const float s = corner[0];
			const float t = corner[1];
			WorldVertex vertex;
			vertex.position = GoldSrcSkyPosition( s, t, axis );
			vertex.tangents = {0.0f, 0.0f, 0.0f, 1.0f};
			// Metal's texture origin is opposite to the legacy OpenGL sky upload.
			// The generic BSP UVs already account for their source convention, but
			// sky t is explicitly inverted by MakeSkyVec in the GL renderer.
			vertex.uv0 = {( s + 1.0f ) * 0.5f, ( t + 1.0f ) * 0.5f};
			vertex.uv1 = {0.5f, 0.5f};
			vertices.push_back( vertex );
		}
		const uint32_t base = uint32_t( axis * 4 );
		indices.insert( indices.end(), {
			base, base + 2, base + 1, base, base + 3, base + 2
		});
	}

	DestroySkyGeometry();
	state.skyVertexBuffer = VertexBuffer::Builder()
		.vertexCount( uint32_t( vertices.size() ))
		.bufferCount( 1 )
		.attribute( VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3,
			offsetof( WorldVertex, position ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::FLOAT4,
			offsetof( WorldVertex, tangents ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( WorldVertex, uv0 ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::UV1, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( WorldVertex, uv1 ), sizeof( WorldVertex ))
		.build( *state.engine );
	state.skyIndexBuffer = IndexBuffer::Builder()
		.indexCount( uint32_t( indices.size() ))
		.bufferType( IndexBuffer::IndexType::UINT )
		.build( *state.engine );
	void *vertexData = std::malloc( vertices.size() * sizeof( WorldVertex ));
	void *indexData = std::malloc( indices.size() * sizeof( uint32_t ));
	if( !state.skyVertexBuffer || !state.skyIndexBuffer || !vertexData || !indexData )
	{
		std::free( vertexData );
		std::free( indexData );
		DestroySkyGeometry();
		return;
	}
	std::memcpy( vertexData, vertices.data(), vertices.size() * sizeof( WorldVertex ));
	std::memcpy( indexData, indices.data(), indices.size() * sizeof( uint32_t ));
	state.skyVertexBuffer->setBufferAt( *state.engine, 0,
		VertexBuffer::BufferDescriptor( vertexData,
			vertices.size() * sizeof( WorldVertex ), ReleaseUploadBuffer ));
	state.skyIndexBuffer->setBuffer( *state.engine,
		IndexBuffer::BufferDescriptor( indexData,
			indices.size() * sizeof( uint32_t ), ReleaseUploadBuffer ));
	for( int side = 0; side < 6; ++side )
	{
		const SkyFace &face = state.skyFaces[side];
		if( !face.width || !face.height || face.rgba.empty() )
			continue;
		Texture *texture = Texture::Builder()
			.width( face.width )
			.height( face.height )
			.levels( 1 )
			.sampler( Texture::Sampler::SAMPLER_2D )
			.format( Texture::InternalFormat::SRGB8_A8 )
			.build( *state.engine );
		uint8_t *upload = static_cast<uint8_t *>(
			std::malloc( face.rgba.size() ));
		if( !texture || !upload )
		{
			if( texture )
				state.engine->destroy( texture );
			std::free( upload );
			continue;
		}
		std::memcpy( upload, face.rgba.data(), face.rgba.size() );
		texture->setImage( *state.engine, 0,
			Texture::PixelBufferDescriptor( upload, face.rgba.size(),
				Texture::Format::RGBA, Texture::Type::UBYTE,
				ReleaseUploadBuffer ));
		state.skyFaceTextures[side] = texture;
	}

	state.skyBatches.reserve( 6 );
	for( int axis = 0; axis < 6; ++axis )
	{
		const int side = kGoldSrcTextureOrder[axis];
		const int textureId = state.skyFaces[side].textureId;
		Texture *texture = state.skyFaceTextures[side];
		MaterialInstance *material = state.skyMaterial->createInstance();
		material->setDoubleSided( true );
		material->setParameter( "baseColor",
			texture ? texture : state.whiteTexture, BaseTextureSampler() );
		state.skyEntities[axis] = EntityManager::get().create();
		RenderableManager::Builder builder( 1 );
		builder.boundingBox({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}})
			.culling( false )
			.castShadows( false )
			.receiveShadows( false )
			.priority( 0 )
			.material( 0, material )
			.geometry( 0, RenderableManager::PrimitiveType::TRIANGLES,
				state.skyVertexBuffer, state.skyIndexBuffer, axis * 6, 6 )
			.build( *state.engine, state.skyEntities[axis] );
		state.scene->addEntity( state.skyEntities[axis] );
		state.skyBatches.push_back({material, textureId, -1});
	}
}

void DestroySkyboxResources( bool createFallback )
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;
	if( state.scene )
		state.scene->setSkybox( nullptr );
	if( state.skybox )
	{
		state.engine->destroy( state.skybox );
		state.skybox = nullptr;
	}
	if( state.skyboxTexture )
	{
		state.engine->destroy( state.skyboxTexture );
		state.skyboxTexture = nullptr;
	}
	if( createFallback )
	{
		state.skybox = Skybox::Builder()
			.color({ 0.012f, 0.018f, 0.035f, 1.0f })
			.build( *state.engine );
		if( state.scene )
			state.scene->setSkybox( state.skybox );
	}
}

void BuildSkyboxTexture()
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.scene )
		return;

	const uint32_t width = state.skyFaces[0].width;
	const uint32_t height = state.skyFaces[0].height;
	if( !width || !height )
		return;
	for( const SkyFace &face : state.skyFaces )
	{
		if( face.width != width || face.height != height ||
			face.rgba.size() != size_t( width ) * size_t( height ) * 4 )
			return;
	}

	const size_t faceBytes = size_t( width ) * size_t( height ) * 4;
	uint8_t minimum = 255, maximum = 0;
	uint64_t sum = 0;
	for( const SkyFace &face : state.skyFaces )
	{
		for( uint8_t channel : face.rgba )
		{
			minimum = std::min( minimum, channel );
			maximum = std::max( maximum, channel );
			sum += channel;
		}
	}

	// Filament's Skybox path assumes a prefiltered linear cubemap. GoldSrc
	// provides six ordinary sRGB images, so render those images on a
	// camera-centred cube instead. This also preserves the original sky face
	// orientation without baking another runtime texture.
	// The built-in solid skybox is only a missing-assets fallback. Leaving it
	// attached here causes it to replace the camera-centred GoldSrc cube in
	// pixels where that cube intentionally does not write depth.
	DestroySkyboxResources( false );
	BuildSkyGeometry();
	MsgDev( D_INFO,
		"Filament: uploaded %ux%u GoldSrc skybox (range %u..%u, mean %.1f)\n",
		width, height, unsigned( minimum ), unsigned( maximum ),
		double( sum ) / double( faceBytes * 6 ));
}

void UploadSkyTexture( int side, int textureId, int width, int height,
	int pixelFormat, const void *pixels )
{
	FilamentState &state = g_filament;
	if( !state.engine || side < 0 || side >= 6 ||
		width <= 0 || height <= 0 || !pixels )
		return;
	FinishRenderedFrameBeforeResourceMutation();
	uint8_t *rgba = ConvertPixelsToRgba( width, height, pixelFormat, pixels );
	if( !rgba )
		return;
	SkyFace &face = state.skyFaces[side];
	face.width = uint32_t( width );
	face.height = uint32_t( height );
	face.textureId = textureId;
	face.rgba.assign( rgba, rgba + size_t( width ) * size_t( height ) * 4 );
	std::free( rgba );
	BuildSkyboxTexture();
}

void ClearSkyTextures()
{
	FilamentState &state = g_filament;
	FinishRenderedFrameBeforeResourceMutation();
	DestroySkyGeometry();
	for( SkyFace &face : state.skyFaces )
		face = {};
	if( state.engine )
		DestroySkyboxResources( true );
}

void DestroyBrushResources()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;

	for( auto &entry : state.brushInstances )
	{
		BrushInstance &instance = entry.second;
		if( instance.inScene && state.scene )
			state.scene->remove( instance.entity );
		if( instance.entity )
		{
			state.engine->destroy( instance.entity );
			EntityManager::get().destroy( instance.entity );
		}
		for( WorldBatch &batch : instance.batches )
		{
			if( batch.material )
				state.engine->destroy( batch.material );
		}
	}
	state.brushInstances.clear();

	for( auto &entry : state.brushMeshes )
	{
		BrushMesh &mesh = entry.second;
		if( mesh.vertexBuffer )
			state.engine->destroy( mesh.vertexBuffer );
		if( mesh.indexBuffer )
			state.engine->destroy( mesh.indexBuffer );
	}
	state.brushMeshes.clear();
}

void DestroyBrushInstance( BrushInstance &instance )
{
	FilamentState &state = g_filament;
	if( instance.inScene && state.scene )
		state.scene->remove( instance.entity );
	if( instance.entity )
	{
		state.engine->destroy( instance.entity );
		EntityManager::get().destroy( instance.entity );
		instance.entity.clear();
	}
	for( WorldBatch &batch : instance.batches )
	{
		if( batch.material )
			state.engine->destroy( batch.material );
	}
	instance.batches.clear();
}

void DestroyWorld()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;

	DestroyBrushResources();
	if( state.scene && state.worldShadowReceiverEntity )
		state.scene->remove( state.worldShadowReceiverEntity );
	if( state.worldShadowReceiverEntity )
	{
		state.engine->destroy( state.worldShadowReceiverEntity );
		EntityManager::get().destroy( state.worldShadowReceiverEntity );
		state.worldShadowReceiverEntity.clear();
	}
	if( state.worldShadowReceiverInstance )
	{
		state.engine->destroy( state.worldShadowReceiverInstance );
		state.worldShadowReceiverInstance = nullptr;
	}
	if( state.scene && state.worldEntity )
		state.scene->remove( state.worldEntity );
	if( state.worldEntity )
	{
		state.engine->destroy( state.worldEntity );
		EntityManager::get().destroy( state.worldEntity );
		state.worldEntity.clear();
	}
	for( WorldBatch &batch : state.worldBatches )
	{
		if( batch.material )
			state.engine->destroy( batch.material );
	}
	state.worldBatches.clear();
	if( state.worldVertexBuffer )
	{
		state.engine->destroy( state.worldVertexBuffer );
		state.worldVertexBuffer = nullptr;
	}
	if( state.worldIndexBuffer )
	{
		state.engine->destroy( state.worldIndexBuffer );
		state.worldIndexBuffer = nullptr;
	}
}

struct CpuWorldBatch
{
	std::vector<WorldVertex> vertices;
	std::vector<uint32_t> indices;
};

struct BatchRange
{
	int baseTexture;
	int lightmapTexture;
	uint32_t offset;
	uint32_t count;
};

void BuildWorld( model_t *worldModel )
{
	FilamentState &state = g_filament;
	DestroyWorld();
	if( !state.engine || !state.worldMaterial || !worldModel ||
		worldModel->type != mod_brush || !worldModel->surfaces )
		return;

	const int lightmapBlockSize = R_BackendLightmapBlockSize();
	const int lightmapSampleSize = R_BackendLightmapSampleSize();
	std::map<std::pair<int, int>, CpuWorldBatch> cpuBatches;

	const int firstSurface = std::max( 0, worldModel->firstmodelsurface );
	const int surfaceEnd = std::min( worldModel->numsurfaces,
		firstSurface + worldModel->nummodelsurfaces );
	for( int surfaceIndex = firstSurface; surfaceIndex < surfaceEnd; ++surfaceIndex )
	{
		msurface_t *surface = worldModel->surfaces + surfaceIndex;
		if( surface->numedges < 3 || !surface->texinfo || !surface->texinfo->texture )
			continue;
		if( surface->flags & SURF_DRAWSKY )
			continue;

		texture_t *baseTexture = surface->texinfo->texture;
		const int baseTextureId = baseTexture->gl_texturenum;
		const bool hasLightmap = worldModel->lightdata &&
			!( surface->flags & SURF_DRAWTILED ) &&
			surface->lightmaptexturenum >= 0;
		const int lightmapTextureId = hasLightmap ?
			R_BackendLightmapTexture( surface->lightmaptexturenum ) : -1;
		CpuWorldBatch &batch = cpuBatches[{baseTextureId, lightmapTextureId}];
		const uint32_t firstVertex = uint32_t( batch.vertices.size() );
		vec3_t goldNormal;
		VectorCopy( surface->plane->normal, goldNormal );
		if( surface->flags & SURF_PLANEBACK )
			VectorNegate( goldNormal, goldNormal );
		const float3 normal = normalize( GoldSrcToFilament( goldNormal ));
		const quatf orientation = quatf::fromDirectedRotation(
			float3{0.0f, 0.0f, 1.0f}, normal );

		for( int edgeIndex = 0; edgeIndex < surface->numedges; ++edgeIndex )
		{
			const int surfEdge = worldModel->surfedges[surface->firstedge + edgeIndex];
			const medge_t &edge = worldModel->edges[surfEdge >= 0 ? surfEdge : -surfEdge];
			const mvertex_t &sourceVertex =
				worldModel->vertexes[surfEdge >= 0 ? edge.v[0] : edge.v[1]];
			const float *position = sourceVertex.position;
			const float rawS = position[0] * surface->texinfo->vecs[0][0] +
				position[1] * surface->texinfo->vecs[0][1] +
				position[2] * surface->texinfo->vecs[0][2] +
				surface->texinfo->vecs[0][3];
			const float rawT = position[0] * surface->texinfo->vecs[1][0] +
				position[1] * surface->texinfo->vecs[1][1] +
				position[2] * surface->texinfo->vecs[1][2] +
				surface->texinfo->vecs[1][3];

			WorldVertex vertex;
			vertex.position = GoldSrcToFilament( sourceVertex.position );
			vertex.tangents = orientation.xyzw;
			vertex.uv0 = {
				rawS / float( baseTexture->width ? baseTexture->width : 1 ),
				1.0f - rawT /
					float( baseTexture->height ? baseTexture->height : 1 )
			};
			if( hasLightmap && lightmapBlockSize > 0 && lightmapSampleSize > 0 )
			{
				vertex.uv1 = {
					(rawS - surface->texturemins[0] +
						surface->light_s * lightmapSampleSize + lightmapSampleSize * 0.5f) /
						float( lightmapBlockSize * lightmapSampleSize ),
					1.0f - (rawT - surface->texturemins[1] +
						surface->light_t * lightmapSampleSize + lightmapSampleSize * 0.5f) /
						float( lightmapBlockSize * lightmapSampleSize )
				};
			}
			else
			{
				vertex.uv1 = {0.5f, 0.5f};
			}
			batch.vertices.push_back( vertex );
		}

		for( int triangle = 0; triangle < surface->numedges - 2; ++triangle )
		{
			batch.indices.push_back( firstVertex );
			batch.indices.push_back( firstVertex + uint32_t( triangle + 1 ));
			batch.indices.push_back( firstVertex + uint32_t( triangle + 2 ));
		}
	}

	std::vector<WorldVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<BatchRange> ranges;
	float3 boundsMin = { 1.0e30f, 1.0e30f, 1.0e30f };
	float3 boundsMax = {-1.0e30f,-1.0e30f,-1.0e30f };

	for( auto &entry : cpuBatches )
	{
		CpuWorldBatch &batch = entry.second;
		const uint32_t vertexOffset = uint32_t( vertices.size() );
		const uint32_t indexOffset = uint32_t( indices.size() );
		for( const WorldVertex &vertex : batch.vertices )
		{
			vertices.push_back( vertex );
			for( int axis = 0; axis < 3; ++axis )
			{
				if( vertex.position[axis] < boundsMin[axis] )
					boundsMin[axis] = vertex.position[axis];
				if( vertex.position[axis] > boundsMax[axis] )
					boundsMax[axis] = vertex.position[axis];
			}
		}
		for( uint32_t index : batch.indices )
			indices.push_back( vertexOffset + index );
		ranges.push_back({
			entry.first.first, entry.first.second, indexOffset,
			uint32_t( batch.indices.size() )
		});
	}

	if( vertices.empty() || indices.empty() || ranges.empty() )
	{
		MsgDev( D_WARN, "Filament: map %s produced no renderable BSP geometry\n",
			worldModel->name );
		return;
	}

	state.worldVertexBuffer = VertexBuffer::Builder()
		.vertexCount( uint32_t( vertices.size() ))
		.bufferCount( 1 )
		.attribute( VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3,
			offsetof( WorldVertex, position ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::FLOAT4,
			offsetof( WorldVertex, tangents ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( WorldVertex, uv0 ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::UV1, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( WorldVertex, uv1 ), sizeof( WorldVertex ))
		.build( *state.engine );
	state.worldIndexBuffer = IndexBuffer::Builder()
		.indexCount( uint32_t( indices.size() ))
		.bufferType( IndexBuffer::IndexType::UINT )
		.build( *state.engine );

	void *vertexData = std::malloc( vertices.size() * sizeof( WorldVertex ));
	void *indexData = std::malloc( indices.size() * sizeof( uint32_t ));
	if( !vertexData || !indexData )
	{
		std::free( vertexData );
		std::free( indexData );
		DestroyWorld();
		return;
	}
	std::memcpy( vertexData, vertices.data(), vertices.size() * sizeof( WorldVertex ));
	std::memcpy( indexData, indices.data(), indices.size() * sizeof( uint32_t ));
	state.worldVertexBuffer->setBufferAt( *state.engine, 0,
		VertexBuffer::BufferDescriptor( vertexData,
			vertices.size() * sizeof( WorldVertex ), ReleaseUploadBuffer ));
	state.worldIndexBuffer->setBuffer( *state.engine,
		IndexBuffer::BufferDescriptor( indexData,
			indices.size() * sizeof( uint32_t ), ReleaseUploadBuffer ));

	state.worldEntity = EntityManager::get().create();
	RenderableManager::Builder builder( ranges.size() );
	const float3 center = ( boundsMin + boundsMax ) * 0.5f;
	const float3 halfExtent = ( boundsMax - boundsMin ) * 0.5f;
	builder.boundingBox({ center, halfExtent })
		.culling( true )
		// Static lighting and occlusion are already baked into the BSP
		// lightmaps. Only dynamic models and brush entities enter the real-time
		// shadow map; a transparent duplicate below receives their silhouettes.
		.castShadows( false )
		.receiveShadows( true );

	state.worldBatches.reserve( ranges.size() );
	for( size_t primitive = 0; primitive < ranges.size(); ++primitive )
	{
		const BatchRange &range = ranges[primitive];
		MaterialInstance *instance = state.worldMaterial->createInstance();
		Texture *base = range.baseTexture >= 0 && range.baseTexture < MAX_TEXTURES ?
			state.textures[range.baseTexture].texture : nullptr;
		Texture *lightmap = range.lightmapTexture >= 0 &&
			range.lightmapTexture < MAX_TEXTURES ?
			state.textures[range.lightmapTexture].texture : nullptr;
		instance->setParameter( "baseColor",
			base ? base : state.whiteTexture,
			BaseTextureSampler() );
		instance->setParameter( "lightmap",
			lightmap ? lightmap : state.whiteTexture,
			LightmapTextureSampler() );
		builder.material( primitive, instance )
			.geometry( primitive, RenderableManager::PrimitiveType::TRIANGLES,
				state.worldVertexBuffer, state.worldIndexBuffer,
				range.offset, range.count );
		state.worldBatches.push_back({
			instance, range.baseTexture, range.lightmapTexture
		});
	}
	builder.build( *state.engine, state.worldEntity );
	state.scene->addEntity( state.worldEntity );

	if( state.worldShadowReceiverMaterial )
	{
		state.worldShadowReceiverInstance =
			state.worldShadowReceiverMaterial->createInstance();
		state.worldShadowReceiverInstance->setParameter( "opacity", 0.62f );
		state.worldShadowReceiverEntity = EntityManager::get().create();
		RenderableManager::Builder( 1 )
			.boundingBox({ center, halfExtent })
			.material( 0, state.worldShadowReceiverInstance )
			.geometry( 0, RenderableManager::PrimitiveType::TRIANGLES,
				state.worldVertexBuffer, state.worldIndexBuffer, 0, indices.size() )
			.culling( true )
			.castShadows( false )
			.receiveShadows( true )
			.priority( 1 )
			.build( *state.engine, state.worldShadowReceiverEntity );
		state.scene->addEntity( state.worldShadowReceiverEntity );
	}

	Msg(
		"Filament: built BSP %s (%u vertices, %u triangles, %u batches)\n",
		worldModel->name, uint32_t( vertices.size() ),
		uint32_t( indices.size() / 3 ), uint32_t( ranges.size() ));
}

BrushMesh *BuildBrushMesh( model_t *model )
{
	FilamentState &state = g_filament;
	if( !state.engine || !model || model->type != mod_brush ||
		model == cl.worldmodel || !model->surfaces )
		return nullptr;

	auto existing = state.brushMeshes.find( model );
	if( existing != state.brushMeshes.end() )
		return &existing->second;

	const int lightmapBlockSize = R_BackendLightmapBlockSize();
	const int lightmapSampleSize = R_BackendLightmapSampleSize();
	std::map<std::pair<int, int>, CpuWorldBatch> cpuBatches;
	const int firstSurface = std::max( 0, model->firstmodelsurface );
	const int surfaceEnd = std::min( model->numsurfaces,
		firstSurface + model->nummodelsurfaces );

	for( int surfaceIndex = firstSurface; surfaceIndex < surfaceEnd; ++surfaceIndex )
	{
		msurface_t *surface = model->surfaces + surfaceIndex;
		if( surface->numedges < 3 || !surface->texinfo ||
			!surface->texinfo->texture || surface->flags & SURF_DRAWSKY )
			continue;

		texture_t *baseTexture = surface->texinfo->texture;
		const int baseTextureId = baseTexture->gl_texturenum;
		const bool hasLightmap = model->lightdata &&
			!( surface->flags & SURF_DRAWTILED ) &&
			surface->lightmaptexturenum >= 0;
		const int lightmapTextureId = hasLightmap ?
			R_BackendLightmapTexture( surface->lightmaptexturenum ) : -1;
		CpuWorldBatch &batch = cpuBatches[{baseTextureId, lightmapTextureId}];
		const uint32_t firstVertex = uint32_t( batch.vertices.size() );
		vec3_t goldNormal;
		VectorCopy( surface->plane->normal, goldNormal );
		if( surface->flags & SURF_PLANEBACK )
			VectorNegate( goldNormal, goldNormal );
		const float3 normal = normalize( GoldSrcToFilament( goldNormal ));
		const quatf orientation = quatf::fromDirectedRotation(
			float3{0.0f, 0.0f, 1.0f}, normal );

		for( int edgeIndex = 0; edgeIndex < surface->numedges; ++edgeIndex )
		{
			const int surfEdge = model->surfedges[surface->firstedge + edgeIndex];
			const medge_t &edge = model->edges[surfEdge >= 0 ? surfEdge : -surfEdge];
			const mvertex_t &sourceVertex =
				model->vertexes[surfEdge >= 0 ? edge.v[0] : edge.v[1]];
			const float *position = sourceVertex.position;
			const float rawS = position[0] * surface->texinfo->vecs[0][0] +
				position[1] * surface->texinfo->vecs[0][1] +
				position[2] * surface->texinfo->vecs[0][2] +
				surface->texinfo->vecs[0][3];
			const float rawT = position[0] * surface->texinfo->vecs[1][0] +
				position[1] * surface->texinfo->vecs[1][1] +
				position[2] * surface->texinfo->vecs[1][2] +
				surface->texinfo->vecs[1][3];

			WorldVertex vertex;
			vertex.position = GoldSrcToFilament( sourceVertex.position );
			vertex.tangents = orientation.xyzw;
			vertex.uv0 = {
				rawS / float( baseTexture->width ? baseTexture->width : 1 ),
				1.0f - rawT /
					float( baseTexture->height ? baseTexture->height : 1 )
			};
			if( hasLightmap && lightmapBlockSize > 0 && lightmapSampleSize > 0 )
			{
				vertex.uv1 = {
					(rawS - surface->texturemins[0] +
						surface->light_s * lightmapSampleSize +
						lightmapSampleSize * 0.5f) /
						float( lightmapBlockSize * lightmapSampleSize ),
					1.0f - (rawT - surface->texturemins[1] +
						surface->light_t * lightmapSampleSize +
						lightmapSampleSize * 0.5f) /
						float( lightmapBlockSize * lightmapSampleSize )
				};
			}
			else
			{
				vertex.uv1 = {0.5f, 0.5f};
			}
			batch.vertices.push_back( vertex );
		}

		for( int triangle = 0; triangle < surface->numedges - 2; ++triangle )
		{
			batch.indices.push_back( firstVertex );
			batch.indices.push_back( firstVertex + uint32_t( triangle + 1 ));
			batch.indices.push_back( firstVertex + uint32_t( triangle + 2 ));
		}
	}

	std::vector<WorldVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<BrushPrimitive> primitives;
	float3 boundsMin = { 1.0e30f, 1.0e30f, 1.0e30f };
	float3 boundsMax = {-1.0e30f,-1.0e30f,-1.0e30f };
	for( auto &entry : cpuBatches )
	{
		CpuWorldBatch &batch = entry.second;
		const uint32_t vertexOffset = uint32_t( vertices.size() );
		const uint32_t indexOffset = uint32_t( indices.size() );
		for( const WorldVertex &vertex : batch.vertices )
		{
			vertices.push_back( vertex );
			for( int axis = 0; axis < 3; ++axis )
			{
				boundsMin[axis] = std::min( boundsMin[axis], vertex.position[axis] );
				boundsMax[axis] = std::max( boundsMax[axis], vertex.position[axis] );
			}
		}
		for( uint32_t index : batch.indices )
			indices.push_back( vertexOffset + index );
		primitives.push_back({
			entry.first.first, entry.first.second, indexOffset,
			uint32_t( batch.indices.size() )
		});
	}
	if( vertices.empty() || indices.empty() )
		return nullptr;

	BrushMesh mesh;
	mesh.model = model;
	mesh.primitives = std::move( primitives );
	mesh.bounds = {
		( boundsMin + boundsMax ) * 0.5f,
		( boundsMax - boundsMin ) * 0.5f
	};
	mesh.vertexBuffer = VertexBuffer::Builder()
		.vertexCount( uint32_t( vertices.size() ))
		.bufferCount( 1 )
		.attribute( VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3,
			offsetof( WorldVertex, position ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::FLOAT4,
			offsetof( WorldVertex, tangents ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( WorldVertex, uv0 ), sizeof( WorldVertex ))
		.attribute( VertexAttribute::UV1, 0, VertexBuffer::AttributeType::FLOAT2,
			offsetof( WorldVertex, uv1 ), sizeof( WorldVertex ))
		.build( *state.engine );
	mesh.indexBuffer = IndexBuffer::Builder()
		.indexCount( uint32_t( indices.size() ))
		.bufferType( IndexBuffer::IndexType::UINT )
		.build( *state.engine );

	void *vertexData = std::malloc( vertices.size() * sizeof( WorldVertex ));
	void *indexData = std::malloc( indices.size() * sizeof( uint32_t ));
	if( !vertexData || !indexData )
	{
		std::free( vertexData );
		std::free( indexData );
		if( mesh.vertexBuffer )
			state.engine->destroy( mesh.vertexBuffer );
		if( mesh.indexBuffer )
			state.engine->destroy( mesh.indexBuffer );
		return nullptr;
	}
	std::memcpy( vertexData, vertices.data(), vertices.size() * sizeof( WorldVertex ));
	std::memcpy( indexData, indices.data(), indices.size() * sizeof( uint32_t ));
	mesh.vertexBuffer->setBufferAt( *state.engine, 0,
		VertexBuffer::BufferDescriptor( vertexData,
			vertices.size() * sizeof( WorldVertex ), ReleaseUploadBuffer ));
	mesh.indexBuffer->setBuffer( *state.engine,
		IndexBuffer::BufferDescriptor( indexData,
			indices.size() * sizeof( uint32_t ), ReleaseUploadBuffer ));

	auto inserted = state.brushMeshes.emplace( model, std::move( mesh ));
	return &inserted.first->second;
}

mat4f EntityTransform( const cl_entity_t *entity )
{
	vec3_t goldForward, goldRight, goldUp;
	GoldSrcAngleVectors( entity->angles, goldForward, goldRight, goldUp );
	const float3 forward = GoldSrcToFilament( goldForward );
	const float3 right = GoldSrcToFilament( goldRight );
	const float3 up = GoldSrcToFilament( goldUp );
	const float3 origin = GoldSrcToFilament( entity->origin );
	return mat4f(
		float4{ right, 0.0f },
		float4{ up, 0.0f },
		float4{-forward, 0.0f },
		float4{ origin, 1.0f } );
}

BrushInstance *GetBrushInstance( cl_entity_t *source )
{
	FilamentState &state = g_filament;
	if( !source || !source->model )
		return nullptr;
	auto existing = state.brushInstances.find( source );
	if( existing != state.brushInstances.end() &&
		existing->second.model == source->model &&
		existing->second.transparent ==
			( source->curstate.rendermode != kRenderNormal ))
		return &existing->second;
	if( existing != state.brushInstances.end() )
	{
		DestroyBrushInstance( existing->second );
		state.brushInstances.erase( existing );
	}

	BrushMesh *mesh = BuildBrushMesh( source->model );
	if( !mesh )
		return nullptr;

	BrushInstance instance;
	instance.source = source;
	instance.model = source->model;
	instance.entity = EntityManager::get().create();
	const bool transparent = source->curstate.rendermode != kRenderNormal;
	instance.transparent = transparent;
	Material *material = transparent ?
		state.worldTransparentMaterial : state.worldMaterial;
	RenderableManager::Builder builder( mesh->primitives.size() );
	builder.boundingBox( mesh->bounds )
		.culling( !transparent )
		.castShadows( !transparent )
		.receiveShadows( true );
	instance.batches.reserve( mesh->primitives.size() );
	for( size_t primitive = 0; primitive < mesh->primitives.size(); ++primitive )
	{
		const BrushPrimitive &range = mesh->primitives[primitive];
		MaterialInstance *materialInstance = material->createInstance();
		Texture *base = range.baseTexture >= 0 && range.baseTexture < MAX_TEXTURES ?
			state.textures[range.baseTexture].texture : nullptr;
		Texture *lightmap = range.lightmapTexture >= 0 &&
			range.lightmapTexture < MAX_TEXTURES ?
			state.textures[range.lightmapTexture].texture : nullptr;
		materialInstance->setParameter( "baseColor", base ? base : state.whiteTexture,
			BaseTextureSampler() );
		materialInstance->setParameter( "lightmap",
			lightmap ? lightmap : state.whiteTexture, LightmapTextureSampler() );
		if( transparent )
			materialInstance->setParameter( "opacity",
				std::clamp( source->curstate.renderamt / 255.0f, 0.0f, 1.0f ));
		builder.material( primitive, materialInstance )
			.geometry( primitive, RenderableManager::PrimitiveType::TRIANGLES,
				mesh->vertexBuffer, mesh->indexBuffer, range.offset, range.count );
		instance.batches.push_back({
			materialInstance, range.baseTexture, range.lightmapTexture
		});
	}
	builder.build( *state.engine, instance.entity );
	auto inserted = state.brushInstances.emplace( source, std::move( instance ));
	return &inserted.first->second;
}

void RenderBrushEntity( cl_entity_t *source )
{
	FilamentState &state = g_filament;
	if( !source || !source->model || source->model->type != mod_brush ||
		source->model == cl.worldmodel )
		return;
	BrushInstance *instance = GetBrushInstance( source );
	if( !instance )
		return;
	instance->seenFrame = state.entityFrame;
	if( source->curstate.rendermode != kRenderNormal )
	{
		const float opacity =
			std::clamp( source->curstate.renderamt / 255.0f, 0.0f, 1.0f );
		for( WorldBatch &batch : instance->batches )
			batch.material->setParameter( "opacity", opacity );
	}
	if( !instance->inScene )
	{
		state.scene->addEntity( instance->entity );
		instance->inScene = true;
	}
	TransformManager &transforms = state.engine->getTransformManager();
	const TransformManager::Instance transform =
		transforms.getInstance( instance->entity );
	if( transform )
		transforms.setTransform( transform, EntityTransform( source ));
}

void UpdateBrushEntities()
{
	FilamentState &state = g_filament;
	++state.entityFrame;
	for( uint32_t i = 0; i < tr.num_static_entities; ++i )
		RenderBrushEntity( tr.static_entities[i] );
	for( uint32_t i = 0; i < tr.num_solid_entities; ++i )
		RenderBrushEntity( tr.solid_entities[i] );
	for( uint32_t i = 0; i < tr.num_trans_entities; ++i )
		RenderBrushEntity( tr.trans_entities[i] );

	for( auto &entry : state.brushInstances )
	{
		BrushInstance &instance = entry.second;
		if( instance.inScene && instance.seenFrame != state.entityFrame )
		{
			state.scene->remove( instance.entity );
			instance.inScene = false;
		}
	}
}

void DestroySceneResources()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;

	DestroyWorld();
	DestroySun();
	DestroyDynamicLights();
	DestroySkyGeometry();
	DestroySkyboxResources( false );
	DestroyR2DResources();
	DestroyStudioResources();
	DestroySpriteResources();
	if( state.worldMaterial )
	{
		state.engine->destroy( state.worldMaterial );
		state.worldMaterial = nullptr;
	}
	if( state.worldTransparentMaterial )
	{
		state.engine->destroy( state.worldTransparentMaterial );
		state.worldTransparentMaterial = nullptr;
	}
	if( state.worldShadowReceiverMaterial )
	{
		state.engine->destroy( state.worldShadowReceiverMaterial );
		state.worldShadowReceiverMaterial = nullptr;
	}
	if( state.skyMaterial )
	{
		state.engine->destroy( state.skyMaterial );
		state.skyMaterial = nullptr;
	}
	if( state.colorGrading )
	{
		if( state.view )
			state.view->setColorGrading( nullptr );
		state.engine->destroy( state.colorGrading );
		state.colorGrading = nullptr;
	}
	if( state.whiteTexture )
	{
		state.engine->destroy( state.whiteTexture );
		state.whiteTexture = nullptr;
	}
	for( TextureSlot &slot : state.textures )
	{
		if( slot.texture )
			state.engine->destroy( slot.texture );
		slot = {};
	}
}

qboolean Filament_Init( const renderer_init_t *init )
{
	FilamentState &state = g_filament;
	SDL_Window *window = static_cast<SDL_Window *>( init->window );
	NSView *contentView = GetContentView( window );
	if( !contentView )
	{
		MsgDev( D_ERROR, "Filament: failed to obtain the SDL NSView\n" );
		return false;
	}
	state.nativeView = contentView;
	state.metalLayer = SetUpMetalLayer( contentView );
	if( !state.metalLayer )
	{
		MsgDev( D_ERROR, "Filament: failed to create the CAMetalLayer\n" );
		return false;
	}

	state.engine = Engine::create( backend::Backend::METAL );
	if( !state.engine )
	{
		MsgDev( D_ERROR, "Filament: failed to create the Metal engine\n" );
		return false;
	}

	state.swapChain = state.engine->createSwapChain( (__bridge void *)state.metalLayer,
		SwapChain::CONFIG_HAS_STENCIL_BUFFER | SwapChain::CONFIG_READABLE );
	state.renderer = state.engine->createRenderer();
	state.scene = state.engine->createScene();
	state.overlayScene = state.engine->createScene();
	state.view = state.engine->createView();
	state.viewModelView = state.engine->createView();
	state.overlayView = state.engine->createView();
	state.cameraEntity = EntityManager::get().create();
	state.viewModelCameraEntity = EntityManager::get().create();
	state.overlayCameraEntity = EntityManager::get().create();
	state.camera = state.engine->createCamera( state.cameraEntity );
	state.viewModelCamera =
		state.engine->createCamera( state.viewModelCameraEntity );
	state.overlayCamera = state.engine->createCamera( state.overlayCameraEntity );
	if( !state.swapChain || !state.renderer || !state.scene || !state.overlayScene ||
		!state.view || !state.viewModelView || !state.overlayView ||
		!state.camera || !state.viewModelCamera || !state.overlayCamera )
	{
		MsgDev( D_ERROR, "Filament: failed to create core rendering objects\n" );
		return false;
	}

	UpdateDrawableSize( state );
	state.logicalWidth = uint32_t( std::max( init->width, 1 ));
	state.logicalHeight = uint32_t( std::max( init->height, 1 ));
	state.view->setScene( state.scene );
	state.view->setCamera( state.camera );
	state.view->setViewport({ 0, 0, state.width, state.height });
	state.view->setPostProcessingEnabled( true );
	state.view->setVisibleLayers( 0xff, 0x01 );
	state.viewModelView->setScene( state.scene );
	state.viewModelView->setCamera( state.viewModelCamera );
	state.viewModelView->setViewport({ 0, 0, state.width, state.height });
	state.viewModelView->setPostProcessingEnabled( false );
	state.viewModelView->setBlendMode( View::BlendMode::TRANSLUCENT );
	state.viewModelView->setVisibleLayers( 0xff, 0x02 );
	state.overlayView->setScene( state.overlayScene );
	state.overlayView->setCamera( state.overlayCamera );
	state.overlayView->setViewport({ 0, 0, state.width, state.height });
	state.overlayView->setPostProcessingEnabled( false );
	state.overlayView->setBlendMode( View::BlendMode::TRANSLUCENT );
	UpdateOverlayCamera();
	ApplyQualitySettings();
	state.camera->setProjection( 60.0, double( state.width ) / double( state.height ),
		0.1, 1000.0, Camera::Fov::VERTICAL );
	state.camera->lookAt({ 0.0, 0.0, 3.0 }, { 0.0, 0.0, 0.0 });
	state.viewModelCamera->setProjection( 60.0,
		double( state.width ) / double( state.height ),
		0.1, 512.0, Camera::Fov::VERTICAL );
	state.viewModelCamera->lookAt(
		{ 0.0, 0.0, 3.0 }, { 0.0, 0.0, 0.0 });

	Renderer::ClearOptions clearOptions;
	clearOptions.clear = true;
	clearOptions.discard = true;
	clearOptions.clearColor = { 0.012, 0.018, 0.035, 1.0 };
	state.renderer->setClearOptions( clearOptions );

	state.skybox = Skybox::Builder()
		.color({ 0.012f, 0.018f, 0.035f, 1.0f })
		.build( *state.engine );
	state.scene->setSkybox( state.skybox );

	state.worldMaterial = Material::Builder()
		.package( kWorldBakedMaterial, sizeof( kWorldBakedMaterial ))
		.build( *state.engine );
	state.worldTransparentMaterial = Material::Builder()
		.package( kWorldTransparentMaterial, sizeof( kWorldTransparentMaterial ))
		.build( *state.engine );
	state.worldShadowReceiverMaterial = Material::Builder()
		.package( kWorldShadowReceiverMaterial,
			sizeof( kWorldShadowReceiverMaterial ))
		.build( *state.engine );
	state.skyMaterial = Material::Builder()
		.package( kSkyMaterial, sizeof( kSkyMaterial ))
		.build( *state.engine );
	state.whiteTexture = Texture::Builder()
		.width( 1 )
		.height( 1 )
		.levels( 1 )
		.sampler( Texture::Sampler::SAMPLER_2D )
		.format( Texture::InternalFormat::RGBA8 )
		.build( *state.engine );
	if( !state.worldMaterial || !state.worldTransparentMaterial ||
		!state.worldShadowReceiverMaterial || !state.skyMaterial ||
		!state.whiteTexture )
	{
		MsgDev( D_ERROR, "Filament: failed to create the BSP material resources\n" );
		return false;
	}
	uint8_t *whitePixel = static_cast<uint8_t *>( std::malloc( 4 ));
	if( !whitePixel )
		return false;
	std::memset( whitePixel, 255, 4 );
	state.whiteTexture->setImage( *state.engine, 0,
		Texture::PixelBufferDescriptor( whitePixel, 4, Texture::Format::RGBA,
			Texture::Type::UBYTE, ReleaseUploadBuffer ));
	if( !CreateR2DResources() )
	{
		MsgDev( D_ERROR, "Filament: failed to create the R2D overlay resources\n" );
		return false;
	}
	if( !CreateStudioResources() )
	{
		MsgDev( D_ERROR, "Filament: failed to create the Studio model resources\n" );
		return false;
	}
	if( !CreateSpriteResources() )
	{
		MsgDev( D_ERROR, "Filament: failed to create the sprite resources\n" );
		return false;
	}

	MsgDev( D_INFO, "Filament: Metal backend initialized at %ux%u\n", state.width, state.height );
	return true;
}

void Filament_Shutdown()
{
	FilamentState &state = g_filament;
	if( !state.engine )
		return;

	DestroySceneResources();
	if( state.overlayCamera )
	{
		state.engine->destroyCameraComponent( state.overlayCameraEntity );
		state.overlayCamera = nullptr;
	}
	if( state.overlayCameraEntity )
	{
		EntityManager::get().destroy( state.overlayCameraEntity );
		state.overlayCameraEntity.clear();
	}
	if( state.viewModelCamera )
	{
		state.engine->destroyCameraComponent( state.viewModelCameraEntity );
		state.viewModelCamera = nullptr;
	}
	if( state.viewModelCameraEntity )
	{
		EntityManager::get().destroy( state.viewModelCameraEntity );
		state.viewModelCameraEntity.clear();
	}
	if( state.camera )
	{
		state.engine->destroyCameraComponent( state.cameraEntity );
		state.camera = nullptr;
	}
	if( state.cameraEntity )
	{
		EntityManager::get().destroy( state.cameraEntity );
		state.cameraEntity.clear();
	}
	if( state.view )
		state.engine->destroy( state.view );
	if( state.viewModelView )
		state.engine->destroy( state.viewModelView );
	if( state.overlayView )
		state.engine->destroy( state.overlayView );
	if( state.scene )
		state.engine->destroy( state.scene );
	if( state.overlayScene )
		state.engine->destroy( state.overlayScene );
	if( state.renderer )
		state.engine->destroy( state.renderer );
	if( state.swapChain )
		state.engine->destroy( state.swapChain );
	Engine::destroy( &state.engine );
	state = {};
}

void Filament_Resize( int width, int height )
{
	FilamentState &state = g_filament;
	if( width > 0 )
		state.logicalWidth = uint32_t( width );
	if( height > 0 )
		state.logicalHeight = uint32_t( height );
	UpdateDrawableSize( state );
	if( state.view )
		state.view->setViewport({ 0, 0, state.width, state.height });
	if( state.viewModelView )
		state.viewModelView->setViewport({ 0, 0, state.width, state.height });
	UpdateOverlayCamera();
	if( state.camera )
		state.camera->setProjection( 60.0, double( state.width ) / double( state.height ),
			0.1, 1000.0, Camera::Fov::VERTICAL );
	if( state.viewModelCamera )
		state.viewModelCamera->setProjection( 60.0,
			double( state.width ) / double( state.height ),
			0.1, 512.0, Camera::Fov::VERTICAL );
}

void Filament_BeginFrame( const ref_params_t * )
{
	FilamentState &state = g_filament;
	ResetR2DCommands();
	ResetStudioCommands();
	ResetSpriteCommands();
	state.renderedThisFrame = false;
	state.scenePrepared = false;
	ApplyQualitySettings();
}

void Filament_RenderScene( const ref_params_t *refdef, qboolean )
{
	FilamentState &state = g_filament;
	if( !state.engine || !state.renderer || !state.swapChain || !refdef )
		return;
	// GoldSrc clients may request additional nextView passes for radar or
	// other auxiliary cameras. The vertical slice has one 3D View, so keep
	// the first (gameplay) camera instead of letting the final auxiliary pass
	// overwrite it just before presentation.
	if( state.scenePrepared )
		return;
	UpdateBrushEntities();
	UpdateDynamicLights();

	vec3_t goldForward, goldRight, goldUp;
	GoldSrcAngleVectors( refdef->viewangles, goldForward, goldRight, goldUp );

	const float3 eye = GoldSrcToFilament( refdef->vieworg );
	const float3 forward = normalize( GoldSrcToFilament( goldForward ));
	const float3 up = normalize( GoldSrcToFilament( goldUp ));

	for( Entity entity : state.skyEntities )
	{
		if( !entity )
			continue;
		TransformManager &skyTransforms = state.engine->getTransformManager();
		const TransformManager::Instance skyTransform =
			skyTransforms.getInstance( entity );
		if( skyTransform )
		{
			// Draw the cube behind normal map geometry while keeping it inside
			// the fixed 8192-unit far plane.
			constexpr float skyScale = 4096.0f;
			skyTransforms.setTransform( skyTransform, mat4f(
				float4( skyScale, 0.0f, 0.0f, 0.0f ),
				float4( 0.0f, skyScale, 0.0f, 0.0f ),
				float4( 0.0f, 0.0f, skyScale, 0.0f ),
				float4( eye, 1.0f )));
		}
	}
	state.camera->lookAt( double3( eye ), double3( eye + forward ), double3( up ));
	state.camera->setProjection( refdef->fov_y > 1.0f ? refdef->fov_y : 60.0,
		double( state.width ) / double( state.height ), 4.0, 8192.0,
		Camera::Fov::VERTICAL );
	state.viewModelCamera->lookAt(
		double3( eye ), double3( eye + forward ), double3( up ));
	state.viewModelCamera->setProjection(
		refdef->fov_y > 1.0f ? refdef->fov_y : 60.0,
		double( state.width ) / double( state.height ), 0.1, 512.0,
		Camera::Fov::VERTICAL );

	state.scenePrepared = true;
}

void RenderPendingFrame()
{
	FilamentState &state = g_filament;
	if( !state.scenePrepared || state.renderedThisFrame ||
		!state.renderer || !state.swapChain )
		return;
	FlushStudio();
	FlushSprites();
	PrepareR2D();
	state.frameStarted = state.renderer->beginFrame( state.swapChain );
	if( !state.frameStarted )
	{
		++state.skippedFrames;
		return;
	}
	++state.begunFrames;
	Renderer::ClearOptions sceneClear;
	sceneClear.clear = true;
	sceneClear.discard = true;
	sceneClear.clearColor = { 0.012, 0.018, 0.035, 1.0 };
	state.renderer->setClearOptions( sceneClear );
	state.renderer->render( state.view );
	if( state.lastStudioBatchCount[1] || !state.r2dBatches.empty() )
	{
		Renderer::ClearOptions overlayClear;
		overlayClear.clear = false;
		overlayClear.discard = false;
		overlayClear.clearColor = { 0.0, 0.0, 0.0, 0.0 };
		state.renderer->setClearOptions( overlayClear );
		if( state.lastStudioBatchCount[1] )
			state.renderer->render( state.viewModelView );
	}
	if( !state.r2dBatches.empty() )
	{
		state.renderer->render( state.overlayView );
	}
	state.renderer->setClearOptions( sceneClear );
	state.renderedThisFrame = true;
	++state.renderedFrames;
}

void Filament_RenderOverlay()
{
	RenderPendingFrame();
}

void Filament_EndFrame()
{
	FilamentState &state = g_filament;
	if( state.frameStarted )
		state.renderer->endFrame();
	state.frameStarted = false;
	state.renderedThisFrame = false;
}

void Filament_OnModelLoaded( model_t * )
{
}

void Filament_OnModelUnloaded( model_t * )
{
}

void Filament_OnMapLoaded( model_t *worldModel )
{
	BuildWorld( worldModel );
	CreateSun( worldModel );
	ApplyQualitySettings();
}

void Filament_OnMapUnloaded()
{
	DestroyWorld();
	DestroySun();
}

qboolean Filament_ReadPixels( renderer_readback_t *request )
{
	FilamentState &state = g_filament;
	RenderPendingFrame();
	if( !request || !request->pixels || request->width <= 0 ||
		request->height <= 0 || request->stride != request->width * 4 ||
		!state.engine || !state.renderer || !state.frameStarted )
		return false;

	const size_t byteCount =
		size_t( request->stride ) * size_t( request->height );
	state.renderer->readPixels( 0, 0, uint32_t( request->width ),
		uint32_t( request->height ),
		backend::PixelBufferDescriptor( request->pixels, byteCount,
			backend::PixelDataFormat::RGBA, backend::PixelDataType::UBYTE,
			1 ));
	state.renderer->endFrame();
	state.frameStarted = false;
	state.renderedThisFrame = false;
	Fence *fence = state.engine->createFence();
	if( !fence )
		return false;
	const bool completed =
		Fence::waitAndDestroy( fence, Fence::Mode::FLUSH ) ==
			backend::FenceStatus::CONDITION_SATISFIED;
	state.engine->flushAndWait();
	if( completed )
	{
		// Metal readback rows arrive in the opposite order from glReadPixels.
		// The shared screenshot writer applies the legacy OpenGL Y flip, so
		// pre-flip here to preserve the same top-left image orientation.
		std::vector<uint8_t> row( size_t( request->stride ));
		uint8_t *pixels = static_cast<uint8_t *>( request->pixels );
		for( int y = 0; y < request->height / 2; ++y )
		{
			uint8_t *top = pixels + size_t( y ) * request->stride;
			uint8_t *bottom = pixels +
				size_t( request->height - 1 - y ) * request->stride;
			std::memcpy( row.data(), top, size_t( request->stride ));
			std::memcpy( top, bottom, size_t( request->stride ));
			std::memcpy( bottom, row.data(), size_t( request->stride ));
		}
	}
	return completed;
}

const renderer_backend_t kFilamentBackend = {
	"filament",
	Filament_Init,
	Filament_Shutdown,
	Filament_Resize,
	Filament_BeginFrame,
	Filament_RenderScene,
	Filament_RenderOverlay,
	Filament_EndFrame,
	Filament_OnModelLoaded,
	Filament_OnModelUnloaded,
	Filament_OnMapLoaded,
	Filament_OnMapUnloaded,
	Filament_ReadPixels
};

} // namespace

extern "C" const renderer_backend_t *FilamentBackend_GetAPI( void )
{
	return &kFilamentBackend;
}

extern "C" void FilamentBackend_TextureUpload( int texture, int width, int height,
	int pixelFormat, const void *pixels, qboolean srgb )
{
	UploadTexture( texture, width, height, pixelFormat, pixels, srgb != false );
}

extern "C" void FilamentBackend_TextureFree( int texture )
{
	FreeTexture( texture );
}

extern "C" void FilamentBackend_SkyTextureUpload( int side, int texture,
	int width, int height, int pixelFormat, const void *pixels )
{
	UploadSkyTexture( side, texture, width, height, pixelFormat, pixels );
}

extern "C" void FilamentBackend_SkyTextureClear( void )
{
	ClearSkyTextures();
}

extern "C" void FilamentBackend_R2DSetColor( unsigned char r, unsigned char g,
	unsigned char b, unsigned char a )
{
	SetR2DColor( r, g, b, a );
}

extern "C" void FilamentBackend_R2DSetRenderMode( int mode )
{
	SetR2DRenderMode( mode );
}

extern "C" void FilamentBackend_R2DDrawQuad( float x, float y,
	float width, float height, float s1, float t1, float s2, float t2,
	int texture )
{
	DrawR2DQuad( x, y, width, height, s1, t1, s2, t2, texture );
}

extern "C" void FilamentBackend_R2DBegin( int primitive )
{
	BeginR2DImmediate( primitive );
}

extern "C" void FilamentBackend_R2DEnd( void )
{
	EndR2DImmediate();
}

extern "C" void FilamentBackend_R2DSetTexture( int texture )
{
	SetR2DTexture( texture );
}

extern "C" void FilamentBackend_R2DTexCoord( float u, float v )
{
	SetR2DTexCoord( u, v );
}

extern "C" void FilamentBackend_R2DVertex( float x, float y )
{
	AddR2DImmediateVertex( x, y );
}

extern "C" void FilamentBackend_StudioSubmit( int texture, int renderMode,
	int faceFlags, const float *positions, const float *texcoords,
	const unsigned char *colors, int firstVertex, int vertexCount,
	const unsigned short *indices, int indexCount, qboolean viewModel )
{
	SubmitStudio( texture, renderMode, faceFlags, positions, texcoords, colors,
		firstVertex, vertexCount, indices, indexCount, viewModel != false );
}

extern "C" void FilamentBackend_SpriteSubmit( int texture, int renderMode,
	int textureFormat, unsigned char r, unsigned char g, unsigned char b,
	unsigned char a, const float *positions )
{
	SubmitSprite( texture, renderMode, textureFormat,
		r, g, b, a, positions );
}

extern "C" qboolean FilamentBackend_DrawableSize( int *width, int *height )
{
	FilamentState &state = g_filament;
	if( !width || !height || !state.metalLayer )
		return false;
	UpdateDrawableSize( state );
	*width = static_cast<int>( state.width );
	*height = static_cast<int>( state.height );
	return true;
}

extern "C" void FilamentBackend_PrintInfo( void )
{
	const FilamentState &state = g_filament;
	Msg( "\n" );
	Msg( "RENDERER: Filament 1.74.0 / Metal\n" );
	Msg( "DEVICE: Apple Metal device (see startup log)\n" );
	Msg( "DRAWABLE: %ux%u\n", state.width, state.height );
	Msg( "FRAMES: begun %llu, rendered %llu, skipped %llu\n",
		static_cast<unsigned long long>( state.begunFrames ),
		static_cast<unsigned long long>( state.renderedFrames ),
		static_cast<unsigned long long>( state.skippedFrames ));
	Msg( "WORLD: %u batches, %u brush meshes, %u brush instances\n",
		static_cast<unsigned>( state.worldBatches.size() ),
		static_cast<unsigned>( state.brushMeshes.size() ),
		static_cast<unsigned>( state.brushInstances.size() ));
	Msg( "STUDIO: %u vertices, %u triangles, %u batches\n",
		static_cast<unsigned>( state.studioVertices.size() ),
		static_cast<unsigned>( state.studioIndices.size() / 3 ),
		static_cast<unsigned>( state.studioBatches.size() ));
	Msg( "SPRITES: %u quads, %u batches\n",
		static_cast<unsigned>( state.spriteVertices.size() / 4 ),
		static_cast<unsigned>( state.spriteBatches.size() ));
	Msg( "LIGHTS: 1 sun, %u/%u dynamic point lights\n",
		state.activeDynamicLights, unsigned( MAX_DLIGHTS ));
	Msg( "QUALITY: shadows %d, SSAO %d, bloom %d, tone mapper %s\n",
		r_shadow_quality ? r_shadow_quality->integer : 0,
		r_ssao ? r_ssao->integer : 0,
		r_bloom ? r_bloom->integer : 0,
		r_tonemapper && r_tonemapper->string ? r_tonemapper->string : "agx" );
}
