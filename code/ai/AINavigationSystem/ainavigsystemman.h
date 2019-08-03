//! -------------------------------------------
//! ainavigsystemman.h
//!
//! AI Navigation System Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//! Date	04/05/06 - Creation
//!--------------------------------------------


#ifndef _AINAVIGSYSTEMMAN_H
#define _AINAVIGSYSTEMMAN_H

#include "ainaviggraphmanager.h"
#include "aisteeringlibrary.h"
#include "editable/enums_ai.h"
#include "game/aicomponent.h"
#include "aileaderman.h"
#include "aiworld.h"
#include "lua/ninjalua.h"
#include "aidiving.h"


#define DEBUG_PRINT_NAVIGSYSTEMMAN(condition_msg) if (g_ShellOptions->m_bBehaviourDebug) { Debug::Printf("AINSM: "); Debug::Printf condition_msg; }

// Forward declaration of classes

class CAIMovement;
class Object_Projectile;

//! -------------------------------------------
//! MovingVolley
//! -------------------------------------------
enum E_MOVING_VOLLEY_STATUS
{
	VOLLEY_FREE = 0,
	VOLLEY_READY,
	VOLLEY_AIM,
	VOLLEY_FIRE
};

struct SMovingVolleyAIData
{
	SMovingVolleyAIData() : pAI(NULL), eVolleyStatus(VOLLEY_FREE), bCommander(false),
							bInVolley(false), bFinishedVolley(false) {}
	SMovingVolleyAIData(CEntity* pEnt) : pAI(pEnt), eVolleyStatus(VOLLEY_FREE), bCommander(false),
							bInVolley(false), bFinishedVolley(false) {}

	CEntity*				pAI;
	E_MOVING_VOLLEY_STATUS	eVolleyStatus;
	bool					bCommander;
	bool					bInVolley;
	bool					bFinishedVolley;
};

struct SMovingVolley
{
	SMovingVolley() : pCommander(NULL), iVolleyShots(-1), fPauseBetweenVolleyShots(2.0f), eVolleyStatus(VOLLEY_FREE) {}
	
    ntstd::Vector<SMovingVolleyAIData,Mem::MC_AI>	vectorVolleyAIs;
	CEntity*							pCommander;
	int									iVolleyShots;
	float								fPauseBetweenVolleyShots;
	E_MOVING_VOLLEY_STATUS				eVolleyStatus;
};

//! -------------------------------------------
//! CAINavigationSystemMan
//! -------------------------------------------
class CAINavigationSystemMan : public Singleton<CAINavigationSystemMan>
{

	public:
		
		// Ctor, dtor...
		CAINavigationSystemMan();
		~CAINavigationSystemMan()	{ LevelUnload(); }

		HAS_LUA_INTERFACE()

		// Lua Binding Commands

		void GenerateAIBotList			( void			) { m_obSteeringLibrary.GenerateAIBotList(); }
		void GenerateDynObstacleList	( void			) { m_obSteeringLibrary.GenerateDynObstacleList(); }
		void RemoveEntity				( AI* pE	) { m_obLeaderMan.RemoveLeader(pE); m_obSteeringLibrary.RemoveEntity(pE); }		// Removes an obstacle from the list
		void DisableMovement			( AI* pEnt ) { pEnt->GetAIComponent()->GetCAIMovement()->SetSteeringFlags(NF_NO_MOVEMENT); } //pEnt->GetAIComponent()->GetCAIMovement()->SetDestinationIdle(); }	// Disables the movement flags of an entity

		void FollowEntity				( AI* pEnt, unsigned int flag = NF_DEF_FOLLOW_ENEMY );
		void ChaseMovingEntity			( AI* pEnt, unsigned int flag = NF_DEF_CHASE_MOVING_ENTITY);
		void SteerToEntity				( AI*, unsigned int = NF_DEFAULT );					// Moves (by steering) towards an Entity
		bool StrafeToCombatPoint		( AI*, const CEntity*, float, const CPoint&, unsigned int = NF_DEF_STRAFE_ENEMY);					// Moves (by steering) towards an Entity
		bool StrafeToFormationPoint		( AI*, const CEntity*, float, const CPoint&, uint32_t& ruiCtrlID, unsigned int = NF_DEF_STRAFE_ENEMY);					// Moves (by steering) towards an Entity
		bool SteerToLocatorNode			( AI*, unsigned int = NF_DEF_STEER_TO_LOCATOR_NODE);					// Moves (by steering) towards an Entity
		bool SteerToDestinationPoint	( AI*, const CPoint&, float fRadius = 0.5, bool = false, unsigned int = NF_DEF_STEER_TO_DEST_POINT );

