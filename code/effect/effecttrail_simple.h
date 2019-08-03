//--------------------------------------------------
//!
//!	\file effecttrail_simple.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _EFFECT_TRAIL_H
#define _EFFECT_TRAIL_H

#include "sortable_effect.h"
#include "effect/effect_resourceman.h"
#include "effect_util.h"
#include "effecttrail_utils.h"
#include "effecttrail_edge.h"
#include "effect_resetable.h"
#include "gfx/texture.h"

class ColourFunction;
class EffectTrail_Simple;
class EffectTrail_SimpleDef;
class EffectTrailBuffer;
class TextureAtlas;

//--------------------------------------------------
//!
//!	EffectTrail_SimpleDefResources
//!
//--------------------------------------------------
class EffectTrail_SimpleDefResources : public EffectResource
{
public:
	EffectTrail_SimpleDefResources();
	virtual ~EffectTrail_SimpleDefResources();
	
	virtual void GenerateResources();
	virtual bool ResourcesOutOfDate() const;

	ColourFunction*		m_pFadePalette;
	ColourFunction*		m_pCrossPalette;

	const Texture::Ptr&	GetFadePalette() const	{ return m_fadePalette; }
	const Texture::Ptr&	GetCrossPalette() const	{ return m_crossPalette; }

private:
	Texture::Ptr	m_fadePalette;	// auto gen'd palette
	Texture::Ptr	m_crossPalette; // auto gen'd palette
};

//--------------------------------------------------
//!
//!	EffectTrail_SimpleDef
//!	Interface defining simple trail behaviour
//!
//--------------------------------------------------
class EffectTrail_SimpleDef
{
public:
	EffectTrail_SimpleDef();
	virtual ~EffectTrail_SimpleDef() {};

	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );

	ntstd::String			m_texName;
	float					m_fFadeTime;
	float					m_fLifeTime;
	float					m_fEdgesPerSecond;
	float					m_fTexRepeatPeriod;
	float					m_fCullRadius;
	RenderStateDef			m_rsDef;
	EffectTrail_SimpleDefResources	m_resources;
	EffectTrail_EdgeDef*	m_pEdgeDefinition;
	TRAIL_TEXTURE_MODE		m_eTexMode;
	Texture::Ptr			m_pTex;
	const TextureAtlas*		m_pAtlas;

	// for housekeeping
	ResetSet<Resetable> m_resetSet;

	u_int GetMaxEdges() const { return m_iMaxEdges; }

private:
	void ResolveTrailTextureMode();
	void LoadTexture();

	u_int	m_iMaxEdges;
};

//--------------------------------------------------
//!
//!	EffectTrail_Simple
//!	Naive implementation of trails, uses simple 
//! Intra-frame interpolation strategy to smooth
//!
//--------------------------------------------------
class EffectTrail_Simple : public SortableEffect, public Resetable
{
public:
	EffectTrail_Simple( const EffectTrail_SimpleDef* pDef,
						const Transform* pTransform );

	EffectTrail_Simple( const EffectTrail_SimpleDef* pDef,
						const EffectTrail_EdgeDef* pEdge,
						const Transform* pTransform );

	virtual ~EffectTrail_Simple();

	virtual void Reset( bool bInDestructor );
	virtual bool WaitingForResources() const;
	virtual bool UpdateEffect();
	virtual void RenderEffect();

private:
	void RetriveEmitterFrame();

	const EffectTrail_SimpleDef*	m_pDef;
	const EffectTrail_EdgeDef*		m_pEdge;
	const Transform*				m_pTransform;

	CPoint		m_averageEdgePos;
	CPoint		m_currAverageEdgePos;
	CMatrix		m_currFrame;
	CPoint		m_framePosAncient;
	
	float	m_fLastAge;
	float	m_fCurrAge;
	float	m_fLastEmitTime;
	float	m_fAccumulator;

	EffectTrailBuffer*		m_pTrail;
};

#endif //_EFFECT_TRAIL_H
