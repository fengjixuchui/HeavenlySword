//--------------------------------------------------
//!
//!	\file game/entityinteractablepushable.h
//!	Definition of the Interactable Pushable entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_PUSHABLE_H
#define	_ENTITY_INTERACTABLE_PUSHABLE_H

#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Att_Pushable.
//! Pushable definition params
//!
//--------------------------------------------------

class Att_Pushable
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Pushable)

public:
	bool m_bAIAvoid;
	float m_fAIAvoidRadius;
	float m_fMass;
	CPoint m_obCenterOfMass;
	float m_fRestitution;
	float m_fFriction;
	float m_fLinearDamping;
	float m_fAngularDamping;
	float m_fMaxLinearVelocity;
	float m_fMaxAngularVelocity;
	CHashedString m_obPhysicsSoundDef;
	float m_fStrikeThreshold;
	CHashedString m_obStrikeAttackData;
	float m_fCollapseThreshold;	
	CDirection m_obPushVelocity;
	float m_fPushDelay;	
	CDirection m_obFastPushVelocity;
	float m_fFastPushDelay;	
	CHashedString m_obAnimPlayerMoveTo;
	CHashedString m_obAnimPlayerRunTo;
	CHashedString m_obAnimPlayerPush;
	CHashedString m_obAnimPlayerFastPush;
	CHashedString m_obPfxImpact;
	CHashedString m_obPfxCollapse;
	CHashedString m_obPfxDestroyed;
	CHashedString m_obSfxCollapse;
	CHashedString m_obSfxDestroyed;
	bool m_bFixedAfterConstruction;

	Att_Pushable()
	:	  m_bAIAvoid (false)
	,     m_fAIAvoidRadius (1.0f)
	,     m_fMass (1.0f)
	,     m_obCenterOfMass( CONSTRUCT_CLEAR )
	,     m_fRestitution( 0.4f )
	,     m_fFriction(0.5f)
	,     m_fLinearDamping(0.0f)
	,     m_fAngularDamping(0.05f)
	,     m_fMaxLinearVelocity(200.0f)
	,     m_fMaxAngularVelocity(100.0f)
	,     m_obPhysicsSoundDef("")
	,     m_fStrikeThreshold(0.0f)
	,     m_obStrikeAttackData("")
	,     m_fCollapseThreshold(0.0f)
	,     m_obPushVelocity( CONSTRUCT_CLEAR )
	,     m_fPushDelay (0.0f)
	,     m_obFastPushVelocity( CONSTRUCT_CLEAR )
	,     m_fFastPushDelay (0.0f)
	,     m_obAnimPlayerMoveTo("")
	,     m_obAnimPlayerRunTo("")
	,     m_obAnimPlayerPush("")
	,     m_obAnimPlayerFastPush("")
	,     m_obPfxImpact("")
	,     m_obPfxCollapse("")
	,     m_obPfxDestroyed("")
	,     m_obSfxCollapse("")
	,     m_obSfxDestroyed("")
	,	  m_bFixedAfterConstruction(false)
	{}
};

//--------------------------------------------------
//!
//! Class Interactable_Pushable.
//! Base interactable pushable entity type
//!
//--------------------------------------------------
class Interactable_Pushable : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Pushable)

public:
	// Constructor
	Interactable_Pushable();

	// Destructor
	~Interactable_Pushable();

	// Post Construct
	void OnPostConstruct();

	Att_Pushable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	// Public Variables (once part of attribute table in the script)
	Character*	m_poOtherEntity;	// Other entity interacting with this object
	int			m_nHits;			// Number of hits taken

protected:
	// Object description
	CHashedString	m_Description;

	// The animation container
	ntstd::String	m_AnimationContainer;

	Att_Pushable* m_pSharedAttributes;	
};


#endif // _ENTITY_INTERACTABLE_PUSHABLE_H
