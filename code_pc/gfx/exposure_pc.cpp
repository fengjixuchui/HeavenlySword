/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/exposure.h"
#include "gfx/hardwarecaps.h"
#include "gfx/shader.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/surfacemanager.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/graphicsdevice.h"
#include "gfx/texturereader.h"
#include "gfx/graphing.h"
#include "gfx/pictureinpicture.h"

#include "core/visualdebugger.h"
#include "core/timer.h"

bool g_ResetExposure = true;

//#define OVERIDE_EXP
#define TRAP_FLOATSPECIAL

//--------------------------------------------------
//!
//!	GetInternalFormat
//!	Returns the internal texture format used by the exposure 
//!
//--------------------------------------------------
GFXFORMAT EXPController::GetInternalFormat()
{
	GFXFORMAT surfaceFormat = GF_ARGB8;

	if ( HardwareCapabilities::Get().IsValidRenderTargetFormat(GF_R16F) )
		surfaceFormat = GF_R16F;
	else if ( HardwareCapabilities::Get().IsValidRenderTargetFormat(GF_R32F) )
		surfaceFormat = GF_R32F;

	return surfaceFormat;
}

//--------------------------------------------------
//!
//!	GetLuminance
//! MUST mirror the luminace evaluation used in the shaders
//!
//--------------------------------------------------
static float GetLuminance( CDirection const& colour )
{
	static const CDirection conversion( 0.212671f, 0.715160f, 0.072169f );
	return colour.Dot( conversion );
}




//--------------------------------------------------
//!
//!	EXPControllerCPU_impl
//! Down samples current frame buffer to generate a key luminance value.
//!
//--------------------------------------------------
class EXPControllerCPU_impl
{
public:
	EXPControllerCPU_impl();

#ifdef OVERIDE_EXP
	~EXPControllerCPU_impl()
	{
		if( HardwareCapabilities::Get().SupportsShaderModel3() )
			NT_DELETE_CHUNK(Mem::MC_GFX,  m_pLogFilterPS );
	}
#endif

	void SampleCurrBackBuffer( RenderTarget::Ptr& pobBackBuffer );
	
private:

#ifdef _DEBUG
	static const int iLOG_LUMINANCE_RES = 64;
#else
	static const int iLOG_LUMINANCE_RES = 256;
#endif
	static const int iSAMPLE_SKIP = 1;

	Shader* m_pLogFilterVS;
	Shader* m_pLogFilterPS;
	CVertexDeclaration m_pFilterDecl;
};

//--------------------------------------------------
//!
//!	EXPControllerCPU_impl::ctor
//! moves back buffer from HDR to linear space
//!
//--------------------------------------------------
EXPControllerCPU_impl::EXPControllerCPU_impl( void )
{	
	m_pLogFilterVS = ShaderManager::Get().FindShader( "exposure_logfilter_vs" );
	m_pLogFilterPS = ShaderManager::Get().FindShader( "exposure_logfilter_ps" );

#ifdef OVERIDE_EXP
	if( HardwareCapabilities::Get().SupportsShaderModel3() )
	{
		DebugShader* pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
		pNewPS->SetHLSLFunction(
			SHADERTYPE_PIXEL,

			"float4 exposure_logfilter_ps( float2 texcoord : TEXCOORD0, "
			"				    uniform sampler2D source : register(s0), "
			"					uniform float delta : register(c0) "
			"					) : COLOR0  "
			"{"
			"	float3 colour = tex2D( source, texcoord ).xyz;"
			"	float luminance = dot( colour, float3( 0.212671f, 0.715160f, 0.072169f ) );"
			"	if ((luminance > 0.0001f) && (luminance < 65504.0f)) "
			"	{"
			"		return log( delta + luminance ).xxxx;"
			"	}"
			"	else"
			"	{"
			"		return 0.0f;"
			"	}"
			"}",

			"exposure_logfilter_ps",
			"ps_3_0"
		);

		m_pLogFilterPS = pNewPS;
	}
#endif

	ntAssert( m_pLogFilterVS && m_pLogFilterPS );

	// create the declarations
	D3DVERTEXELEMENT9 stFilterDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof( float ),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_pFilterDecl = CVertexDeclarationManager::Get().GetDeclaration( stFilterDecl );
}

