/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef _AI_COMPONENT_H
#define _AI_COMPONENT_H

////////////////////////////////
// Necessary Includes
////////////////////////////////

#include "ai/aiattackselection.h"
#include "ai/aibehaviourcontroller.h"
#include "ai/aidefines.h"
#include "game/ai_lua.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "lua/ninjalua.h"

#include "ai/ainavigationsystem/aimovement.h"	// !!! - Dario
#include "ai/aivision.h" // dario
#include "ai/aihearing.h" // dario
#include "ai/airangedtargeting.h"
#include "editable/enums_ai.h"

////////////////////////////////
// External Class Declarations
////////////////////////////////
struct	lua_State;
class	AI;
class	AIStates;
class	CAIStateCMD;
class	CAINavFollowPoint;
class	CAINavPath;
class   AIFormation;
class	CMessageHandler;
class	CAITaskMan;
class	AIFormationAttack;
class	AICombatDef;
class	AICombatComponent;
class	CEntity;
class	AIFormationComponent;
class	CAIVision;
class	CAIRangedTargeting;

#define ACTIVATE_OLD_NAVGRAPH 0 // !!! - (Dario)
#define DBG_OUT(x) {ntPrintf("%s(%d): %s\n", __FILE__, __LINE__, x);}

//------------------------------------------------------------------------------------------
//!
//!	MovementSet
//!	XML Definition of a set of movement controllers for an entity(AI) to use.  - JML
//! This should be modified so we can specify arbitrary movement controllers for
//! downloadable content later if required.  The CAIComponent may not be the ideal place
//! for it either.
//!
//------------------------------------------------------------------------------------------
class MovementSet
{
public:
	MovementSet();

	class MovementControllerDef* m_pobWalkingController;
	class MovementControllerDef* m_pobStrafingController;
	class MovementControllerDef* m_pobCloseStrafingController;
	class MovementControllerDef* m_pobShuffleController;
	class MovementControllerDef* m_pobPatrolWalkController;
	class MovementControllerDef* m_pobInvestigateWalkController;
	class MovementControllerDef* m_pobCrossbowStrafeController;
	class MovementControllerDef* m_pobCrossbowWalkingController;
	class MovementControllerDef* m_pobCrossbowCloseStrafingController;
};

//------------------------------------------------------------------------------------------
//!
//!	AnimSet
//!	XML Definition of a set of single anim movement controllers for an entity(AI) to use.
//!
//------------------------------------------------------------------------------------------
class AnimSet
{
public:
	AnimSet(){};

	class MovementControllerDef* m_pobInvestigateLookAnim;
	class MovementControllerDef* m_pobInvestigateShrugAnim;
	class MovementControllerDef* m_pobPatrolLookAAnim;
	class MovementControllerDef* m_pobPatrolLookBAnim;
};

/***************************************************************************************************
*
*	CLASS			CAIComponentDef
*
*	DESCRIPTION		XML definition structure
*
*	NOTES			Has all things AI related in one lump
*
***************************************************************************************************/
class CAIComponentDef
{
public:
	CAIComponentDef( void );

	// This stays
	//-----------------------
	enum AI_START_STATE	m_eStartState;		// default state to start in
	enum AI_ACTION_USING m_eUsingState;
	AICombatDef*        m_pobCombatDef;
	MovementSet*        m_pobMovementSet;
	AnimSet*			m_pobAnimSet;
	CKeyString			m_obAIBehaviourSet;
	bool				m_bGrouped;
	bool				m_bDrawViewCones;
};

/***************************************************************************************************
*
*	CLASS			CAIComponent
*
*	DESCRIPTION		shell entity component that interfaces with the AI manager
*
*	NOTES			will grow to encompase AI states and all that jazz
*
***************************************************************************************************/
class CAIComponent : public AI_Lua
{
public:
	CAIComponent( AI* pobParent, const CAIComponentDef* pobDef );
	~CAIComponent( void );

	// Called once the AIComponent has been fully constructed (c++ term)
	void	Constructed(void);

	void	Update( float fTimeChange );

	void	Reset();

