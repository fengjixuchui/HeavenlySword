//--------------------------------------------------
//!
//!	\file psystem_simple_ps3.cpp
//!	render method for PS3 based particle systems
//!
//--------------------------------------------------

#include "effect/psystem_simple.h"

//--------------------------------------------------
//!
//!	render our particles
//!
//--------------------------------------------------
void PSystemSimple::RenderEffect( void )
{	
	m_pParticles->PreRender();
	
	m_renderstates.SetRenderstates();

	UploadStdParameters(	m_pParticles->GetVertexShader(),
							m_pParticles->GetPixelShader(),
							m_pParticles->UsingPointSprites(),
							m_pParticles->UsingCPUParticles() );

	m_pParticles->Render();

	m_renderstates.ClearRenderstates();
}
