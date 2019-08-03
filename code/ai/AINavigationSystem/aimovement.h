//! -------------------------------------------
//! AIMovement.h
//!
//! Movement AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIMOVEMENT_H
#define _AIMOVEMENT_H

#include "ainavigpath.h"
#include "editable/enums_ai.h"
#include "game/randmanager.h"
#include "game/entity.inl"
#include "game/entity.h"
#include "game/chatterboxman.h"
#include "ai/aibehaviour_whackamole.h"

// Forward declaration of classes and structures
class CEntity;
class CAIComponent;
class CAINavigNode;
class CAIPatrolGraph;
class CAISingleQueue;

#define MIN_TIME_BETWEEN_SHOTS (2.5f)
#define MIN_WHACKAMOLE_HIDDING_TIME (2.5f)
#define MAX_WHACKAMOLE_HIDDING_TIME (10.0f)

#define AIMOV_SLOWWALK_SPEED (0.4f)
#define AIMOV_FASTWALK_SPEED (0.6f)
#define AIMOV_RUN_SPEED		 (0.9f)
#define AIMOV_SLOWDOWN_DEFAULT_TIME (2.0f)

//#define DIVING_FREE_TIME (2.0f)

//! -------------------------------------------
//! SMovParams
//! -------------------------------------------
typedef struct _SMovParams
{
	_SMovParams () :	m_fMaxSpeed(1.0f), m_fWalkSpeed(0.6f), m_fChaseSpeed(1.0f), m_fPatrolSpeed(7.0f), m_fApproachSpeed(8.5f), m_fFaceDirectionSpeed(0.19f),
						m_fPanicDistSQR(4.0f),
						m_fAIAvoidanceRadius(0.5f), m_fAIAvoidanceRadiusSQR(0.25f), m_fNormalAIAvoidanceRadius(0.5f), m_fCombatAIAvoidanceRadius(0.15f), m_fFormationAIAvoidanceRadius(0.5f), 
						m_fAIFlockRadius(5.0f), 
						m_fResidentEvilRadiusSQR(225.0f), m_fAttackRangeSQR(25.0f), m_fAttackPointRadius(0.5f), m_fAttackPointRadiusSQR(0.25f),  
						m_fFormationRadiusSQR(49.0f), m_fFormationMoveClockwise(false),
						m_bTrimPath(false), m_fMaxWaitingTimeAfterPathNotFound(0.5f),
						m_fFollowPathRadiusPercentage(0.9f),
						m_fHalfWidthDetectionBox(0.5f), m_fSideDetectionLength(1.0f),
						m_fMinObstacleDetectionRadius(1.0f), m_fMinWallDetectionRadius(1.0f), 
						m_fFollowEntityRadiusSQR(16.0f), m_fCharacterRadius(0.5f), 
						m_bGoToLastKnownPlayerPosInAttack(false), m_uiIdleFlags(NF_NO_MOVEMENT), m_uSteeringFlags(NF_NO_MOVEMENT)
						{
							m_afSteeringWeight[NWI_D_OBSTACLE]			= 2.0f;
							m_afSteeringWeight[NWI_FLEE]				= 1.0f;
							m_afSteeringWeight[NWI_FOLLOW_PATH]			= 3.0f;
							m_afSteeringWeight[NWI_ARRIVE]				= 1.0f;
							m_afSteeringWeight[NWI_S_OBSTACLE]			= 3.0f;
							m_afSteeringWeight[NWI_AI_OBSTACLE]			= 1.0f;
							m_afSteeringWeight[NWI_FOLLOW_LEADER]		= 1.0f;
							m_afSteeringWeight[NWI_FOLLOW_PATROL_PATH]	= 1.0f;
							m_afSteeringWeight[NWI_CHASE_ENEMY]			= 1.0f;
							m_afSteeringWeight[NWI_FLOCK]				= 2.0f;
							m_afSteeringWeight[NWI_FOLLOW_ENTITY]		= 1.0f;
							m_afSteeringWeight[NWI_TO_COMBAT_POINT]		= 1.0f;
							m_afSteeringWeight[NWI_GO_AROUND_VOLUMES]	= 1.0f;
														
							// !!! - To be completed
						}

	float	m_fMaxSpeed;
	float	m_fWalkSpeed;
	float	m_fChaseSpeed;
	float	m_fPatrolSpeed;
	float	m_fApproachSpeed;
	float	m_fFaceDirectionSpeed;
	float	m_fPanicDistSQR;		// For FleeSolo
	float	m_fAIAvoidanceRadius;	// My repuslion Radius (for AI_AVOIDANCE)
	float	m_fAIAvoidanceRadiusSQR;	// My repuslion Radius (for AI_AVOIDANCE)
	float	m_fNormalAIAvoidanceRadius;	// 
	float	m_fCombatAIAvoidanceRadius; 
	float	m_fFormationAIAvoidanceRadius; 
	float	m_fAIFlockRadius;
	float	m_fResidentEvilRadiusSQR;
	float	m_fAttackRangeSQR;
	float	m_fAttackPointRadius;
	float	m_fAttackPointRadiusSQR;
	float	m_fFormationRadiusSQR;
	bool	m_fFormationMoveClockwise;
	bool	m_bTrimPath;
	float	m_fMaxWaitingTimeAfterPathNotFound;

	//

	float	m_fFollowPathRadiusPercentage;

	// Detection Box

	float	m_fHalfWidthDetectionBox;
	float	m_fSideDetectionLength;
	float	m_fMinObstacleDetectionRadius;
	float	m_fMinWallDetectionRadius;
	float	m_fFollowEntityRadiusSQR;
	float	m_fCharacterRadius;

	bool	m_bGoToLastKnownPlayerPosInAttack;

	unsigned int	m_uiIdleFlags;

	unsigned int	m_uSteeringFlags;
	float			m_afSteeringWeight[NWI_LAST_INDEX];

} SMovParams;

enum E_DESTINATION
{
	DEST_IDLE = 0,		// I'm not going anywhere
	DEST_LEADER,		// I'm following a leader
	DEST_NODE,			// I'm trying to get to a node
	DEST_PLAYER,		// I'm trying to get to the player
	DEST_POINT			// I want to go to a particular point (needed?)
};