	// !!! - Dario. New Navigation and Vision

	CAIMovement*	GetCAIMovement			( void ) { return (&m_Movement) ; }
	CAIVision*		GetAIVision				( void ) { return (&m_AIVision) ; }
	CAIHearing*		GetAIHearing			( void ) { return (&m_AIHearing) ; }
	CAIRangedTargeting* GetAIRangedTargeting ( void ) { return (&m_AIRangedTargeting); }
	void			SetVision				( bool b ) { m_AIVision.SetVision(b); }
	void			SetIntention			( unsigned int eIntentions, float fSpeed, unsigned int eMovFlag );
	void			SetEntityToGoTo			( CEntity* );
	const CEntity*	GetEntityToAttack		( void ) const;
	void			SetEntityToAttack		( const CEntity* );
	void			SetEntityDescriptionsToAttack	(const char* pEntityDescriptions);
	void			SetEntityToFollow		( CEntity* );
	void			SetStartEndNodes		( const char*, const char*);
	void			SetDestinationNode		( const char* );
	void			SetPatrolling			( const char* );
	void			SetMovementParam		( unsigned int, float );
	void			SetVisionParam			( unsigned int, float );
	void			SetRangedTargetingParam ( unsigned int, float );
	void			SetDestinationRadius	( float );
	void			SetAttackRange			( float );
	void			SetIdleFlags			( unsigned int );
	bool			GetFormFlagAttacking	( void ) const;
	bool			IsInFormFormation		( void ) const;
	void			SetIdleClearsIntention	( bool );
	void			SetExternalControlState ( bool );
	void			SetCoverAttitude		( unsigned int );
	void			SetReuseCoverPoints		( bool );
	void			SetMinWallDetRadius		( float );
	void			SetMinMaxRadii			( float, float );
	void			SetTimeBetweenShoots	( float );

	void			SetHearingParam			( unsigned int, float );
	void			SetNumberOfConsecShots	( unsigned int );
	void			SetWhackAMoleNode		( CHashedString );
	void SetVolleyShots(int iVolleyShots) { m_Movement.SetVolleyShots(iVolleyShots); }
	void SetVolleySquad(int iVolleySquad) { m_Movement.SetVolleySquad(iVolleySquad); }
	void SetVolleyReloadPause(float fPauseMin, float fPauseMax) { m_Movement.SetVolleyReloadPause(fPauseMin, fPauseMax); }
	void SetVolleyAimPause(float fPause) { m_Movement.SetVolleyAimPause(fPause); }
	void SetVolleyPauseBetweenShots(float fPauseMin, float fPauseMax) { m_Movement.SetVolleyPauseBetweenShots(fPauseMin, fPauseMax); }
	
	void			SetCannonTarget			( CEntity* );
//	void			SetCannonTargetLocatorPos ( const CPoint&  );
	void			ShootTheCannon			( void );

	void			SetOffsetRadius			( float );
	void			SetShootingAccuracy		( float );
	void			SetAlwaysMiss			( bool );
	void			SetRangedParam			( unsigned int, float );
	void			SetAIBoolParam			( unsigned int, bool );

	void			SetAIUpdateIndex				( unsigned int i )	{ m_uiAIUpdateIndex = i; }
	unsigned int	GetAIUpdateIndex				( void ) const		{ return m_uiAIUpdateIndex; }
	//void			SetTotalNumberofUpdateGroups	( unsigned int i )	{ static_TotalNumberUpdateGroups = i; }
	//unsigned int	GetTotalNumberofUpdateGroups	( void ) const		{ return static_TotalNumberUpdateGroups; }
	//void			SetCurrentUpdateGroups			( unsigned int i )	{ static_CurrentUpdateGroups = i; }
	//unsigned int	GetCurrentUpdateGroups			( void ) const		{ return static_CurrentUpdateGroups; }

	void			SetUseObjectPitch			( float f	)		{ m_fUseObjectPitch = f; }
	void			SetUseObjectYaw				( float f	)		{ m_fUseObjectYaw = f; }
	float			GetUseObjectPitch			( void		) const	{ return m_fUseObjectPitch; }
	float			GetUseObjectYaw				( void		) const	{ return m_fUseObjectYaw; }

