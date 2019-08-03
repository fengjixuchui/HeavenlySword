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
#include "gfx/surfacemanager.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/graphicsdevice.h"
#include "gfx/texturereader.h"
#include "gfx/graphing.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/pictureinpicture.h"

#include "core/visualdebugger.h"

// includes just for overide of generated value
#include "gfx/texturemanager.h"
#include "input/inputhardware.h"

bool g_ResetExposure = true;

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
	void			AllocateGeometry();

	DebugShader *	m_simpleQuadVS;
	DebugShader	*	m_logFilterPS;

	DebugShader	*	m_recursiveSumVS;
	DebugShader	*	m_recursiveSumPS;
	DebugShader *	m_interpolationPS;
	DebugShader *	m_srgbToLinear;


	VBHandle				m_hLogFilterData;
	VBHandle				m_hRecSumDataData;
	VBHandle				m_hSimpleQuadData;


	int m_iKeyLevelCount;
	CScopedArray<RenderTarget::Ptr> m_pKeyValues;
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
	GFXFORMAT surfaceFormat = GF_R32F;

	// create the render target surfaces
	m_pKeyValues.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) RenderTarget::Ptr[m_iKeyLevelCount] );

	int iStartRez = m_iKeyLevelCount - 1;

#ifdef _PROFILING
	if (CRendererSettings::bProfileLowFillrate)
		iStartRez = (int)(_R(iStartRez)*0.01f);
	else if (CRendererSettings::bUseLowRezMode)
		iStartRez = iStartRez>>1;
#endif

	for( int iLevel = 0; iLevel < m_iKeyLevelCount; ++iLevel )
	{
		int iResolution = max( 1, ( 1 << ( iStartRez - iLevel ) ) );
		m_pKeyValues[iLevel] = SurfaceManager::Get().CreateRenderTarget( iResolution, iResolution, surfaceFormat );
	}

	// find the shaders
	m_simpleQuadVS = DebugShaderCache::Get().LoadShader( "passthrough_pos_tex_vp.sho" );
	m_recursiveSumVS = DebugShaderCache::Get().LoadShader( "exposure_recursivesum_vp.sho" );

	m_logFilterPS = DebugShaderCache::Get().LoadShader( "exposure_logfilter_fp.sho" );
	m_recursiveSumPS = DebugShaderCache::Get().LoadShader( "exposure_recursivesum_fp.sho" );
	m_interpolationPS = DebugShaderCache::Get().LoadShader( "exposure_interpolation_fp.sho" );
	m_srgbToLinear = DebugShaderCache::Get().LoadShader( "exposure_srgbconv_fp.sho" );
}

EXPControllerGPU_impl::~EXPControllerGPU_impl()
{
	for( int iLevel = 0; iLevel < m_iKeyLevelCount; ++iLevel )
		SurfaceManager::Get().ReleaseRenderTarget( m_pKeyValues[iLevel] );
}

//--------------------------------------------------
//!
//!	EXPControllerGPU_impl::AllocateGeometry
//! allocate geometry for simple screen sprites
//!
//--------------------------------------------------
void EXPControllerGPU_impl::AllocateGeometry()
{
	ntAssert_p( RenderingContext::Get(), ("Must be called within a valid render context") );
	ntAssert_p( !m_hLogFilterData, ("Geometry allready allocated") );
	ntAssert_p( !m_hRecSumDataData, ("Geometry allready allocated") );
	ntAssert_p( !m_hSimpleQuadData, ("Geometry allready allocated") );

	// for the log luminance shader
	GcStreamField	logFilterDesc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),	// 2 * 4 bytes
		GcStreamField( FwHashedString( "IN.texcoord" ), 8, Gc::kFloat, 2 ),	// 2 * 4 bytes
	};

	float fSampleArea = RenderingContext::Get()->m_exposureSettings.m_fSamplingArea;
	float const fLeft = 0.5f*( 1.0f - fSampleArea );
	float const fRight = 0.5f*( 1.0f + fSampleArea );

	float const logFilterQuad[] = 
	{
		-1.0f,  1.0f,	fLeft,	fLeft,		// 0
		 1.0f,  1.0f,	fRight, fLeft,		// 1
		 1.0f, -1.0f,	fRight, fRight,		// 2
		-1.0f, -1.0f,	fLeft,	fRight,		// 3
	};

	m_hLogFilterData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, logFilterDesc );
	m_hLogFilterData->Write( logFilterQuad );

	// for the recursive sum shader
	GcStreamField	reqQuadDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ); 

	float const recSumQuad[] = 
	{
		-1.0f,  1.0f,
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
	};

	m_hRecSumDataData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 2, 1, &reqQuadDesc );
	m_hRecSumDataData->Write( recSumQuad );

	float const simpleQuad[] = 
	{
		-1.0f,  1.0f,	0.0f,	0.0f,
		 1.0f,  1.0f,	1.0f,   0.0f,
		 1.0f, -1.0f,	1.0f,   1.0f,
		-1.0f, -1.0f,	0.0f,	1.0f,
	};

	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, logFilterDesc );
	m_hSimpleQuadData->Write( simpleQuad );	
}

