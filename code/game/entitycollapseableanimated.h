//--------------------------------------------------
//!
//!	\file game/entitycollapseableanimated.h
//!	Definition of the animated collapse-able entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_COLLAPSABLEANIMATED_H
#define	_ENTITY_INTERACTABLE_COLLAPSABLEANIMATED_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Collapsable_Animated.
//! Collapse-able object that animates in stages when breaking.
//!
//--------------------------------------------------
#define MAX_ACTIONS 5 

class Att_Collapsable
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Collapsable)

public:
	bool	m_bVulnerable_SpeedAtk;
	bool	m_bVulnerable_RangeAtk;
	bool	m_bVulnerable_PowerAtk;
	bool	m_bVulnerable_Lvl1_PowerAtk;
	bool	m_bVulnerable_Lvl2_PowerAtk;
	bool	m_bVulnerable_Lvl3_PowerAtk;
	bool	m_bVulnerable_Bolt;
	bool	m_bVulnerable_Rocket;
	bool	m_bVulnerable_ObjectStrike;

	Att_Collapsable()
	:	m_bVulnerable_SpeedAtk( false )
	, 	m_bVulnerable_RangeAtk( false )
	, 	m_bVulnerable_PowerAtk( false )
	, 	m_bVulnerable_Lvl1_PowerAtk( false )
	, 	m_bVulnerable_Lvl2_PowerAtk( false )
	, 	m_bVulnerable_Lvl3_PowerAtk( false )
	, 	m_bVulnerable_Bolt( false )
	, 	m_bVulnerable_Rocket( false )
	, 	m_bVulnerable_ObjectStrike( false )
	{}
};

//For collapsable-animated objects that want to have an initial "static" stage, this entity provides the
//simple state-machine to detect hits and re-activate the destructable part.
class Collapsable_StaticPart : public CEntity
{
	HAS_INTERFACE(Collapsable_StaticPart);
public:
	Collapsable_StaticPart();
	~Collapsable_StaticPart();

	void OnPostConstruct();

	void SetParent(CEntity* pParent) { m_pParentCollapsable = pParent; }

	CEntity* m_pParentCollapsable;
};


class Collapsable_Animated : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Collapsable_Animated)

public:
	// Constructor
	Collapsable_Animated();

	// Destructor
	~Collapsable_Animated();

	// Post Construct
	void OnPostConstruct();

	//void OnHit();
	//void PlayAnim(CHashedString anim);
	void Destroy();
	void Action(int iAction);
	bool CanCollapse( void );

	int		m_iHits;
	int		m_iMaxHits;
	int		m_iAction;
	bool	m_bAnimMessagesOnly;
	bool	m_bHasCollapseAnim;
	bool	m_bCollapsable;
	bool	m_bHasStaticPart;

	CHashedString	m_AnimCollapse;
	CHashedString	m_AnimAction[MAX_ACTIONS];
	CHashedString	m_SfxCollapse;
	CHashedString	m_PfxCollapse;
	CHashedString	m_obStaticClumpName;

	bool	m_bVulnerable_SpeedAtk;
	bool	m_bVulnerable_RangeAtk;
	bool	m_bVulnerable_PowerAtk;
	bool	m_bVulnerable_Lvl1_PowerAtk;
	bool	m_bVulnerable_Lvl2_PowerAtk;
	bool	m_bVulnerable_Lvl3_PowerAtk;
	bool	m_bVulnerable_Bolt;
	bool	m_bVulnerable_Rocket;
	bool	m_bVulnerable_ObjectStrike;

	Att_Collapsable* m_pobPresetVulnerability;

	Collapsable_StaticPart* m_pobStaticPart;

	// Some direction for the smash
	CDirection m_obSmashDirection;
protected:
	// Object description
	CHashedString	m_Description;

	// The animation container
	CHashedString	m_AnimationContainer;
};


#endif // _ENTITY_INTERACTABLE_COLLAPSABLEANIMATED_H