		void RemoveLeader				( AI* pL	) { m_obLeaderMan.RemoveLeader(pL); }
		
		void AddPatroller				( AI* pP, const char* psName) { m_obNavigGraphManager.AddPatroller(pP,psName); }
		void RemovePatroller			( AI* pP, const char* psName) { m_obNavigGraphManager.AddPatroller(pP,psName); }
		void FollowPatrolPath			( AI*,	float = 1.0f, unsigned int = NF_DEFAULT ); 	// Walks a Patrol Path
		
		bool FindPatrolPath				(AI* pEnt, float fMaxSpeed, bool * bSuccess );
		
		CAINavigGraph*					GetPatrollersGraph	( CEntity* pEnt) { return (m_obNavigGraphManager.GetPatrollersGraph(pEnt)); }

		CAINavigNode*					GetFirstVisibleNode		( const CPoint & obP, CAINavigGraph* pNG = NULL ) { return (m_obNavigGraphManager.GetFirstVisibleNode(obP,pNG)); }
		CAINavigNode*					GetAbsoluteClosestNode	( const CPoint & obP, CAINavigGraph* pNG = NULL ) { return (m_obNavigGraphManager.GetAbsoluteClosestNode(obP,pNG)); }
		CAINavigNode*					GetNodeByName			( CHashedString hsNodeName ) { return (m_obNavigGraphManager.GetNodeWithName(hsNodeName)); }

		bool							DestroyCoverPoint	( CAINavigCoverPoint* pCP ) { if (pCP) { pCP->Destroy(); return true; } else return false; }

		bool IsNewAIActive				( void   ) const { return m_bNewNavigationSystem; }

		unsigned int					GetNavigationIntentions	( CEntity* pEnt ); 

		// High Level AI Navigation Commands
        
		bool FollowPath			( AI*, bool*, unsigned int = NF_DEFAULT );
		bool FollowPathTo		( AI*, const CEntity*, bool*, float = 1.0f, unsigned int = NF_DEFAULT );	// Moves towards an entity, using the NavigGraph
		bool FollowPathTo		( AI*, const CPoint&,	float = 1.0f, unsigned int = NF_DEFAULT );	// Moves towards a point, using the NavigGraph
		bool FollowPathTo		( AI*, CAINavigNode*,	float = 1.0f, unsigned int = NF_DEFAULT );  // Moves towards a node, using the NavigGraph
		bool FollowPathTo		( AI*, const char*,	float = 1.0f, unsigned int = NF_DEFAULT );  // Moves towards a node (specified by name), using the NavigGraph
		bool FollowPathToReporter ( AI*, AI*, float = 1.0f, unsigned int = NF_DEFAULT ); 
		
		bool FollowPathToCoverPointInMinMaxRange ( AI*, bool, bool*, float = 1.0f, bool = false, unsigned int = NF_DEFAULT );
		bool GoAroundVolume ( AI* , const CEntity*, bool*, unsigned int = NF_DEFAULT );
		bool SetAIVolumeCentre ( CAIWorldVolume* pCV, const CPoint& obPos ) { return (m_obAIWorldMan.SetCentre(pCV, obPos)); } 
		CAIWorldVolume*			GetVolumeByName	( const char* pcName ) { return m_obAIWorldMan.GetVolumeByName(pcName);}
		bool RemoveAIVolume	( const char* pcName )
		{
			CAIWorldVolume* pWV = GetVolumeByName(pcName);
			if (!pWV)
				return false;
			m_obAIWorldMan.RemoveVolume(pWV);
			return true;
		}

//		CAINavigNode*	GetFirstVisibleNode	( const CPoint & obPos, CAINavigGraph* = NULL  ) { return (m_obNavigGraphManager.GetFirstVisibleNode(pEnt)); }

