//--------------------------------------------------
//!
//!	\file effecttrail_line.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _EFFECTTRAIL_LINE_H
#define _EFFECTTRAIL_LINE_H

#include "sortable_effect.h"
#include "effect_util.h"
#include "effecttrail_utils.h"
#include "effect_resetable.h"
#include "effect/effect_resourceman.h"
#include "gfx/texture.h"

class EffectTrailLineBuffer;
class TextureAtlas;

//--------------------------------------------------
//!
//!	EffectTrail_LineDefResources
//!
//--------------------------------------------------
class EffectTrail_LineDefResources : public EffectResource
{
public:
	EffectTrail_LineDefResources();
	virtual ~EffectTrail_LineDefResources();
	
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
//!	EffectTrail_LineDef
//!	Interface defining simple trail behaviour
//!
//--------------------------------------------------
class EffectTrail_LineDef
{
public:
	EffectTrail_LineDef();
	virtual ~EffectTrail_LineDef() {};

	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter param, CallBackParameter );

	ntstd::String	m_texName;
	CPoint			m_offset;
	float			m_fWidth;	
	float			m_fFadeTime;
	float			m_fLifeTime;
	float			m_fEmitPerSecond;
	float			m_fTexRepeatPeriod;
	float			m_fCullRadius;
	RenderStateDef	m_rsDef;
	EffectTrail_LineDefResources	m_resources;
	TRAIL_TEXTURE_MODE		m_eTexMode;
	Texture::Ptr			m_pTex;
	const TextureAtlas*		m_pAtlas;

	// for housekeeping
	ResetSet<Resetable> m_resetSet;

	u_int GetMaxPoints() const { return m_iMaxPoints; }

private:
	void ResolveTrailTextureMode();
	void LoadTexture();

	u_int	m_iMaxPoints;
};

//--------------------------------------------------
//!
//!	EffectTrail_Line
//!	Line based rather than edge based trail effect
//!
//--------------------------------------------------
class EffectTrail_Line : public SortableEffect, public Resetable
{
public:
	EffectTrail_Line(	const EffectTrail_LineDef* pDef,
						const Transform* pTransform,
						const CPoint* pOffsetOveride = 0 );

	virtual ~EffectTrail_Line();

	virtual void Reset( bool bInDestructor );
	virtual bool UpdateEffect();
	virtual void RenderEffect();
	virtual bool WaitingForResources() const;

private:
	const EffectTrail_LineDef*	m_pDef;
	const Transform*			m_pTransform;

	CPoint		m_offsetOveride;
	bool		m_bUseOffsetOveride;
	CPoint		m_currEmitPos;
	CPoint		m_lastEmitPos;
	
	float	m_fLastAge;
	float	m_fCurrAge;
	float	m_fLastEmitTime;
	float	m_fAccumulator;

	Texture::Ptr			m_pTex;
	const TextureAtlas*		m_pAtlas;
	EffectTrailLineBuffer*	m_pLine;
};

#endif //_EFFECTTRAIL_LINE_H
