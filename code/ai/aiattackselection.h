//------------------------------------------------------------------------------------------
//!
//!	\file attackselection.h
//!
//------------------------------------------------------------------------------------------

//
//  Originally the classes defined in this file held LuaObjects that it used to call
//  lua functions when specific events occur.  Now in order to allow live editing
//  we just remember the string and find the function every time we call it...  If this
//  causes performance issues then we will have to look at how to optimise this.
//

#ifndef _ATTACKSELECTION_INC
#define _ATTACKSELECTION_INC

//------------------------------------------------------------------------------------------
// Headers for inherited/contained classes
//------------------------------------------------------------------------------------------

#include "lua/ninjalua.h"
#include "editable/enums_ai.h"
#include "game/keywords.h"

//------------------------------------------------------------------------------------------
// External Decls
//------------------------------------------------------------------------------------------

class AI;
class Character;
class CEntity;
class CAttackData;
class CAttackComponent;
class CAIAttackRoot;
class CAIVision; // Dario

//------------------------------------------------------------------------------------------
// QueuedAttack
//------------------------------------------------------------------------------------------
struct QueuedAttack
{
	QueuedAttack()							{eType = AAT_NONE; iInfo = 0;m_bForceLockon = false;}
	QueuedAttack(AI_ATTACK_TYPE e, int i=0)	{eType = e; iInfo = i;m_bForceLockon = false;}
	QueuedAttack(float f)					{eType = AAT_PAUSE; fDelay = f;m_bForceLockon = false;}
	
	AI_ATTACK_TYPE	eType;
	
	bool			m_bForceLockon;
	union
	{
		float	   fDelay;
		int		   iInfo;
	};

	ntstd::String			m_strData;
	
	
	bool IsQueueFlush(void) const;
	bool IsCombatFlush(void) const;
	bool IsBlock(void) const;
	bool IsAttack(void) const;
	bool IsEvade(void) const;
	bool IsGrab(void) const;
	bool IsRetToFormation(void) const;
	bool IsAction(void) const;
	bool IsLuaCallback(void) const;
	bool IsMsgFormation(void) const;
	bool IsPlayAnim(void) const;
	bool IsDirectAttack(void) const;
};
typedef ntstd::List<QueuedAttack, Mem::MC_AI> QueuedAttackList;


//------------------------------------------------------------------------------------------
//!
//!	AICombatDef
//!	Combat Definition for an AI.
//!
//------------------------------------------------------------------------------------------
class AICombatDef
{
public:
	HAS_INTERFACE( AICombatDef )
	AICombatDef();
	~AICombatDef();

// Welded Attributes
private:
	friend class AICombatDefI;
	friend class AICombatComponent;
	friend class CAIComponent;
	CHashedString	m_obScriptOverrides;
	CHashedString	m_obFormation;
	CHashedString	m_obGroupCombat;
};

//------------------------------------------------------------------------------------------
//!
//!	
//!	
//!
//------------------------------------------------------------------------------------------
enum AI_COMBAT_STATE
{
	ACS_COUNTERING = CS_COUNT,
	ACS_POST_STRIKE_RECOVERY,

	ACS_COUNT
};

//------------------------------------------------------------------------------------------
//!
//!	AICombatComponent
//!	Combat Component for an AI.
//!
//------------------------------------------------------------------------------------------
class AICombatComponent
{
public:
	AICombatComponent(const AICombatDef* pDef, AI* pEnt);
	~AICombatComponent();

	HAS_LUA_INTERFACE()

	// Allow externals forces to update the combat def
	void ApplyNewCombatDef(const AICombatDef*);

	// Static Init / CleanUp functions
	//////////////////////////////////
	static void Init();
	static void CleanUp();

	// Update
	//////////////////////////////////
	void Update(float fTimeDelta, bool bUpdateMovement);
	void UpdateMovement(float fTimeDelta);
	void UpdateCombat(float fTimeDelta);

	// Calc stop chance
	//////////////////////////////////
	bool ShouldQueueMove( AI_ATTACK_TYPE eType, float ) const;

	// Events
	//////////////////////////////////
	void CombatStateChanged( COMBAT_STATE eCombatState );

	// Attack Selection
	//////////////////////////////////
	void				QueueMove(int eType) const;
	//void				QueuePause(float fDelay) const;
	void				QueueFormationMsg( const char* pcString ) const;
	void				QueuePlayAnim( const char* pcString ) const;
	void				QueueDirectAttack( const char* pcString ) const;
	bool				DirectAttack( const char* pcString );
	void				PerformInstantKORecovery( void ) const;

	void				InsertPause(float fDelay) const;
	void				EmptyQueue() const;
	QueuedAttack		PopAttack();
	bool				HasAttackQueued() const;
	void				ClearAttackQueue() const;
//	void				SetBlockTime(float fTime)	{ m_fBlockTime = fTime; }
	void				SetBlockTimer(float fTime)	{ m_fBlockTimer = fTime; }
	void				PauseUntilBlockComplete(bool bValue) { m_bWaitDeflectionComplete = bValue; }
	void				ClearAttackTimer(void) const { m_fAttackTimer = 0.0f; }
	void				SetAttackTimer(float Time) { m_fAttackTimer = Time; }

