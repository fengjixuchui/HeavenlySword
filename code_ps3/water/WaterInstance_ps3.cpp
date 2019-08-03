//--------------------------------------------------
//!
//!	\file WaterInstance_ps3.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "water/WaterInstance_ps3.h"
#include "water/WaterInstanceDef.h"
#include "water/watermanager.h"
#include "water/waterwaveemitter.h"
#include "water/waterdmadata.h"
#include "water/waterbuoyproxy.h"
#include "anim/transform.h"
#include "game/randmanager.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/pictureinpicture.h"
#include "gfx/depthhazeconsts.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "core/user.h"

#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/sputask_ps3.h"

#include "core/visualdebugger.h"
#include "core/timer.h"
#include "input/inputhardware.h"
#include "game/randmanager.h"

#include "area/arearesourcedb.h"

//
// DMA limits
//
static const unsigned int MAX_GRID_SIZE = 18 * 1000;

// TODO_OZZ: bump MC_PROCEDURAL limits and change default chunk to that
#define WATER_DMA_ALIGNMENT  128
#define WATER_DMA_CHUNK Mem::MC_MISC

// 
// water shaders
//

static const char* WATER_DEPTH_VP		= "water_depth_vp.sho";
static const char* WATER_DEPTH_FP		= "water_depth_fp.sho";
static const char* WATER_GAME_VP		= "water_vp.sho";
static const char* WATER_GAME_FP		= "water_fp.sho";
static const char* WATER_NORMAL_MAP0	= "cellbump_normal.dds";
static const char* WATER_NORMAL_MAP1	= "fractalbump_normal.dds";

//
// ProceduralVB slots
//
static const int WATER_VERTEX_SLOT_POSITION		= 0;
static const int WATER_VERTEX_SLOT_BINORMAL		= 1;
static const int WATER_VERTEX_SLOT_TANGENT		= 2;

static const uint16_t PRIMITIVE_RESET			= 0xffff;
static const uint32_t TRISTRIP_LENGTH			= 4;

static const float DEBUG_RENDER_EMITTER_TIME	= 1.2f;
static const float DEBUG_RENDER_EMITTER_SIZE	= 0.2f;

//
// Some helpful macros
//
#ifndef _RELEASE
#	define CHECK_MATRIX( m ) user_warn_p( m != CMatrix(CONSTRUCT_CLEAR), ("World matrix is zero!!!\n") );
#else 
#	define CHECK_MATRIX( m )
#endif

#ifndef _RELEASE
#	define CLEAN_MEMORY( ptr, size ) memset( ptr, 0, size );
#else
#	define CLEAN_MEMORY( ptr, size ) memset( ptr, 0, size );
#endif


#define WATER_SECTORED_DMA_RESOURCES

