//--------------------------------------------------
//!
//!	\file psystem_complex.cpp
//!	implementation of complex particle system and its
//! definition object
///!
//--------------------------------------------------

#include "effect/psystem_complex.h"
#include "Gc/GcKernel.h"

//--------------------------------------------------
//!
//!	render our particles
//!
//--------------------------------------------------
void PSystemComplex::RenderEffect( void )
{	
	m_pParticles->PreRender();

	m_renderstates.SetRenderstates();

	UploadStdParameters(	m_pParticles->GetVertexShader(),
							m_pParticles->GetPixelShader(),
							m_pParticles->UsingPointSprites(),
							m_pParticles->UsingCPUParticles() );

	m_pParticles->Render();

	m_renderstates.ClearRenderstates();

#ifndef _RELEASE
	if (!m_pParticles->UsingCPUParticles())
		GcKernel::DisableVertexProgramTexture(0);
#endif
}
