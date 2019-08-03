//------------------------------------------------------------------------------------------
//!
//!	\file aiformationattack.h
//!
//------------------------------------------------------------------------------------------


#ifndef _AIFORMATIONATTACK_INC
#define _AIFORMATIONATTACK_INC

#include "lua/ninjalua.h"
#include "editable/enumlist.h"
#include "game/keywords.h"
#include "ai/aiformationxml.h"

//------------------------------------------------------------------------------------------
// External Decls.                           
//------------------------------------------------------------------------------------------

class CEntity;
class AI;
class AIFormation;
class AIGromboStateMachine;
class FormationComponent;
class CGromboState;
class CGromboAttack;
class CGromboEntity;
class CGromboInstance;
class CGromboAttackPattern;
class CGromboAttackList;


//static const char* GC_GOTO_STATE	= "gc_goto";
//static const char* GC_DEFAULT_STATE = GC_GOTO_STATE;


//------------------------------------------------------------------------------------------
//!
//!	CAIFormationAttackString
//!	
//!
//------------------------------------------------------------------------------------------

class CAIFormationAttackString
{
public:

	// From a given list of entities, find a match that'll suit the 
	// needs of the current attack query
	AI* FindValid( const ntstd::List<AI*>& obEntList, CEntity* pobPlayer, CEntity* pobLastFoundEnt );

	// Storage for the type of character required for this stage in the group combat
	//char	m_acType[32];
	CKeywords m_obType;

	// the first 8 bits represent a direction based on the player current facing direction
	u_int	m_AngleBits;

	// the first 8 bits represent a direction based on the camera direction
	u_int	m_CameraBits;

	// the first 8 bits represent a direction based on the last entity chosen for the group attack
	int		m_RelativeBits;

	// The time an entity should wait until it's not attacked.
	float	m_TimeNotInGrombo;

	// Should the entity be in formation position for the attack to start. 
	float	m_FormationPositionOffset;

	// Distance the entity must be from the target
	float	m_Distance;

	// This needs refactoring, first stage though is to include pointers to the successor class...
	const CGromboAttackPattern* m_pPattern;
	const CGromboEntity*        m_pEntity;
};
typedef ntstd::List<CAIFormationAttackString*, Mem::MC_AI> AIFormationAttackStringList;

//------------------------------------------------------------------------------------------
//!
//!	AIFormationAttack
//!	An attack for use with a formation
//!
//------------------------------------------------------------------------------------------
class AIFormationAttack
{
public:
	typedef ntstd::List<AIFormationAttack*, Mem::MC_AI>	List;

	AIFormationAttack( FormationComponent*, const CGromboInstance* pGromboInstance, const ntstd::String& );
	~AIFormationAttack();

	HAS_LUA_INTERFACE()
	
	// Standard update function
	bool Update(float fTimeDelta);

	// Passed a list of entities that allows the attack to check if it can run
	bool Validate( ntstd::List<AI*>&, const List& rActiveAttacks );

	void          AddQuery(CAIFormationAttackString* pQuery) {m_obQuerys.push_back(pQuery);}
	int           StringCount() const {return m_obQuerys.size();}
	const AIFormationAttackStringList& GetStringList(void) const { return m_obQuerys; }
	
	// Return the weighting of the attack
	float		GetWeight(void) const { return m_fWeighting; }

	// Validity
	bool			IsValid() const  {return m_bValid;}
	//void			SetValid(bool b) {m_bValid = b;}

	// Paused
	bool			IsPaused() const  {return m_bPaused;}
	void			SetPaused(bool b) {m_bPaused = b;}

	// Can the attack be interrupted
	bool			IsInterruptible(void) const;

	// Is the attack really an incidental
	bool			IsIncidental() const { return m_Incidental; }

	bool			AllowedToRunSimultaneously(void) const { return m_bAllowedToRunSimultaneously; }

	// Get the attack priority
	float			GetPriority(void) const { return m_Priority; }

	// 
	bool			IsFinished() const;

	// Is the formation attack active?
	bool			IsActive(void) const { return m_pGromboStateMachine != 0; }

	// Prepare and initiate the attack
	void			Initiate( );

	// Remove an entity fromthe list. 
	void			RemoveEntity( CEntity* );