//--------------------------------------------------
//!
//!	EXPControllerCPU_impl::SampleCurrBackBuffer
//! grab current back buffer and generate a key luminace value
//!
//--------------------------------------------------
void EXPControllerCPU_impl::SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer )
{
	GFXFORMAT surfaceFormat = EXPController::GetInternalFormat();

	RenderTarget::Ptr pTarget = SurfaceManager::Get().CreateRenderTarget( iLOG_LUMINANCE_RES, iLOG_LUMINANCE_RES, surfaceFormat );
	Renderer::Get().m_targetCache.SetColourTarget( pTarget );

	// render the log luminance in
	Renderer::Get().SetVertexShader( m_pLogFilterVS );
	Renderer::Get().SetPixelShader( m_pLogFilterPS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pFilterDecl );

	CVector const obDelta( 0.001f, 0.0f, 0.0f, 0.0f );
	float fSampleArea = RenderingContext::Get()->m_exposureSettings.m_fSamplingArea;
	float const fLeft = 0.5f*( 1.0f - fSampleArea );
	float const fRight = 0.5f*( 1.0f + fSampleArea );
	float const afScreenQuad[] = 
	{
		-1.0f,  1.0f,	fLeft, fLeft,
		 1.0f,  1.0f,	fRight, fLeft,
		 1.0f, -1.0f,	fRight, fRight,
		 1.0f, -1.0f,	fRight, fRight,
		-1.0f, -1.0f,	fLeft, fRight,
		-1.0f,  1.0f,	fLeft, fLeft,
	};

	Renderer::Get().SetPixelShaderConstant( 0, obDelta );

	Renderer::Get().SetTexture( 0, pBackBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, afScreenQuad, 4*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );

	// now we generate our float sum from this
	Surface::Ptr pRenderResult = SurfaceManager::Get().CreateSurface( 
		Surface::CreationStruct( iLOG_LUMINANCE_RES,iLOG_LUMINANCE_RES, pTarget->m_Platform.GetDXFormat(), D3DPOOL_SYSTEMMEM ) );

	GetD3DDevice()->GetRenderTargetData( pTarget->m_Platform.GetSurface(), pRenderResult->m_Platform.GetSurface() );

	int iNumPixels = (iLOG_LUMINANCE_RES * iLOG_LUMINANCE_RES) / iSAMPLE_SKIP;

	// NOTE: Pitch isn't used this could be bad oneday...
	uint32_t pitch;
	void* pData = pRenderResult->CPULock( pitch );
	TextureReader textureData( pData, surfaceFormat );

	float fSum = 0.0f;
	for (int i = 0; i < iNumPixels; i++, textureData.Next(iSAMPLE_SKIP) )	
		fSum += textureData.Current().X();
	
	float fLastVal = Renderer::Get().m_pPIPManager->GetCurrentView().m_fExposureLastVal;

	if (g_ResetExposure)
	{
		fLastVal = expf( fSum / _R( iNumPixels ) ) - 0.001f;
		g_ResetExposure = false;
	}
	else
	{
		float fCorrection = RenderingContext::Get()->m_exposureSettings.m_fErrorReduction * 0.01f;
		fCorrection = clamp( fCorrection, 0.0f, 1.0f );

		float fNewVal = expf( fSum / _R( iNumPixels ) ) - 0.001f;
		fLastVal = CMaths::Lerp( fLastVal, fNewVal, fCorrection );
	}

	pRenderResult->CPUUnlock();

	// its possible to get chuff in here on the first frame, not entirely sure why.
	// in the mean time, please enjoy this marvellous hack.
	u_int uiBitPattern = *((u_int*)&fLastVal);

	if	(
		(_isnan(fLastVal)) ||			// -1.#QNAN000
		(uiBitPattern == 0xffc00000) ||	// -1.#IND0000
		(uiBitPattern == 0x7fe40000) ||	// -1.#QNAN0000
		(uiBitPattern == 0x7f800000)	// -1.#INF0000
		)
	{
#ifdef TRAP_FLOATSPECIAL

		void* pData = pRenderResult->CPULock( pitch );
		TextureReader textureData( pData, surfaceFormat );

		float fCurr = 0.0f;

		for (int i = 0; i < iNumPixels; i++, textureData.Next(iSAMPLE_SKIP) )	
		{
			fCurr = textureData.Current().X();
			uiBitPattern = *((u_int*)&fCurr);

			if	(
				(_isnan(fCurr)) ||				// -1.#QNAN000
				(uiBitPattern == 0xffc00000) ||	// -1.#IND0000
				(uiBitPattern == 0x7fe40000) ||	// -1.#QNAN0000
				(uiBitPattern == 0x7f800000)	// -1.#INF0000
				)
				break;
		}
		
		pRenderResult->CPUUnlock();

		char errorType[ MAX_PATH ];
		if (_isnan(fCurr))
			sprintf( errorType, "QNAN" );
		else if (uiBitPattern == 0xffc00000)
			sprintf( errorType, "IND" );
		else if (uiBitPattern == 0x7fe40000)
			sprintf( errorType, "QNAN" );
		else if (uiBitPattern == 0x7f800000)
			sprintf( errorType, "INF" );

		static char acFileName[ MAX_PATH ];
		sprintf( acFileName, "ErrorLogLum%d(%s)", CTimer::Get().GetSystemTicks(),errorType );
		pRenderResult->m_Platform.SaveToDisk( acFileName, D3DXIFF_DDS );

		sprintf( acFileName, "ErrorFrame%d(%s)", CTimer::Get().GetSystemTicks(),errorType );
		pBackBuffer->SaveToDisk( acFileName );

#endif

		fLastVal = 1.0f;
	}

	Renderer::Get().m_pPIPManager->GetCurrentView().m_fExposureLastVal = fLastVal;

	SurfaceManager::Get().ReleaseSurface( pRenderResult );
	SurfaceManager::Get().ReleaseRenderTarget( pTarget );
}

