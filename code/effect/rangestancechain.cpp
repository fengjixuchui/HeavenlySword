//--------------------------------------------------
//!
//!	\file rangestancechain.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "rangestancechain.h"
#include "weaponchains.h"
#include "core/timer.h"
#include "anim/transform.h"
#include "objectdatabase/dataobject.h"

//--------------------------------------------------
//!
//!	RangeStanceChain::ctor
//!
//--------------------------------------------------
RangeStanceChain::RangeStanceChain( const Transform* pParentTransform, const Transform* pLinkTransform ) :
	CRenderable(pParentTransform,true,true,true, RT_RANGESTANCE_CHAIN),
	m_pLinkTransform(pLinkTransform)
{
	WeaponChainDef* pDef = ObjectDatabase::Get().GetPointerFromName<WeaponChainDef*>("RangeStanceChains");
	m_pChainEffect = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) WeaponChain(2,pDef);

	m_iLastRefresh = 0xffffffff;
}

RangeStanceChain::~RangeStanceChain()
{
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pChainEffect );
}

//--------------------------------------------------
//!
//!	RangeStanceChain::SynchChainPos
//! make sure chain is setup
//!
//--------------------------------------------------
void RangeStanceChain::SynchChainPos()
{
	if (m_iLastRefresh == CTimer::Get().GetSystemTicks())
		return;
	
	m_iLastRefresh = CTimer::Get().GetSystemTicks();

	// GetWorldMatrixFast() is valid here, as our heirachy MUST have been updated by now
	CPoint chainStart = m_pobTransform->GetWorldMatrixFast().GetTranslation();
	CPoint chainEnd = m_pLinkTransform->GetWorldMatrixFast().GetTranslation();

	// set rendereable bounds
	CPoint max = chainEnd * m_pobTransform->GetWorldMatrixFast().GetAffineInverse();

	m_obBounds.Min() = max.Min( CPoint(0.0f,0.0f,0.0f) );
	m_obBounds.Max() = max.Max( CPoint(0.0f,0.0f,0.0f) );

	// update the chain
	m_pChainEffect->SetVertexPosition( chainStart, 0 );
	m_pChainEffect->SetVertexPosition( chainEnd, 1 );
	m_pChainEffect->BuildVertexBuffer();
}

//--------------------------------------------------
//!
//!	RangeStanceChain::PreUpdate
//! Called from ListSpace::SetVisibleFrustum
//!
//--------------------------------------------------
void RangeStanceChain::PreUpdate()
{
    SynchChainPos();
}

//--------------------------------------------------
//!
//!	RangeStanceChain::PostUpdate
//! Called from ListSpace::SetVisibleFrustum
//!
//--------------------------------------------------
void RangeStanceChain::PostUpdate()
{
	m_pChainEffect->m_FrameFlags = m_FrameFlags;
}


//--------------------------------------------------
//!
//!	RangeStanceChain::RenderMaterial
//!
//--------------------------------------------------
void RangeStanceChain::RenderMaterial()
{
	m_pChainEffect->RenderMaterial();
}

//--------------------------------------------------
//!
//!	RangeStanceChain::RenderShadowMap
//!
//--------------------------------------------------
void RangeStanceChain::RenderShadowMap()
{
	m_pChainEffect->RenderShadowMap();
}

//--------------------------------------------------
//!
//!	RangeStanceChain::RenderShadowOnly
//!
//--------------------------------------------------
void RangeStanceChain::RenderShadowOnly()
{
	m_pChainEffect->RenderRecieveShadowMap();
}

//--------------------------------------------------
//!
//!	RangeStanceChain::RenderDepth
//!
//--------------------------------------------------
void RangeStanceChain::RenderDepth()
{
	m_pChainEffect->RenderDepth();
}

//--------------------------------------------------
//!
//!	RangeStanceChain::GetUpdateFrameFlagsCallback
//!
//--------------------------------------------------
IFrameFlagsUpdateCallback* RangeStanceChain::GetUpdateFrameFlagsCallback() const
{
    return (IFrameFlagsUpdateCallback*)this;
}
