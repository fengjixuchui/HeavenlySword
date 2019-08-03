//--------------------------------------------------
//!
//!	\file WaterInstanceDef.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#include "water/WaterInstanceDef.h"
#include "water/WaterInstance.h"
#include "water/watermanager.h"
#include "water/waterwaveemitter.h"
#include "water/waterbuoyproxy.h"
#include "anim/transform.h"
#include "anim/hierarchy.h"
#include "game/randmanager.h"
#include "objectdatabase/dataobject.h"




// yay! another force-link function
void ForceLinkFunctionWaterInstanceDef()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionWaterInstanceDef() !ATTN!\n");
}


static const CPoint		DEF_POSITION		( 0.0f, 0.0f, 0.0f );
static const CQuat		DEF_ORIENTATION		( CONSTRUCT_IDENTITY );
static const float		DEF_ROTATION		= 0.0f;
static const float		DEF_RESOLUTION		= 0.5f;
static const float		DEF_LENGTH			= 10.0f;
static const float		DEF_WIDTH			= 10.0f;
static const float		DEF_VSCALE			= 1.0f;

static const float		DEF_SPECULARPOWER	= 16.0f;
static const float		DEF_REFLECTIVITY	= 0.5f;
static const float		DEF_FRESNEL			= 0.5f;
static const CVector	DEF_MAPSPEED		( 0.05f, 0.03f, 0.08f, 0.05f );
static const CVector	DEF_MAPSIZE			( 0.5f, 0.5f, 0.0f, 0.0f );
static const CVector	DEF_BASECOLOUR		( 0.001f, 0.001f, 0.008f, 1.0f );


START_STD_INTERFACE( WaterInstanceDef )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obPos,							DEF_POSITION,				Position )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obRot,							DEF_ORIENTATION,			Orientation )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fWidth,							DEF_WIDTH,					Width )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fLength,						DEF_LENGTH,					Length )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fResolution,					DEF_RESOLUTION,				Resolution )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fVScale,						DEF_VSCALE,					VScale )
	PUBLISH_PTR_CONTAINER_AS( m_obEmitters,														Emitters )
	PUBLISH_PTR_CONTAINER_AS( m_obBuoyProxies,													Buoys )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obNormalMap0,					"cellbump_normal.dds",		NormalMap0 )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obNormalMap1,					"fractalbump_normal.dds",	NormalMap1 )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obMapSpeed,						DEF_MAPSPEED,				NormalMapSpeed )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obMapSize,						DEF_MAPSIZE,				NormalMapSize )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obBaseColour,					DEF_BASECOLOUR,				BaseColour )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fFresnelStrength,				DEF_FRESNEL,				FresnelStrength )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fReflectivity,					DEF_REFLECTIVITY,			Reflectivity )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fSpecularPower,					DEF_SPECULARPOWER,			SpecularPower )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iSectorBits,					0,							SectorBits )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue );
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK( PostPostConstruct );
END_STD_INTERFACE


WaterInstanceDef::WaterInstanceDef()
: m_obTransform()
, m_pobInstance( 0 )
{
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( &m_obTransform );
}

WaterInstanceDef::~WaterInstanceDef()
{
	if ( m_pobInstance )
	{
		WaterManager::Get().DestroyWaterInstance( *this );
	}

	if (m_obTransform.GetParent())
	{
		m_obTransform.RemoveFromParent();
	}

}

void WaterInstanceDef::PostPostConstruct( void )
{
	//if ( WaterManager::Get().IsEnabled() )
	{
		m_pobInstance = WaterManager::Get().GetWaterInstance( *this );
		EditorChangeValue(0, 0);
	}
}

bool WaterInstanceDef::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	CMatrix obMat( m_obRot, m_obPos );
	m_obTransform.SetLocalMatrix( obMat );
	
	if ( /*WaterManager::Get().IsEnabled() &&*/  m_pobInstance )
	{
		for ( ntstd::List<WaveEmitter*>::iterator it = m_obEmitters.begin(); it != m_obEmitters.end(); ++it )
		{
			if ( !(*it)->GetParentWaterInstance()  ) 
				(*it)->InstallParentWaterInstance( m_pobInstance );
		}

		for ( ntstd::List<BuoyProxy*>::iterator it = m_obBuoyProxies.begin(); it != m_obBuoyProxies.end(); ++it )
		{
			if ( !(*it)->GetParentWaterInstance()  ) 
				(*it)->InstallParentWaterInstance( m_pobInstance );
		}

		return true;
	}

	return false;
}



//eof