//--------------------------------------------------
//!
//!	EXPControllerCPU
//! Public interface to our CPU based exposure
//!
//--------------------------------------------------
EXPControllerCPU::EXPControllerCPU()
{
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX) EXPControllerCPU_impl;
}
EXPControllerCPU::~EXPControllerCPU()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pImpl );
}
void EXPControllerCPU::SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer )
{
	m_pImpl->SampleCurrBackBuffer( pBackBuffer );
}




//--------------------------------------------------
//!
//!	EXPControllerGPU_impl
//! Down samples current frame buffer to generate a key luminance value.
//!
//--------------------------------------------------
class EXPControllerGPU_impl
{
public:
	EXPControllerGPU_impl();
	~EXPControllerGPU_impl();

	void SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer );

private:
	Shader* m_pLogFilterVS;
	Shader* m_pLogFilterPS;
	CVertexDeclaration m_pFilterDecl;

	Shader* m_pRecursiveSumVS;
	Shader* m_pRecursiveSumPS;
	CVertexDeclaration m_pSumDecl;

	Shader* m_pInterpolationVS;
	Shader* m_pInterpolationPS;
	Shader* m_pExpOnlyPS;

	CScopedArray<RenderTarget::Ptr> m_pKeyValues;
	int m_iKeyLevelCount;
};

