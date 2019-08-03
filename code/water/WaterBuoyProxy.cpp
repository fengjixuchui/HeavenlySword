//--------------------------------------------------
//!
//!	\file WaterBuoyProxy.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------



#include "water/waterbuoyproxy.h"
#include "water/waterdmadata.h"
#include "water/watermanager.h"
#include "water/waterinstance.h"
#include "anim/transform.h"
#include "game/randmanager.h"
#include "editable/enumlist.h"
#include "objectdatabase/dataobject.h"



static const float		DEF_BUOYANCY		= 0.8f;
static const float		DEF_TRAVELSPEED		= 0.4f;


START_STD_INTERFACE( BuoyProxy )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obUpDirection,					CVecMath::GetYAxis(),	UpDirection )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fBuoyancy,						DEF_BUOYANCY,			Buoyancy )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fTravelSpeed,					DEF_TRAVELSPEED,		TravelSpeed )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, Active, IsActive, SetActive, false )
	PUBLISH_PTR_AS( m_pobTransform, Transform )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue );
END_STD_INTERFACE


BuoyProxy::BuoyProxy()
	: m_obUpDirection( CVecMath::GetYAxis() )
	, m_fBuoyancy( 0 )
	, m_fTravelSpeed( 0 )
	, m_pobTransform( 0 )
	, m_pobWaterInstance( 0 )
	, m_pobBuoyDma( 0 )
	, m_bActive( false )
{
	// nothing
}


BuoyProxy::~BuoyProxy()
{
	SetActive( false );
}


bool BuoyProxy::Update( void )
{
	
	ntAssert_p( m_pobWaterInstance, ("WATER - BuoyProxy doesn't have an installed waterinstance\n") );

	if ( m_bActive && m_pobBuoyDma && m_pobTransform )
	{
		const CMatrix& waterToWorld = m_pobWaterInstance->GetWaterToWorldMatrix();
		m_pobTransform->SetLocalMatrixFromWorldMatrix( m_pobBuoyDma->m_obLocalMatrix * waterToWorld );
		return true;
	}
	
	

	return false;
}

bool BuoyProxy::IsActive( void ) const
{
	return m_bActive; 
}


void BuoyProxy::SetActive( const bool& bState )
{
	if ( bState && m_pobWaterInstance && !m_pobBuoyDma )
	{
		m_pobBuoyDma = m_pobWaterInstance->GetFirstAvailableBuoySlot();
		user_warn_p( m_pobBuoyDma, ("WATER - couldn't get an available buoy slot\n") );
		if ( m_pobBuoyDma && m_pobTransform )
		{
			const CMatrix& worldToWater = m_pobWaterInstance->GetWorldToWaterMatrix();
			m_pobBuoyDma->m_obLocalMatrix = m_pobTransform->GetWorldMatrix() * worldToWater;
		}
	}
	else if ( m_pobBuoyDma )
	{
		memset( m_pobBuoyDma, 0, sizeof(BuoyDma) );
		m_pobBuoyDma->m_iFlags |= kBF_Control_Invalid;
		m_pobBuoyDma = 0;
	}
	
}


bool BuoyProxy::EditorChangeValue(CallBackParameter, CallBackParameter)
{	
	if ( m_pobBuoyDma )
	{
		m_pobBuoyDma->m_fTravelSpeed = m_fTravelSpeed;
		m_pobBuoyDma->m_fBuoyancy = m_fBuoyancy;
		m_pobBuoyDma->m_obUpDirection = m_obUpDirection;
		return true;
	}

	return false;
}


void BuoyProxy::InstallParentWaterInstance( WaterInstance* pobWaterInstance )
{
	ntAssert_p( pobWaterInstance, ("WATER - BuoyProxy cannot install a NULL waterinstance\n") );
	
	SetActive( false );
	m_pobWaterInstance = pobWaterInstance;
}




//eof