	bool			SetWalkRunMovementController( CHashedString );

	void			SetNavigGraphActiveLUA		( CHashedString, bool );
	void			SetGoToLastKnownPlayerPosInAttack ( bool );

	float			GetRangedParameter ( unsigned int );

	// !!! - Dario End.

	// accesors
	AI*									GetParent()				const {return m_pobEntity;}
	const	CAIComponentDef*			GetDefinition( void )	const {return m_pobDefinition;}
	const	AIStates*					GetAIState( void )		const {return m_pobState;}


	// Find another attack target

	const CEntity* FindAttackTarget(void) const;


	AIStates*					GetAIState_ForMessagesONLY( void )			{ return m_pobState; }

	// To enable and disable for callbacks
	// static void SetDisabled( CEntity* pobEntity, bool bDisabled );

	void	SetDisabled( bool bDisabled );
	bool	GetDisabled( void ) const;

	// Shutdown the ai component
	void Shutdown(void);

	//CAINavigator* GetNavigatorP() { return m_pobNavigator; }
	//CMessageHandler* GetMessageHandlerP() { return m_pobEntity->GetMessageHandler(); }

	// Allow internal components to expose the data to be used to drive other components
	void SetMovementMagnitude( float fMovementMagnitude ) 
	{
		ntAssert(fMovementMagnitude >= 0.0f && fMovementMagnitude <= 1.0f);
		m_fMovementMagnitude = fMovementMagnitude; 
	}
	void SetMovementDirection( const CDirection& obMovementDirection ) { m_obMovementDirection = obMovementDirection; }
	void SetMovementFacing(const CDirection& dirFacing) {m_dirMovementFacing = dirFacing;}
	void SetDirectTarget( CEntity* pobDirectTarget ) { m_pobDirectTarget = pobDirectTarget; }

	// An interface to provide information analagous to the input controller
	float				GetMovementMagnitude( void ) const { return m_fMovementMagnitude; }
	const CDirection&	GetMovementDirection( void ) const { return m_obMovementDirection; }
	const CDirection&	GetMovementFacing()          const {return m_dirMovementFacing;}
	CEntity*			GetDirectTarget( void )      const { return m_pobDirectTarget; }

	// JML AI Combat
	const AICombatComponent& GetCombatComponent()    const	{return m_combatComponent;}
	AICombatComponent& GetCombatComponent()					{return m_combatComponent;}
	const AICombatComponent* GetCombatComponentP()   const  {return &m_combatComponent;}
	void  UpdateCombatComponent(float fTimeDelta,bool bUpdateMovement) {m_combatComponent.Update(fTimeDelta, bUpdateMovement);}
	void	SetRecovering( bool recovering );
	bool	IsRecovering() const 					{ return m_recovering; }

	// navigation
	void				MakePathToDest(const CPoint* pobDest = 0);
	void				KillPath();
	CAINavPath*			GetPath()			         const {return m_pobNavPath;}
	CAINavFollowPoint&	GetFollowPoint()             const {return *m_pobFollowPoint;}
	//int					GetAvoidanceID()             const {return m_iAvoidanceID;}
	bool				IsStuck(void) const			{ return m_bStuck; }
	bool				JustStuck(void) const		{ return !m_bStuckLast && m_bStuck; }

	// The Behaviour Manager
	class CAIBehaviourManager* GetBehaviourManager();

	// action to perform, set by the behaviours
	void				SetAction( const CAIState_ACTION eAction );
	void				SetActionStyle( const CAIState_ACTIONSTYLE eStyle );
	void				SetActionUsing( AI_ACTION_USING eUsing );
	void				SetActionDest( const CPoint& obDest );
	void				SetActionFacing(const CDirection& dirFacing);
	void				SetActionMoveSpeed(float fSpeed);
	CAIState_ACTION		GetAction( void ) const;
	const CPoint&		GetActionDest( void ) const;
	const CDirection&	GetActionFacing() const;
	bool				IsSimpleActionComplete() const	{ return !m_bPlayingSingleAnim; }
	bool				IsPlayingFacingAction() const	{ return m_bPlayingFacingAction; }	// Dario
	void				SetPlayingFacingAction( bool b ){ m_bPlayingFacingAction = b; }		// Dario
	void				CompleteSimpleAction()			{ m_bPlayingSingleAnim = false; }
	void				PlaySingleAnimUntilComplete(bool bState) { m_bPlaySingleAnimUntilComplete = bState; }

