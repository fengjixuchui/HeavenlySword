//--------------------------------------------------
//!
//!	\file game/entitycollapseableanimated.h
//!	Definition of the animated collapse-able entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_COLLAPSABLE_HIERARCHY_H
#define	_ENTITY_COLLAPSABLE_HIERARCHY_H

#include "game/entity.h"
#include "game/entity.inl"
#include "fsm.h"


//--------------------------------------------------
//!
//! Class CollapsableHierarchySubPartDef.
//! Definition for a sub part
//!
//--------------------------------------------------
class CollapsableHierarchySubPartDef
{
public:

	CollapsableHierarchySubPartDef()
	:	m_Description( "" ),
		m_Clump( "" ),
		m_PositionOffset( CPoint(0.0f, 0.0f, 0.0f) )
	{}

	// Object description
	CKeyString	m_Description;

	// Clump the sub part uses
	ntstd::String	m_Clump;

	// Position Offset from the parent clump
	CPoint			m_PositionOffset;

	// Index for linking purposes
	int				m_iPartIndex;

	// Indices of its children
	ntstd::List<int>	m_aiChildIndices;
};


//--------------------------------------------------
//!
//! Class Att_CollapsableHierarchy.
//! Shared Attributes for the hierarchy object
//!
//--------------------------------------------------
class Att_CollapsableHierarchy
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_CollapsableHierarchy)

public:

	// Particle effects
	ntstd::List<CHashedString>	m_PfxListPartHit;
	ntstd::List<CHashedString>	m_PfxListPartBreakOff;
	ntstd::List<CHashedString>	m_PfxListPartDestroy;

	// Sound effects
	CKeyString		m_SfxPartHit;
	CKeyString		m_SfxPartBreakOff;
	CKeyString		m_SfxPartDestroy;

	// Recursive breaking parameters
	int		m_iNumLevelsToInheritHitVelocity;
	float	m_fPercentageVelocityInheritedFromParent;

	// Velocities when hit
	float	m_fMaxBreakVelocity;
	float	m_fPercentageVelocityInheritedFromProjectile;

	// What attacks is this object vulnerable to?
	bool	m_bVulnerable_Bolt;

	// The definitions of all the subparts that make up the object
	ntstd::List<CollapsableHierarchySubPartDef*, Mem::MC_ENTITY> m_SubPartDefinitions;

	Att_CollapsableHierarchy()
	:	m_SfxPartHit( "" ),
		m_SfxPartBreakOff( "" ),
		m_SfxPartDestroy( "" ),
		m_iNumLevelsToInheritHitVelocity( 2 ),
		m_fPercentageVelocityInheritedFromParent( 20.0f ),
		m_fMaxBreakVelocity( 0.5f ),
		m_fPercentageVelocityInheritedFromProjectile( 5.0f ),
		m_bVulnerable_Bolt( true )
	{}
};



// Forward declare the parent/base
class Object_CollapsableHierarchy;
//--------------------------------------------------
//!
//! Class Object_CollapsableHierarchySubPart.
//! Sub part of the collapsable hierarchy object
//! that animates and goes under physics when the
//! parent tells it to.
//!
//--------------------------------------------------
class Object_CollapsableHierarchySubPart : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_CollapsableHierarchySubPart)

public:

	// Mimics the FSM state
	enum COLLAPSE_STATE
	{
		COLLAPSE_STATE_DEFAULT,
		COLLAPSE_STATE_BROKEN_OFF,
		COLLAPSE_STATE_DESTROYED
	};

	// Constructor
	Object_CollapsableHierarchySubPart();

	// Destructor
	~Object_CollapsableHierarchySubPart();

	// Post Construct
	void OnPostConstruct();

	// Only to be called by the parent sub part
	void BreakOffPart( const CDirection& obInitialVelocity, int iRecursionLevels );

	// Removes pointer from the child part list
	void RemoveChildSubPartPtr( Object_CollapsableHierarchySubPart* pChildPart );

	// Object description
	CKeyString	m_Description;

	// Collapse state, this should be the same as the FSM state.  OnEnter events of the FSM, set it.
	COLLAPSE_STATE	m_eCollapseState;

	// Parent entity (not sub part, but the main Mr Ed placed bit)
	Object_CollapsableHierarchy*	m_pMainParent;

	// Parent sub part entities for this part
	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>	m_apParentSubParts;

	// Child sub part entities for this part
	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>	m_apChildSubParts;

	// Initial Velocity that this part was broken off with
	CDirection m_obInitialVelocity;

	// Shared attributes for the object
	Att_CollapsableHierarchy*	m_pSharedAttributes;

	// Definition for this sub part
	CollapsableHierarchySubPartDef*	m_pDef;

private:

};

//--------------------------------------------------
//!
//! Class Object_CollapsableHierarchy.
//! Main parent part to a collapsable hierarchy.
//! Where the sub parts break in a specified order
//! to leave just this bas (parent) part
//!
//--------------------------------------------------
class Object_CollapsableHierarchy : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_CollapsableHierarchy)

public:
	// Constructor
	Object_CollapsableHierarchy();

	// Destructor
	~Object_CollapsableHierarchy();

	// Post Construct
	void OnPostConstruct();

	// Called to remove a sub part from the list
	void SubPartBrokenOff( Object_CollapsableHierarchySubPart* pBrokenOffSubPart );

	// Enable collisions for this part
	void SubPartEnableCollisions( Object_CollapsableHierarchySubPart* pSubPart );

protected:

	// Constructs all the sub parts for the object
	void ConstructSubParts( void );

	// Disable all collisions between all parts (called from PostConstruct for setup only)
	void DisableAllInternalCollisions( void );

	// Object description
	CKeyString	m_Description;

	// Shared attributes for the object
	Att_CollapsableHierarchy*	m_pSharedAttributes;

	// The sub parts, once broken off they will be removed from the list
	ntstd::List< Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY >	m_obSubParts;
};


#endif // _ENTITY_COLLAPSABLE_HIERARCHY_H

