//--------------------------------------------------
//!
//!	\file psystem_simple_pc.cpp
//!	render method for PC based particle systems
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

	UploadStdParameters( m_pParticles->GetEffect() );

	m_pParticles->Render();

	m_renderstates.ClearRenderstates();
}
