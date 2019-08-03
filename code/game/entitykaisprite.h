//--------------------------------------------------
//!
//!	\file game/entitykaisprite.h
//!	Definition of the kai sprite entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_KAISPRITE_H
#define	_ENTITY_KAISPRITE_H

#include "game/entity.h"
#include "game/entity.inl"
#include "anim/animator.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class KaiSprite.
//! Kai's sprite entity object.
//!
//--------------------------------------------------
class KaiSprite : public CEntity
{
	//Declare dataobject interface
	HAS_INTERFACE(KaiSprite)

public:
	//Constructor
	KaiSprite();

	// Destructor
	~KaiSprite();

	//Post Construct
	void OnPostConstruct();

	bool GlobalHandlers(Message& msg);

	void AnimPlay (CHashedString pcAnim, float fSpeed = 1.0f, bool bLocomoting = false, bool bLooping = false);


	// Old attribute table parameters. public for FSM access
	CHashedString	m_obAnimToPlay;
	float			m_fAnimPlaySpeed;
	bool			m_bForceLoop;

	CAnimationPtr	m_pobAnim;

	//Whether we need to notify a script upon finishing all anims in the queue.
	bool m_bNotifyAllAnimsDone;
	//The name of the script to notify.
	CEntity* m_pobNotifyScriptEntity;

	//Our list of anims
	ntstd::List<CHashedString> m_obAnimQueue;

protected:
	//Object description
	CKeyString	m_Description;

	//Initial State
	CHashedString	m_InitialState;

	//The animation container
	CHashedString	m_AnimationContainer;
};


#endif // _ENTITY_KAISPRITE_H
