//--------------------------------------------------
//!
//!	\file chainmanchains.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "chainmanchains.h"
#include "weaponchains.h"
#include "core/timer.h"
#include "anim/transform.h"
#include "anim/hierarchy.h"
#include "objectdatabase/dataobject.h"
#include "core/visualdebugger.h"

//--------------------------------------------------
//!
//!	ChainmanChains::ctor
//!
//-------------------------------------------------
ChainmanChains::ChainmanChains( const Transform* pParentTransform ) :
	CRenderable(pParentTransform,true,true,true, RT_CHAINMAN_CHAINS)
{
	ATTACH_LUA_INTERFACE(ChainmanChains);

	WeaponChainDef* pDef = ObjectDatabase::Get().GetPointerFromName<WeaponChainDef*>("ChainmanChains");
	m_pWeaponChain = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) WeaponChain((LINKS_PER_SEGMENT * WEAPON_CHAIN_SEGMENTS) + 1,pDef);
	m_pHandChain = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) WeaponChain((LINKS_PER_SEGMENT * HAND_CHAIN_SEGMENTS) + 1,pDef);
	m_iLastRefresh = 0xffffffff;

	// Cache heirachy transforms
	CHierarchy* pHierarchy = m_pobTransform->GetParentHierarchy();
	ntAssert(pHierarchy);
	int iIndex;

	iIndex = pHierarchy->GetTransformIndex( CHashedString( "l_weapon" ) );
	ntAssert( iIndex != -1 );		
	m_pLWeapon = pHierarchy->GetTransform( iIndex );	

	iIndex = pHierarchy->GetTransformIndex( CHashedString( "l_chain_01" ) );
	ntAssert( iIndex != -1 );		
	m_pLChain1 = pHierarchy->GetTransform( iIndex );	

	iIndex = pHierarchy->GetTransformIndex( CHashedString( "r_chain_01" ) );
	ntAssert( iIndex != -1 );		
	m_pRChain1 = pHierarchy->GetTransform( iIndex );	

	iIndex = pHierarchy->GetTransformIndex( CHashedString( "r_chain_02" ) );
	ntAssert( iIndex != -1 );		
	m_pRChain2 = pHierarchy->GetTransform( iIndex );	

	iIndex = pHierarchy->GetTransformIndex( CHashedString( "r_chain_03" ) );
	ntAssert( iIndex != -1 );		
	m_pRChain3 = pHierarchy->GetTransform( iIndex );	

	iIndex = pHierarchy->GetTransformIndex( CHashedString( "r_chain_04" ) );
	ntAssert( iIndex != -1 );		
	m_pRChain4 = pHierarchy->GetTransform( iIndex );	
}

ChainmanChains::~ChainmanChains()
{
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pWeaponChain );
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pHandChain );
}

//--------------------------------------------------
//!
//!	ChainmanChains::SynchChainPos
//! make sure chain is setup
//!
//--------------------------------------------------
void ChainmanChains::SynchChainPos()
{
	if (m_iLastRefresh == CTimer::Get().GetSystemTicks())
		return;
	
	m_iLastRefresh = CTimer::Get().GetSystemTicks();

	m_cachedTransInv = m_pobTransform->GetWorldMatrixFast().GetAffineInverse();
	m_obBounds.Min() = CPoint( MAX_POS_FLOAT, MAX_POS_FLOAT, MAX_POS_FLOAT );
	m_obBounds.Max() = CPoint( -MAX_POS_FLOAT, -MAX_POS_FLOAT, -MAX_POS_FLOAT );

	// construct our current interpolated weapon curve	
	m_weaponCurveNodes[1] = m_pRChain4->GetWorldMatrixFast().GetTranslation();
	m_weaponCurveNodes[2] = m_pRChain3->GetWorldMatrixFast().GetTranslation();
	m_weaponCurveNodes[3] = m_pRChain2->GetWorldMatrixFast().GetTranslation();
	m_weaponCurveNodes[4] = m_pRChain1->GetWorldMatrixFast().GetTranslation();

	m_weaponCurveNodes[0] = (3.0f*m_weaponCurveNodes[1]) - (3.0f*m_weaponCurveNodes[2]) + m_weaponCurveNodes[3];
	m_weaponCurveNodes[5] = (3.0f*m_weaponCurveNodes[4]) - (3.0f*m_weaponCurveNodes[3]) + m_weaponCurveNodes[2];

	// construct our current interpolated hand chain curve
	m_handCurveNodes[1] = m_pLWeapon->GetWorldMatrixFast().GetTranslation();
	m_handCurveNodes[2] = m_pLChain1->GetWorldMatrixFast().GetTranslation();
	m_handCurveNodes[3] = m_pRChain1->GetWorldMatrixFast().GetTranslation();

	m_handCurveNodes[0] = (3.0f*m_handCurveNodes[1]) - (3.0f*m_handCurveNodes[2]) + m_handCurveNodes[3];
	m_handCurveNodes[4] = (3.0f*m_handCurveNodes[3]) - (3.0f*m_handCurveNodes[2]) + m_handCurveNodes[1];

	// now populate our chain primitives from these curves
//	DebugRenderSegmentList( WEAPON_CHAIN_SEGMENTS, m_weaponCurveNodes );
//	DebugRenderSegmentList( HAND_CHAIN_SEGMENTS, m_handCurveNodes );

	PopulateChainEffect( WEAPON_CHAIN_SEGMENTS, m_weaponCurveNodes, m_pWeaponChain );
	PopulateChainEffect( HAND_CHAIN_SEGMENTS, m_handCurveNodes, m_pHandChain );
}