		bool IsLeader			( AI* pEnt ) { return m_obLeaderMan.IsLeader(pEnt); }

		// The calculated perpendicular is:
		//			Perpendicular
		//           |
		//           |
		//  A|---------------->B

		CDirection GetPerpendicular	(const CDirection& obDir) { return CDirection(obDir.Z(),obDir.Y(),-obDir.X()); }

		// Steering Action Calculation

		CDirection CalculateSteeringAction	( CAIMovement*, float );
	
		const static unsigned int NF_DEF_IDLE_FLOCK				= NF_D_OBSTACLE | NF_S_OBSTACLE | NF_FLOCK;
		const static unsigned int NF_DEF_IDLE					= NF_D_OBSTACLE | NF_S_OBSTACLE | NF_AI_OBSTACLE;
		const static unsigned int NF_DEF_IDLE_STOP				= NF_NO_MOVEMENT;
		const static unsigned int NF_DEF_FOLLOW_PATH			= NF_FOLLOW_PATH | NF_D_OBSTACLE | NF_S_OBSTACLE | NF_FLOCK; //NF_AI_OBSTACLE;
		const static unsigned int NF_DEF_FOLLOW_ENTITY			= NF_DEF_IDLE_FLOCK | NF_FOLLOW_ENTITY;
		const static unsigned int NF_DEF_PATROL_WALK			= NF_FOLLOW_PATROL_PATH | NF_S_OBSTACLE | NF_D_OBSTACLE | NF_AI_OBSTACLE;
		const static unsigned int NF_DEF_CHASE_ENEMY			= NF_DEF_IDLE | NF_CHASE_ENEMY;
		const static unsigned int NF_DEF_STRAFE_ENEMY			= NF_DEF_IDLE | NF_TO_COMBAT_POINT;
		const static unsigned int NF_DEF_STEER_TO_LOCATOR_NODE	= NF_DEF_IDLE_FLOCK | NF_ARRIVE_AT_POINT;
		const static unsigned int NF_DEF_STEER_TO_ENTITY		= NF_ARRIVE_AT_POINT; //NF_DEF_IDLE_FLOCK | NF_ARRIVE_AT_POINT;
		const static unsigned int NF_DEF_STEER_TO_DEST_POINT	= NF_ARRIVE_AT_POINT;
		const static unsigned int NF_DEF_FOLLOW_ENEMY			= NF_FOLLOW_ENTITY; //NF_DEF_IDLE_FLOCK | NF_FOLLOW_ENTITY | NF_FLEE;
		const static unsigned int NF_DEF_CHASE_MOVING_ENTITY	= NF_DEF_IDLE | NF_CHASE_MOVING_ENTITY;
		const static unsigned int NF_DEF_FOLLOW_PATH_COVER		= NF_FOLLOW_PATH_COVER | NF_S_OBSTACLE; //| NF_S_OBSTACLE;// | NF_AI_OBSTACLE;// | NF_S_OBSTACLE;
		const static unsigned int NF_DEF_FOLLOW_PATH_DYN_COVER	= NF_FOLLOW_PATH_DYNCOVER; // | NF_S_OBSTACLE;// | NF_AI_OBSTACLE;// | NF_S_OBSTACLE;
		const static unsigned int NF_DEF_GO_AROUND_VOLUMES		= NF_GO_AROUND_VOLUMES | NF_S_OBSTACLE | NF_AI_OBSTACLE;
		
		

		// NavigGraphManager Accessors

		CAINavigGraphManager*	GetNavigGraphManager	( void )	{ return (&m_obNavigGraphManager); }
		CAISteeringLibrary*		GetSteeringLibrary		( void )	{ return (&m_obSteeringLibrary); }
		CAIWorldMan*			GetAIWorldManager		( void )	{ return (&m_obAIWorldMan); }

		// Wall and dynamic obstacles detection primitives
		