typedef struct _SDestination
{
	_SDestination() : eType(DEST_IDLE), pLeader(NULL) {}

	void Set( E_DESTINATION e, CAINavigNode* pN , CEntity* pEnt = NULL ) { eType = e; pNode = pN; pLeader=pEnt; }
	void Set ( _SDestination* pD ) { eType=pD->eType,pNode = pD->pNode,pLeader = pD->pLeader; }
	E_DESTINATION	eType;		// Why am I going there?
	CAINavigNode*	pNode;		// If I'm going to a node, store it
	//CPoint			Position;	// If I go to a particular position, sotre it here (i.e., node or point)
	CEntity*		pLeader;	// But if I am following a leader, point here to him
} SDestination;

typedef struct _SAIMovCompleted
{
	_SAIMovCompleted() : bFollowPathCompleted(true), bChaseTargetCompleted(true), bFollowEntityCompleted(true),
						 bMoveToCombatPointCompleted(true), bMoveToSelectedPointCompleted(true), bGoAroundCompleted(false) {}

	bool bFollowPathCompleted;
	bool bChaseTargetCompleted;
	bool bFollowEntityCompleted;
	bool bMoveToCombatPointCompleted;
	bool bMoveToSelectedPointCompleted;
	bool bGoAroundCompleted;
} SAIMovCompleted;

typedef struct _SAIMovBehaviourParams
{
	_SAIMovBehaviourParams() : fMaxSpeed(0.0f), eFlags(0) {}
	
	void Set(const char* ps, float fMS, unsigned int eF, CEntity* pE) 
	{	if (!ps && !pE) {fMaxSpeed=0.0f; eFlags=0; }
		fMaxSpeed = fMS; eFlags = eF;
		if (pE) pLeader = pE;
		if (ps) ksNodeName = CKeyString(ps);		
	}
	CKeyString		ksNodeName;
	float			fMaxSpeed;
	unsigned int	eFlags;
	CEntity*		pLeader;
} SAIMovBehaviourParams;

typedef struct _SAIIntentions
{
	_SAIIntentions() :	eNavIntentions(0), pEntityToAttack(NULL), 
						pEntityToFollow(NULL), pEntityToGoTo(NULL), /*pCoverNode(NULL),*/ pCoverPoint(NULL),
						obAttackPoint(CONSTRUCT_CLEAR), obDestPos(CONSTRUCT_CLEAR), obStartPos(CONSTRUCT_CLEAR),
						fDestPointRadiusSQR(1.0f), bValidNode(false), bValidStartNode(false),
						bMoveFacingTarget(false), bIdleClearsIntention(true), bIdlePlaysAnims(false),
						m_bUsingObjectInPath(false), m_pobEntityToUseInPath(NULL) {}

	unsigned int	eNavIntentions;
	CHashedString	ksDestNodeName;
	CHashedString	ksStartNodeName;
	CHashedString	ksIntermediateNodesNames;
	const CEntity*	pEntityToAttack;
	const CEntity*	pEntityToFollow;
	const CEntity*	pEntityToGoTo;
//	CAINavigNode*	pCoverNode;
	CAINavigCoverPoint* pCoverPoint;
	CPoint			obAttackPoint;		// Selected Attack point in Hand2Hand combat
	CPoint			obDestPos;
	CPoint			obStartPos;
//	CPoint			obCoverPos;
	float			fDestPointRadiusSQR;
	bool			bValidNode;
	bool			bValidStartNode;
	bool			bMoveFacingTarget;
	bool			bIdleClearsIntention;
	bool			bIdlePlaysAnims;
	bool			m_bUsingObjectInPath;
	CEntity*		m_pobEntityToUseInPath;
} SAIIntentions;

typedef struct _SCoverParams
{
	_SCoverParams () :	fMultiplier(1.0f), pLastCoverPointUsed(NULL), pBookedCoverPoint(NULL), bReuseCoverPoint(false), fCoverTime(4.0f),
						fPeekProbability(0.75f) {}

	float				fMultiplier;
	CAINavigCoverPoint*	pLastCoverPointUsed;
	CAINavigCoverPoint* pBookedCoverPoint;
	bool				bReuseCoverPoint;
	float				fCoverTime;
	float				fPeekProbability;
} SCoverParams;

typedef struct _SShootingParams
{
	_SShootingParams () : uiMaxNumberOfConsecutiveShots(3), fMaxTimeBetweenShots(3.0f), fAccuracy(1.0f), bAlwaysMiss(false), fOffsetRadius(1.5f),
							obShootingPoint(CONSTRUCT_CLEAR), uiCurrentNumOfShots(0), fMaxDistanceToEnemySQR(400.0f), fMinDistanceToEnemySQR(100.0f) {}

	unsigned int	uiMaxNumberOfConsecutiveShots;
	float			fMaxTimeBetweenShots;
	float			fAccuracy;
	bool			bAlwaysMiss;
	float			fOffsetRadius;
	CPoint			obShootingPoint; // DebugRender
	bool			bIsShotAccurate; // DebugRender
	unsigned int	uiCurrentNumOfShots;
	
	float			fMaxDistanceToEnemySQR;
	float			fMinDistanceToEnemySQR;

} SShootingParams;

typedef struct _SCannonUserParams
{
	_SCannonUserParams () : pTargetEntity(NULL), obTargetLocatorPos(CONSTRUCT_CLEAR), bUsingCannon(false), bShootTheCannon(false), bAreCannonBallParamsSet(false), bCannonPointingToTarget(false),
							fCannonBallVo(10.0f), fCannonBallVo_SQR(100.0f), fCannonBallG(9.81f)  {}

	CEntity*			pTargetEntity;			// Use either this (by passing an entity to aim to...
	CPoint				obTargetLocatorPos;		// ... or this (by passing the coordinates of a locator)
	bool				bUsingCannon;
	bool				bShootTheCannon;
	bool				bAreCannonBallParamsSet;
	bool				bCannonPointingToTarget;
	float				fCannonBallVo;
	float				fCannonBallVo_SQR;
	float				fCannonBallG;
} SCannonUserParams;

typedef struct _SLookAtParams
{
	_SLookAtParams () : pEntityToLookAt(NULL) {}

	CEntity*	pEntityToLookAt;

} SLookAtParams;

typedef struct _SLadderQueueData
{
	_SLadderQueueData () : pLadder(NULL), pSingleQueue(NULL), iQueueIndex(-1), bQueueIndexUpdated(true) {}

	CEntity*		pLadder;
	CAISingleQueue* pSingleQueue;
	int				iQueueIndex;
	bool			bQueueIndexUpdated;

} SLadderQueueData;