//--------------------------------------------------
//!
//!	EXPControllerGPU_impl::ctor
//! moves back buffer from HDR to linear space
//!
//--------------------------------------------------
EXPControllerGPU_impl::EXPControllerGPU_impl() :
	m_iKeyLevelCount( 9 )
{
	GFXFORMAT surfaceFormat = EXPController::GetInternalFormat();

	// create the render target surfaces
	m_pKeyValues.Reset( NT_NEW_CHUNK(Mem::MC_GFX) RenderTarget::Ptr[m_iKeyLevelCount] );
	for( int iLevel = 0; iLevel < m_iKeyLevelCount; ++iLevel )
	{
		int iResolution = ( 1 << ( m_iKeyLevelCount - iLevel - 1 ) );
		m_pKeyValues[iLevel] = SurfaceManager::Get().CreateRenderTarget( iResolution, iResolution, surfaceFormat );
	}

	// find the shaders
	m_pLogFilterVS = ShaderManager::Get().FindShader( "exposure_logfilter_vs" );
	m_pLogFilterPS = ShaderManager::Get().FindShader( "exposure_logfilter_ps" );

#ifdef OVERIDE_EXP
	if( HardwareCapabilities::Get().SupportsShaderModel3() )
	{
		DebugShader* pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
		pNewPS->SetHLSLFunction(
			SHADERTYPE_PIXEL,

			"float4 exposure_logfilter_ps( float2 texcoord : TEXCOORD0, "
			"				    uniform sampler2D source : register(s0), "
			"					uniform float delta : register(c0) "
			"					) : COLOR0 "
			"{"
			"	float3 colour = tex2D( source, texcoord ).xyz;"
			"	float luminance = dot( colour, float3( 0.212671f, 0.715160f, 0.072169f ) );"
			"	if ((luminance > 0.0001f) && (luminance < 65504.0f)) "
			"	{"
			"		return log( delta + luminance ).xxxx;"
			"	}"
			"	else"
			"	{"
			"		return 0.0f;"
			"	}"
			"}",

			"exposure_logfilter_ps",
			"ps_3_0"
		);

		m_pLogFilterPS = pNewPS;
	}
#endif

	ntAssert( m_pLogFilterVS && m_pLogFilterPS );

	m_pRecursiveSumVS = ShaderManager::Get().FindShader( "exposure_recursivesum_vs" );
	m_pRecursiveSumPS = ShaderManager::Get().FindShader( "exposure_recursivesum_ps" );
	ntAssert( m_pRecursiveSumVS && m_pRecursiveSumPS );

	m_pInterpolationVS = ShaderManager::Get().FindShader( "exposure_interpolation_vs" );
	m_pInterpolationPS = ShaderManager::Get().FindShader( "exposure_interpolation_ps" );
	m_pExpOnlyPS = ShaderManager::Get().FindShader( "exposure_exponly_ps" );
	ntAssert( m_pInterpolationVS && m_pInterpolationPS );

	// create the declarations
	D3DVERTEXELEMENT9 stFilterDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof( float ),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_pFilterDecl = CVertexDeclarationManager::Get().GetDeclaration( stFilterDecl );
	D3DVERTEXELEMENT9 stSumDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		D3DDECL_END()
	};
	m_pSumDecl = CVertexDeclarationManager::Get().GetDeclaration( stSumDecl );
}

EXPControllerGPU_impl::~EXPControllerGPU_impl()
{
	for( int iLevel = 0; iLevel < m_iKeyLevelCount; ++iLevel )
		SurfaceManager::Get().ReleaseRenderTarget( m_pKeyValues[iLevel] );

#ifdef OVERIDE_EXP
	if( HardwareCapabilities::Get().SupportsShaderModel3() )
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pLogFilterPS );
#endif
}

