//--------------------------------------------------
//!
//!	\file particle_simple.cpp
//!	Implementation of the various types of particle
//! handlers used by the simple particle system
//!
//--------------------------------------------------

#include "effect\particle_simple.h"
#include "effect\psystem_simple.h"

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::PreRender
//! upload local parameters... these are specific to this particle effect
//!
//--------------------------------------------------
void ParticleHandlerSimple::PreRender()
{
	PSystemUtils::SetFXTechique( m_ppFX->Get(), m_pPSystemDef->m_eTexMode, m_pOwner->GetRenderstates() );

	if (!m_bUsingCPUParticle)
	{
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_particleLifetime", &m_pPSystemDef->m_fParticleLifetime, sizeof(float) );

		float fRCPParticleLifetime = 1.0f / m_pPSystemDef->m_fParticleLifetime;
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_RCPlifetime", &fRCPParticleLifetime, sizeof(float) );

		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_force", &m_pPSystemDef->m_acceleration, sizeof(float) * 3 );
	}

	CVector colStart( m_pPSystemDef->GetColourStart() );
	CVector	colourDiff( m_pPSystemDef->GetColourEnd() - m_pPSystemDef->GetColourStart() );

	colStart.W() = m_pPSystemDef->m_fAlphaStart;
	colourDiff.W() = m_pPSystemDef->m_fAlphaEnd - colStart.W();

	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_colourStart", &colStart, sizeof(float) * 4 );
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_colourDiff", &colourDiff, sizeof(float) * 4 );
}

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::Render
//!
//--------------------------------------------------
void ParticleHandlerSimple::Render()
{
	u_int iNumPasses;
	(*m_ppFX)->Begin( &iNumPasses, 0 );

	ntError_p( iNumPasses == 1, ("Multipass not supported") );
	(*m_ppFX)->BeginPass(0);

	m_pQuads->Render();

	(*m_ppFX)->EndPass();
	(*m_ppFX)->End();
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
		float fRotAcc = m_pParticleDef->GetRotationAcc();
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_rotAcc", &fRotAcc, sizeof(float) );
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

	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_Xaxis", &Xaxis, sizeof(float) * 3 );
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_Zaxis", &Zaxis, sizeof(float) * 3 );
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
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_rotAxis", &rotAxis, sizeof(float) * 3 );
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
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_scales", &scales, sizeof(float) * 2 );

	if (!m_bUsingCPUParticle)
	{
		float fTimeScale = m_pParticleDef->m_bFixedTime ? (1.0f / 30.0f) : m_fLastTimeInterval;
		CVector	times( fTimeScale / m_pPSystemDef->m_fParticleLifetime, fTimeScale, 0.0f, 0.0f );

		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_timeOffset", &times, sizeof(float) * 2 );
	}
}