///////////////////////////////////////////////////////////////////////////////////////////
//
//										Helpers
//
///////////////////////////////////////////////////////////////////////////////////////////
template<class T>
T* FindFirstElemWithFlags( T* pArray, uint32_t elemCount, int flags )
{
	for ( uint32_t iElem = 0; iElem < elemCount; ++iElem )
		if( pArray[iElem].m_iFlags & flags )
			return pArray + iElem;

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//										WaterInstance
//
///////////////////////////////////////////////////////////////////////////////////////////

WaterInstance::WaterInstance( WaterInstanceDef* pobDef )
: CRenderable( &pobDef->m_obTransform, true, false, false, RT_WATER )
, m_pobWaterDma( 0 )
, m_pVertexDmaArray( 0 )
, m_pWaveDmaArray( 0 )
, m_pBuoyDmaArray( 0 )
, m_pobDef( pobDef )
, m_hVB()
{
	ntError( pobDef );

	m_hVB.PushVertexElement( VD_STREAM_TYPE_FLOAT4,		WATER_VERTEX_SLOT_POSITION,		"in_position" );
	m_hVB.PushVertexElement( VD_STREAM_TYPE_FLOAT4,		WATER_VERTEX_SLOT_BINORMAL,		"in_binormal" );
	m_hVB.PushVertexElement( VD_STREAM_TYPE_FLOAT4,		WATER_VERTEX_SLOT_TANGENT,		"in_tangent" );

	m_pobVertexShader_Colour		= DebugShaderCache::Get().LoadShader( WATER_GAME_VP, true );
	m_pobPixelShader_Colour			= DebugShaderCache::Get().LoadShader( WATER_GAME_FP, true );
	m_pobVertexShader_Depth			= DebugShaderCache::Get().LoadShader( WATER_DEPTH_VP, true );
	m_pobPixelShader_Depth			= DebugShaderCache::Get().LoadShader( WATER_DEPTH_FP, true );

	m_bIsAlphaBlended				= true;

#ifdef WATER_SECTORED_DMA_RESOURCES
	const char* name = ntStr::GetString( ObjectDatabase::Get().GetNameFromPointer(m_pobDef) );
	uint32_t areaMask = pobDef->m_iSectorBits;
	if ( areaMask == 0 )
	{
		if (AreaManager::Get().LevelActive())
			areaMask = 1 << ( AreaResourceDB::Get().DebugGetLastAreaLoaded()-1 );
		else
			areaMask = 0xffffffff;
	}
	AreaResourceDB::Get().AddAreaResource( name, AreaResource::WATER, areaMask );
#else
	CreateDmaResources();
#endif
}


WaterInstance::~WaterInstance()
{
	DestroyDmaResources();
}


bool WaterInstance::HasDmaResources( void ) const 
{
#ifdef	PLATFORM_PS3
	return m_pobWaterDma && m_pVertexDmaArray && m_pWaveDmaArray && m_pBuoyDmaArray;
#else
	return m_pobWaterDma;
#endif
}	


// NOTE: these thing recurses if resulting grid has too many cells. Just so you know...
void WaterInstance::CreateDmaResources( void )
{	
	u_int subDivsWidth  = Util::Align( static_cast<int>(floor(m_pobDef->m_fWidth/m_pobDef->m_fResolution)), 4 );
	u_int subDivsLength = Util::Align( static_cast<int>(floor(m_pobDef->m_fLength/m_pobDef->m_fResolution)), 4 );
	u_int totalGridCells = subDivsWidth * subDivsLength;

	ntAssert( Util::IsAligned( totalGridCells, 4 ) );
	
	if ( totalGridCells <= MAX_GRID_SIZE )
	{
		m_pobWaterDma = reinterpret_cast<WaterInstanceDma*>( NT_MEMALIGN_CHUNK( WATER_DMA_CHUNK, sizeof(WaterInstanceDma), WATER_DMA_ALIGNMENT ) );
		CLEAN_MEMORY( m_pobWaterDma, sizeof(WaterInstanceDma) );
		m_pobWaterDma->m_iGridSize[0] = subDivsLength;
		m_pobWaterDma->m_iGridSize[1]	= subDivsWidth;
		m_pobWaterDma->m_fWidth = m_pobDef->m_fWidth;
		m_pobWaterDma->m_fLength = m_pobDef->m_fLength;
		m_pobWaterDma->m_fMaxHeight = EPSILON;
		m_pobWaterDma->m_fMinHeight = -EPSILON;
		m_pobWaterDma->m_fResolution = m_pobDef->m_fResolution;
		m_pobWaterDma->m_fTime = 0;
		m_pobWaterDma->m_obWorldMatrix		= m_pobTransform->GetWorldMatrix();
		m_pobWaterDma->m_obWorldMatrixInv	= m_pobWaterDma->m_obWorldMatrix.GetAffineInverse();

		// init AABB to grid limits. MinY/MaxY will be updated by the spu
		m_obBounds.Min().X() = -m_pobWaterDma->m_fWidth/2;
		m_obBounds.Min().Z() = -m_pobWaterDma->m_fLength/2;
		m_obBounds.Max().X() = m_pobWaterDma->m_fWidth/2;
		m_obBounds.Max().Z() = m_pobWaterDma->m_fLength/2;
		m_obBounds.Min().Y() = m_pobWaterDma->m_fMinHeight;
		m_obBounds.Max().Y() = m_pobWaterDma->m_fMaxHeight;
		
		// init the procedural vertex buffer
		m_hVB.BuildMe( totalGridCells );
		CLEAN_MEMORY( m_hVB.GetVertex(0) , m_hVB.GetMaxVertices() * m_hVB.GetVertexSize() );

		ntAssert_p( m_hVB.GetVertexSize() == sizeof(VertexDma), ("WaterVertex size mismatch (%i,%i)\n", m_hVB.GetVertexSize(), sizeof(VertexDma)) );
		
		// we alias the VB internal data ptr just to make it clear that it's being DMA'ed
		m_pVertexDmaArray = reinterpret_cast<VertexDma*>( m_hVB.GetVertex(0) );

#ifdef PLATFORM_PS3
		// init our grid positions to lie on flat on the plane
		// TODO_OZZ: should be merged into one call since they're tied in together
		InitGridPositions();
		InitIndexBuffer();

		// TODO_OZZ: single DMAAlloc ?
		// now init wave dma array
		m_pWaveDmaArray = reinterpret_cast<WaveDma*>( NT_MEMALIGN_CHUNK( WATER_DMA_CHUNK, MAX_NUM_WAVES * sizeof(WaveDma), WATER_DMA_ALIGNMENT ) );
		CLEAN_MEMORY( m_pWaveDmaArray, MAX_NUM_WAVES * sizeof(WaveDma) );
		for ( uint32_t i = 0; i < MAX_NUM_WAVES; ++i )
		{
			m_pWaveDmaArray[i].m_iFlags = kWF_Control_Invalid;
		}

		// init buoys dma array
		m_pBuoyDmaArray = reinterpret_cast<BuoyDma*>( NT_MEMALIGN_CHUNK( WATER_DMA_CHUNK, MAX_NUM_BUOYS * sizeof(BuoyDma), WATER_DMA_ALIGNMENT ) );
		CLEAN_MEMORY( m_pBuoyDmaArray, MAX_NUM_WAVES * sizeof(BuoyDma) );
		for ( uint32_t i = 0; i < MAX_NUM_BUOYS; ++i )
		{
			m_pBuoyDmaArray[i].m_iFlags = kBF_Control_Invalid;
		}
#endif //PLATFORM_PS3

#ifndef _NO_DBGMEM_OR_RELEASE
		static_assert( sizeof(SPU_DebugLine) == 48, SPU_DebugLine_not_equal_48_bytes );
		m_pobWaterDma->m_iNumDebugLines = (uint32_t*)NT_MEMALIGN_CHUNK( Mem::MC_DEBUG, sizeof(uint32_t), WATER_DMA_ALIGNMENT );
		m_pobWaterDma->m_pDebugLineBuffer = (SPU_DebugLine*)NT_MEMALIGN_CHUNK( Mem::MC_DEBUG, sizeof(SPU_DebugLine) * MAX_DEBUG_LINES, WATER_DMA_ALIGNMENT );
		CLEAN_MEMORY( m_pobWaterDma->m_pDebugLineBuffer, sizeof(SPU_DebugLine) * MAX_DEBUG_LINES );
		AtomicSet( m_pobWaterDma->m_iNumDebugLines, 0 );

		user_code_start(Ozz)
			uint32_t nrows = m_pobWaterDma->m_iGridSize[0];
			uint32_t ncolumns = m_pobWaterDma->m_iGridSize[1];
			uint32_t ntotal = nrows * ncolumns;
			ntPrintf("*** WaterInstance resources created. Grid: %ix%i, VertexCount: %i, - IndexCount: %i, TriCount: %i ***\n", 
						nrows, ncolumns, ntotal, m_iIndexCount, m_iTriangleCount  );
		user_code_end()
#endif

		// finally, load textures
		m_pobNormalMap0 = TextureManager::Get().LoadTexture_Neutral( m_pobDef->m_obNormalMap0.c_str() );
		if ( !m_pobNormalMap0 )
		{
			user_warn_msg(("WATER - couldn't load texture %s. Using default one instead\n",m_pobDef->m_obNormalMap0.c_str()  ));
			m_pobNormalMap0 = TextureManager::Get().LoadTexture_Neutral( WATER_NORMAL_MAP0 );
		}

		m_pobNormalMap1 = TextureManager::Get().LoadTexture_Neutral( m_pobDef->m_obNormalMap1.c_str() );
		if ( !m_pobNormalMap1 )
		{
			user_warn_msg(("WATER - couldn't load texture %s. Using default one instead\n",m_pobDef->m_obNormalMap1.c_str()  ));
			m_pobNormalMap1 = TextureManager::Get().LoadTexture_Neutral( WATER_NORMAL_MAP1 );
		}
	}
	else
	{
		m_pobDef->m_fResolution *= 1.1f;
		user_warn_msg(("Water grid has too many cells. Using a coarser resolution(%f)...\n", m_pobDef->m_fResolution));
		CreateDmaResources();
	}
}



void WaterInstance::DestroyDmaResources( void )
{
#ifndef _NO_DBGMEM_OR_RELEASE
	if ( m_pobWaterDma && m_pobWaterDma->m_pDebugLineBuffer )
	{
		NT_FREE_CHUNK( Mem::MC_DEBUG, (uintptr_t)m_pobWaterDma->m_pDebugLineBuffer );
		m_pobWaterDma->m_pDebugLineBuffer = 0;
	}

	if ( m_pobWaterDma && m_pobWaterDma->m_iNumDebugLines )
	{
		NT_FREE_CHUNK( Mem::MC_DEBUG, (uintptr_t)m_pobWaterDma->m_iNumDebugLines );
		m_pobWaterDma->m_iNumDebugLines = 0;
	}
#endif

	if ( m_pWaveDmaArray )
	{
		NT_FREE_CHUNK( WATER_DMA_CHUNK, (uintptr_t)m_pWaveDmaArray );
		m_pWaveDmaArray = 0;
	}

	if ( m_pBuoyDmaArray )
	{
		NT_FREE_CHUNK( WATER_DMA_CHUNK, (uintptr_t)m_pBuoyDmaArray );
		m_pBuoyDmaArray = 0;
	}

	if ( m_pobWaterDma )
	{
		NT_FREE_CHUNK( WATER_DMA_CHUNK, (uintptr_t)m_pobWaterDma );
		m_pobWaterDma = 0;
	}

	TextureManager::Get().UnloadTexture_Neutral( m_pobDef->m_obNormalMap0.c_str() );
	TextureManager::Get().UnloadTexture_Neutral( m_pobDef->m_obNormalMap1.c_str() );
}



void WaterInstance::Update( float fTimeStep )
{
	if ( HasDmaResources() )
	{
#ifndef _RELEASE
		//
		// No grid re-generation in release please!
		//
		if ( abs(m_pobWaterDma->m_fWidth - m_pobDef->m_fWidth) > EPSILON 
			|| abs(m_pobWaterDma->m_fLength - m_pobDef->m_fLength) > EPSILON
			|| abs(m_pobWaterDma->m_fResolution - m_pobDef->m_fResolution) > EPSILON )
		{
			DestroyDmaResources();
			CreateDmaResources();
		}
#endif

		// bounding box Y dimention is updated with a 1-frame delay for this is computed on the spu
		m_obBounds.Min().Y() = m_pobWaterDma->m_fMinHeight;
		m_obBounds.Max().Y() = m_pobWaterDma->m_fMaxHeight;

		// sync out data from previous frame
		//BuoysSyncOut();
		
		// update the necessary water instance information
		m_pobWaterDma->m_obWorldMatrix		= m_pobTransform->GetWorldMatrix();
		m_pobWaterDma->m_obWorldMatrixInv	= m_pobWaterDma->m_obWorldMatrix.GetAffineInverse();
		m_pobWaterDma->m_fVScale			= m_pobDef->m_fVScale;

		m_pobWaterDma->m_fTime				+= fTimeStep;
		m_pobWaterDma->m_fTimeStep			= fTimeStep;


		// process the emitter to re-fill the wave array
		ProcessEmitters();

		// process buoyproxies and update associated transforms
		ProcessBuoys();

		// 
		UpdateWavesPPU( fTimeStep );
	}
}

void WaterInstance::RenderDepth(void)
{
#ifdef PLATFORM_PS3
	if ( HasDmaResources() )
	{
		const CMatrix& modelToWorld = m_pobTransform->GetWorldMatrix();
		const CMatrix modelViewProj( modelToWorld * RenderingContext::Get()->m_worldToScreen );

		CHECK_MATRIX( modelToWorld );
		CHECK_MATRIX( modelViewProj );

		m_pobVertexShader_Colour->SetVSConstantByName( "modelViewProj", modelViewProj, 4 );	
		
		Renderer::Get().SetVertexShader( m_pobVertexShader_Depth );
		Renderer::Get().SetPixelShader( m_pobPixelShader_Depth );

		m_hVB.SubmitToGPU();

		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
		Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangleStrip, 0, m_iIndexCount, m_hIB );
		Renderer::Get().m_Platform.ClearStreams();
	}
#endif
}


