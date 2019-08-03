//------------------------------------------------------------------------------------------
//!
//!	\file aibehaviourcontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIBEHAVIOURCONTROLLER_H
#define _AIBEHAVIOURCONTROLLER_H

class CAIBehaviourManager;
class CAIComponent;

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviourController
//!
//------------------------------------------------------------------------------------------

enum BEHAVIOUR_MESSAGE
{
	ATTACK_INCOMING,
	ATTACK_TOFARFROMTARGET,
	ATTACK_IN_SHOOTING_RANGE,
	ATTACK_IN_MELE_RANGE,
	ATTACK_LOSTTARGET,
	ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME,
	
	COMBAT_NOATTACKQUEUED,
	
	CHASE_LOSTTARGET,
	CHASE_INATTACKRANGE,
	
	COVER_FINISHEDHIDING,

	INVESTIGATE_FOUNDNOTHING,
	INVESTIGATE_FOUNDTARGET,

	INITIAL_REACTION_FIRST_REPORTER,
	INITIAL_REACTION_NON_FIRST_REPORTER,
	INITIAL_REACTION_RESPONSE_FINISHED,

	SEENENEMY,

    PATROL_SEENSOMETHING,
	PATROL_SEENENEMY,
	ENEMY_DEAD,
	ENEMY_DEAD_ATTACKING_ANOTHER,

	ENTER_FORMATION,
	EXIT_FORMATION,

	PATH_FINISHED,
	PATH_NOT_FOUND,
	PATH_NOT_FOUND_TO_SUITABLE_COVER_POINT,
	MSG_MOVING_TO_COVER_POINT,
	COVER_POINT_REACHED,
	COVER_POINT_TIME_OUT,
	DESTINATION_REACHED,
	DESTINATION_UNREACHABLE,
	TIME_OUT,

	GO_AROUND_FAILURE,

	LOCKON_LOCKOFF,

	ANIM_STARTED,
	ANIM_FAILED,
	ANIM_COMPLETE,
	ANIM_LOOP_COMPLETED,
	LOCATOR_REACHED,
	LOCATOR_UNREACHABLE,
	USING_OBJECT,
	OBJECT_USED,
	OBJECT_NOT_FOUND,
	OBJECT_WAIT_AVAILABLE_TIMEOUT,
	OBJECT_USING_TIMEOUT,
	OBJECT_UNREACHABLE,
	NEW_QUEUE_POINT_REACHED,
	OBJECT_TO_USE_REACHED,
	OBJECT_TO_USE_NOT_INTERACTABLE,
	FACING_ENTITY,
	ENTITY_NOT_FOUND,

	FINDCOVER_NONEFOUND,
	FINDCOVER_INCOVER,
	OPENFIRE_ATFIREPOINT,
	OPENFIRE_FIREDSHOT,

	BALLISTA_TARGETACQUIRED,
	BALLISTA_TARGETLOST,
	BALLISTA_SHOTFIRED,

	BEHAVIOUR_MESSAGE_MAX
};

class AIBehaviourController
{
public:

	AIBehaviourController()				{};
	virtual ~AIBehaviourController()	{};

	virtual bool ProcessMessage( BEHAVIOUR_MESSAGE, CAIBehaviourManager* ) = 0;
	virtual bool ProcessMessage( const char* ) { ntAssert(false); return false; }

private:

};


//------------------------------------------------------------------------------------------
//!
//!	AIMeleeBehaviourController
//!
//------------------------------------------------------------------------------------------


class AIMeleeBehaviourController : public AIBehaviourController
{
public:

	AIMeleeBehaviourController()	{};
	~AIMeleeBehaviourController()	{};

	virtual bool ProcessMessage( BEHAVIOUR_MESSAGE, CAIBehaviourManager* );

private:

};


//------------------------------------------------------------------------------------------
//!
//!	AIRangedBehaviourController
//!
//------------------------------------------------------------------------------------------


class AIRangedBehaviourController : public AIBehaviourController
{
public:

	AIRangedBehaviourController()	{};
	~AIRangedBehaviourController()	{};

	virtual bool ProcessMessage( BEHAVIOUR_MESSAGE, CAIBehaviourManager* );

private:

};

//------------------------------------------------------------------------------------------
//!
//!	AILuaBehaviourController
//!
//------------------------------------------------------------------------------------------

class AILuaBehaviourController : public AIBehaviourController
{
public:

	AILuaBehaviourController(const CAIComponent& obParent);
	~AILuaBehaviourController();

	virtual bool ProcessMessage( BEHAVIOUR_MESSAGE, CAIBehaviourManager* );
	virtual bool ProcessMessage( const char* );

private:
	//LuaObject			m_obLuaController;
	const CAIComponent&	m_obParent;
};



#endif //_AIBEHAVIOURCONTROLLER_H

