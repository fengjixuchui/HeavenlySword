//--------------------------------------------------
//!
//!	\file psystem_complex.cpp
//!	implementation of complex particle system and its
//! definition object
///!
//--------------------------------------------------

#include "effect/psystem_complex.h"

//--------------------------------------------------
//!
//!	render our particles
//!
//--------------------------------------------------
void PSystemComplex::RenderEffect( void )
{	
	m_pParticles->PreRender();

	m_renderstates.SetRenderstates();

	UploadStdParameters( m_pParticles->GetEffect() );

	m_pParticles->Render();

	m_renderstates.ClearRenderstates();
}