void WaterInstance::RenderMaterial( void )
{
#ifdef PLATFORM_PS3
	if ( HasDmaResources() )
	{
		const CMatrix& modelToWorld = m_pobTransform->GetWorldMatrix();
		const CMatrix modelViewProj( modelToWorld * RenderingContext::Get()->m_worldToScreen );

		CHECK_MATRIX( modelToWorld );
		CHECK_MATRIX( modelViewProj );

		CMatrix mWorld2Local = modelToWorld.GetAffineInverse();

		m_pobVertexShader_Colour->SetVSConstantByName( "modelViewProj", modelViewProj, 4 );	
		m_pobVertexShader_Colour->SetVSConstantByName( "modelToWorld", modelToWorld, 4 );	
		m_pobVertexShader_Colour->SetVSConstantByName( "lightDirWS", RenderingContext::Get()->m_toKeyLight );
		m_pobVertexShader_Colour->SetVSConstantByName( "cameraPosWorldSpace", RenderingContext::Get()->GetEyePos() );
		m_pobVertexShader_Colour->SetVSConstantByName( "cameraPosObjSpace", RenderingContext::Get()->GetEyePos() * mWorld2Local );
		CVector temp( CTimer::Get().GetGameTime(), 0.0f, 0.0f, 0.0f );
		m_pobVertexShader_Colour->SetVSConstantByName( "game_time", temp );

		m_pobVertexShader_Colour->SetVSConstantByName( "depth_haze_consts_A", CDepthHazeSetting::GetAConsts() );
		m_pobVertexShader_Colour->SetVSConstantByName( "depth_haze_consts_G", CDepthHazeSetting::GetGConsts() );
		m_pobVertexShader_Colour->SetVSConstantByName( "depth_haze_b1_plus_b2", CDepthHazeSetting::GetBeta1PlusBeta2() );
		m_pobVertexShader_Colour->SetVSConstantByName( "depth_haze_bdash1", CDepthHazeSetting::GetBetaDash1() );
		m_pobVertexShader_Colour->SetVSConstantByName( "depth_haze_bdash2", CDepthHazeSetting::GetBetaDash2() );
		m_pobVertexShader_Colour->SetVSConstantByName( "depth_haze_recip_b1_plus_b2", CDepthHazeSetting::GetOneOverBeta1PlusBeta2() );
		m_pobVertexShader_Colour->SetVSConstantByName( "sunDir", CDepthHazeSetting::GetSunDir() * mWorld2Local);
		m_pobVertexShader_Colour->SetVSConstantByName( "sunColour", CDepthHazeSetting::GetSunColour() );
		m_pobVertexShader_Colour->SetVSConstantByName( "water_size", m_pobDef->m_obMapSize );
		m_pobVertexShader_Colour->SetVSConstantByName( "water_speed", m_pobDef->m_obMapSpeed );

		Renderer::Get().SetTexture( 0, RenderingContext::Get()->m_pStencilTarget );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT);

		RenderingContext::Get()->m_reflectanceMap->m_Platform.GetTexture()->SetGammaCorrect( Gc::kGammaCorrectSrgb );
		Renderer::Get().SetTexture( 1, RenderingContext::Get()->m_reflectanceMap );
		Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_TRILINEAR);
		
		PIPView& viewport = Renderer::Get().m_pPIPManager->GetCurrentView();
		Renderer::Get().SetTexture( 2, viewport.GetMSAAColour()->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_BILINEAR);

		Renderer::Get().SetTexture( 3, m_pobNormalMap0 );
		Renderer::Get().SetSamplerAddressMode( 3, TEXTUREADDRESS_WRAPALL );
		Renderer::Get().SetSamplerFilterMode( 3, TEXTUREFILTER_TRILINEAR);

		Renderer::Get().SetTexture( 4, m_pobNormalMap1 );
		Renderer::Get().SetSamplerAddressMode( 4, TEXTUREADDRESS_WRAPALL );
		Renderer::Get().SetSamplerFilterMode( 4, TEXTUREFILTER_TRILINEAR);

		CVector posToUV( CONSTRUCT_CLEAR );
		posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
		posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);
		m_pobPixelShader_Colour->SetPSConstantByName( "posToUV", posToUV );	


		CVector specularPower_Reflectivity_Fresnel( m_pobDef->m_fSpecularPower, m_pobDef->m_fReflectivity, m_pobDef->m_fFresnelStrength, 0.0f );
		m_pobPixelShader_Colour->SetPSConstantByName( "in_specularPower_Reflectivity_Fresnel", specularPower_Reflectivity_Fresnel );	
		m_pobPixelShader_Colour->SetPSConstantByName( "in_baseColour", m_pobDef->m_obBaseColour );		
		m_pobPixelShader_Colour->SetPSConstantByName( "in_key_dir", RenderingContext::Get()->m_toKeyLight );	
		m_pobPixelShader_Colour->SetPSConstantByName( "in_key_dir_colour", RenderingContext::Get()->m_keyColour );	
		m_pobPixelShader_Colour->SetPSConstantByName( "in_fill_sh", RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12 );	


		Renderer::Get().SetVertexShader( m_pobVertexShader_Colour );
		Renderer::Get().SetPixelShader( m_pobPixelShader_Colour );

		m_hVB.SubmitToGPU();
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
		Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangleStrip, 0, m_iIndexCount, m_hIB );
		Renderer::Get().m_Platform.ClearStreams();
	}
