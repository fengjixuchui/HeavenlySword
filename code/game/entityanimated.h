//--------------------------------------------------
//!
//!	\file game/entityanimated.h
//!	Definition of the Animated entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_ANIMATED_H
#define	_ENTITY_ANIMATED_H

#include "game/entity.h"
#include "game/entity.inl"
#include "anim/animator.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Object_Animated.
//! Base interactable rigid body entity type
//!
//--------------------------------------------------
class Object_Animated : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Animated)

public:
	// Constructor
	Object_Animated();

	// Destructor
	~Object_Animated();

	// From CEntity
	virtual void OnLevelStart();

	// Post Construct
	void OnPostConstruct();

	bool GlobalHandlers(Message& msg);

	void AnimPlay (CHashedString pcAnim, float fSpeed=1.0f, bool bLocomoting=false, bool bLooping=false, float fPercentage = 0.0f);


	// Old attribute table parameters. public for FSM access
	bool			m_bPlayAnimOnStart;
	CHashedString	m_obAnimToPlay;
	float			m_fAnimPlaySpeed;
	bool			m_bForceLoop;

	CAnimationPtr	m_pobAnim;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CHashedString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;

	//friend STATEMACHINE(OBJECT_ANIMATED_FSM, Object_Animated);
};


#endif // _ENTITY_ANIMATED_H