typedef struct _SWhackAMoleData
{
	_SWhackAMoleData () :	/*obWhackAMoleDir(CONSTRUCT_CLEAR), */ uiTotalCoverCycles(6), uiCurrentCoverCycle(0), pNode(NULL), iVolleyShots(-1), iVolleySquad(0),
							fVolleyReloadPauseMin(3.0f), fVolleyReloadPauseMax(4.0f), fVolleyAimPause(3.0f), fVolleyPauseBetweenShotsMin(1.5f), fVolleyPauseBetweenShotsMax(1.7f),
							fMinHiddingTime(MIN_WHACKAMOLE_HIDDING_TIME), fMaxHiddingTime(MAX_WHACKAMOLE_HIDDING_TIME),
							fAimingMaxTime(5.0f), fAimingMinTime(2.0f), bFiringSquadOn(false), bHidden(false) {}

//	CDirection		obWhackAMoleDir;		// Direction to constantly face in Time-Crisis like moves.
	unsigned int	uiTotalCoverCycles;
	unsigned int	uiCurrentCoverCycle;
	CAINavigNode*	pNode;
	int iVolleyShots;
	int iVolleySquad;
	float fVolleyReloadPauseMin;
	float fVolleyReloadPauseMax;
	float fVolleyAimPause;
	float fVolleyPauseBetweenShotsMin;
	float fVolleyPauseBetweenShotsMax;
	float			fMinHiddingTime;
	float			fMaxHiddingTime;
	float			fAimingMaxTime;
	float			fAimingMinTime;
	bool			bFiringSquadOn;
	bool			bHidden;

} SWhackAMoleData;

typedef struct _SAIAvoidance
{
	_SAIAvoidance () : m_fRemainingBreakTime(-1.0f), m_fBreakSpeed(0.0f), m_fMaxSlowDownTime(AIMOV_SLOWDOWN_DEFAULT_TIME) {}

	float m_fRemainingBreakTime;
	float m_fBreakSpeed;
	float m_fMaxSlowDownTime;
} SAIAvoidance;

typedef struct _SAIDiving
{
	_SAIDiving () : fCoolingTime(2.0f), fTimeSinceLastDive(0.0f), fBoltsConeLength(20.0f), fBoltsConeLengthSQR(400.0f), fBoltsConeHalfRadius(2.0f), 
					fDivingProbability(0.5f), fPanicProbability(0.1f), fAfterTouchPanicProbability(0.5f), fAftertouchDivingProbability(0.7f) {}

	float fCoolingTime;
	float fTimeSinceLastDive;
	float fBoltsConeLength;
	float fBoltsConeLengthSQR;
	float fBoltsConeHalfRadius;
	float fDivingProbability;
	float fPanicProbability;
	float fAfterTouchPanicProbability;
	float fAftertouchDivingProbability;

} SAIDiving;

//! -------------------------------------------
//! CAIMovement
//! -------------------------------------------
class CAIMovement
{
	public:
		
		// Ctor and Enable
		CAIMovement() : m_pEnt(NULL), m_pPlayer(NULL), m_pCAIComp(NULL), m_pPatrolGraph(NULL), 
						m_SpeedMag(0), m_uiActionStyle(0), m_bExternalControlState(false), m_bGoingToCover(false) {}
							
		void			SetParent	( AI*			pEnt)	{ if (pEnt) { m_pEnt		= pEnt; } }
		void			SetPlayer	( Player*		pEnt)	{ if (pEnt) { m_pPlayer		= pEnt; } }
		void			SetLeader	( AI*			pEnt)	{ if (pEnt) { m_SAIMovBehaviourParams.pLeader = pEnt; m_SDestination.Set(DEST_LEADER,NULL, pEnt); } }
		void			SetCAIComp	( CAIComponent* pCAI)	{ if (pCAI) { m_pCAIComp	= pCAI; } }

		CEntity*		GetParent	( void )				{ return m_pEnt;					}
		Player*			GetPlayer	( void )				{ return m_pPlayer;					}
		CEntity*		GetLeader	( void )				{ return m_SAIMovBehaviourParams.pLeader; }//return m_SDestination.pLeader;	}
		CAIComponent*	GetCAIComp	( void )				{ return m_pCAIComp;				}
		CPoint			GetPosition ( void )				{ return m_pEnt->GetPosition();		}
		bool			IsLeader	( void );
				
		// Navigation Parameters and Path
		void			SetSteeringFlags	( unsigned int ui )			{ m_SMovementParams.m_uSteeringFlags = ui; }
		unsigned int	GetSteeringFlags	( void )					{ return (m_SMovementParams.m_uSteeringFlags); }
		unsigned int	GetIdleFlags		( void )					{ return (m_SMovementParams.m_uiIdleFlags); }
		void			SetIdleFlags		( unsigned int mask )		{ m_SMovementParams.m_uiIdleFlags = mask; }
		bool			IsFlagActive		( unsigned int mask ) const	{ return ( m_SMovementParams.m_uSteeringFlags & mask); }
		void			ActivateFlag		( unsigned int mask )		{ m_SMovementParams.m_uSteeringFlags = m_SMovementParams.m_uSteeringFlags | mask; }
		void			DeactivateFlag		( unsigned int mask )		{ if (IsFlagActive(mask)) {m_SMovementParams.m_uSteeringFlags = m_SMovementParams.m_uSteeringFlags ^ mask;} }
		void			DeactivateMotion	( void )					{ m_SMovementParams.m_uSteeringFlags = NF_NO_MOVEMENT; }


		float*			GetSteeringWeights	( void )							{ return m_SMovementParams.m_afSteeringWeight; }
		float			GetSteeringWeight	( ENUM_NAVIGATION_WEIGHT_INDEX e )	{ return m_SMovementParams.m_afSteeringWeight[e]; } // !!! - Needed?
		CAINavigPath*	GetPathContainer	( void )							{ return &m_Path; }

		// Set, Get Movement Params
	
