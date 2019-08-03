//--------------------------------------------------
//!
//!	\file army_impostors.cpp
//! 
//!
//--------------------------------------------------

#include "army_impostors.h"
#include "anim/hierarchy.h"
#include "game/randmanager.h"

//--------------------------------------------------
//!
//!	ArmyImpostors::ctor
//!
//--------------------------------------------------
ArmyImpostors::ArmyImpostors( int iMaxImpostors, const ImpostorDef& def ) :
	CRenderable(&m_transform,true,true,true, RT_ARMYIMPOSTOR)
{
	m_transform.SetLocalMatrix( CMatrix( CONSTRUCT_IDENTITY ) );
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( &m_transform );

	m_pImpostors = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PointImpostor( def, iMaxImpostors );
}

ArmyImpostors::~ArmyImpostors()
{
	m_transform.RemoveFromParent();
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pImpostors );
}

//--------------------------------------------------
//!
//!	ArmyImpostors::PostUpdate
//! Called from ListSpace::SetVisibleFrustum
//!
//--------------------------------------------------
void ArmyImpostors::PostUpdate()
{
	m_pImpostors->m_FrameFlags = m_FrameFlags;
}

//--------------------------------------------------
//!
//!	ArmyImpostors::RenderMaterial
//!
//--------------------------------------------------
void ArmyImpostors::RenderMaterial()
{
	m_pImpostors->RenderMaterial();
}

//--------------------------------------------------
//!
//!	ArmyImpostors::RenderShadowMap
//!
//--------------------------------------------------
void ArmyImpostors::RenderShadowMap()
{
	m_pImpostors->RenderShadowMap();
}

//--------------------------------------------------
//!
//!	ArmyImpostors::RenderDepth
//!
//--------------------------------------------------
void ArmyImpostors::RenderDepth()
{
	m_pImpostors->RenderDepth();
}

//--------------------------------------------------
//!
//!	ArmyImpostors::GetUpdateFrameFlagsCallback
//!
//--------------------------------------------------
IFrameFlagsUpdateCallback* ArmyImpostors::GetUpdateFrameFlagsCallback() const
{
    return (IFrameFlagsUpdateCallback*)this;
}
