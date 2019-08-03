//------------------------------------------------------------------------------------------
//!
//!	ghostgirl.h
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_GHOSTGIRL_H
#define GAME_GHOSTGIRL_H

#ifndef _EFFECT_H
#include "effect/effect.h"
#endif

class CEntity;
class NSEnt;

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlController
//! Control logic structure owned by parent NSEnt
//!
//------------------------------------------------------------------------------------------
class GhostGirlController
{
public:
	enum GG_EFFECT_TYPE
	{
		GG_WHITE,
		GG_BLUE,
		GG_RED,
	};

	GhostGirlController( GG_EFFECT_TYPE type, CEntity* pParentEnt, NSEnt* pNSEnt );
	~GhostGirlController();

	bool Active() const { return m_active; }
	void Activate();
	void Deactivate();

private:
	GG_EFFECT_TYPE	m_type;
	CEntity*		m_pParentEnt;
	NSEnt*			m_pNSEnt;
	bool			m_active;
	uint32_t		m_effectID;
};

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlWhite
//! Simplest of ghost girl effects, attaches multiple P-systems to transforms of an 
//! invisible heroine. Used to indicate ninja sequence directional movement.
//!
//------------------------------------------------------------------------------------------
class GhostGirlWhite : public Effect
{
public:
	GhostGirlWhite( CEntity* pParentEnt, NSEnt* pNSEnt );
	virtual ~GhostGirlWhite() {};

	virtual bool UpdateEffect();
	virtual bool WaitingForResources() const { return false; }

	// is not really an effect, but an object that controls sub
	// effects, hence the null render methods.
	virtual void RenderEffect() {};
	virtual bool HighDynamicRange() const { return true; }

private:
	CEntity*				m_pParentEnt;
	NSEnt*					m_pNSEnt;
	ntstd::List<uint32_t>	m_subEffects;
};

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlBlue
//! As above, but with a more complicated pulsating colour overide for the P-systems.
//! Used to indicate a button mashing ninja sequence action.
//!
//------------------------------------------------------------------------------------------
class GhostGirlBlue : public Effect
{
public:
	GhostGirlBlue( CEntity* pParentEnt, NSEnt* pNSEnt );
	virtual ~GhostGirlBlue() {};

	virtual bool UpdateEffect();
	virtual bool WaitingForResources() const { return false; }

	// is not really an effect, but an object that controls sub
	// effects, hence the null render methods.
	virtual void RenderEffect() {};
	virtual bool HighDynamicRange() const { return true; }

private:
	CEntity*				m_pParentEnt;
	NSEnt*					m_pNSEnt;
	ntstd::List<uint32_t>	m_subEffects;
};

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlRed
//! Local space core P-systems are attached to a visible heroine entity, to indicate
//! she should attack. Additionally an introductory flare up is used.
//!
//-------------------------------------------------------------------------s-----------------
class GhostGirlRed : public Effect
{
public:
	GhostGirlRed( CEntity* pParentEnt, NSEnt* pNSEnt );
	virtual ~GhostGirlRed() {};

	virtual bool UpdateEffect();
	virtual bool WaitingForResources() const { return false; }

	// is not really an effect, but an object that controls sub
	// effects, hence the null render methods.
	virtual void RenderEffect() {};
	virtual bool HighDynamicRange() const { return true; }

private:
	CEntity*				m_pParentEnt;
	NSEnt*					m_pNSEnt;
	ntstd::List<uint32_t>	m_subEffects;
};

#endif // GAME_GHOSTGIRL_H