//--------------------------------------------------
//!
//!	EXPControllerGPU_impl::SampleCurrBackBuffer
//! grab current back buffer and generate a key luminace value
//!
//--------------------------------------------------
void EXPControllerGPU_impl::SampleCurrBackBuffer( RenderTarget::Ptr& pobBackBuffer )
{
	if( CRendererSettings::bShowExposureSamplingArea )
	{
		// show the exposure area
		float fWidth = Renderer::Get().m_targetCache.GetWidth();
		float fHeight = Renderer::Get().m_targetCache.GetHeight();

		CPoint obViewDim( fWidth, fHeight, 0.0f );

		float fSampleArea = RenderingContext::Get()->m_exposureSettings.m_fSamplingArea;
		CPoint obTopLeft = obViewDim*0.5f*( 1.0f - fSampleArea );
		CPoint obBottomRight = obViewDim*0.5f*( 1.0f + fSampleArea );
		CPoint obTopRight( obBottomRight.X(), obTopLeft.Y(), 0.0f );
		CPoint obBottomLeft( obTopLeft.X(), obBottomRight.Y(), 0.0f );
		
		g_VisualDebug->RenderLine( obTopLeft, obTopRight, 0xffffff00, DPF_WIREFRAME | DPF_VIEWPORTSPACE );
		g_VisualDebug->RenderLine( obTopRight, obBottomRight, 0xffffff00, DPF_WIREFRAME | DPF_VIEWPORTSPACE );
		g_VisualDebug->RenderLine( obBottomRight, obBottomLeft, 0xffffff00, DPF_WIREFRAME | DPF_VIEWPORTSPACE );
		g_VisualDebug->RenderLine( obBottomLeft, obTopLeft, 0xffffff00, DPF_WIREFRAME | DPF_VIEWPORTSPACE );
	}

	// set the viewport to the first texture
	Renderer::Get().m_targetCache.SetColourTarget( m_pKeyValues[0] );

	// render the log luminance in
	Renderer::Get().SetVertexShader( m_pLogFilterVS );
	Renderer::Get().SetPixelShader( m_pLogFilterPS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pFilterDecl );
	
	CVector const obDelta( 0.001f, 0.0f, 0.0f, 0.0f );
	float fSampleArea = RenderingContext::Get()->m_exposureSettings.m_fSamplingArea;
	float const fLeft = 0.5f*( 1.0f - fSampleArea );
	float const fRight = 0.5f*( 1.0f + fSampleArea );
	float const afScreenQuad[] = 
	{
		-1.0f,  1.0f,			fLeft, fLeft,
		 1.0f,  1.0f,			fRight, fLeft,
		 1.0f, -1.0f,			fRight, fRight,
		 1.0f, -1.0f,			fRight, fRight,
		-1.0f, -1.0f,			fLeft, fRight,
		-1.0f,  1.0f,			fLeft, fLeft,
	};

	Renderer::Get().SetPixelShaderConstant( 0, obDelta );

	Renderer::Get().SetTexture( 0, pobBackBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, afScreenQuad, 4*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );

	// recursively sum the texture down into the lowest level
	Renderer::Get().SetVertexShader( m_pRecursiveSumVS );
	Renderer::Get().SetPixelShader( m_pRecursiveSumPS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pSumDecl );

	static float const afScreenTri[] = 
	{
		-1.0f,	 1.0f,
		 3.0f,	 1.0f, 
		-1.0f,	-3.0f
	};

	for( int iLevel = 1; iLevel < m_iKeyLevelCount; ++iLevel )
	{
		// set the viewport
		Renderer::Get().m_targetCache.SetColourTarget( m_pKeyValues[iLevel] );

		// render the sum into the level
		float fSampleOffset = 0.25f/_R( m_pKeyValues[iLevel]->GetWidth() );
		Renderer::Get().SetVertexShaderConstant( 1, CVector( fSampleOffset, fSampleOffset, 0.0f, 0.0f ) );

		Renderer::Get().SetTexture( 0, m_pKeyValues[iLevel - 1]->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, afScreenTri, 2*sizeof( float ) );

		Renderer::Get().SetTexture( 0, Texture::NONE );
	}

	// interpolate into the used key luminance texture
	Renderer::Get().SetVertexShader( m_pInterpolationVS );

	RenderTarget::Ptr pInterpolatedKey		= Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastVal;
	RenderTarget::Ptr pInterpolatedKeyTemp	= Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastValTemp;

	Renderer::Get().m_targetCache.SetColourTarget( pInterpolatedKeyTemp );

	Renderer::Get().SetTexture( 0, m_pKeyValues[m_iKeyLevelCount - 1]->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	if (g_ResetExposure)
	{
		g_ResetExposure = false;
		Renderer::Get().SetPixelShader( m_pExpOnlyPS );
	}
	else
	{
		Renderer::Get().SetPixelShader( m_pInterpolationPS );

		Renderer::Get().SetTexture( 1, pInterpolatedKey->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_POINT );

		float fCorrection = RenderingContext::Get()->m_exposureSettings.m_fErrorReduction * 0.01f;
		fCorrection = clamp( fCorrection, 0.0f, 1.0f );
		CVector obCorrection( fCorrection, 0.0f, 0.0f, 0.0f );
		Renderer::Get().SetPixelShaderConstant( 0, obCorrection );
	}

	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, afScreenTri, 2*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );

	// make current
	Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastVal.Swap(
		Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastValTemp );
}