#endif
}


void WaterInstance::ProcessEmitters( void )
{
	for ( ntstd::List<WaveEmitter*>::const_iterator it = m_pobDef->m_obEmitters.begin(); it != m_pobDef->m_obEmitters.end(); ++it )
	{
		(*it)->Update( m_pobWaterDma->m_fTimeStep );
	}
}	


void WaterInstance::DebugRender( bool bRenderBuoys, bool bRenderSPULines )
{
#ifndef _GOLD_MASTER
	ntAssert_p( HasDmaResources(), ("WATER - water instance has no dma resources\n") );

	const CMatrix& world = m_pobTransform->GetWorldMatrix();
	CPoint obFrom( CONSTRUCT_CLEAR );
	CPoint obTo( CONSTRUCT_CLEAR );

	// render the bounding box
	m_obBounds.DebugRender( world, DC_BLUE );

	// get some useful info there too
	g_VisualDebug->Printf3D( obFrom * world, 0.0f, 0.0f, DC_BLUE, 0, "(0,0)" );
	obFrom.X() = 0.5f * m_pobWaterDma->m_fWidth;
	obFrom.Z() = 0.5f * m_pobWaterDma->m_fLength;
	g_VisualDebug->Printf3D( obFrom * world, 0.0f, 0.0f, DC_BLUE, 0, "Max(%.2f,%.2f)  Resolution: %f", obFrom.X(), obFrom.Z(), m_pobWaterDma->m_fResolution );
	obFrom *= -1.0f;
	g_VisualDebug->Printf3D( obFrom * world, 0.0f, 0.0f, DC_BLUE, 0, "Min(%.2f,%.2f)  Resolution: %f", obFrom.X(), obFrom.Z(), m_pobWaterDma->m_fResolution );

	// render the emitters
	for ( ntstd::List<WaveEmitter*>::const_iterator it = m_pobDef->m_obEmitters.begin(); it != m_pobDef->m_obEmitters.end(); ++it )
	{
		uint32_t colour = (*it)->m_bAutoEmit ? DC_YELLOW : DC_GREY;
		
		switch ( (*it)->m_eType )
		{
			case WAVETYPE_ATTACK0:
			case WAVETYPE_ATTACK1:
			case WAVETYPE_ATTACK2:
				colour = DC_PURPLE;
				break;
			default:
				break;
		}

		if ( (*it)->m_bAutoEmit && (*it)->m_fTimeSinceLastEmission < DEBUG_RENDER_EMITTER_TIME ) 
		{
			colour = DC_GREEN;
		}
	
		DataObject* pDO =  ObjectDatabase::Get().GetDataObjectFromPointer( *it );
		const CKeyString& emitterName = pDO ? pDO->GetName() : CKeyString("Invalid");

		const CPoint& posLS = (*it)->m_obPos;
		const CPoint posWS = posLS * world;
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), posWS, DEBUG_RENDER_EMITTER_SIZE, colour );
		g_VisualDebug->Printf3D( posWS, 0.0f, -0.12f, colour, 0, "%s \n(%.2f,%.2f,%.2f)", emitterName.GetString(), posLS.X(), posLS.Y(), posLS.Z() );

		switch ( (*it)->m_eType )
		{
		case WAVETYPE_CIRCULAR:
			break;
		case WAVETYPE_DIRECTIONAL:
		case WAVETYPE_ATTACK0:
		case WAVETYPE_ATTACK1:
			obFrom =  posWS;
			obTo = CPoint((*it)->m_obAvgWaveDir * world) ;
			obTo /= (  1.1f * obTo.Length() );
			obTo += obFrom;
			g_VisualDebug->RenderLine( obFrom, obTo,  colour ); 
			break;
		case WAVETYPE_ATTACK2:
			break;
		}
		
	}

	if ( bRenderBuoys )
		DebugRenderBuoys();

	UNUSED( bRenderSPULines );