	// End the current attack
	void			EndCurrentAttack(void);

	// Return the debug context for the attack
	void			SetDebugContext(int DebugContext) { m_DebugContext = DebugContext; }
	int				GetDebugContext(void) const { return m_DebugContext; }

	// Called when the combat state changes
	void CombatStateChanged( CEntity*, COMBAT_STATE );

	// Access to the delay setting mech's...
	void SetStartDelay(float fTime) { m_StartDelay = fTime; }
	void SetEndDelay(float fTime) { m_EndDelay = fTime; }
	void SetOnComplete( ntstd::String& obMsg ) { m_OnCompleteMsg = obMsg; }

	// Return a reference to the name of the attack
	const ntstd::String& GetName() const { return m_Name; }

	// Inform the entity that it's ready to start another attack
	void AttackStateEnded( CEntity* );

	// Can the entity perform non-formation attacks?
	bool CanEntityAttack( CEntity* );
	bool CanEntityBlock( CEntity* );

	bool IsImpossible(void) const { return m_bImpossible; }
	void SetImpossible( bool bValue )  { m_bImpossible = bValue; }

	// bool IsOneOnOneAttackRequirement() const { return m_OneOnOneAttacker.length() > 1; } - ALEXEY_TODO - can be a problem
	bool IsOneOnOneAttackRequirement() const { return !ntStr::IsNull(m_OneOnOneAttacker); }
	const CHashedString& OneOnOneAttackRequirement() const { return m_OneOnOneAttacker; }

	CEntity* OneOnOneAttackTest( const CHashedString& ) const;

	// Return the player combo requests
	const NinjaLua::LuaObject& GetPlayerComboReq(void) const { return m_PlayerCombo; }

	// Return the valid entity list
	const ntstd::List<AI*>& GetValidEntList( void ) const { return m_ValidEntList; }


	// Debug code.. 
	const char* m_pcValidFailedReason;

// Helper-funcs
protected:


// Members
private:

	// Attack name
	ntstd::String m_Name;

	// Set the grombo instance. 
	const CGromboInstance*		m_pGromboInstance;

	// Pointer to the parent of the attack
	FormationComponent*			m_pobParent;

	// The debug Context
	int							m_DebugContext;

	// Is this attack valid on it's formation.
	bool						m_bValid;			
	
	// Allow the formation attack to be paused
	bool						m_bPaused;

	// Is the attack impossible to run? This would be beacuse there aren't enough entities in the 
	// formation to trigger the attack
	bool						m_bImpossible;

	// Is the attack really an incidental
	bool						m_Incidental;

	// Can this attack run simultaneously with another attack?
	bool						m_bAllowedToRunSimultaneously;

	// Can the attack be interrupted
	float						m_Interruptible;

	// The higher the number, the lower the priority
	float						m_Priority;

	// Target type for the attack
	ntstd::String				m_obTargetType;

	// Is there a One on One attacker requirement? The requirement comes in the form of
	// a attack string the attacker is currently playing
	CHashedString				m_OneOnOneAttacker;

	// Return the weighting for the attack
	float						m_fWeighting;

	// Once chosen, delay the start of the attack for this given time.. 
	float						m_StartDelay;

	// Once the attack has finished, then remain active for the following time... 
	float						m_EndDelay;

	// Name of the on complete msg. 
	ntstd::String				m_OnCompleteMsg;

	// Metadata for the attack
	CKeywords					m_obMetadata;

	// List
	ntstd::List<AI*>		m_ValidEntList;

	// Allows the formation attack to wait for a player combo
	NinjaLua::LuaObject			m_PlayerCombo;

	// Entities that are required to play this group combat sequence
	AIFormationAttackStringList	m_obQuerys;

	// The grombo statemachine
	AIGromboStateMachine* m_pGromboStateMachine;
};
typedef AIFormationAttack::List AIFormationAttackList;


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
class AIGromboActor
{
public:

	~AIGromboActor(void)
	{
		Exit();
		ntAssert( m_bFinished == true );
		ntAssert( m_Attacking == false );
	}

	// Update for the grombo actor
	void Update(float fTimeDelta);

	// Allow the actor to 
	void Initialise(void);

