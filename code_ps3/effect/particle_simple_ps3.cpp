//--------------------------------------------------
//!
//!	\file particle_simple.cpp
//!	Implementation of the various types of particle
//! handlers used by the simple particle system
//!
//--------------------------------------------------

#include "effect\particle_simple.h"
#include "effect\psystem_simple.h"
#include "gfx\renderer.h"

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::SetPixelShader
//! Retrieve the right kind of pixel shader for us
//!
//--------------------------------------------------
void ParticleHandlerSimple::SetPixelShader()
{
	if (m_bUsingPointSprites)
	{
		switch( m_pPSystemDef->m_eTexMode )
		{
		case PTM_UNTEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_point_notex_fp.sho" );
			break;

		case PTM_SIMPLE_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_point_tex_fp.sho" );
			break;

		case PTM_ANIM_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_point_animtex_fp.sho" );
			break;

		case PTM_RAND_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_point_randtex_fp.sho" );
			break;
		}
	}
	else
	{
		switch( m_pPSystemDef->m_eTexMode )
		{
		case PTM_UNTEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_quad_notex_fp.sho" );
			break;

		case PTM_SIMPLE_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_quad_tex_fp.sho" );
			break;

		case PTM_ANIM_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_quad_animtex_fp.sho" );
			break;

		case PTM_RAND_TEXTURED:
			m_pPixelShader = DebugShaderCache::Get().LoadShader( "psystem_simple_quad_randtex_fp.sho" );
			break;
		}
	}
}

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::PreRender
//! upload local parameters... these are specific to this particle effect
//!
//--------------------------------------------------
void ParticleHandlerSimple::PreRender()
{
	if (!m_bUsingCPUParticle)
	{
		CVector RCPParticleLifetime( 1.0f / m_pPSystemDef->m_fParticleLifetime, 0.0f, 0.0f, 0.0f );
		m_pVertexShader->SetVSConstantByName( "m_RCPlifetime", RCPParticleLifetime );

		m_pVertexShader->SetVSConstantByName( "m_force", m_pPSystemDef->m_acceleration );
	}

	CVector colStart( m_pPSystemDef->GetColourStart() );
	CVector	colourDiff( m_pPSystemDef->GetColourEnd() - m_pPSystemDef->GetColourStart() );

	colStart.W() = m_pPSystemDef->m_fAlphaStart;
	colourDiff.W() = m_pPSystemDef->m_fAlphaEnd - colStart.W();

	m_pVertexShader->SetVSConstantByName( "m_colourStart", colStart );
	m_pVertexShader->SetVSConstantByName( "m_colourDiff", colourDiff );
}

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::Render
//!
//--------------------------------------------------
void ParticleHandlerSimple::Render()
{
	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	m_pQuads->Render();
}

//--------------------------------------------------
//!
//!	PHS_RotatingSprite::PreRender
//! Rotation acceleration upload
//!
//--------------------------------------------------
void PHS_RotatingSprite::PreRender()
{
	ParticleHandlerSimple::PreRender();

	if (!m_bUsingCPUParticle)
	{
		CVector rotAcc( m_pParticleDef->GetRotationAcc(), 0.0f, 0.0f, 0.0f );
		m_pVertexShader->SetVSConstantByName( "m_rotAcc", rotAcc );
	}
}

//--------------------------------------------------
//!
//!	PHS_OrientedQuad::PreRender
//! Rotation acceleration upload
//!
//--------------------------------------------------
void PHS_OrientedQuad::PreRender()
{
	ParticleHandlerSimple::PreRender();

	CDirection Xaxis( m_pParticleDef->GetOrientation().GetXAxis() );
	CDirection Zaxis( m_pParticleDef->GetOrientation().GetZAxis() );

	m_pVertexShader->SetVSConstantByName( "m_Xaxis", Xaxis );
	m_pVertexShader->SetVSConstantByName( "m_Zaxis", Zaxis );
}

//--------------------------------------------------
//!
//!	PHS_OrientedQuad::PreRender
//! Rotation acceleration upload
//!
//--------------------------------------------------
void PHS_AxisAlignedRay::PreRender()
{
	ParticleHandlerSimple::PreRender();

	CDirection rotAxis = m_pParticleDef->GetDirection();
	m_pVertexShader->SetVSConstantByName( "m_rotAxis", rotAxis );
}

//--------------------------------------------------
//!
//!	PHS_VelScaledRay::PreRender
//! vel aligment and scale upload
//!
//--------------------------------------------------
void PHS_VelScaledRay::PreRender()
{
	ParticleHandlerSimple::PreRender();

	CVector	scales = m_pParticleDef->GetScales();
	m_pVertexShader->SetVSConstantByName( "m_scales", scales );

	if (!m_bUsingCPUParticle)
	{
		float fTimeScale = m_pParticleDef->m_bFixedTime ? (1.0f / 30.0f) : m_fLastTimeInterval;
		CVector	times( fTimeScale / m_pPSystemDef->m_fParticleLifetime, fTimeScale, 0.0f, 0.0f );

		m_pVertexShader->SetVSConstantByName( "m_timeOffset", times );
	}
}