#ifndef _NO_DBGMEM_OR_RELEASE
	uint32_t iNumLines = ntstd::Min( *m_pobWaterDma->m_iNumDebugLines, MAX_DEBUG_LINES );
	for( uint32_t i = 0; bRenderSPULines && i < iNumLines; ++i )
	{
		const SPU_DebugLine* line = &m_pobWaterDma->m_pDebugLineBuffer[i];
		g_VisualDebug->RenderLine( line->a * world, line->b * world, line->col );
	}
	AtomicSet( m_pobWaterDma->m_iNumDebugLines, 0 );
#endif

#endif
}


void WaterInstance::InitGridPositions( void )
{
	VertexDma vertex;
	CLEAN_MEMORY( &vertex, sizeof(VertexDma) );
	float x, z;

	for ( uint32_t i = 0; i < m_pobWaterDma->m_iGridSize[0]; ++i )
	{
		z = i * m_pobDef->m_fResolution - 0.5f * m_pobWaterDma->m_fLength;
		for ( uint32_t j = 0; j < m_pobWaterDma->m_iGridSize[1]; ++j )
		{
			x = j * m_pobDef->m_fResolution - 0.5f * m_pobWaterDma->m_fWidth;
			
			// set position
			vertex.m_position.X() 	= x;
			vertex.m_position.Z() 	= z;
			vertex.m_position.W() 	= 1.0f;

			// we store the initial position in the unused components of the tanget basis for later retrieval on the spu
			vertex.m_binormal.W()	= x;
			vertex.m_tangent.W()	= z;

			m_hVB.SetVertex( i * m_pobWaterDma->m_iGridSize[1] + j, &vertex );
		}
	}
}