		SMovParams* GetMovementParams		( void ) { return (&m_SMovementParams); }
		void		SetMovementParam		( unsigned int, float);
		void		SetMaxSpeed				( float fN	) { m_SMovementParams.m_fMaxSpeed		= fN > 1.0f ? 1.0f : fN; }
		void		SetFleeDistSQR			( float fN	) { m_SMovementParams.m_fPanicDistSQR	= fN > 0.0f ? fN : 4.0f;} // !!! - To be defined as a constant (4.0f)
		void		SetAIFlockingRadius		( float fN	) { m_SMovementParams.m_fAIFlockRadius = fN > 0.0f ? fN : 0.0f; }
		void		SetAIAvoidanceRadius	( float fN	) { m_SMovementParams.m_fAIAvoidanceRadius=fN; m_SMovementParams.m_fAIAvoidanceRadiusSQR = fN*fN; }
		void		ActivateCombatAIAvoidanceRadius ( bool b ) 
					{ if (b) SetAIAvoidanceRadius(m_SMovementParams.m_fCombatAIAvoidanceRadius);
					  else SetAIAvoidanceRadius(m_SMovementParams.m_fNormalAIAvoidanceRadius);
					}	
		void		ActivateFormationAIAvoidanceRadius ( bool b ) 
					{ if (b) SetAIAvoidanceRadius(m_SMovementParams.m_fFormationAIAvoidanceRadius);
					  else SetAIAvoidanceRadius(m_SMovementParams.m_fNormalAIAvoidanceRadius);
					}	

		void		SetTrimPath				( bool b )	   { m_SMovementParams.m_bTrimPath = b; }
		bool		GetTrimPath				( void ) const { return m_SMovementParams.m_bTrimPath; }
		float		GetMaxSpeed				( void ) const { return m_SMovementParams.m_fMaxSpeed; }
		float		GetWalkSpeed			( void ) const { return m_SMovementParams.m_fWalkSpeed; }
		float		GetPatrolSpeed			( void ) const { return m_SMovementParams.m_fPatrolSpeed; }
		float		GetChaseSpeed			( void ) const { return m_SMovementParams.m_fChaseSpeed; }
		float		GetApproachSpeed		( void ) const { return m_SMovementParams.m_fApproachSpeed; }
		float		GetFleeDistSQR			( void ) const { return m_SMovementParams.m_fPanicDistSQR; }
		float		GetAIAvoidanceRadius	( void ) const { return m_SMovementParams.m_fAIAvoidanceRadius; }
		float		GetAIAvoidanceRadiusSQR	( void ) const { return m_SMovementParams.m_fAIAvoidanceRadiusSQR; }
		float		GetAIFlockingRadius		( void ) const { return m_SMovementParams.m_fAIFlockRadius; }
		float		GetMinObstDetRadius		( void ) const { return m_SMovementParams.m_fMinObstacleDetectionRadius; }
		float		GetMinWallDetRadius		( void ) const { return m_SMovementParams.m_fMinWallDetectionRadius; }
		float		GetMinDynObstDetRadius	( void ) const { return m_SMovementParams.m_fMinObstacleDetectionRadius; }
		float		GetSideDetLength		( void ) const { return m_SMovementParams.m_fSideDetectionLength; }
		float		GetSpeedMagnitude		( void ) const { return m_SpeedMag; }
		float		GetFollowEntityRadiusSQR( void ) const { return m_SMovementParams.m_fFollowEntityRadiusSQR; }
		float		GetHalfDetBoxWidth		( void ) const { return m_SMovementParams.m_fHalfWidthDetectionBox; }
		float		GetResidentEvilRadiusSQR( void ) const { return m_SMovementParams.m_fResidentEvilRadiusSQR; }
		float		GetAttackRangeSQR		( void ) const { return m_SMovementParams.m_fAttackRangeSQR; }
		float		GetAttackPointRadius	( void ) const { return m_SMovementParams.m_fAttackPointRadius; }
		float		GetAttackPointRadiusSQR	( void ) const { return m_SMovementParams.m_fAttackPointRadiusSQR; }
		float		GetFormationRangeSQR	( void ) const { return m_SMovementParams.m_fFormationRadiusSQR; }
		float		GetCombatAIAvoidanceRadius ( void ) const { return m_SMovementParams.m_fCombatAIAvoidanceRadius; }
		CPoint		GetAttackPoint			( void ) const { return m_obAttackPoint; }
		void		SetAttackPoint			( const CPoint& p ) { m_obAttackPoint = p; }
		void		SetResidentEvilRadiusSQR( float f )			{ m_SMovementParams.m_fResidentEvilRadiusSQR = f*f; }
		void		SetAttackRange			( float f )			{ m_SMovementParams.m_fAttackRangeSQR = f*f; }
		void		SetFormationRange		( float f )			{ m_SMovementParams.m_fFormationRadiusSQR = f*f; }
		void		SetFormationRangeSQR	( float f )			{ m_SMovementParams.m_fFormationRadiusSQR = f; }
		void		SetFormationMoveClockwise ( bool b)			{ m_SMovementParams.m_fFormationMoveClockwise = b; }
		bool		IsFormationMoveClockwise( void )			{ return m_SMovementParams.m_fFormationMoveClockwise; }
		bool		IsInFormationMovement	( void )			{ return IsFlagActive(NF_TO_COMBAT_POINT); }
		bool		IsInDynamicCoverMovement( void )			{ return IsFlagActive(NF_FOLLOW_PATH_DYNCOVER); }
		void		SetMinWallDetRadius		( float f ) 		{ if (!(f<0.0f)) m_SMovementParams.m_fMinWallDetectionRadius = f; }
		void		SetFollowPathRadiusPercentage ( float f )	{ m_SMovementParams.m_fFollowPathRadiusPercentage = f > 0.9f ? 0.9f : ( f > 0.0f ? f : 0.0f ); }
		float		GetFollowPathRadiusPercentage ( void ) const{ return m_SMovementParams.m_fFollowPathRadiusPercentage; }
		float		GetMaxWaitingTimeAfterPathNotFound ( void ) const	{ return m_SMovementParams.m_fMaxWaitingTimeAfterPathNotFound; }
		bool		DoIGoToLastKnownPlayerPosInAttack ( void )	const	{ return m_SMovementParams.m_bGoToLastKnownPlayerPosInAttack; }
		void		SetGoToLastKnownPlayerPosInAttack ( bool b )		{ m_SMovementParams.m_bGoToLastKnownPlayerPosInAttack = b; }

		friend class CAISteeringLibrary;

		void			SetDestination		( SDestination* pD );
		void			SetDestination		( E_DESTINATION, CAINavigNode*, CEntity* = NULL );
		void			SetDestinationIdle	( void ) { SetSteeringFlags(GetIdleFlags()); m_SDestination.Set(DEST_IDLE,NULL,NULL); }
		SDestination*	GetDestination		( void ) { return &m_SDestination; }

		void			SetPatrolGraph		( CAIPatrolGraph* pPG) { if (pPG) m_pPatrolGraph = pPG; }
		CAIPatrolGraph* GetPatrolGraph		( void ) { return m_pPatrolGraph; }