//--------------------------------------------------
//!
//!	EXPControllerGPU_impl::SampleCurrBackBuffer
//! grab current back buffer and generate a key luminace value
//!
//--------------------------------------------------
void EXPControllerGPU_impl::SampleCurrBackBuffer( RenderTarget::Ptr& pobBackBuffer )
{
	if	(
		(!m_hLogFilterData) ||
		(!m_hSimpleQuadData) || 
		(!m_hRecSumDataData)
		)
		AllocateGeometry();

#ifndef _GOLD_MASTER
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
#endif

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	// set the viewport to the first texture
	Renderer::Get().m_targetCache.SetColourTarget( m_pKeyValues[0] );

	// render the log luminance in
	Renderer::Get().SetVertexShader( m_simpleQuadVS );
	
	//CVector delta( 0.001f, 0.0f, 0.0f, 0.0f );
	//m_logFilterPS.SetPSConstantByName( "delta", delta );
	Renderer::Get().SetPixelShader( m_logFilterPS );

	Renderer::Get().SetTexture( 0, pobBackBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	Renderer::Get().m_Platform.SetStream( m_hLogFilterData );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetTexture( 0, Texture::NONE );

	// recursively sum the texture down into the lowest level
	Renderer::Get().SetVertexShader( m_recursiveSumVS );
	Renderer::Get().SetPixelShader( m_recursiveSumPS );
	
	Renderer::Get().m_Platform.SetStream( m_hRecSumDataData );

	for( int iLevel = 1; iLevel < m_iKeyLevelCount; ++iLevel )
	{
		// set the viewport
		Renderer::Get().m_targetCache.SetColourTarget( m_pKeyValues[iLevel] );

		// render the sum into the level
		float fSampleOffset = 0.25f/_R( m_pKeyValues[iLevel]->GetWidth() );
		m_recursiveSumVS->SetVSConstantByName( "sample_offset", CVector( fSampleOffset, fSampleOffset, 0.0f, 0.0f ) );

		Renderer::Get().SetTexture( 0, m_pKeyValues[iLevel - 1]->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );

		Renderer::Get().SetTexture( 0, Texture::NONE );
	}

	RenderTarget::Ptr pInterpolatedKey		= Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastVal;
	RenderTarget::Ptr pInterpolatedKeyTemp	= Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastValTemp;

	// interpolate into the used key luminance texture
	Renderer::Get().SetVertexShader( m_simpleQuadVS );
	Renderer::Get().m_targetCache.SetColourTarget( pInterpolatedKeyTemp );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	if ( CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_F12 ) )
		Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureOveridden = true;
	else
		Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureOveridden = false;

	if (g_ResetExposure)
	{
		g_ResetExposure = false;

		m_srgbToLinear->SetPSConstantByName( "luminance_clamp",
			CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyLuminanceMin,  RenderingContext::Get()->m_exposureSettings.m_fKeyLuminanceMax, 0.0f, 0.0f ) );

		Renderer::Get().SetPixelShader( m_srgbToLinear );

		Renderer::Get().SetTexture( 0, m_pKeyValues[m_iKeyLevelCount - 1]->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	}
	else
	{
		Renderer::Get().SetTexture( 0, m_pKeyValues[m_iKeyLevelCount - 1]->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

		Renderer::Get().SetTexture( 1, pInterpolatedKey->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_POINT );

		float fCorrection = RenderingContext::Get()->m_exposureSettings.m_fErrorReduction * 0.01f;
		fCorrection = clamp( fCorrection, 0.0f, 1.0f );
		m_interpolationPS->SetPSConstantByName( "correction", CVector( fCorrection, 0.0f, 0.0f, 0.0f ) );
		m_interpolationPS->SetPSConstantByName( "luminance_clamp",
			CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyLuminanceMin,  RenderingContext::Get()->m_exposureSettings.m_fKeyLuminanceMax, 0.0f, 0.0f ) );

		Renderer::Get().SetPixelShader( m_interpolationPS );
	}

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );

	// make current
	Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastVal.Swap(
		Renderer::Get().m_pPIPManager->GetCurrentView().m_exposureLastValTemp );

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );

}

extern CVector g_vHackDinamicRange;

//--------------------------------------------------
//!
//!	EXPControllerGPU
//! Public interface to our GPU based exposure
//!
//--------------------------------------------------
EXPControllerGPU::EXPControllerGPU()
{
	m_pImpl = NT_NEW_CHUNK( Mem::MC_GFX ) EXPControllerGPU_impl;
}
EXPControllerGPU::~EXPControllerGPU()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pImpl );
}
void EXPControllerGPU::SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer )
{
	m_pImpl->SampleCurrBackBuffer( pBackBuffer );
}




//--------------------------------------------------
//!
//!	EXPControllerCPU
//! Public interface to our CPU based exposure
//!
//--------------------------------------------------
EXPControllerCPU::EXPControllerCPU()
{
	ntError_p(0,("EXPControllerCPU not supported on PS3"));
}
EXPControllerCPU::~EXPControllerCPU()
{
	ntError_p(0,("EXPControllerCPU not supported on PS3"));
}
void EXPControllerCPU::SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer )
{
	ntError_p(0,("EXPControllerCPU not supported on PS3"));
}