void WaterInstance::InitIndexBuffer( void )
{
	ntAssert( m_pobWaterDma );
	
	uint32_t numOfStripColums = m_pobWaterDma->m_iGridSize[1] / (TRISTRIP_LENGTH-1);

	ntstd::Vector<uint16_t> indices;
	for ( uint32_t iStripCol = 0; iStripCol < numOfStripColums; ++iStripCol )
	{
		for ( uint32_t iRow = 0; iRow < m_pobWaterDma->m_iGridSize[0] - 1; ++iRow )
	{
			for ( uint32_t iCol = 0; iCol < TRISTRIP_LENGTH; ++iCol )
		{
				uint32_t colOffset = iStripCol*(TRISTRIP_LENGTH-1) + iCol;
				if ( colOffset < m_pobWaterDma->m_iGridSize[1] )
			{
					indices.push_back( (iRow+0) * m_pobWaterDma->m_iGridSize[1] + colOffset );
					indices.push_back( (iRow+1) * m_pobWaterDma->m_iGridSize[1] + colOffset );
				}
			}
			indices.push_back( PRIMITIVE_RESET );
		}
	
		//	indices.push_back( PRIMITIVE_RESET );
	}
	
	// we don't need the last reset so we won't count it
	m_hIB = RendererPlatform::CreateIndexStream(Gc::kIndex16,indices.size(),Gc::kStaticBuffer);
	m_hIB->Write(&(indices.front()));


	m_iIndexCount = indices.size();

	m_iTriangleCount = (TRISTRIP_LENGTH-1) * 2 * numOfStripColums * m_pobWaterDma->m_iGridSize[0] - 1;
}

