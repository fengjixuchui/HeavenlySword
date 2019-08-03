//--------------------------------------------------
//!
//!	\file particle_complex.cpp
//!	Implementation of the various types of particle
//! handlers used by the complex particle system
//!
//--------------------------------------------------

#include "effect\particle_complex.h"
#include "effect\psystem_complex.h"
#include "gfx\renderer.h"
#include "Gc/GcKernel.h"

//--------------------------------------------------
//!
//!	ParticleHandlerComplex::SetPixelShader
//! Retrieve the right kind of pixel shader for us
//!
//--------------------------------------------------
void ParticleHandlerComplex::SetPixelShader()
{
	if (m_bUsingPointSprites)
	{
		switch( m_pPSystemDef->m_eTexMode )
		{
		case PTM_UNTEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_point_notex_fp.sho" );
			break;

		case PTM_SIMPLE_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_point_tex_fp.sho" );
			break;

		case PTM_ANIM_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_point_animtex_fp.sho" );
			break;

		case PTM_RAND_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_point_randtex_fp.sho" );
			break;
		}
	}
	else
	{
		switch( m_pPSystemDef->m_eTexMode )
		{
		case PTM_UNTEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_quad_notex_fp.sho" );
			break;

		case PTM_SIMPLE_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_quad_tex_fp.sho" );
			break;

		case PTM_ANIM_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_quad_animtex_fp.sho" );
			break;

		case PTM_RAND_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_complex_quad_randtex_fp.sho" );
			break;
		}
	}
}

//--------------------------------------------------
//!
//!	ParticleHandlerComplex::PreRender
//! upload local parameters... these are specific to this particle effect
//!
//--------------------------------------------------
void ParticleHandlerComplex::PreRender()
{
	if (!m_bUsingCPUParticle)
	{
		CVector RCPParticleLifetime( 1.0f / m_pPSystemDef->m_fParticleLifetime, 0.0f, 0.0f, 0.0f );
		m_pVertexShader->SetVSConstantByName( "m_RCPlifetime", RCPParticleLifetime );

		m_pVertexShader->SetVSConstantByName( "m_force", m_pPSystemDef->m_acceleration );

		static const float FUNC_WIDTH = 256.f;
		CVector functionWidth( FUNC_WIDTH, 0.0f, 0.0f, 0.0f );
		m_pVertexShader->SetVSConstantByName( "m_functionWidth",functionWidth );
		
		CVector functionWidthRCP( 1.0f / FUNC_WIDTH, 0.0f, 0.0f, 0.0f );
		m_pVertexShader->SetVSConstantByName( "m_functionWidthRCP",functionWidthRCP );

		GcTextureHandle functions = m_pPSystemDef->m_resources.GetSizeFunctionTex()->m_Platform.GetTexture();
		functions->SetWrapS( Gc::kWrapModeClampToEdge );
		functions->SetWrapT( Gc::kWrapModeClampToEdge );
		functions->SetFilter( Gc::kFilterNearest, Gc::kFilterNearest );

		GcKernel::SetVertexProgramTexture( 0, functions );
	}
		
	Renderer::Get().SetTexture( 1, m_pPSystemDef->m_resources.GetPalette() );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );
}

//--------------------------------------------------
//!
//!	ParticleHandlerComplex::Render
//!
//--------------------------------------------------
void ParticleHandlerComplex::Render()
{
	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	m_pQuads->Render();
}

//--------------------------------------------------
//!
//!	PHC_RotatingSprite::PreRender
//! Rotation acceleration upload
//!
//--------------------------------------------------
void PHC_RotatingSprite::PreRender()
{
	ParticleHandlerComplex::PreRender();

	if (!m_bUsingCPUParticle)
	{
		CVector rotAcc( m_pParticleDef->GetRotationAcc(), 0.0f, 0.0f, 0.0f );
		m_pVertexShader->SetVSConstantByName( "m_rotAcc", rotAcc );
	}
}

//--------------------------------------------------
//!
//!	PHC_OrientedQuad::PreRender
//! Rotation acceleration upload
//!
//--------------------------------------------------
void PHC_OrientedQuad::PreRender()
{
	ParticleHandlerComplex::PreRender();

	CDirection Xaxis( m_pParticleDef->GetOrientation().GetXAxis() );
	CDirection Zaxis( m_pParticleDef->GetOrientation().GetZAxis() );

	m_pVertexShader->SetVSConstantByName( "m_Xaxis", Xaxis );
	m_pVertexShader->SetVSConstantByName( "m_Zaxis", Zaxis );
}

//--------------------------------------------------
//!
//!	PHC_AxisAlignedRay::PreRender
//! orientation axis upload
//!
//--------------------------------------------------
void PHC_AxisAlignedRay::PreRender()
{
	ParticleHandlerComplex::PreRender();

	CDirection rotAxis = m_pParticleDef->GetDirection();
	m_pVertexShader->SetVSConstantByName( "m_rotAxis", rotAxis );
}

//--------------------------------------------------
//!
//!	PHC_VelScaledRay::PreRender
//! vel aligment and scale upload
//!
//--------------------------------------------------
void PHC_VelScaledRay::PreRender()
{
	ParticleHandlerComplex::PreRender();

	CVector	scales = m_pParticleDef->GetScales();
	m_pVertexShader->SetVSConstantByName( "m_scales", scales );

	if (!m_bUsingCPUParticle)
	{
		float fTimeScale = m_pParticleDef->m_bFixedTime ? (1.0f / 30.0f) : m_fLastTimeInterval;
		CVector	times( fTimeScale / m_pPSystemDef->m_fParticleLifetime, fTimeScale, 0.0f, 0.0f );

		m_pVertexShader->SetVSConstantByName( "m_timeOffset", times );
	}
}