	CDirection			ChooseAwesomeEvadeDirection(AI_ATTACK_TYPE eType) const;

	// Lua Override Events
	//////////////////////////////////
	void Event_OnAttackWarning(float fImpactTime, const CEntity* pFrom) const;
	void Event_OnAttacked(AI_ATTACK_TYPE eType, float fDelay, const CEntity*, bool bPreStrike, bool bIncidental) const;
	void ProcessTargetStateChange( AI_COMBAT_STATE, float fTime = 0.0f );

	// Information
	//////////////////////////////////
	bool IsConscious() const {return m_bConscious;} // If we've been KOed, should we get up?
	void RegainConsciousness() const {m_bConscious = true;}

	// Combat state information
	//////////////////////////////////
	float GetRadius() const { return m_fRadius; }
	void  SetRadius(float fValue) { m_fRadius = fValue; }
	float GetTargetDistance(void) const;
	void  SetMoveSpeed(float f) {m_fSpeedToMoveTarget = f;}
	void  SetMovePause(float f) {m_fNewMoveTargetPause = f;}
	void  ResetMovementState() { m_eMovementState = MS_INITIALISE; }
	u_int GetAttackerCount(void) const { return m_AttackerCount; }
	float GetAttackFreeTime(void) const { return m_AttackFreeTimer; }

	// Dario

	void	SetVision(const CAIVision* pV) { m_pVision = pV; }
	void	SetEntityToAttack( const CEntity* pEntity );
	void SetEntityDescriptionsToAttack(const char* szAttackTargetTypes);
	float	RequestMovementRadius( void ) const;
	float	RequestMovementSpeed( void ) const;
	float	RequestMovementPause( void ) const;
	float	RequestMovementTargetThreshold( void ) const;

#ifndef _RELEASE
	const char* GetMovementState(void) const {  static char* pNames[] = {"MS_INITIALISE","MS_WAITING_FOR_RADIUS","MS_MOVING","MS_ATPOINT"}; return pNames[m_eMovementState]; }
#endif 

	AI* GetEntity(void) const { return m_pEntity; }

	void UpdateAttackTarget(float fTimeDelta);

	bool IsAttackingPlayer(void) const;

	// Allow access to setting a new attack tree type
	bool SetNewAttackTree(const CAIAttackRoot* pAIAttackTree);

// Members
//////////
private:


	const CAIAttackRoot*	m_pAttackRoot;


	CHashedString			m_sScriptOverrides;
	//CHashedString			m_sFormationCombat;
	//CHashedString			m_sGroupCombat;

	AI*					m_pEntity;

	const CAIVision*	m_pVision;
	CAttackComponent*	m_pAttackComp;

	// Status
	mutable AI_ATTACK_TYPE						m_eCurrentAttack;			// Our current attack...
	mutable int                                 m_iCurrentAttackInfo;		// ...with user-info.
	mutable QueuedAttackList					m_AttackQueue;				// Waiting attacks
	mutable float								m_fBlockTimer;				// Time to block for
	mutable float								m_fAttackTimer;				// Time to wait for attack
	mutable bool                                m_bConscious;				// If we've been KOed, should we get up?
	float										m_AttackFreeTimer;			// How long has the entity been without attacking?

	// The last state for ower entity
	COMBAT_STATE								m_eLastCombatState;

	// The last state for the player
	COMBAT_STATE								m_eTargetLastCombatState;

	// 
	enum { AIC_ACTIVE, AIC_PASSIVE }			m_eLockonState;
	mutable bool								m_bProcessIncomingAttack;
	mutable bool								m_bProcessIncomingCounter;
	mutable AI_ATTACK_TYPE						m_eIncomingAttackType;
	mutable float								m_fIncomingAttackStrikeTime;

	// 
	mutable bool								m_bWaitDeflectionComplete;

	// Post strike window process only once logic
	mutable bool								m_bInLocalPostStrikeWindow;
	mutable bool								m_bInTargetPostStrikeWindow;


	mutable float		m_fAttackTargetTime;

	CKeywords m_AttackTargetDescriptions;

	// Radius the entity should maintain from the lockon target
	float			m_fRadius;

	// The speed (normalised unit) at which the entity will travel to the move target
	float			m_fSpeedToMoveTarget;

	// A bit like a watchdog, if this timer runs out, then move to another position
	float			m_fMoveTargetTimeLimit;

	// The Point the entity is currently moving towards.
	CPoint			m_obMoveTarget;

	// The amount of time the entity will wait in the new move point default = 3(secs)
	float			m_fNewMoveTargetPause;

	// Point where the target was when the movement started
	CPoint			m_obTargetStart;

	// 
	bool			m_bResetCombatMovement;

	// Number of entities attacking this one. 
	mutable u_int	m_AttackerCount;

	// Movement state for the entity
	enum {
		MS_INITIALISE,
		MS_WAITING_FOR_RADIUS,
		MS_MOVING,
		MS_ATPOINT,
	}m_eMovementState;