		// Movement Completion Queries

		bool				IsChaseTargetCompleted			( void ) const	{ return m_SMovCompleted.bChaseTargetCompleted; }
		bool				IsFollowPathCompleted			( void ) const	{ return m_SMovCompleted.bFollowPathCompleted; }
		bool				IsFollowEntityCompleted			( void ) const	{ return m_SMovCompleted.bFollowEntityCompleted; }
		bool				IsMoveToCombatPointCompleted	( void ) const	{ return m_SMovCompleted.bMoveToCombatPointCompleted; }
		bool				IsMoveToSelectedPointCompleted	( void ) const	{ return m_SMovCompleted.bMoveToSelectedPointCompleted; }
		bool				IsGoAroundCompleted				( void ) const	{ return m_SMovCompleted.bGoAroundCompleted; }
		void				SetChaseTargetCompleted			( bool b )	{ m_SMovCompleted.bChaseTargetCompleted = b; }
		void				SetMoveToSelectedPointCompleted	( bool b )	{ m_SMovCompleted.bMoveToSelectedPointCompleted = b; }
		void				SetMoveToCombatPointCompleted	( bool b )	{ m_SMovCompleted.bMoveToCombatPointCompleted = b; }
		void				SetFollowPathCompleted			( bool b )	{ m_SMovCompleted.bFollowPathCompleted = b; }
		void				SetFollowEntityCompleted		( bool b )	{ m_SMovCompleted.bFollowEntityCompleted = b; }
		void				SetGoAroundCompleted			( bool b )	{ m_SMovCompleted.bGoAroundCompleted = b; }
		
		SAIMovCompleted*	GetSMovCompleted				( void )	{ return (&m_SMovCompleted); }
		const SAIMovCompleted*	GetSMovCompleted			( void ) const { return (&m_SMovCompleted); }

		// Facing Object Methods, cover, and use object in path

		void		SetFacingAction		( const CDirection& obDir ) { m_obFacingActionDir = obDir; }
		CDirection	GetFacingAction		( void )	const	{ return m_obFacingActionDir; }
		void		SetGoingToCover		( bool b )			{ m_bGoingToCover = b; }
		bool		IsMovingToCover		( void )	const	{ return m_bGoingToCover; }
		void		SetUsingObjectInPath( bool b )			{ m_SAIIntentions.m_bUsingObjectInPath = b; }
		bool		IsUsingObjectInPath	( void )	const	{ return m_SAIIntentions.m_bUsingObjectInPath; }
		void		SetEntityToUseInPath( CEntity* pE )		{ m_SAIIntentions.m_pobEntityToUseInPath = pE; }
		CEntity*	GetEntityToUseInPath( void )	const	{ return m_SAIIntentions.m_pobEntityToUseInPath; }
		float		GetCoverTime		( void )	const	{ return m_SCoverParams.fCoverTime; }
		float		GetInCoverPeekChance( void )	const	{ return m_SCoverParams.fPeekProbability; }

		// Intentions
		
			// SetCoverPoint is used to Flags a CP from the moment the AI travel from the linked node to the CP until it breakes cover
		void				SetCoverPoint		( CAINavigCoverPoint* pCP ){ if(pCP) {m_SAIIntentions.pCoverPoint = pCP; m_SCoverParams.pLastCoverPointUsed = pCP; } }
		CAINavigCoverPoint*	GetCoverPoint		( void )			{ return (m_SAIIntentions.pCoverPoint); }

		
		
		void			SetEntityToGoTo				( CEntity* pEnt )	{ if (pEnt) {m_SAIIntentions.pEntityToGoTo = pEnt; m_SAIIntentions.obDestPos = pEnt->GetPosition();  } }//m_SAIIntentions.fDestPointRadiusSQR = m_SMovementParams.m_fFollowEntityRadiusSQR; } }
		void			SetEntityToAttack			( const CEntity* pEnt )	{ if (pEnt) {m_SAIIntentions.pEntityToAttack = pEnt;} }
		void			SetEntityToFollow			( CEntity* pEnt )	{ if (pEnt) {m_SAIIntentions.pEntityToFollow = pEnt;} }
		void			SetUseNodesSet				( CHashedString hs ) { m_SAIIntentions.ksIntermediateNodesNames = hs; }
		void			ClearNodesSet				( void ) { m_SAIIntentions.ksIntermediateNodesNames = CHashedString(" "); }
		CHashedString	GetNodesSetName				( void ) { return (m_SAIIntentions.ksIntermediateNodesNames); }
		bool			SetDestinationNode			( const char* );
		bool			SetStartEndNodes			( const char*, const char* );
		bool			SetStartEndNodes			( CAINavigNode*, CAINavigNode* );
		void			ClearStartNode				( void )			{ m_SAIIntentions.ksStartNodeName = CHashedString(" "); m_SAIIntentions.bValidStartNode = false; }
		void			ClearDestinationNode		( void )			{ m_SAIIntentions.ksDestNodeName = CHashedString(" "); m_SAIIntentions.bValidNode = false; }
		void			SetDestinationRadius		( float f )			{ m_SAIIntentions.fDestPointRadiusSQR = f*f; }
		void			SetDestinationRadiusSQR		( float f )			{ m_SAIIntentions.fDestPointRadiusSQR = f; }
		void			SetDestinationPoint			( const CPoint& obP, float f ) { m_SAIIntentions.obDestPos = obP; m_SAIIntentions.fDestPointRadiusSQR = f*f; }
		void			SetMovingWhileFacingTgt		( bool b  )			{ m_SAIIntentions.bMoveFacingTarget = b; }
		SAIIntentions*	GetIntention				( void )			{ return &m_SAIIntentions; }
		const SAIIntentions*	GetIntention		( void ) const 		{ return &m_SAIIntentions; }
		const CEntity*		GetEntityToGoTo			( void ) const		{ return m_SAIIntentions.pEntityToGoTo; }
		const CEntity*		GetEntityToAttack		( void ) const		{ return m_SAIIntentions.pEntityToAttack; }
		const CEntity*		GetEntityToFollow		( void ) const		{ return m_SAIIntentions.pEntityToFollow; }
		CHashedString	GetStartNodeName		( void ) const		{ return m_SAIIntentions.ksStartNodeName; }
		CHashedString	GetDestinationNodeName	( void ) const		{ return m_SAIIntentions.ksDestNodeName; }
		CPoint			GetDestinationPos		( void ) const		{ return m_SAIIntentions.obDestPos; }
		float			GetDestinationRadiusSQR	( void ) const		{ return m_SAIIntentions.fDestPointRadiusSQR; }
		bool			IsDestinationNodeValid	( void ) const		{ return m_SAIIntentions.bValidNode; }
		bool			IsStartNodeValid		( void ) const		{ return m_SAIIntentions.bValidStartNode; }
		bool			IsMovingWhileFacingTgt	( void ) const		{ return m_SAIIntentions.bMoveFacingTarget; }
		bool			GetIdleClearsIntention	( void ) const		{ return m_SAIIntentions.bIdleClearsIntention; }		
		void			SetIdleClearsIntention	( bool b )			{ m_SAIIntentions.bIdleClearsIntention = b; }
		bool			GetIdlePlaysAnim		( void ) const		{ return m_SAIIntentions.bIdlePlaysAnims; }
		void			SetIdlePlaysAnim		( bool b ) 			{ m_SAIIntentions.bIdlePlaysAnims = b; }

