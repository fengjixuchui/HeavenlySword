//--------------------------------------------------
//!
//!	\file sortable_effect.h
//!	Effect that lives in world space and can be 
//! sorted before rendering
//!
//--------------------------------------------------

#ifndef _SORTABLE_EFFECT_H
#define _SORTABLE_EFFECT_H

#include "effect/effect.h"
#include "effect/renderstate_block.h"

//--------------------------------------------------
//!
//!	SortableEffect
//! Effect base class that all other world space
//! sortable effects derive from
//!
//--------------------------------------------------
class SortableEffect : public Effect
{
public:
	SortableEffect() : m_cullingOrigin(0.0f, 0.0f, 0.0f),
		m_fCullingRadius(1.0f),
		m_fSortingPush(0.0f),
		m_fSortingDistance(0.0f)
	{}

	CPoint GetCullingOrigin() const { return m_cullingOrigin; }
	float GetCullingRadius() const { return m_fCullingRadius; }
	float GetSortingDistance() const { return m_fSortingDistance; }
	
	// this allows us to offset the sorting order of effects
	inline void CalcSortingDistanceTo( const CPoint& pos )
	{
		m_fSortingDistance = (pos - m_cullingOrigin).Length() + m_fSortingPush;
	}

	//! debug display of our culling parameters
	void DebugRenderCullVolume() const;

	//! get our renderstate block
	const RenderStateBlock& GetRenderstates() const { return m_renderstates; }

	//! these are easy if we have a render state block
	virtual bool HighDynamicRange()	const
	{
		if	(
			( m_renderstates.m_renderType == ERT_HIGH_DYNAMIC_RANGE ) ||
			( m_renderstates.m_renderType == ERT_HDR_DEPTH_HAZED )
			)
			return true;
		return false;
	}	

	virtual bool Opaque() const { return m_renderstates.m_bZWriteEnable; }
	virtual bool Invisible() const { return (m_renderstates.m_blendMode == EBM_DISABLED); }

protected:
	// NB make sure any effect you want rendering sets these...
	CPoint				m_cullingOrigin;
	float				m_fCullingRadius;
	float				m_fSortingPush;
	float				m_fSortingDistance;
	RenderStateBlock	m_renderstates;
};

#endif // _SORTABLE_EFFECT_H
