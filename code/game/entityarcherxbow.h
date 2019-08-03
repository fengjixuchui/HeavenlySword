//--------------------------------------------------
//!
//!	\file game/entityarcherxbow.h
//!	Definition of the archers crossbow object
//!
//--------------------------------------------------

#ifndef	_ENTITY_ARCHERXBOW_H
#define	_ENTITY_ARCHERXBOW_H

#include "game/entityinteractable.h"

class Object_Projectile;
class Projectile_Attributes;
class Archer;

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Object_ArcherXBow.
//! The archers cross bow
//!
//--------------------------------------------------
class Object_ArcherXBow : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_ArcherXBow)

public:
	// Constructor
	Object_ArcherXBow();

	// Destructor
	~Object_ArcherXBow();

	// Post Construct
	void OnPostConstruct();
	void OnPostPostConstruct();

	
	Archer* GetParent() {return (Archer*)m_pobParentEntity;}

	CEntity* GetBolt() {return m_pBolt;}
	void     SetBolt(CEntity* pBolt) {m_pBolt = pBolt;}

private:
	CHashedString m_sAnimationContainer;

	CEntity* m_pBolt;
};


#endif // _ENTITY_ARCHERXBOW_H