		void			SetIntention			( unsigned int eIntentions, float fSpeed, unsigned int eMovFlag );
		void			SetAIBoolParam			( unsigned int eParam, bool bValue );
		//void			SetIntention			( unsigned int eIntentions, float fSpeed, unsigned int eMovFlag ) 
		//				{ m_SAIIntentions.eNavIntentions = eIntentions; SetMaxSpeed(fSpeed); SetSteeringFlags(eMovFlag); }
		
		// Animation, movement styles and external control

		bool			PlayAnimation	( int  );
		bool			PlayDiveAnimation ( int );
		void			CancelSingleAnim( void );
		bool			IsSimpleActionComplete ( void );
		void			SetActionStyle	( unsigned int );
		unsigned int	GetActionStyle	( void ) const		{ return m_uiActionStyle; }
		void			SetExternalControlState ( bool b )	{ m_bExternalControlState = b; }
		bool			GetExternalControlState ( void )	{ return m_bExternalControlState; }

		// Cover Points

		SCoverParams*	GetCoverPointParams			( void )		{ return &m_SCoverParams; }
		float			GetCoverAttitudeMultiplier	( void ) const	{ return m_SCoverParams.fMultiplier; }
		bool			AllowToReuseCoverPoints		( void ) const	{ return m_SCoverParams.bReuseCoverPoint; }
		void			SetReuseCoverPoints			( bool b )		{ m_SCoverParams.bReuseCoverPoint=b; }
		bool			IsDifferentFromPreviousCoverPoint ( CAINavigCoverPoint* pCP ) const { return ( pCP && (pCP != m_SCoverParams.pLastCoverPointUsed) ); }
		void			SetCoverAttitude			( unsigned int ); 
		
		CAINavigCoverPoint * GetClaimedCoverPoint	( void ) const	{ return m_SCoverParams.pBookedCoverPoint; }
		void			SetClaimCoverPoint			( CAINavigCoverPoint * );

		// Shooting && Min-Max

		void			SetShootingPoint	( const CPoint& p )	{ m_SShootingParams.obShootingPoint = p; }		// For DebugRender
		CPoint			GetShootingPoint	( void )	const	{ return (m_SShootingParams.obShootingPoint); } // For DebugRender
		void			SetIsAccurateShot	( bool b )			{ m_SShootingParams.bIsShotAccurate = b; }		// For DebugRender
		bool			GetIsAccurateShot	( void )			{ return m_SShootingParams.bIsShotAccurate; }	// For DebugRender
		void			SetOffsetRadius		( float f )			{ m_SShootingParams.fOffsetRadius = f<0.0f ? 0.0f : f; }
		float			GetOffsetRadius		( void )	const	{ return m_SShootingParams.fOffsetRadius; }
		void			SetShootingAccuracy	( float f )			{ m_SShootingParams.fAccuracy = (f < 0.0f) ? 0.0f : (( f > 1.0f ) ? 1.0f : f); }
	//	bool			GetShootingAccuracy	( void )	const	{ return (m_SShootingParams.fAccuracy); }
		void			SetAlwaysMiss		( bool b )			{ m_SShootingParams.bAlwaysMiss = b; }
		bool			GetAlwaysMiss		( void )	const	{ return (m_SShootingParams.bAlwaysMiss); }
		unsigned int	IncrementShotsCount	( void )			{ return (m_SShootingParams.uiCurrentNumOfShots++); }
		void			ClearShotsCount		( void )			{ m_SShootingParams.uiCurrentNumOfShots=0; }
		unsigned int	GetShotsCount		( void )	const	{ return m_SShootingParams.uiCurrentNumOfShots; }
		bool			EnoughShooting		( void )			{ return (IncrementShotsCount() > GetNumberOfConsecShoots()); }
		void			SetNumberOfConsecShots	( unsigned int u ) { m_SShootingParams.uiMaxNumberOfConsecutiveShots = u; }

		unsigned int	GetNumberOfConsecShoots	( bool bRand = true ) const 
						{ if (!bRand) return m_SShootingParams.uiMaxNumberOfConsecutiveShots; 
							else return (grand() % m_SShootingParams.uiMaxNumberOfConsecutiveShots); }
		float			GetShootingAccuracy		( bool bRand = true ) const 
						{ if (!bRand) return m_SShootingParams.fAccuracy; 
							else return (grandf(m_SShootingParams.fAccuracy)); }
		void			SetTimeBetweenShoots	( float f ) { m_SShootingParams.fMaxTimeBetweenShots = f < MIN_TIME_BETWEEN_SHOTS ? MIN_TIME_BETWEEN_SHOTS : f; }
		float			GetTimeBetweenShoots	( bool = true ) const;
		bool			IsPointWithinMinMaxRange( const CPoint& ) const;
		bool			IsPointCloserThanMinRange (const CPoint& ) const;
		float			GetRangeMaxDistSQR		( void )	const	{ return m_SShootingParams.fMaxDistanceToEnemySQR; }
		float			GetRangeMinDistSQR		( void )	const	{ return m_SShootingParams.fMinDistanceToEnemySQR; }
		void			SetMinMaxRadii			( float fMin, float fMax )	{ m_SShootingParams.fMinDistanceToEnemySQR = fMin>0.0f ? fMin*fMin : 0.0f;
																			  m_SShootingParams.fMaxDistanceToEnemySQR = fMax>0.0f ? fMax*fMax : 0.0f; }
		float			GetRangedParameter		( unsigned int  );

