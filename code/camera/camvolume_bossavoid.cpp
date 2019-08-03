//------------------------------------------------------------------------------------------
//!
//!	\file camvolume_bossavoid.cpp
//!
//------------------------------------------------------------------------------------------

#include "camvolume_bossavoid.h"
#include "objectdatabase/dataobject.h"
#include "game/entitymanager.h"

START_CHUNKED_INTERFACE( CamVolBossAvoid, Mem::MC_CAMERA )
	PUBLISH_VAR_AS( m_obPosition, Position )
	PUBLISH_VAR_AS( m_fHeight, Height )
	PUBLISH_VAR_AS( m_fRadius, Radius )
	PUBLISH_VAR_AS( m_fFieldStrength, FieldStrength )
END_STD_INTERFACE

CamVolBossAvoid::CamVolBossAvoid( void )
: m_obPosition( CONSTRUCT_CLEAR ),
  m_fHeight(1.0f),
  m_fRadius(1.0f),
  m_fFieldStrength(25.0f)
{
}

CamVolBossAvoid::~CamVolBossAvoid( void )
{
}