//--------------------------------------------------
//!
//!	ChainmanChains::DebugRenderSegmentList
//!
//--------------------------------------------------
void ChainmanChains::DebugRenderSegmentList( int iNumSegs, CPoint* pPoints )
{
#ifndef _GOLD_MASTER
	CVector coeffs[4];

	static const float gafBasisMat[4][4] =
	{
		{ -0.5f,  1.5f, -1.5f,  0.5f },
		{  1.0f, -2.5f,  2.0f, -0.5f },
		{ -0.5f,  0.0f,  0.5f,  0.0f },
		{  0.0f,  1.0f,  0.0f,  0.0f }
	};

	for (int i = 0; i < iNumSegs; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			coeffs[j].Clear();
			coeffs[j] += CVector(pPoints[i+0]) * gafBasisMat[j][0];
			coeffs[j] += CVector(pPoints[i+1]) * gafBasisMat[j][1];
			coeffs[j] += CVector(pPoints[i+2]) * gafBasisMat[j][2];
			coeffs[j] += CVector(pPoints[i+3]) * gafBasisMat[j][3];
		}

		static const int iSubdivs = 10;
		static const float fInterval = 1.0f / iSubdivs;
		for (int k = 0; k < iSubdivs; k++)
		{
			CPoint start = EvaluateSegment( coeffs, fInterval * k );
			CPoint end = EvaluateSegment( coeffs, fInterval * (k+1) );
			g_VisualDebug->RenderLine( start, end, 0xffff0000, 0 );
		}
	}
#endif
}

//--------------------------------------------------
//!
//!	ChainmanChains::PopulateChainEffect
//!
//--------------------------------------------------
void ChainmanChains::PopulateChainEffect( int iNumSegs, CPoint* pPoints, WeaponChain* pChain )
{
	CVector coeffs[4];

	static const float gafBasisMat[4][4] =
	{
		{ -0.5f,  1.5f, -1.5f,  0.5f },
		{  1.0f, -2.5f,  2.0f, -0.5f },
		{ -0.5f,  0.0f,  0.5f,  0.0f },
		{  0.0f,  1.0f,  0.0f,  0.0f }
	};

	for (int i = 0; i < iNumSegs; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			coeffs[j].Clear();
			coeffs[j] += CVector(pPoints[i+0]) * gafBasisMat[j][0];
			coeffs[j] += CVector(pPoints[i+1]) * gafBasisMat[j][1];
			coeffs[j] += CVector(pPoints[i+2]) * gafBasisMat[j][2];
			coeffs[j] += CVector(pPoints[i+3]) * gafBasisMat[j][3];
		}

		float fInterval = 1.0f / LINKS_PER_SEGMENT;
		for (int k = 0; k < LINKS_PER_SEGMENT; k++)
		{
			CPoint linkPos = EvaluateSegment( coeffs, fInterval * k );
			pChain->SetVertexPosition( linkPos, (i * LINKS_PER_SEGMENT) + k );
			ConsiderForMinMax( linkPos );
		}
	}

	// fill in final one
	CPoint linkPos = EvaluateSegment( coeffs, 1.0f );
	pChain->SetVertexPosition( linkPos, (iNumSegs * LINKS_PER_SEGMENT) );
	ConsiderForMinMax( linkPos );
}

//--------------------------------------------------
//!
//!	ChainmanChains::PreUpdate
//! Called from ListSpace::SetVisibleFrustum
//!
//--------------------------------------------------
void ChainmanChains::PreUpdate()
{
    SynchChainPos();
}

//--------------------------------------------------
//!
//!	ChainmanChains::PostUpdate
//! Called from ListSpace::SetVisibleFrustum
//!
//--------------------------------------------------
void ChainmanChains::PostUpdate()
{
	m_pWeaponChain->m_FrameFlags = m_FrameFlags;
	m_pHandChain->m_FrameFlags = m_FrameFlags;
}

//--------------------------------------------------
//!
//!	ChainmanChains::RenderMaterial
//!
//--------------------------------------------------
void ChainmanChains::RenderMaterial()
{
	m_pWeaponChain->RenderMaterial();
	m_pHandChain->RenderMaterial();
}

//--------------------------------------------------
//!
//!	ChainmanChains::RenderShadowMap
//!
//--------------------------------------------------
void ChainmanChains::RenderShadowMap()
{
	m_pWeaponChain->RenderShadowMap();
	m_pHandChain->RenderShadowMap();
}

//--------------------------------------------------
//!
//!	ChainmanChains::RenderShadowOnly
//!
//--------------------------------------------------
void ChainmanChains::RenderShadowOnly()
{
	m_pWeaponChain->RenderRecieveShadowMap();
	m_pHandChain->RenderRecieveShadowMap();
}

//--------------------------------------------------
//!
//!	ChainmanChains::RenderShadowOnly
//!
//--------------------------------------------------
void ChainmanChains::RenderDepth()
{
	m_pWeaponChain->RenderDepth();
	m_pHandChain->RenderDepth();
}

//--------------------------------------------------
//!
//!	ChainmanChains::GetUpdateFrameFlagsCallback
//!
//--------------------------------------------------
IFrameFlagsUpdateCallback* ChainmanChains::GetUpdateFrameFlagsCallback() const
{
    return (IFrameFlagsUpdateCallback*) this;
}

