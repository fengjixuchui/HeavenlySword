//--------------------------------------------------
//!
//!	\file game/entityinteractable.h
//!	Definition of the Interactable entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_H
#define	_ENTITY_INTERACTABLE_H

#include "game/entity.h"
#include "game/entity.inl"
#include "game/interactableparams.h"

//--------------------------------------------------
//!
//! Class Interactable.
//! Base interactable entity type
//!
//--------------------------------------------------
class Interactable : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable)

public:
	Interactable();
	~Interactable();

	void OnPostConstruct();
	void OnPostPostConstruct();

    enum EntTypeInteractable
	{
		EntTypeInteractable_Undefined				=0,
		EntTypeInteractable_Ladder					=(1<<0),
		EntTypeInteractable_Thrown					=(1<<1),
		EntTypeInteractable_Simple_Usable			=(1<<2),
		EntTypeInteractable_Boulder					=(1<<3),
		EntTypeInteractable_HoistStatue				=(1<<4),
		EntTypeInteractable_Pushable				=(1<<5),

		// defines inheritance relationship
		EntTypeInteractable_Switch_Trigger  		=(1<<6), 
		EntTypeInteractable_SimpleSwitch			=(1<<7)|EntTypeInteractable_Switch_Trigger,
		EntTypeInteractable_ButtonMash				=(1<<8)|EntTypeInteractable_Switch_Trigger,
		EntTypeInteractable_TwoWaySwitch			=(1<<9)|EntTypeInteractable_Switch_Trigger,
		
		EntTypeInteractable_Spear					=(1<<10),
		EntTypeInteractable_Traverser				=(1<<11),
		EntTypeInteractable_Turret_Point			=(1<<12),
		EntTypeInteractable_TurretWeapon			=(1<<13),
		EntTypeInteractable_Object_Ranged_Weapon	=(1<<14),
		EntTypeInteractable_Object_Pickup			=(1<<15),
		EntTypeInteractable_Moving_Platform			=(1<<16),
		EntTypeInteractable_Object_Catapult			=(1<<17),
		EntTypeInteractable_Object_Catapult_Breaky  =(1<<18),
		EntTypeInteractable_Spawner                 =(1<<19)
	};

	EntTypeInteractable GetInteractableType() const { return m_eInteractableType; }
	virtual const CUsePointAttr* GetUsePointAttributes() const { return 0;}

	virtual bool InteractableInUse( void ) const { return false; }

	bool HeroCanUse()		{ return m_bHeroCanUse; }
	bool ArcherCanUse()		{ return m_bArcherCanUse; }
	bool EnemyAICanUse()	{ return m_bEnemyAICanUse; }
	bool AllyAICanUse()		{ return m_bAllyAICanUse; }

protected:
	
	bool 	m_bHeroCanUse;					// flags whether a hero character can use.
	bool 	m_bArcherCanUse;				// flags whether an archer character can use.
	bool 	m_bEnemyAICanUse;				// flags whether an enemy character can use.
	bool 	m_bAllyAICanUse;				// flags whether an ally character can use.

	EntTypeInteractable m_eInteractableType;
	CHashedString m_obSceneElementDef;
};


#endif // _ENTITY_INTERACTABLE_H