		unsigned int	GetTotalCoverCycles		( void ) const		{ return m_SWhackAMoleData.uiTotalCoverCycles; }
		unsigned int	GetCurrentCoverCycle	( void ) const		{ return m_SWhackAMoleData.uiCurrentCoverCycle; }
		void			SetTotalCoverCycles		( unsigned int u )	{ m_SWhackAMoleData.uiTotalCoverCycles = u; }
		void			SetCurrentCoverCycle	( unsigned int u )	{ m_SWhackAMoleData.uiCurrentCoverCycle = u; }
		void			IncCurrentCoverCycle	( void )			{ m_SWhackAMoleData.uiCurrentCoverCycle++; }
		void			ClearCurrentCoverCycle	( void )			{ m_SWhackAMoleData.uiCurrentCoverCycle = 0; }
		bool			AreCoverCyclesLeft		( void ) const		{ return (GetCurrentCoverCycle() < GetTotalCoverCycles()); }

		void SetVolleyShots(int iVolleyShots) { m_SWhackAMoleData.iVolleyShots = iVolleyShots; if (iVolleyShots == -1) CChatterBoxMan::Get().Trigger("AI_XBow_Ready", CAIWhackAMoleBehaviour::GetVolleyCommander(m_SWhackAMoleData.iVolleySquad)); }
		void SetVolleySquad(int iSquad) { m_SWhackAMoleData.iVolleySquad = iSquad; }
		void SetVolleyReloadPause(float fPauseMin, float fPauseMax) { m_SWhackAMoleData.fVolleyReloadPauseMin = fPauseMin; m_SWhackAMoleData.fVolleyReloadPauseMax = fPauseMax; }
		void SetVolleyAimPause(float fPause) { m_SWhackAMoleData.fVolleyAimPause = fPause; }
		void SetVolleyPauseBetweenShots(float fPauseMin, float fPauseMax) { m_SWhackAMoleData.fVolleyPauseBetweenShotsMin = fPauseMin; m_SWhackAMoleData.fVolleyPauseBetweenShotsMax = fPauseMax; }


		int GetVolleyShots() const { return m_SWhackAMoleData.iVolleyShots; }
		int GetVolleySquad() const { return m_SWhackAMoleData.iVolleySquad; }
		float GetVolleyReloadPauseMin() const { return m_SWhackAMoleData.fVolleyReloadPauseMin; }
		float GetVolleyReloadPauseMax() const { return m_SWhackAMoleData.fVolleyReloadPauseMax; }
		float GetVolleyAimPause() const { return m_SWhackAMoleData.fVolleyAimPause; }
		float GetVolleyPauseBetweenShotsMin() const { return m_SWhackAMoleData.fVolleyPauseBetweenShotsMin; }
		float GetVolleyPauseBetweenShotsMax() const { return m_SWhackAMoleData.fVolleyPauseBetweenShotsMax; }

		void			SetWhackAMoleNode		( CHashedString );
		CAINavigNode*	GetWhackAMoleNode		( void ) const		{ return m_SWhackAMoleData.pNode; }
		//void			SetWhackAMoleDir		( const CDirection & obDir )	{ m_SWhackAMoleData.obWhackAMoleDir = obDir; }
		//CDirection	GetWhackAMoleDir		( void ) const					{ return m_SWhackAMoleData.obWhackAMoleDir; }
		void			GetMinMaxHiddingTime	( float* fMin, float* fMax ) 	{ *fMin = m_SWhackAMoleData.fMinHiddingTime; *fMax = m_SWhackAMoleData.fMaxHiddingTime; }
		void			SetRangedParam			( unsigned int uiParam, float fValue);
		void			GetMinMaxWMAimingTime	( float* fMin, float* fMax ) 	{ *fMin = m_SWhackAMoleData.fAimingMinTime; *fMax = m_SWhackAMoleData.fAimingMaxTime; }
		void			SetFiringSquadOn		( bool bValue )		{ m_SWhackAMoleData.bFiringSquadOn = bValue; }
		bool			GetFiringSquadOn		( void ) const		{ return (m_SWhackAMoleData.bFiringSquadOn); }
		void			SetWMIsHidden			( bool bValue )		{ m_SWhackAMoleData.bHidden = bValue; }
		bool			GetWMIsHidden			( void ) const		{ return (m_SWhackAMoleData.bHidden); }

		// Look At Params

		void			SetEntityToLookAt	( CEntity* pE ) { m_SSLookAtParams.pEntityToLookAt = pE; }
		CEntity*		GetEntityToLookAt	( void )		{ return m_SSLookAtParams.pEntityToLookAt; }

		// Ladder Queuing Data

		void			SetObjectToQueue( CEntity* pE	)	{ m_SLadderQueueData.pLadder = pE; }
		CEntity*		GetObjectToQueue( void			)	{ return (m_SLadderQueueData.pLadder); }
		void			SetSingleQueue	( CAISingleQueue* pSQ )	{ m_SLadderQueueData.pSingleQueue = pSQ; }
		CAISingleQueue*	GetSingleQueue	( void			)		{ return (m_SLadderQueueData.pSingleQueue); }
		void			SetQueueIndex	( int i			)		{ m_SLadderQueueData.iQueueIndex = i; }
		int				GetQueueIndex	( void			) const	{ return  m_SLadderQueueData.iQueueIndex; }
		bool			IsQueuingNeeded ( void			) const	{ return (m_SLadderQueueData.iQueueIndex>=0); }
		void			SetQueueIndexUpdated ( bool b	) { m_SLadderQueueData.bQueueIndexUpdated = b; }
		bool			IsQueueIndexUpdated ( void		) { if (m_SLadderQueueData.bQueueIndexUpdated) { m_SLadderQueueData.bQueueIndexUpdated = false; return true;  }
															 else return false; }
		// Cannon Stuff