		bool	HasLineOfSightThroughGoAroundVolumes	( const CPoint& obFrom, const CPoint& obTo )
				{ return (m_obAIWorldMan.HasLineOfSightThroughGoAroundVolumes(obFrom, obTo)); }

		bool	HasLineOfSightExcludingGoAroundVolumes	( const CPoint& obFrom, const CPoint& obTo )
				{ return (m_obAIWorldMan.HasLineOfSightExcludingGoAroundVolumes(obFrom, obTo)); }
		bool	HasLineOfSight	( const CPoint& obFrom, const CPoint& obTo, float fThreshold = 0.0f )
				{ return (m_obAIWorldMan.HasLineOfSight(obFrom, obTo, fThreshold)); }
		bool	HasShootingLineOfSight	( const CPoint& obFrom, const CPoint& obTo, float fThreshold = 0.0f )
				{ return (m_obAIWorldMan.HasShootingLineOfSight(obFrom, obTo, fThreshold)); }
		bool	IntersectsAIVolume	( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection * pNormal )
				{ return (m_obAIWorldMan.IntersectsAIVolume(obFrom, obTo, pPoint, pNormal )); }

		const CAIWorldVolume*	IntersectsVaultingVolume( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection * pNormal ) const
				{ return (m_obAIWorldMan.IntersectsVaultingVolume(obFrom, obTo, pPoint, pNormal )); }

		const CAIWorldVolume*	CloseToVaultingVolume( const CPoint& obFrom, float fRadius ) const
		{ return (m_obAIWorldMan.CloseToVaultingVolume(obFrom, fRadius )); }

		bool	IsThereObstaclesAtPos	( const CPoint& obPos, float fRadiusSQR ) { return (m_obSteeringLibrary.IsThereObstaclesAtPos(obPos,fRadiusSQR)); }
		bool	IsPosValidForFormation	( const CPoint& obOrigin, const CPoint& obTestPos, float fThreshold = 1.0f ) 
				{ return ( (fabsf(obOrigin.Y() - obTestPos.Y())<0.3f) && !IsThereObstaclesAtPos(obTestPos,fThreshold) && HasLineOfSight(obOrigin,obTestPos,fThreshold)); }
	
		bool	IsPosValidForFormation_OPF ( const CPoint& obOrigin, const CPoint& obTestPos, bool *, float fThreshold = 1.0f ); 

	//	void	UpdateAIList ( void ) { CEntityManager::Get().FindEntitiesByType(m_obSteeringLibrary.m_obAIBotList, CEntity::EntType_AI); }
		
		QueryResultsContainerType* GetAIList ( void ) { return m_pAIList; } 

		void Update (float);

		// Kai-related cover counters and accessors

		void	SetPostArrowHitRadius		( float f )		{ m_fPostArrowHitRadiusSQR = f*f; }
		void	SetPostArrowHitCoverTime	( float f )		{ m_fPostArrowHitCoverTime = (f>0.0f) ? f : 0.0f; }
		bool	IsValidCoverTime			( void ) const	{ return (m_fPostArrowHitCoverTime > m_fTimeWithoutArrowHits); }
		void	ResetBoltCoverTimer			( void )		{ m_fTimeWithoutArrowHits = 0.0f; }
		float	GetBoltCoverTimer			( void ) const	{ return m_fPostArrowHitCoverTime; }
		float	GetTimeFromLastValidBolt	( void ) const	{ return m_fTimeWithoutArrowHits; }
		void	TriggerKaiHitAI				( CEntity* );

		void	SetNavigGraphActiveLUA		( CHashedString hsName, bool bOn ) { m_obNavigGraphManager.SetNavigGraphActiveLUA(hsName,bOn); }

		// Diving

		ENUM_DIVE_FULL_DIRECTION	GetDivingAction ( CEntity*, bool* );
		CEntity*	TestDive_AICollision		( CEntity* pEnt, ENUM_DIVE_FULL_DIRECTION eDiveDir ) { return (m_obAIWorldMan.TestDive_AICollision(pEnt, eDiveDir)); }
		bool		TestDive_NoWallCollision	( CEntity* pEnt, ENUM_DIVE_FULL_DIRECTION eDiveDir ) { return (m_obAIWorldMan.TestDive_NoWallCollision(pEnt, eDiveDir)); }
		void		RemoveBoltFromDivingList	( Object_Projectile* pBolt ) { m_obAIDivingMan.RemoveBolt(pBolt); }
		void		AddBolt						( Object_Projectile* pBolt ) { m_obAIDivingMan.AddBolt(pBolt); }
		void		SetDebugDiveCone			( float, float );