	// interface for bind functions to select movement speeds, target positions etc.
	void				SetScriptAnimName(CHashedString animName)				{ m_strAnimName = animName; }
	CHashedString		GetScriptAnimName() const 								{ return m_strAnimName; }
	void				SetFormationIdleAnim( CHashedString animName )			{ m_strFormationAnimName = animName; }
	CHashedString		GetFormationIdleAnim( void ) const						{ return m_strFormationAnimName; }
	void				SetScriptAnimLooping( const bool animLoop )				{ m_bAnimLoop = animLoop; }
	const bool			GetScriptAnimLooping() const 							{ return m_bAnimLoop; }
	void				SetScriptLocatorName( const char* locatorName )			{ m_strLocatorName = locatorName; }
	const char*			GetScriptLocatorName() const							{ return m_strLocatorName.c_str(); }
	void				SetScriptFacingLocatorName( const char* locatorName )	{ m_strFacingLocatorName = locatorName; }
	const char*			GetScriptFacingLocatorName() const						{ return m_strFacingLocatorName.c_str(); }
	void				SetScriptLocatorThreshold( float threshold )			{ m_fThreshold = threshold; }
	float				GetScriptLocatorThreshold() const						{ return m_fThreshold; }
	void				SetScriptObjectName( const char* objectName )			{ m_strObjectName = objectName; }
	const char*			GetScriptObjectName() const								{ return m_strObjectName.c_str(); }
	void				SetScriptRun( const bool run )							{ m_bRun = run; }
	bool				GetScriptRun() const									{ return m_bRun; }
	void				SetScriptFacingEntityName( const char* entityName )		{ m_strFacingEntityName = entityName; }
	const char*			GetScriptFacingEntityName() const						{ return m_strFacingEntityName.c_str(); }
	void				SetScriptCoverPointName( const char* name )				{ m_strCoverPointName = name; }
	const char*			GetScriptCoverPointName() const							{ return m_strCoverPointName.c_str(); }
	
	void				SetScriptBallistaTarget( const char* name )				{ m_strBallistaTargetName = name; m_bBallistaTargetChanged = true; }
	const char*			GetScriptBallistaTarget() const							{ return m_strBallistaTargetName.c_str(); }
	bool				BallistaTargetChanged()									{ return m_bBallistaTargetChanged; }
	void				AcknowledgeBallistaTargetChange()						{ m_bBallistaTargetChanged = false; }
	void				SetScriptBallistaTargetOffset( float x,float y,float z)	{ m_obBallistaTargetOffset.X() = x; m_obBallistaTargetOffset.Y() = y; m_obBallistaTargetOffset.Z() = z; }
	CPoint				GetScriptBallistaTargetOffset() const					{ return m_obBallistaTargetOffset; }
	void				ScriptFireBallista()									{ m_bNeedToFireBallista = true; }
	void				BallistaFired()											{ m_bNeedToFireBallista = false; }
	bool				BallistaFireRequested()									{ return m_bNeedToFireBallista; }
    
	// vision
	bool				CanSeePlayer() const				{ return m_fPlayerVis > 0.9f; }
	bool				CanSeeSuspicious() const			{ return m_fPlayerVis > 0.4f; }
	void				SetActualPlayerPos(const CPoint& pt){ m_obActualPlayerPos = pt; }
	CPoint				GetActualPlayerPos() const 			{ return m_obActualPlayerPos; }
	CPoint				GetLastKnownPlayerPos() const 		{ return m_obLastKnownPlayerPos; }
	void				CanAlwaysSeePlayer( bool bCanSee)	{ m_bCanAlwaysSee = bCanSee; }
	bool				HasSeenPlayer() const				{ return m_bPlayerSeenEver; }
	void				SetAlerted( const bool bAlerted )	{ m_bAlerted = bAlerted; }