		bool			IsUsingCannon		( void			) const	{ return m_SCannonUserParams.bUsingCannon; }
		void			SetUsingCannon		( bool b		)		{  m_SCannonUserParams.bUsingCannon = b; }
		void			SetCannonBallParams ( float fV0, float fG ) { m_SCannonUserParams.fCannonBallVo = fV0; m_SCannonUserParams.fCannonBallVo_SQR = fV0*fV0; m_SCannonUserParams.fCannonBallG = fG; m_SCannonUserParams.bAreCannonBallParamsSet = true; }
		bool			GetCannonBallParams ( float* fV0,float* fG ){ *fV0 = m_SCannonUserParams.fCannonBallVo; *fG = m_SCannonUserParams.fCannonBallG; return m_SCannonUserParams.bAreCannonBallParamsSet; }
		float			GetCannonBallV0		( void )		  const	{ return m_SCannonUserParams.fCannonBallVo;}
		float			GetCannonBallV0_SQR	( void )		  const	{ return m_SCannonUserParams.fCannonBallVo_SQR;}
		float			GetCannonBallG		( void )		  const	{ return m_SCannonUserParams.fCannonBallG;}
		void			SetCannonTarget		( CEntity* pEnt	)		{ m_SCannonUserParams.pTargetEntity = pEnt; }
		const CEntity*	GetCannonTarget		( void	)		  const	{ return m_SCannonUserParams.pTargetEntity; }
		CPoint			GetCannonTargetLocatorPos	( void )		const { return m_SCannonUserParams.obTargetLocatorPos; }
		void			SetCannonTargetLocatorPos	( const CPoint & obPos ) { m_SCannonUserParams.obTargetLocatorPos = obPos; }
		void			ShootTheCannon		( void )				{ m_SCannonUserParams.bShootTheCannon = true; }
		bool			HasShootCannonRequest	( void )			{ return m_SCannonUserParams.bShootTheCannon; }
		void			ClearShootCannonRequest	( void )			{ m_SCannonUserParams.bShootTheCannon = false; }
		bool			IsCannonFacingTarget	( void			) const	{ return m_SCannonUserParams.bCannonPointingToTarget; }
		void			SetCannonFacingTarget	( bool b		)		{  m_SCannonUserParams.bCannonPointingToTarget = b; }

		// AI Avoidance

		void			SetSLowDownTime				( float f )		{ m_SAIAvoidance.m_fMaxSlowDownTime = f > 0.75f ? f : 0.75f; }
		bool			IsSlowingDown				( void ) const	{ return (m_SAIAvoidance.m_fRemainingBreakTime>0.0f); }
		float			GetRemainingSlowDownTime	( void ) const	{ return m_SAIAvoidance.m_fRemainingBreakTime; }
		void			SetRemainingSlowDownTime	( float f )		{ m_SAIAvoidance.m_fRemainingBreakTime = f > 0.0f ? f : 0.0f; }
		void			ResetRemainingSlowDownTime	( void )		{ m_SAIAvoidance.m_fRemainingBreakTime = m_SAIAvoidance.m_fMaxSlowDownTime; }
		void			DecRemainingSlowDownTime	( float f )		{ m_SAIAvoidance.m_fRemainingBreakTime = f > 0.0f ? (m_SAIAvoidance.m_fRemainingBreakTime-f) : m_SAIAvoidance.m_fRemainingBreakTime; }
		void			ClearRemainingSlowDownTime	( void )		{ m_SAIAvoidance.m_fRemainingBreakTime = 0.0f; }
		void			SetBreakSpeed				( float f )		{ m_SAIAvoidance.m_fBreakSpeed = f > 0.0f ? f : 0.0f; }
		float			GetBreakSpeed				( void ) const	{ return m_SAIAvoidance.m_fBreakSpeed; }

		// AI Diving

		float			GetDivingFreeTime			( void ) const	{ return m_SAIDiving.fCoolingTime; }
		void			UpdateTimeSinceLastDive		( float fTimeChange )		{ m_SAIDiving.fTimeSinceLastDive = m_SAIDiving.fTimeSinceLastDive > m_SAIDiving.fCoolingTime ? m_SAIDiving.fTimeSinceLastDive : m_SAIDiving.fTimeSinceLastDive+fTimeChange; }
		bool			IsInTimeForDiving			( void ) const	{ return (m_SAIDiving.fTimeSinceLastDive>m_SAIDiving.fCoolingTime); }
		void			ResetTimeSinceLastDive		( void )		{ m_SAIDiving.fTimeSinceLastDive = 0.0f; }
		float			GetTimeSinceLastDive		( void ) const	{ return m_SAIDiving.fTimeSinceLastDive; }
		float			GetBoltConeLength			( void ) const	{ return m_SAIDiving.fBoltsConeLength; }
		float			GetBoltConeLengthSQR		( void ) const	{ return m_SAIDiving.fBoltsConeLengthSQR; }
		float			GetBoltConeHalfRadius		( void ) const	{ return m_SAIDiving.fBoltsConeHalfRadius; }
		float			GetDivingProbability		( void ) const	{ return m_SAIDiving.fDivingProbability; }
		float			GetDivingPanicProbability	( void ) const	{ return m_SAIDiving.fPanicProbability; }
		float			GetDivingAftertouchProbability		( void ) const	{ return m_SAIDiving.fAftertouchDivingProbability; }
		float			GetDivingAftertouchPanicProbability ( void ) const	{ return m_SAIDiving.fAfterTouchPanicProbability; }

		// Update & Debug
		void Update		( float );
		void DebugRender( void );

		// Static Info

	private:

		AI*					m_pEnt;			// AI Bot
		Player*				m_pPlayer;		// Player
		CAIComponent*		m_pCAIComp;		// Pointer to my parent CAIComponent
		CAINavigPath		m_Path;			// Navigation Path
		CAIPatrolGraph*		m_pPatrolGraph;	

	
		CDirection			m_Speed;		// Last Calculated Speed !!! - Needed?
		float				m_SpeedMag;		// Last Calculated Speed Magnitude

		SMovParams			m_SMovementParams; // Navigation Parameters and Tunning
		SDestination		m_SDestination;	// Where am I going to and why
		SAIMovCompleted		m_SMovCompleted;
		
		SAIMovBehaviourParams	m_SAIMovBehaviourParams;
		CDirection			m_obFacingActionDir;
		SAIIntentions		m_SAIIntentions;
		SCoverParams		m_SCoverParams;
		SShootingParams		m_SShootingParams;
		SLookAtParams		m_SSLookAtParams;
		SLadderQueueData	m_SLadderQueueData;
		SWhackAMoleData		m_SWhackAMoleData;
		SCannonUserParams	m_SCannonUserParams;
		SAIAvoidance		m_SAIAvoidance;
		SAIDiving			m_SAIDiving;

		CPoint				m_obAttackPoint;

		unsigned int		m_uiActionStyle;

		bool				m_bExternalControlState;
		bool				m_bGoingToCover;
};

#endif // _AIMOVEMENT_H



