//--------------------------------------------------
//!
//!	\file particle_complex.cpp
//!	Implementation of the various types of particle
//! handlers used by the complex particle system
//!
//--------------------------------------------------

#include "effect\particle_complex.h"
#include "effect\psystem_complex.h"

//--------------------------------------------------
//!
//!	ParticleHandlerComplex::PreRender
//! upload local parameters... these are specific to this particle effect
//!
//--------------------------------------------------
void ParticleHandlerComplex::PreRender()
{
	PSystemUtils::SetFXTechique( m_ppFX->Get(), m_pPSystemDef->m_eTexMode, m_pOwner->GetRenderstates() );
	
	D3DXHANDLE h;

	if (!m_bUsingCPUParticle)
	{
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_particleLifetime", &m_pPSystemDef->m_fParticleLifetime, sizeof(float) );

		float fRCPParticleLifetime = 1.0f / m_pPSystemDef->m_fParticleLifetime;
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_RCPlifetime", &fRCPParticleLifetime, sizeof(float) );

		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_force", &m_pPSystemDef->m_acceleration, sizeof(float) * 3 );

		float functionWidth = 256.0f;
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_functionWidth", &functionWidth, sizeof(float) );

		float functionWidthRCP = 1.0f / functionWidth;
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_functionWidthRCP", &functionWidthRCP, sizeof(float) );

		FX_GET_HANDLE_FROM_NAME( (*m_ppFX), h, "m_functions" );
		(*m_ppFX)->SetTexture( h, m_pPSystemDef->m_resources.GetSizeFunctionTex()->m_Platform.Get2DTexture() );
	}
	
	FX_GET_HANDLE_FROM_NAME( (*m_ppFX), h, "m_palette0" );
	(*m_ppFX)->SetTexture( h, m_pPSystemDef->m_resources.GetPalette()->m_Platform.Get2DTexture() );
}

//--------------------------------------------------
//!
//!	ParticleHandlerComplex::Render
//!
//--------------------------------------------------
void ParticleHandlerComplex::Render()
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
//!	PHC_RotatingSprite::PreRender
//! Rotation acceleration upload
//!
//--------------------------------------------------
void PHC_RotatingSprite::PreRender()
{
	ParticleHandlerComplex::PreRender();

	if (!m_bUsingCPUParticle)
	{
		float fRotAcc = m_pParticleDef->GetRotationAcc();
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_rotAcc", &fRotAcc, sizeof(float) );
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

	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_Xaxis", &Xaxis, sizeof(float) * 3 );
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_Zaxis", &Zaxis, sizeof(float) * 3 );
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

	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_rotAxis", &rotAxis, sizeof(float) * 3 );
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
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_scales", &scales, sizeof(float) * 2 );

	if (!m_bUsingCPUParticle)
	{
		float fTimeScale = m_pParticleDef->m_bFixedTime ? (1.0f / 30.0f) : m_fLastTimeInterval;
		CVector	times( fTimeScale / m_pPSystemDef->m_fParticleLifetime, fTimeScale, 0.0f, 0.0f );

		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_timeOffset", &times, sizeof(float) * 2 );
	}
}

