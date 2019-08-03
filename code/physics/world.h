/***************************************************************************************************
*
*   $Header:: /game/Physics/world.h $
*
*	
*
*	CREATED
*
*	06.12.2002	John	Created
*
***************************************************************************************************/

#ifndef	_PHYSICSWORLD_H
#define	_PHYSICSWORLD_H

#include "config.h"

// Necessary includes
#include "collisionbitfield.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/basetypes/hkStepInfo.h>
#include <hkdynamics/world/hkWorld.h>
#endif

// Foward declarations
class	CEntity;
class	hkPhysicsContext;
class	hkWorld;
class	hkVisualDebugger;
class	hkCollidable;
class	hkMemory;
class	hkThreadMemory;
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
class	hsMultithreadingUtil;
#else
class	hkMultithreadingUtil;
#endif
class	hkVector4;
class	hkEntity;
class	hkCollidable;
struct	hkCollisionInput;
class	hkCdBodyPairCollector;
class	hkPhantom;
struct	hkProcessCollisionInput;
class	hkConstraintInstance;
class	hkAction;
class   hkAabbPhantom;

#ifdef USE_HAVOK_ON_SPUS
#include "physics/hkSpuUtil.h"
class hkSpuMonitorCache;
#else
class	hkSpuThreadUtil;
#endif

class	hkMonitorStreamAnalyzer;

// Constants
const float fGRAVITY				= -9.81f;
const float fBROADPHASE_WORLD_SIZE	= 2400.0f;
const float fCOLLISION_TOLERANCE	= 0.07f;

const float fPHYSICS_WANTED_STEP_SPEED		= ( 0.0166667f * 2.0f );
const float fPHYSICS_WANTED_STEP_FRAMERATE	= 1.0f / ( 0.0166667f * 2.0f );

// This functor used to implement filters for castray functions
class CastRayFilter
{
public:
	virtual ~CastRayFilter() {};
	virtual bool operator() (CEntity *pobEntity) const = 0; // return false to ignore entity
};

class GroupCastRayFilter : public CastRayFilter
{
public:
	bool operator() (CEntity *pobEntity) const	
	{
		for(ntstd::Vector<const CastRayFilter *>::const_iterator it = m_filters.begin(); it != m_filters.end(); ++it)
		{
			if (!((*it)->operator()(pobEntity)))
				return false;
		}
		return true;
	}

	void AddFilter(const CastRayFilter * filter)
	{
		m_filters.push_back(filter);
	}

protected:
	ntstd::Vector<const CastRayFilter *> m_filters;
};


namespace Physics
{
	class CVolumeData;
	class ParticleCollisionListener;
	class WriteAccess;
	class ReadAccess;
	class psPhysicsMaterial;

	/***************************************************************************************************
	*	
	*	CLASS			TRACE_LINE_QUERY
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	class TRACE_LINE_QUERY
	{
	public:

		// Point on the line where we hit something
		float fFraction; 

		// Intersection point
		CPoint obIntersect; 

		// Normal of surface at intersection
		CDirection obNormal; 

		// What we hit
		CEntity* pobEntity;

		// What bit did we hit
		Physics::EntityCollisionFlag obCollidedFlag;

		// What material was hit. Search for material is done only is this pointer in nonzero
		psPhysicsMaterial ** ppobPhysicsMaterial;

		TRACE_LINE_QUERY():
			fFraction( -1.0f ),
			pobEntity( 0 ),
			ppobPhysicsMaterial( 0 )
		{}
	};


	/***************************************************************************************************
	*	
	*	CLASS			CPhysicsWorld
	*
	*	DESCRIPTION		This is a wrapper for the havok world
	*
	*					In preproduction this wrapper should be written so that none of the internals 
	*					of the physics system is exposed.  For example here we have a call 
	*					"GetIntersecting" which takes a havok type as a parameter - which means we 
	*					are not wrapping the implementation very effectively.  This was done to keep
	*					the flexibility of the havok shapes implementation.  We should really write 
	*					calls such as "GetIntersectingBox" and "GetIntersectingSphere" which then use
	*					the same internal call.  Either that or write our own shape implementation - not 
	*					sure how useful that would be though - GH
	*
	***************************************************************************************************/
	class CPhysicsWorld : public Singleton<CPhysicsWorld>
	{
	public:

		// Construction / destruction
		CPhysicsWorld( void );
		~CPhysicsWorld( void );

		// To be called at the beginning of a level to set up filters
		void SetupCollisionFilter( void );

		// The main update
		void Update( float fTimeDelta );

		//** The functions below properly wrap havok from the rest of the code base ***//

		// Ray cast functionality
		const CEntity* CastRay( const CPoint &obSrc, const CPoint &obTarg, Physics::RaycastCollisionFlag obFlag, bool bFixedOrKeyframedOnly = false ) const;
		const CEntity* CastRayFiltered( const CPoint &obSrc, const CPoint &obTarg, CastRayFilter *pfnFilter, Physics::RaycastCollisionFlag obFlag ) const;

		// This returns details of the first surface that is found to intersect the described ray - returns true if an intersection is found
		bool GetClosestIntersectingSurfaceDetails( const CPoint& obRayStart, const CPoint& obRayEnd, float& fHitFraction, CDirection& obIntersectNormal, Physics::RaycastCollisionFlag obFlag ) const;

		// Like ray casts - but more, if phantom is set perform raycast on phantom... 
		bool TraceLine( const CPoint& obStart, const CPoint& obEnd, const CEntity* pobIgnoreEntity, TRACE_LINE_QUERY& stQuery, Physics::RaycastCollisionFlag obFlag, CastRayFilter *pfnFilter=NULL, hkAabbPhantom * phantom = NULL ) const;
		bool TraceLine( const CPoint& obStart, const CPoint& obEnd, ntstd::List<CEntity*>& obIgnoreList, TRACE_LINE_QUERY& stQuery, Physics::RaycastCollisionFlag obFlag, CastRayFilter *pfnFilter=NULL, hkAabbPhantom * phantom = NULL ) const;

		// Raycast but for characters raycast is done allways against their ragdolls so it is more exact... 		
		bool TraceLineExactCharacters( const CPoint& obStart, const CPoint& obEnd, float characterOverlap, 
			TRACE_LINE_QUERY& stQuery, Physics::RaycastCollisionFlag obFlag, CastRayFilter *pfnFilter=NULL) const;

		// Find all the entities that are intersected within a radius from a point - returns true if there is anything in the list
		bool FindIntersectingRigidEntities( const CPoint& obCentre, float fRadius, ntstd::List<CEntity*>& obIntersecting ) const;
		bool IsBoxIntersectingStaticGeometry(const CPoint& obCentre, const CPoint& obHalfExtents, float fYRotation);

		void Start_AddingPhysicsObjects();
		void End_AddingPhysicsObjects();

		const hkVector4 &	GetGravity		()	const;
		hkTime				GetFrameTime	()	const;

		void				AddEntity		( hkEntity *entity, hkEntityActivation initialActivationState = HK_ENTITY_ACTIVATION_DO_ACTIVATE );
		void				AddEntityBatch	( hkEntity * const *entityBatch, int numEntities, hkEntityActivation initialActivationState = HK_ENTITY_ACTIVATION_DO_ACTIVATE );

		hkPhantom *			AddPhantom		( hkPhantom *phantom );
		void				RemovePhantom	( hkPhantom *phantom );

		hkAction *			AddAction		( hkAction *action );
		void				RemoveAction	( hkAction *action );

		hkConstraintInstance *AddConstraint( hkConstraintInstance *constraint );
		hkBool RemoveConstraint( hkConstraintInstance *constraint );