	// Get the formation component
	AIFormationComponent* GetAIFormationComponent() const { return m_pFormationComponent; }

	// Get a reference to a lua state
	const NinjaLua::LuaObject& GetLuaState(void) const { return m_obLuaState; }
	void                       SetLuaState(NinjaLua::LuaObject obState) {m_obLuaState = obState;}
	void						ResetLuaFSM(void) const;

	// Called if the combat state is changed
	void CombatStateChanged( COMBAT_STATE eCombatState );

	// The new health of the charater
	void HealthChanged( float fNewHealth, float fBaseHealth ) const;

	void PlayVO( const char* bank, const char* cue );

	// Return the current state of the AI in Text form
	const char* GetCurrentState () const;

	// Apply a new AI def 
	void ApplyNewAIDef(const char*);

	// Cover point management
	void	SetCoverPoint( int pointIdx )	{ m_CoverPointIdx = pointIdx; };
	int		GetCoverPoint()					{ return m_CoverPointIdx; };	
	bool	InCover()						{ return m_bInCover; };	
	void 	SetInCover( bool inCover)		{ m_bInCover = inCover; };	

private:
	NinjaLua::LuaObject SendMessageInt( const char* pcMsg, int iArgs );

// AIComponent Enums
public:
	enum MOVEMENT_CONTROLLER
	{
		MC_NONE = -1,
		MC_WALKING,
		MC_STRAFING,
		MC_CLOSE_STRAFING,
		MC_CROSSBOW_WALKING,
		MC_CROSSBOW_STRAFE,
		MC_SHUFFLE,
		MC_PATROL,
		MC_INVESTIGATE,


		MC_SINGLEANIM
	};

	enum PATHFIND_STATUS
	{
		PF_SEARCHING,
		PF_SUCCESS,
		PF_FAILED,
	};

	PATHFIND_STATUS	GetPathfindStatus() const { return m_ePathfindStatus; }
	void			SetPathfindStatus( PATHFIND_STATUS status )  { m_ePathfindStatus = status; }

	HAS_LUA_INTERFACE()


	uint64_t	m_uiBehaviourTimeStamp;

private:
	void PathfindStatusUpdate();

	PATHFIND_STATUS	m_ePathfindStatus;

public:
	// JML - AI Movement controllers - Perhaps functionality should go outside AI component later though?
	bool ActivateController(enum MOVEMENT_CONTROLLER eMC, bool bAlwaysSet = false);
	void RefreshController();
	bool ActivateSingleAnim(CHashedString pAnimName, float fAnimSpeed = 1.f, float fAnimTimeOffsetPercentage = 0.f, float fAnimBlend = 0.15f, bool bSendFinishedMsg = false );
	bool ActivateSingleAnimDef(const char* pAnimName) { return (ActivateSingleAnim(pAnimName,1.f,0.f,0.15f,true)); }
	float TimeRemainingOnSingleAnim(void) const { return m_fSingleAnimRemaining; }
	void CancelSingleAnim();

private:
	void	VisionUpdate();
	void	HeadFacingUpdate();

	mutable bool 						m_bReqMoveTransition;
	bool								m_bFirstUpdate;

	// definition structure
	const CAIComponentDef*				m_pobDefinition;
	
	// members
	AIStates*							m_pobState;			// performs high level command logic.
	ntstd::List<CAIStateCMD*>			m_obStateCMDs;		// list of commands to send to the interpreter from the states


	// AI Vision params
	CAIVision*			m_pVision;

	// navigation info ( !!! - obsolete (Dario ) )
	CAINavPath*							m_pobNavPath;
	CAINavFollowPoint*					m_pobFollowPoint;		// for navigation
	int									m_iAvoidanceID;

	// The entities last position
	CPoint								m_ptEntLastPos;
	float								m_fStuckTimer;
	bool								m_bStuck;
	bool								m_bStuckLast;

