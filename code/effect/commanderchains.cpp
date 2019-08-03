//--------------------------------------------------
//!
//!	\file commanderchains.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "commanderchains.h"
#include "weaponchains.h"
#include "core/timer.h"
#include "objectdatabase/dataobject.h"
#include "anim/transform.h"

//--------------------------------------------------
//!
//!	CommanderChains::ctor
//!
//-------------------------------------------------
CommanderChains::CommanderChains(	Transform* pParentTransform,
									const ChainAnimation2Render* pChains ) :
	CRenderable(pParentTransform,true,true,true, RT_COMMANDER_CHAINS),
	m_pSrcChains(pChains)
{
	ntAssert( m_pSrcChains );

	WeaponChainDef* pDef = ObjectDatabase::Get().GetPointerFromName<WeaponChainDef*>("CommanderChains");
	
	// construct our list of weapon chains from our animation curves
	for (	ChainIt it = m_pSrcChains->m_container.begin();
			it != m_pSrcChains->m_container.end(); ++it )
	{
		ntAssert_p( (*it)->size() > 1, ("Invalid chain length") );

		int iCurves = ((*it)->size() - 1);
		int iVertsPerChain = (SEGMENTS_PER_CURVE * iCurves) + 1;
		m_chains.push_back( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) WeaponChain( iVertsPerChain, pDef ) );
	}
	
	m_iLastRefresh = 0xffffffff;
}

CommanderChains::~CommanderChains()
{
	while (!m_chains.empty())
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_chains.back() );
		m_chains.pop_back();
	}
}

//--------------------------------------------------
//!
//!	CommanderChains::SynchChainPos
//! make sure chain is setup
//!
//--------------------------------------------------
void CommanderChains::SynchChainPos()
{
	if (m_iLastRefresh == CTimer::Get().GetSystemTicks())
		return;
	
	m_iLastRefresh = CTimer::Get().GetSystemTicks();

	m_cachedTransInv = m_pobTransform->GetWorldMatrixFast().GetAffineInverse();
	m_obBounds.Min() = CPoint( MAX_POS_FLOAT, MAX_POS_FLOAT, MAX_POS_FLOAT );
	m_obBounds.Max() = CPoint( -MAX_POS_FLOAT, -MAX_POS_FLOAT, -MAX_POS_FLOAT );

	WeaponChainList::iterator chain = m_chains.begin();
	for (	ChainIt it = m_pSrcChains->m_container.begin();
			it != m_pSrcChains->m_container.end(); ++it, ++chain )
	{
		PopulateChainEffect( *(*it), *chain );
	}
}

//--------------------------------------------------
//!
//!	CommanderChains::PopulateChainEffect
//!
//--------------------------------------------------
void CommanderChains::PopulateChainEffect( const ChainAnimation2Render::OneCurve& src, WeaponChain* pDest )
{
	CVector coeffs[4];

	static const float gafBasisMat[4][4] =
	{
		{ -0.5f,  1.5f, -1.5f,  0.5f },
		{  1.0f, -2.5f,  2.0f, -0.5f },
		{ -0.5f,  0.0f,  0.5f,  0.0f },
		{  0.0f,  1.0f,  0.0f,  0.0f }
	};

	int iNumCurves = src.size() - 1;

	for (int i = 0; i < iNumCurves; i++)
	{
		// construct our catmull segment points
		CPoint aCurvePoints[4];
		aCurvePoints[1] = src[i+0].GetWorldPosition();
		aCurvePoints[2] = src[i+1].GetWorldPosition();
		
		// predict previous point if required, else use
		if (i == 0)
		{
			aCurvePoints[0] = (3.0f*aCurvePoints[1]) - (3.0f*aCurvePoints[2]);
			if (iNumCurves > 1)
				aCurvePoints[0] += src[i+2].GetWorldPosition();
		}
		else
			aCurvePoints[0] = src[i-1].GetWorldPosition();

		// predict new point if required, else use
		if (i == (iNumCurves-1))
			aCurvePoints[3] = ((3.0f*aCurvePoints[2]) - (3.0f*aCurvePoints[1])) + aCurvePoints[0];
		else
			aCurvePoints[3] = src[i+2].GetWorldPosition();

		// construct catmull coeffs from these points
		for (int j = 0; j < 4; j++)
		{
			coeffs[j].Clear();
			coeffs[j] += CVector(aCurvePoints[0]) * gafBasisMat[j][0];
			coeffs[j] += CVector(aCurvePoints[1]) * gafBasisMat[j][1];
			coeffs[j] += CVector(aCurvePoints[2]) * gafBasisMat[j][2];
			coeffs[j] += CVector(aCurvePoints[3]) * gafBasisMat[j][3];
		}

		float fInterval = 1.0f / SEGMENTS_PER_CURVE;
		for (int k = 0; k < SEGMENTS_PER_CURVE; k++)
		{
			CPoint linkPos = EvaluateSegment( coeffs, fInterval * k );
			pDest->SetVertexPosition( linkPos, (i * SEGMENTS_PER_CURVE) + k );
			ConsiderForMinMax( linkPos );
		}
	}

	// fill in final one
	CPoint linkPos = EvaluateSegment( coeffs, 1.0f );
	pDest->SetVertexPosition( linkPos, (iNumCurves * SEGMENTS_PER_CURVE) );
	ConsiderForMinMax( linkPos );
}

//--------------------------------------------------
//!
//!	CommanderChains::RenderMaterial
//!
//--------------------------------------------------
void CommanderChains::RenderMaterial()
{
	SynchChainPos();
	for (	WeaponChainList::iterator it = m_chains.begin();
			it != m_chains.end(); ++it )
	{
		(*it)->RenderMaterial();
	}
}

//--------------------------------------------------
//!
//!	CommanderChains::RenderShadowMap
//!
//--------------------------------------------------
void CommanderChains::RenderShadowMap()
{
	SynchChainPos();
	for (	WeaponChainList::iterator it = m_chains.begin();
			it != m_chains.end(); ++it )
	{
		(*it)->RenderShadowMap();
	}
}

//--------------------------------------------------
//!
//!	CommanderChains::RenderShadowOnly
//!
//--------------------------------------------------
void CommanderChains::RenderShadowOnly()
{
	SynchChainPos();
	for (	WeaponChainList::iterator it = m_chains.begin();
			it != m_chains.end(); ++it )
	{
		(*it)->RenderRecieveShadowMap();
	}
}

//--------------------------------------------------
//!
//!	CommanderChains::RenderShadowOnly
//!
//--------------------------------------------------
void CommanderChains::RenderDepth()
{
	SynchChainPos();
	for (	WeaponChainList::iterator it = m_chains.begin();
			it != m_chains.end(); ++it )
	{
		(*it)->RenderDepth();
	}
}