		void UpdateCollisionFilterOnEntity( hkEntity *entity, hkUpdateCollisionFilterOnEntityMode updateMode, hkUpdateCollectionFilterMode updateShapeCollectionFilter );
		void UpdateCollisionFilterOnPhantom( hkPhantom *phantom, hkUpdateCollectionFilterMode updateShapeCollectionFilter );

		//** The functions below do no wrapping at all ***//
		// Direct access to havok
		hkWorld* GetHavokWorldP( void ) { return m_pobHavokWorld; }
		hkWorld* GetAuxiliaryHavokWorldP( void ) { return m_pobAuxiliaryWorld; }

		// Find intersecting stuff
		void GetIntersecting( const hkCollidable *pobCollision, ntstd::List<CEntity*>& obIntersecting, CastRayFilter *pfnFilter = NULL ) const;

		void GetPenetrations( const hkCollidable *collA, const hkCollisionInput &input, hkCdBodyPairCollector &collector );

		void GetClosestPoints( const hkCollidable *collA, const hkCollisionInput &input, hkCdPointCollector &collector );
		hkProcessCollisionInput *GetCollisionInput() const;

		// Get the time step information
		class hkStepInfo GetStepInfo( void );
		float GetLastStep() const {return m_lastWorldStep;};

		CriticalSection m_CritSec;

		static int GetNumberOfStepsWorldWillTakeNextFrame( const hkWorld* world, hkReal frameDeltaTime, hkReal physicsDeltaTime );

		void CreateLocalThreadMem( hkThreadMemory** ppThreadMem, char** ppThreadStack );
		void FreeLocalThreadMem( hkThreadMemory** ppThreadMem, char** ppThreadStack );

	private:
		friend class Physics::WriteAccess;
		friend class Physics::ReadAccess;


#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		hkStepInfo					m_stepInfo;					// Time step information used by havok
		hkWorld *					m_pobHavokWorld;			// A pointer to the entire havok simulation
		hkThreadMemory *			m_ThreadMemory;				// The memory our threads use.
		hkMemory *					m_memoryManager;            // The memory manager
		char *						m_StackBuffer;				// Fast memory for Havok.
		hkWorld *					m_pobAuxiliaryWorld;        // This auxiliary world used only for detection collision
															    // or other stuff which could be done effectivly in different.
																// it will be not simulated at the moment
		hkCollisionListener       * m_pobCollisionListener; 

#ifdef USE_HAVOK_MULTI_THREADING 
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
		hsMultithreadingUtil *	m_ThreadHelper;				// Helper functions for threads.
#else
		hkMultithreadingUtil *	m_ThreadHelper;				// Helper functions for threads.
#endif

#ifdef USE_HAVOK_ON_SPUS
	public:
		struct SpursParams
		{
			hkSpuUtil m_spuUtil;
			unsigned int m_numberOfTasks; // number of Task started during sim step.
			// Info needed to be passed as parameters to the Spurs tasks
			hkWorldDynamicsStepInfo* m_dynamicsStepInfo;
			hkJobQueue* m_jobQueue;
			hkSpuMonitorCache* m_monitorCaches[8]; // max number of SPU-s 
		};

		unsigned int NumberOfSpursTasks() const {return m_spursParams.m_numberOfTasks;};

	private:
		SpursParams m_spursParams; // Params for Spurs tasks.
#endif  //USE_HAVOK_ON_SPUS

#endif  //USE_HAVOK_MULTI_THREADING

		hkVisualDebugger *			m_pobDebugger;				// A pointer to the Havok debugger.
#endif //_PS3_RUN_WITHOUT_HAVOK_BUILD

		ParticleCollisionListener *	m_pobParticleListener;		// Our collision listener.

		int32_t						m_TimerFrameCount;

		//#if defined( PLATFORM_PS3 )
		//float m_fTimeRemaining;
		//float m_fLastFrameChange;
		//#endif

		hkMonitorStreamAnalyzer* p_obMainStream;

		float					m_lastWorldStep;    // last step of havok world...
	};
}

#endif //_PHYSICSWORLD_H
