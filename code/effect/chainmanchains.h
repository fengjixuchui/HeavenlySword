//--------------------------------------------------
//!
//!	\file chainmanchains.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _CHAINMAN_CHAINS_H
#define _CHAINMAN_CHAINS_H

#include "gfx/renderable.h"
#include "lua/ninjalua.h"

class WeaponChain;

//--------------------------------------------------
//!
//!	ChainmanChains
//!	Renderable that represents the chainmans's chains
//!
//--------------------------------------------------
class ChainmanChains : public CRenderable, IFrameFlagsUpdateCallback
{
public:
	ChainmanChains( const Transform* pParentTransform );
	~ChainmanChains();

	HAS_LUA_INTERFACE()

	virtual void RenderDepth();
	virtual void RenderMaterial();
	virtual void RenderShadowMap();
	virtual void RenderShadowOnly();

    // IFrameFlagsUpdateCallback
    virtual void PreUpdate();
    virtual void PostUpdate();

    virtual IFrameFlagsUpdateCallback* GetUpdateFrameFlagsCallback() const;

private:
	static const int WEAPON_CHAIN_SEGMENTS = 3;
	static const int HAND_CHAIN_SEGMENTS = 2;
	static const int LINKS_PER_SEGMENT = 15;

	void SynchChainPos();

	Transform*		m_pLWeapon;
	Transform*		m_pLChain1;

	Transform*		m_pRChain1;
	Transform*		m_pRChain2;
	Transform*		m_pRChain3;
	Transform*		m_pRChain4;

	WeaponChain*	m_pWeaponChain;			//!< allocated in MC_EFFECTS
	WeaponChain*	m_pHandChain;				//!< allocated in MC_EFFECTS
	u_long			m_iLastRefresh;

	CPoint			m_weaponCurveNodes[ WEAPON_CHAIN_SEGMENTS + 3 ];
	CPoint			m_handCurveNodes[ HAND_CHAIN_SEGMENTS + 3 ];
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

	void DebugRenderSegmentList( int iNumSegs, CPoint* pPoints );
	void PopulateChainEffect( int iNumSegs, CPoint* pPoints, WeaponChain* pChain );
	
	void ConsiderForMinMax( const CPoint& pos )
	{
		CPoint bounds = pos * m_cachedTransInv;

		m_obBounds.Min() = m_obBounds.Min().Min( bounds );
		m_obBounds.Max() = m_obBounds.Max().Max( bounds );
	}
};

LV_DECLARE_USERDATA(ChainmanChains);

#endif // _CHAINMAN_CHAINS_H
