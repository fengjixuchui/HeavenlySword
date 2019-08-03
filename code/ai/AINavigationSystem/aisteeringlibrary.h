//! -------------------------------------------
//! aisteeringlibrary.h
//!
//! AI Steering Library
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//! Date	04/05/06 - Creation
//!--------------------------------------------


#ifndef _AISTEERINGLIBRARY_H
#define _AISTEERINGLIBRARY_H

#include "ainaviggraphmanager.h"
#include "game/query.h"

// Forward declaration of classes

class CAIMovement;
class CAINavigNode;

typedef struct _SSphericRepulsiveEnts
{
	_SSphericRepulsiveEnts () : pEnt(NULL), fRadius(0.0f), fRadiusSQR(0.0f) {}
	_SSphericRepulsiveEnts (CEntity* pE, float fR) : pEnt(pE), fRadius(fR), fRadiusSQR(fR*fR) {}

	CEntity*	pEnt;
	float		fRadius;
	float		fRadiusSQR;
} SSphericRepulsiveEnts;
typedef ntstd::Vector<SSphericRepulsiveEnts*, Mem::MC_AI> SSphericRepulsiveEntsVector;

typedef struct _SBoxedRepulsiveEnts
{
	_SBoxedRepulsiveEnts () : pEnt(NULL), fRadius(0) {}
	_SBoxedRepulsiveEnts (CEntity* pE, float fR): pEnt(pE), fRadius(fR) {}

	CEntity*	pEnt;
	float		fRadius;
} SBoxedRepulsiveEnts;
typedef ntstd::Vector<SBoxedRepulsiveEnts*, Mem::MC_AI> SBoxedRepulsiveEntsVector;

//! -------------------------------------------
//! CAINavigationSystemMan
//! -------------------------------------------
class CAISteeringLibrary
{

	public:
		
		CAISteeringLibrary() : b_AIAvoidanceTriggered(false), m_pLocalAIList(NULL) {}
		~CAISteeringLibrary();
		void FreeData (void);
		void GenerateAIBotList(void);
		void GenerateDynObstacleList(void);

		// Basic Steering Actions
		CDirection Seek						( const CPoint&, const CPoint&, const float );
		CDirection StrafeToCombatPoint		( CAIMovement* );
		CDirection StrafeToFormationPoint	( CAIMovement* );
		CDirection ChaseTarget				( CAIMovement* );
		CDirection ChaseMovingEntity		( CAIMovement* );
		CDirection FollowEntity				( CAIMovement* );
		//CDirection FollowPlayer		( CAIMovement* );
		CDirection Flee						( CAIMovement*, const CPoint& );
		CDirection Pursuit					( const CPoint&, const CPoint&, const float )	NOT_IMPLEMENTED;	// Seek with prediction
		CDirection Evade					( const CPoint&, const CPoint&, const float )	NOT_IMPLEMENTED;	// Flee with prediction
		CDirection Wander					( const CPoint&, const CPoint&, const float )	NOT_IMPLEMENTED;
		CDirection Arrive					( CAIMovement*, const CPoint&				);		
		CDirection ArriveAndNotify			( CAIMovement*, const CPoint&				);
		CDirection ArriveAtPoint			( CAIMovement* );
		CDirection FollowWall				( void			)								NOT_IMPLEMENTED;	// Follows a wall
		CDirection FollowPath				( CAIMovement*	);
		CDirection FollowPathWithCover		( CAIMovement*, bool* );
		CDirection FollowPatrolPath			( CAIMovement*	);
		CDirection Flock					( const CPoint&, const CPoint&, const float )	NOT_IMPLEMENTED;
		CDirection GoAroundVolume			( CAIMovement* pMov );

		// Obstacle Avoidance

		CDirection CalculateWallAvoidance	( CAIMovement*, bool* );
		CDirection CalculateAIAvoidance		( CAIMovement* );
		CDirection CalculateDynObstAvoidance( CAIMovement*	);
		CDirection CalculateFlocking		( CAIMovement*	);

		// Calculates the Final Steering Action

		CDirection CalculateSteeringAction	( CAIMovement*, float );

		// List Management
		
		void RemoveEntity		( CEntity* pEnt		);

		// Query about obtacles obtructing a particular location

		bool IsThereObstaclesAtPos ( const CPoint&, float ); // Position, Radius SQR

		// The calculated perpendicular is:
		//			Perpendicular
		//           |
		//           |
		//  A|---------------->B

		CDirection GetPerpendicular	(const CDirection& obDir) { return CDirection(obDir.Z(),obDir.Y(),-obDir.X()); }
		CDirection Rotate			(const CDirection& obDir, float fAngle_Rad);
	
	private:

		void AddAIBot			( CEntity*			);
		void AddSphericEntity	( CEntity*, float	);	
		
		bool IsAIAvoidanceTriggered		( void ) const  { return b_AIAvoidanceTriggered; }
		void SetAIAvoidanceTriggered	( bool b )		{ b_AIAvoidanceTriggered = b; }  

	private:
		
		SSphericRepulsiveEntsVector	m_vSRE;			// List of Spheric Obstables 
		SBoxedRepulsiveEntsVector	m_vBRE;			// List of Boxed Obstables

		ntstd::Vector<CEntity*>					m_vAIBots;		// List of AI's to avoid when navigating

		bool									b_AIAvoidanceTriggered;

		QueryResultsContainerType*				m_pLocalAIList;
};



#endif // _AISTEERINGLIBRARY_H