		// Moving Volley
		
		void		MovingVolley_AddAI			( CEntity* pEnt );
		void		MovingVolley_RemoveAI		( CEntity* pEnt );
		void		MovingVolley_SetVolleyStatus( E_MOVING_VOLLEY_STATUS e )		{ m_sMovingVolley.eVolleyStatus = e; }
		void		MovingVolley_SetVolleyShots	( int i )				{ m_sMovingVolley.iVolleyShots = i; }
		void		MovingVolley_SetCommander	( CEntity* pEnt )		{ m_sMovingVolley.pCommander = pEnt; }
		CEntity*	MovingVolley_GetCommander	( void ) const			{ return m_sMovingVolley.pCommander; }
		int			MovingVolley_GetVolleyShots ( void ) const			{ return m_sMovingVolley.iVolleyShots; }
		int			MovingVolley_FireAndDecreaseShots ( void )			{ return (--m_sMovingVolley.iVolleyShots); }
		float		MovingVolley_GetVolleyPauseBetweenShots ( void ) const	{ return m_sMovingVolley.fPauseBetweenVolleyShots; }
		E_MOVING_VOLLEY_STATUS MovingVolley_GetVolleyStatus ( void ) const	{ return m_sMovingVolley.eVolleyStatus; }
		void		MovingVolley_SetVolleyStatus( CEntity* pEnt, E_MOVING_VOLLEY_STATUS e );
		void		MovingVolley_StartVolley	( void )				{ if (m_sMovingVolley.eVolleyStatus == VOLLEY_FREE) { m_sMovingVolley.eVolleyStatus = VOLLEY_READY; } }

		// For DebugRender

		void TestAIVolumes(void);
		void MovingVolleyDebugRender(void);

		

		bool m_bCollisionWithAIVolumes; // To be removed
		bool m_bNewNavigationSystem;
		
		bool m_bRenderAIAvoidance;
		bool m_bFormationSlotChanging;
		bool m_bRenderWallAvoidance;
		bool m_bRenderTotalSteeringAction;
		bool m_bRenderNavigGraph;
		bool m_bRenderPatrolGraph;
		bool m_bRenderViewCones;
		bool m_bRenderKnowledge;
		bool m_bRenderAIWorldVolumes;
		bool m_bRenderVideoText;
		bool m_bMovingVolleyDebugRender;

	private:

		void SetSteeringFlags	( AI* pEnt, unsigned int flags	) { if (pEnt) { pEnt->GetAIComponent()->GetCAIMovement()->SetSteeringFlags(flags); } }
		void SetMaxSpeed		( AI* pEnt, float fSpeed		) { if (pEnt) { pEnt->GetAIComponent()->GetCAIMovement()->SetMaxSpeed(fSpeed); } }
		
		// Cleaning up
		void FreeData			( void );
		void LevelUnload		( void );

	private:

		CAINavigGraphManager	m_obNavigGraphManager;
		CAISteeringLibrary		m_obSteeringLibrary;
		CAILeaderMan			m_obLeaderMan;
		CAIWorldMan				m_obAIWorldMan;
		CAIDivingMan			m_obAIDivingMan;
		SMovingVolley			m_sMovingVolley;

		CEntityQuery			m_obAIListQuery;
		QueryResultsContainerType*		m_pAIList;

		float					m_fPostArrowHitRadiusSQR;	// Radius of influence to alert AIs after Kai hits an AI
		float					m_fPostArrowHitCoverTime;	// Max time since Kai hit ans AI that AIs will be allowed to cover
		float					m_fTimeWithoutArrowHits;	// Time passed since Kai didn't hit any AI

};

LV_DECLARE_USERDATA(CAINavigationSystemMan);

#endif // _AINAVIGSYSTEMMAN_H


