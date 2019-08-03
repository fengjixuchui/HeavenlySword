//------------------------------------------------------------------------------------------
//!
//!	\file aibehaviour_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef AIBEHAVIOUR_LUA_INC
#define AIBEHAVIOUR_LUA_INC

/////////////////////////////////////
// Includes required for inheritance 
/////////////////////////////////////
#include "ai/aistatemachine.h"

enum AI_EVENT_TYPE
{
	AIE_SEENSOMETHING,
	AIE_BEENHIT,
	AIE_MORALEFAILURE
};

class AIEvent
{
public:
	AIEvent() {;}
	virtual ~AIEvent() {;}

	virtual AI_EVENT_TYPE GetType() = 0;
};

class AIEvent_SeenSomething : public AIEvent
{
public:
	AIEvent_SeenSomething(CEntity* pThing) {m_pThing = pThing;}
	virtual ~AIEvent_SeenSomething();

	virtual AI_EVENT_TYPE GetType() {return AIE_SEENSOMETHING;}

	class CEntity* GetThing() {return m_pThing;}

private:
	CEntity* m_pThing;
};


class AIBehaviour_Lua : public CAIStateMachine
{
public:

	AIBehaviour_Lua(AI* pobEnt) : CAIStateMachine(pobEnt) {};
	~AIBehaviour_Lua() {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::LUA;}

// State Functions
private:
	virtual bool States(const STATE_MACHINE_EVENT, const int, const float);

// Events
private:
	virtual void Event(AIEvent* pEvent) {UNUSED(pEvent);}

// Lua Helpers
private:

// Members
private:
};

#endif //AIBEHAVIOUR_LUA_INC