//--------------------------------------------------
//!
//!	EXPControllerGPU
//! Public interface to our GPU based exposure
//!
//--------------------------------------------------
EXPControllerGPU::EXPControllerGPU()
{
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX) EXPControllerGPU_impl;
}
EXPControllerGPU::~EXPControllerGPU()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pImpl );
}
void EXPControllerGPU::SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer )
{
	m_pImpl->SampleCurrBackBuffer( pBackBuffer );
}






//--------------------------------------------------
//!
//!	LevelsGraph_impl
//!	Given an 8-bit texture, the levels graph locks the results and produces a 
//! histograph of the levels within the image.
//!
//--------------------------------------------------
class LevelsGraph_impl
{
public:
	LevelsGraph_impl() :
		m_iNumLevels( 0 ), 
		m_iMaxCount( 1 ), 
		m_iWidth( 0 ), 
		m_iHeight( 0 ) 
	{}

	void Update( int iNumLevels, const Surface::Ptr& pRenderTarget )
	{
		UpdateLockable( pRenderTarget );
		SampleLockable( iNumLevels );
	}

	void RenderGraph( const CPoint& topLeft, const CPoint& bottomRight );

private:
	LevelsGraph_impl* m_pImpl;
	void UpdateLockable( const Surface::Ptr& pRenderTarget );
	void SampleLockable( int iNumLevels );

	int m_iNumLevels;					//!< The number of levels to sample.
	CScopedArray<int> m_aiCounters;		//!< An array of counters, one for each level.
	int m_iMaxCount;					//!< The maximum counter value for the current dataset.

	Surface::Ptr m_pLockable;			//!< Lockable texture storage.
	int m_iWidth, m_iHeight;			//!< The lockable texture dimensions.

	CScopedPtr<CGraph> m_pGraph;		//!< The graph of the levels.
};

//--------------------------------------------------
//!
//!	LevelsGraph_impl::UpdateLockable
//! Updates the lockable texture with the given rendertarget texture.
//!
//--------------------------------------------------
void LevelsGraph_impl::UpdateLockable( const Surface::Ptr& pRenderTarget )
{
	if( !m_pLockable )
	{
		m_pLockable = SurfaceManager::Get().CreateSurface( 
				Surface::CreationStruct(	pRenderTarget->GetWidth(), 
											pRenderTarget->GetHeight(), 
											pRenderTarget->m_Platform.GetDXFormat() , 
											D3DPOOL_SYSTEMMEM ) );
		m_iWidth = (int) m_pLockable->GetWidth();
		m_iHeight = (int) m_pLockable->GetHeight();
	}

	// check we have a matching texture 
	if( m_pLockable->GetWidth() != pRenderTarget->GetWidth() ||
		m_pLockable->GetHeight() != pRenderTarget->GetHeight() ||
		m_pLockable->GetFormat() != pRenderTarget->GetFormat() )
	{
		// kill the old one
		if (m_pLockable)
			SurfaceManager::Get().ReleaseSurface( m_pLockable );

		m_pLockable = SurfaceManager::Get().CreateSurface( 
				Surface::CreationStruct(	pRenderTarget->GetWidth(), 
												pRenderTarget->GetHeight(), 
												pRenderTarget->m_Platform.GetDXFormat() , 
												D3DPOOL_SYSTEMMEM ) );
		m_iWidth = (int) m_pLockable->GetWidth();
		m_iHeight = (int) m_pLockable->GetHeight();
	}

	// copy the texture data over
	HRESULT hr;
	hr = GetD3DDevice()->GetRenderTargetData( pRenderTarget->m_Platform.GetSurface(), m_pLockable->m_Platform.GetSurface() );
	ntAssert( SUCCEEDED( hr ) );
}