	// Our local combat state
	enum AI_ATTACK_STATE
	{
		AIC_IDLE,
		AIC_PROCESS_QUEUE,
		AIC_BLOCKING,
		AIC_BLOCK_RECOVER,
		AIC_ATTACK_PENDING,
		AIC_ATTACKING,
	}m_eCombatState;
};

LV_DECLARE_USERDATA(AICombatComponent);


//------------------------------------------------------------------------------------------
// Mapping Arrays
//------------------------------------------------------------------------------------------
const ATTACK_MOVE_TYPE AAT_2_AM[AAT_COUNT + AAT_FIRST] =
{
	AM_NONE,			//AAT_NONE,			
	AM_NONE,			//AAT_PAUSE,		
	AM_SPEED_FAST,		//AAT_SPEED_FAST,	
	AM_SPEED_MEDIUM,	//AAT_SPEED_MEDIUM,	
	AM_SPEED_GRAB,		//AAT_SPEED_GRAB	
	AM_POWER_FAST,		//AAT_POWER_FAST,	
	AM_POWER_MEDIUM,	//AAT_POWER_MEDIUM,	
	AM_POWER_GRAB,		//AAT_POWER_GRAB	
	AM_RANGE_FAST,		//AAT_RANGE_FAST,	
	AM_RANGE_MEDIUM,	//AAT_RANGE_MEDIUM,	
	AM_RANGE_GRAB,		//AAT_RANGE_GRAB	
	AM_ACTION,			//AAT_ACTION
	AM_DODGE_FORWARD,	//AAT_EVADE,		
	AM_NONE,			//AAT_SPEED_BLOCK,	
	AM_NONE,			//AAT_POWER_BLOCK,	
	AM_NONE,			//AAT_RANGE_BLOCK,	
	AM_NONE,			//AAT_GRAB_BLOCK,	
	AM_DODGE_LEFT,		//AAT_EVADE_LEFT,	
	AM_DODGE_RIGHT,		//AAT_EVADE_RIGHT,	
	AM_DODGE_FORWARD,	//AAT_EVADE_FORWARD,
	AM_DODGE_BACK		//AAT_EVADE_BACK	
};											

// This conversion doesn't really make sense - i have got this noted as an 
// issue but the relationship between AI_ATTACK_TYPE and ATTACK_MOVE_TYPE
// and ATTACK_CLASS really needs to be clarified
const AI_ATTACK_TYPE AC_2_AAT[8] =
{
	AAT_SPEED_FAST,		//AC_SPEED_FAST
	AAT_SPEED_MEDIUM,	//AC_SPEED_MEDIUM
	AAT_POWER_FAST,		//AC_POWER_FAST
	AAT_POWER_MEDIUM,	//AC_POWER_MEDIUM
	AAT_RANGE_FAST,		//AC_RANGE_FAST
	AAT_RANGE_MEDIUM,	//AC_RANGE_MEDIUM
	AAT_SPEED_GRAB,		//AC_GRAB	// ???
	AAT_EVADE_LEFT		//AC_EVADE  // ???
};


const char * const AAT_2_TEXT[AAT_COUNT + AAT_FIRST] =
{
	"NONE",				//AAT_NONE,
	"NONE",				//AAT_PAUSE,
	"SPEED FAST",		//AAT_SPEED_FAST,
	"SPEED MEDIUM",		//AAT_SPEED_MEDIUM,
	"SPEED GRAB"		//AAT_SPEED_GRAB
	"POWER FAST",		//AAT_POWER_FAST,
	"POWER MEDIUM",		//AAT_POWER_MEDIUM,
	"POWER GRAB"		//AAT_POWER_GRAB
	"RANGE FAST",		//AAT_RANGE_FAST,
	"RANGE MEDIUM",		//AAT_RANGE_MEDIUM,
	"RANGE GRAB"		//AAT_RANGE_GRAB
	"BLOCK",			//AAT_BLOCK,
	"EVADE LEFT",		//AAT_EVADE_LEFT,
	"EVADE RIGHT",		//AAT_EVADE_RIGHT,
	"EVADE FORWARD",	//AAT_EVADE_FORWARD,
	"EVADE BACK"		//AAT_EVADE_BACK
};

const AI_ATTACK_RESPONSE CS_2_ATR[CS_COUNT+1] =
{
	AAR_NONE,			//CS_STANDARD
	AAR_NONE,			//CS_ATTACKING
	AAR_RECOILED,		//CS_RECOILING
	AAR_NONE,           //CS_BLOCKING
	AAR_DEFLECTED,		//CS_DEFLECTING
	AAR_KOED,			//CS_KO
	AAR_KOED,			//CS_FLOORED
	AAR_NONE,			//CS_RISE_WAIT
	AAR_STAGGERED,		//CS_BLOCK_STAGGERING
	AAR_STAGGERED,		//CS_IMPACT_STAGGERING
	AAR_NONE,			//CS_INSTANTRECOVER
	AAR_HELD,			//CS_HELD
	AAR_NONE,			//CS_RECOVERING
	AAR_NONE,			//CS_DYING
	AAR_DEAD,			//CS_DEAD
	AAR_NONE			//CS_COUNT
};

#endif //ATTACK_SELECTION_INC