void WaterInstance::SetupDma( SPUTask& task ) const
{
	uint32_t vertexArray_EA = (uint32_t)(uintptr_t)m_pVertexDmaArray;
	DMABuffer waterInfo( m_pobWaterDma, sizeof(WaterInstanceDma) );
	DMABuffer waveArray( m_pWaveDmaArray, MAX_NUM_WAVES*sizeof(WaveDma) );
	DMABuffer buoyArray( m_pBuoyDmaArray, MAX_NUM_BUOYS*sizeof(BuoyDma) );

	task.SetArgument( SPUArgument( SPUArgument::Mode_InputAndOutput,	waterInfo ),		WATER_SPU_PARAM_WINSTANCE );
	task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			waveArray ),		WATER_SPU_PARAM_WAVEARRAY );
	task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			vertexArray_EA ),	WATER_SPU_PARAM_VERTARRAY );
}


uint32_t	WaterInstance::GetNumOfWaveSlots( void ) const
{
	return MAX_NUM_WAVES;
}


WaveDma*	WaterInstance::GetWaveSlot( uint32_t index )
{
	ntAssert( index < GetNumOfWaveSlots() );
	return m_pWaveDmaArray + index;
}


WaveDma*	WaterInstance::GetFirstAvailableWaveSlot( void )
{
	return FindFirstElemWithFlags( m_pWaveDmaArray, GetNumOfWaveSlots(), kWF_Control_Invalid );
}

