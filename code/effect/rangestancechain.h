//--------------------------------------------------
//!
//!	\file rangestancechain.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _RANGE_STANCE_CHAIN_H
#define _RANGE_STANCE_CHAIN_H

#include "gfx/renderable.h"

class WeaponChain;

//--------------------------------------------------
//!
//!	RangeStanceChain
//!	Renderable that represents the hero's chains
//!
//--------------------------------------------------
class RangeStanceChain : public CRenderable, IFrameFlagsUpdateCallback
{
public:
	RangeStanceChain( const Transform* pParentTransform, const Transform* pLinkTransform );
	~RangeStanceChain();

	//! render depths for z pre-pass
	virtual void RenderDepth();

	//! Renders the game material for this renderable.
	virtual void RenderMaterial();

	//! Renders the shadow map depths.
	virtual void RenderShadowMap();

	//! Renders with a shadow map compare only. 
	virtual void RenderShadowOnly();

    // IFrameFlagsUpdateCallback
    virtual void RangeStanceChain::PreUpdate();
    virtual void RangeStanceChain::PostUpdate();

    virtual IFrameFlagsUpdateCallback* GetUpdateFrameFlagsCallback() const;

private:
	void SynchChainPos();

	const Transform*	m_pLinkTransform;
	WeaponChain*		m_pChainEffect;
	u_long				m_iLastRefresh;
};


#endif // _RANGE_STANCE_CHAIN_H