	// Exit the attack
	void Exit(bool bError = false);

	// Called when the attack for the actor has completed
	void AttackStateEnded(void);

	// Combat state change handling
	void CombatStateChange( COMBAT_STATE );

	// Can the entity process a non-formation attack?
	bool CanAttack( void ) const { return m_OneOnOneState; }
	bool CanBlock( void ) const { return true; }
	bool IsFinished() const { return m_bFinished; }
	bool IsInOneOnOne() const { return m_OneOnOneState; }
	bool HasError() const { return m_bError; }
	void SetEarlyExit(void) { m_bEarlyExit = true; }

	// Process the state change for the actors grombo
	void ProcessStateChange(void);

	// Pointer to the entity the actor is controlling
	AI* m_pEntity;

	// Grombo State machine pointer
	AIGromboStateMachine* m_pGromboState;

	// The Id of Grombo Actor
	int m_Id;

	// Pointer to the grombo entity
	CGromboEntity*	m_pGromboEntity;

private:

	// Flag the actor as finished
	bool m_bFinished;

	// Flag the actor as an error having occured
	bool m_bError;

	// Should the formation perform an early exit
	bool m_bEarlyExit;

	// Is the entity blocking?
	bool	m_Blocking;

	// Time to wait until the anims are triggered
	float	m_fDelay;

	// One on one states
	bool	m_OneOnOneState;
	bool	m_UntilKO;
	bool	m_UntilRecoiling;
	bool	m_Attacking;
	bool	m_FaceTarget;
	float	m_Timeout;
	float	m_UntilRangeSqrd;


	// Early attack range.. only works if in a goto state
	float m_EarlyAttackRange;

	// Reference to the defining state of the grombo attack for the entity
	CGromboState*		m_pAttackData;
	CGromboState*		m_pNextAttack;

	// The active animation index playing
	GromboAttackList::const_iterator	m_ActiveAttack;


};

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
class AIGromboStateMachine
{
public:
	AIGromboStateMachine( AIFormationAttack* pParent, const ntstd::List<AI*>& obEntities, const CGromboInstance* pGromboInstance );

	// 
	~AIGromboStateMachine(void)
	{
		// Remove the actors
		NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_pActors );
	}


	// Method for updating the grombo
	bool Update( float fTimeDelta );

	// Return the formation attack.
	AIFormationAttack* GetFormationAttack() const { return m_pFormationAttack; }

	// Called when the attack for the given entity has finished
	void AttackStateEnded( CEntity* );

	// Combat state change handling
	void CombatStateChange( CEntity*, COMBAT_STATE );

	// Can the entity attack
	bool CanEntityAttack( CEntity* pEntity );
	bool CanEntityBlock( CEntity* pEntity );

	// Set the start and end delays. 
	void SetStartDelay(float fDelay) { m_StartDelay = fDelay; }
	void SetEndDelay(float fDelay) { m_EndDelay = fDelay; }

	// Remove Entity from the state machine
	void RemoveEntity( CEntity* );

	// One on One attack name test
	CEntity* OneOnOneAttackTest( const CHashedString& robAttackName ) const;

	// Return one of the actors. 
	const AIGromboActor* GetActor( u_int Index ) const { if( Index >= (u_int)m_ActorCount ) return 0; return m_pActors + Index; }

	// Return the running time of the grombo
	float GetUpTime(void) const { return m_RunningTime; }

	// With the current state of this attack, can another attack run?
	bool AllowSimultaneousAttack(void) const { return m_AllowSimultaneousAttack; }

private:

	// Formation attack pointer
	AIFormationAttack* m_pFormationAttack;

	// 
	const CGromboInstance* m_pGromboInstance;

	// List of entities playing the grombo
	ntstd::List<AI*> m_Entities;

	// grcombo actors
	AIGromboActor*	m_pActors;

	// Number of actors
	int	m_ActorCount;

	// How long the grombo has been running for
	float m_RunningTime;

	// 
	bool m_bClosingDown;

	// Start and end delays
	float m_StartDelay;
	float m_EndDelay;

	// Allow 
	bool m_AllowSimultaneousAttack;
};

LV_DECLARE_USERDATA(AIFormationAttack);

#endif //_AIFORMATIONATTACK_INC

