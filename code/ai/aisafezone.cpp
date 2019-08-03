//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aisafezone.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------

                                                                                            
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aisafezone.h"
#include "ai/ainavgraphmanager.h"

#include "core/visualdebugger.h"


void AISafeZone::PostConstruct()
{
	CAINavGraphManager::Get().AddSafeZone( this );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AISafeZone::SetCentre		                                            
//! Set the centre of the safezone                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
void AISafeZone::SetCentre( const CPoint& centre ) 
{
    m_obCentre = centre;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AISafeZone::SetRadius		                                            
//! Set the radius of the safezone                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
void AISafeZone::SetRadius( const float radius ) 
{
    m_fRadius = radius;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AISafeZone::InZone		                                            
//! returns true if the circle described falls entirely within the safezone                                  
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AISafeZone::InZone( const CPoint& pos, const float radius ) const
{
	// if dist from pos to zone centre is less than the sum of the radii, then point is in zone
	return (pos - m_obCentre).LengthSquared() < ((radius + m_fRadius) * (radius + m_fRadius));
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AISafeZone::DebugRender		                                            
//!                                                                                         
//------------------------------------------------------------------------------------------

void AISafeZone::PaulsDebugRender() const
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderSphere( CVecMath::GetQuatIdentity(), m_obCentre, m_fRadius, 0xffffffff, DPF_WIREFRAME );
#endif
}