//--------------------------------------------------
//!
//!	LevelsGraph_impl::SampleLockable
//! Samples the levels of the lockable texture.
//!
//--------------------------------------------------
void LevelsGraph_impl::SampleLockable( int iNumLevels )
{
	// make sure we have enough storage
	if( iNumLevels > m_iNumLevels )
		m_aiCounters.Reset( NT_NEW_CHUNK(Mem::MC_GFX) int[iNumLevels] );
	m_iNumLevels = iNumLevels;

	// clear the current data
	memset( m_aiCounters.Get(), 0, sizeof( int )*m_iNumLevels );

	// lock the texture data
	void* pData;
	uint32_t pitch;
	pData = m_pLockable->CPULock( pitch );
	for( int iY = 0; iY < m_iHeight; ++iY )
	{
		// get this row
		uint32_t const* pdwBuffer = reinterpret_cast<uint32_t const*>( 
			reinterpret_cast<char const*>( pData ) + iY*pitch 
		);

		// process it
		CDirection colour( CONSTRUCT_CLEAR );
		for( int iX = 0; iX < m_iWidth; ++iX )
		{
			// get the colour as a float [0, 1]
			colour.X() = _R( ( pdwBuffer[iX] >> 16 ) & 0xff )*( 1.0f/255.0f );
			colour.Y() = _R( ( pdwBuffer[iX] >>  8 ) & 0xff )*( 1.0f/255.0f );
			colour.Z() = _R( ( pdwBuffer[iX] >>  0 ) & 0xff )*( 1.0f/255.0f );

			// convert to luminance
			float fLuminance = GetLuminance( colour );

			// put into a band
			int iBand = clamp( int( _R( m_iNumLevels )*fLuminance ), 0, m_iNumLevels - 1 );

			// increment the counter for this band
			++m_aiCounters[iBand];
		}
	}
	m_pLockable->CPUUnlock();	

	// normalise the results
	m_iMaxCount = 1;
	for( int iBand = 0; iBand < m_iNumLevels; ++iBand )
		m_iMaxCount = max( m_aiCounters[iBand], m_iMaxCount );
}

//--------------------------------------------------
//!
//!	LevelsGraph_impl::RenderGraph
//! Renders a debug graph of the current sample set.
//!
//--------------------------------------------------
void LevelsGraph_impl::RenderGraph( const CPoint& topLeft, const CPoint& bottomRight )
{
	// create the graph
	m_pGraph.Reset( NT_NEW_CHUNK(Mem::MC_GFX) CGraph( GRAPH_TYPE_STANDARD ) );
	m_pGraph->SetBackColour( 0x3fffffff );

	// create a sample set for the levels curve
	CGraphSampleSet* pobSampleSet = m_pGraph->AddSampleSet( "levels", m_iNumLevels, 0xff000000 );
	pobSampleSet->SetUsedSamples( m_iNumLevels );

	// fill in the samples with the current data
	CGraphSample* pobSamples = pobSampleSet->GetSampleData();
	for( int iIndex = 0; iIndex < m_iNumLevels; ++iIndex )
	{
		pobSamples[iIndex].X() = _R( iIndex )/_R( m_iNumLevels );
		pobSamples[iIndex].Y() = _R( m_aiCounters[iIndex] )/_R( m_iMaxCount );
	}

	// ensure the graph is bounded properly
	m_pGraph->RecomputeAxes();

	// render the graph
	g_VisualDebug->RenderGraph( m_pGraph.Get(), topLeft, bottomRight );
}

//--------------------------------------------------
//!
//!	LevelsGraph
//! Public interface to our levels graph
//!
//--------------------------------------------------
LevelsGraph::LevelsGraph()
{
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX) LevelsGraph_impl;
}
LevelsGraph::~LevelsGraph()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pImpl );
}
void LevelsGraph::Update( int iNumLevels, const Surface::Ptr& pRenderTarget )
{
	m_pImpl->Update( iNumLevels, pRenderTarget );
}
void LevelsGraph::RenderGraph( const CPoint& topLeft, const CPoint& bottomRight )
{
	return m_pImpl->RenderGraph( topLeft, bottomRight );
}