	ntstd::String						m_strStatus;

	CHashedString						m_strAnimName;
	CHashedString						m_strFormationAnimName;
	ntstd::String						m_strLocatorName;
	ntstd::String						m_strFacingLocatorName;
	float								m_fThreshold;
	ntstd::String						m_strObjectName;
	bool								m_bRun;
	ntstd::String						m_strFacingEntityName;
	bool								m_bAnimLoop;
	ntstd::String						m_strCoverPointName;

	ntstd::String						m_strBallistaTargetName;
	bool								m_bBallistaTargetChanged;
	CPoint								m_obBallistaTargetOffset;
	bool								m_bNeedToFireBallista;

	//Used to look up movement
	AI*									m_pobEntity;

	// Movement request values
	float								m_fMovementMagnitude;
	CDirection							m_obMovementDirection;
	CDirection  						m_dirMovementFacing;
	CEntity*							m_pobDirectTarget;

	// vision
	float								m_fPlayerVis;
	CPoint								m_obLastKnownPlayerPos;
	CPoint								m_obActualPlayerPos;
	bool								m_bCanAlwaysSee;
	bool								m_bPlayerSeenEver;
	bool								m_bAlerted;
	CDirection							m_obHeadFacing;

	// JML - AI Combat Component
	AICombatComponent					m_combatComponent;
	bool								m_recovering;

	// JML - Formations
	//AIFormation*		m_pFormation;
	//AIFormation*		m_pPendingFormation;
	//CPoint				m_ptFormationTarget;
	//AIFormationAttack*	m_pFormationAttack;

	// Formation entities need to pause sometimes. 
	float								m_fFormationMovementPaused;

	// Once the entity has died, this count down timer is used to kill the ai component. 
	// It gives time for the message handler to process any outstanding messages. 
	int									m_iShutdownTicker;

	// Movement
	MOVEMENT_CONTROLLER					m_eCurrentMC;
	
	bool								m_bPlayingSingleAnim;
	bool								m_bPlaySingleAnimUntilComplete;
	bool								m_bPlaySingleAnimCompleteMsg;
	float								m_fSingleAnimRemaining;

	// The Lua behaviour controller
	NinjaLua::LuaObject					m_obLuaState;

	// Formation component instance
	AIFormationComponent*				m_pFormationComponent;

	// Cover point management
	int									m_CoverPointIdx;
	bool								m_bInCover;

private:
	
	// !!! - Dario. New Navigation
	CAIMovement							m_Movement; 
	CAIVision							m_AIVision; 
	CAIHearing							m_AIHearing; 
	CAIRangedTargeting					m_AIRangedTargeting;
	bool								m_bPlayingFacingAction;
	unsigned int						m_uiAIUpdateIndex;
	float								m_fUseObjectPitch;
	float								m_fUseObjectYaw;

	//static unsigned int					static_TotalNumberUpdateGroups;
	//static unsigned int					static_CurrentUpdateGroups;

public:
	AI_MOVEMENT_CONTROLLER_MODIFIER GetControllerModifier() { return m_eControllerModifier; }
	void SetControllerModifier(AI_MOVEMENT_CONTROLLER_MODIFIER eModifier) { m_eControllerModifier = eModifier; }
	void SetWalkController(MovementControllerDef* pobWalkController) { m_MyWalkController = pobWalkController; }
	void SetStrafeController(MovementControllerDef* pobStrafeController) { m_MyStrafeController = pobStrafeController; }
	void SetCloseStrafeController(MovementControllerDef* pobCloseStrafeController) { m_MyCloseStrafeController = pobCloseStrafeController; }
private:
	//GavinC - Generic modify-able movement controller pointers (can vary based on status or what they're carrying)
	MovementControllerDef*	m_MyWalkController;
	MovementControllerDef*	m_MyStrafeController;
	MovementControllerDef*	m_MyCloseStrafeController;
	AI_MOVEMENT_CONTROLLER_MODIFIER	m_eControllerModifier;
};

LV_DECLARE_USERDATA(CAIComponent);

#endif // _AI_COMPONENT_H