uint32_t WaterInstance::GetNumOfBuoySlots( void ) const
{
	return MAX_NUM_BUOYS;
}

BuoyDma*	WaterInstance::GetBuoySlot( uint32_t index )
{
	ntAssert( index < GetNumOfBuoySlots() );
	return m_pBuoyDmaArray + index;	
}

BuoyDma*	WaterInstance::GetFirstAvailableBuoySlot( void )
{
	return FindFirstElemWithFlags( m_pBuoyDmaArray, GetNumOfBuoySlots(), kBF_Control_Invalid );
}



const VertexDma& WaterInstance::GetVertexAtCoords( uint32_t i, uint32_t j ) const
{
	ntAssert( i < m_pobWaterDma->m_iGridSize[0] && j < m_pobWaterDma->m_iGridSize[1] );
	return m_pVertexDmaArray[ i * m_pobWaterDma->m_iGridSize[1] + j ]; 
}


const VertexDma& WaterInstance::GetVertexAtAproxPos( float x, float z ) const
{
	float nx = ntstd::Clamp( (x + 0.5f * m_pobWaterDma->m_fWidth)/m_pobWaterDma->m_fWidth, 0.0f, 1.0f );
	float nz = ntstd::Clamp( (z + 0.5f * m_pobWaterDma->m_fLength)/ m_pobWaterDma->m_fLength, 0.0f, 1.0f );
	uint32_t i = static_cast<uint32_t>( floor( nz * m_pobWaterDma->m_iGridSize[0] ) );
	uint32_t j = static_cast<uint32_t>( floor( nx * m_pobWaterDma->m_iGridSize[1] ) );

	return GetVertexAtCoords( i, j );
}

const CMatrix&	WaterInstance::GetWaterToWorldMatrix( void ) const
{
	return m_pobWaterDma->m_obWorldMatrix;
}

const CMatrix&	WaterInstance::GetWorldToWaterMatrix( void ) const
{
	return m_pobWaterDma->m_obWorldMatrixInv;
}


void WaterInstance::ProcessBuoys( void )
{
	for ( ntstd::List<BuoyProxy*>::iterator it = m_pobDef->m_obBuoyProxies.begin(); it != m_pobDef->m_obBuoyProxies.end(); ++it )
	{
		if ( (*it)->Update() )
		{
			UpdateBuoyPPU( *(*it)->m_pobBuoyDma );
		}
	}
}


void WaterInstance::UpdateBuoyPPU( BuoyDma& buoy )
{
	const CPoint& pos = buoy.m_obLocalMatrix.GetTranslation();
	const VertexDma& vertex = GetVertexAtAproxPos( pos.X(), pos.Z() );
	CDirection surfaceNormal( vertex.m_tangent.Cross( vertex.m_binormal ) );
	CQuat rot( buoy.m_obUpDirection, surfaceNormal );
	CPoint newPos( pos + buoy.m_fTravelSpeed * surfaceNormal );
	newPos.Y() = vertex.m_position.Y();
	CMatrix newMatrix( rot, newPos );
	buoy.m_obLocalMatrix = CMatrix::Lerp( buoy.m_obLocalMatrix, newMatrix, buoy.m_fBuoyancy );
}



void WaterInstance::DebugRenderBuoys( void )
{
#ifndef _GOLD_MASTER
	for ( ntstd::List<BuoyProxy*>::iterator it = m_pobDef->m_obBuoyProxies.begin(); it != m_pobDef->m_obBuoyProxies.end(); ++it )
	{
		if ( (*it)->IsActive() )
		{
			BuoyDma& buoy = *(*it)->m_pobBuoyDma;
			uint32_t colour = DC_CYAN;
			g_VisualDebug->RenderCube( buoy.m_obLocalMatrix * m_pobWaterDma->m_obWorldMatrix , colour, 0.3f );
		}
	}
#endif
}

void WaterInstance::UpdateWavesPPU( float fTimeStep )
{
	for ( uint32_t iWave = 0; iWave < MAX_NUM_WAVES; ++iWave )
	{
		m_pWaveDmaArray[iWave].Update( fTimeStep );
	}
}



//eof


