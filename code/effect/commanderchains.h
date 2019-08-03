//--------------------------------------------------
//!
//!	\file commanderchains.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _COMMANDER_CHAINS_H
#define _COMMANDER_CHAINS_H

#include "gfx/renderable.h"
#include "hair/chain2render.h"

class WeaponChain;

//--------------------------------------------------
//!
//!	CommanderChains
//!	Renderable that represents the chainmans's chains
//!
//--------------------------------------------------
class CommanderChains : public CRenderable
{
public:
	CommanderChains( Transform* pParentTransform, const ChainAnimation2Render* pChains );
	~CommanderChains();

	virtual void RenderDepth();
	virtual void RenderMaterial();
	virtual void RenderShadowMap();
	virtual void RenderShadowOnly();

private:
	static const int SEGMENTS_PER_CURVE = 15;

	void SynchChainPos();

	const ChainAnimation2Render* m_pSrcChains;
	typedef ntstd::List<WeaponChain*, Mem::MC_EFFECTS> WeaponChainList;
	WeaponChainList m_chains;
	u_long			m_iLastRefresh;
	CMatrix			m_cachedTransInv;

	CPoint EvaluateSegment( CVector* pCoeffs, float u )
	{
		CVector result = pCoeffs[0];
		for (int i = 1; i < 4; i++)
		{
			result *= u;
			result += pCoeffs[i];
		}
		result.W() = 0.0f;
		return CPoint(result);
	}

	void PopulateChainEffect( const ChainAnimation2Render::OneCurve& src, WeaponChain* pDest );
	
	void ConsiderForMinMax( const CPoint& pos )
	{
		CPoint bounds = pos * m_cachedTransInv;

		m_obBounds.Min() = m_obBounds.Min().Min( bounds );
		m_obBounds.Max() = m_obBounds.Max().Max( bounds );
	}

	typedef ChainAnimation2Render::Container::const_iterator ChainIt;
};


#endif // _COMMANDER_CHAINS_H
