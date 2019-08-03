/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS_CONTINUOUS_SIMULATION_H
#define HK_DYNAMICS_CONTINUOUS_SIMULATION_H

#include <hkdynamics/world/simulation/hkSimulation.h>
#include <hkbase/memory/hkLocalBuffer.h>
#include <hkmath/basetypes/hkContactPointMaterial.h>

struct hkConstraintSchemaInfo;
struct hkConstraintSolverResources;

class hkToiResourceMgr;
struct hkProcessCollisionInput;
class hkBroadPhase;
class hkStepInfo;
struct hkSolverInfo;
class hkConstraintQueryIn;
class hkDynamicsContactMgr;

struct hkProcessCollisionOutput;
class hkWorldTimeEvent;
struct hkToiResources;


	// Holds information necessary for processing a Time-Of-Impact (TOI) event.
struct hkToiEvent
{
	public:
			// Time-Of-Impact, value may be postponed (increased) depending on the qualities of involved bodies.
		hkTime         m_time;

			// The two colliding bodies.
		hkEntity*      m_entities[2];

			// Separating velocity of bodies at TOI. This is a negative value, as the bodies are approaching
			// each other.
		hkReal         m_seperatingVelocity;

			// Friction, restitution and user data.
		hkContactPointMaterial m_material;

			// Contact manager.
		hkDynamicsContactMgr* m_contactMgr;

			// Contact point recorded at the TOI.
		hkContactPoint m_contactPoint;
};

	// A class which supports continuous physics 
class hkContinuousSimulation : public hkSimulation
{
	public: 

		hkContinuousSimulation( hkWorld* world );

		~hkContinuousSimulation();


			// TOI events are reported by predictive collision detection performed during a PSI step or at the end of 
			// code handling TOI events themselves.
			// 
			// Handling toi's:
			// Each new TOI event is appended onto an unsorted list of events. When handling events, the earliest one is chosen to
			// be handled. Handling a TOI event consists of backstepping an appropriate set of bodies, calculating proper collision
			// response, reintegrating bodies transforms and re-running collision detection. Therefore, this may result in removal 
			// of some of the previously reported events, and addition of new ones.
			// 
			// Executing simulation:
			// Current frame time and frameDeltaTime define the 'current time' or 'result' for the simulation. All TOI events which
			// have a time-stamp earlier than that time must be handled. 
		hkStepResult advanceTime();


			// implementation of hkSimulation::reintegrateAndRecollideEntityBatchImmediately
		virtual void reintegrateAndRecollideEntities( hkEntity** entityBatch, int numEntities, hkWorld* world );

		//
		// Internal 
		//

			// Warps internal time variables by the specified value.
			// Here: all TOI-events are processed.
		virtual void warpTime( hkReal warpDeltaTime );

			// Calls hkSimulation's version (invalidate TIM's + clear Manifolds) + removes TOI events.
		virtual void resetCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world, hkBool skipReinitializationOfAgents = false );

			// Checks that there is no Toi Events relating to the entities.  
		virtual void assertThereIsNoCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world );

			// Removes Toi Events created by the agent.
		virtual void removeCollisionInformationForAgent( hkAgentNnEntry* agent );

			// Asserts when Toi Events created by the agent are found.
		virtual void assertThereIsNoCollisionInformationForAgent( hkAgentNnEntry* agent );

	public:

			// Broad phase continuous collision detection for a set of hkEntities
	    void collideEntitiesBroadPhaseContinuous ( hkEntity** entities, int numEntities, hkWorld* world, hkChar* exportFinished = HK_NULL);

			// Narrow phase continuous collide detection an entire hkSimulationIsland
		void collideIslandNarrowPhaseContinuous  ( hkSimulationIsland* isle, const hkProcessCollisionInput& input);

			// Used by reintegrateAndRecollideEntityBatchImmediately
		void collideEntitiesNarrowPhaseContinuous( hkEntity** entities, int numEntities, const hkProcessCollisionInput& input);

	protected:
			// Callback function. Runs continuous collision detection for an agent entry.
		void processAgentCollideContinuous(hkAgentNnEntry* entry, const hkProcessCollisionInput& processInput, hkProcessCollisionOutput& processOutput);

			// Performs continuous collision detection.
			// The only difference (relative to hkSimulation verison) is usage of
			// predictive/continuous implementations of Broad/Narrow Phase functions.
		virtual void collideInternal( const hkStepInfo& stepInfoIn );

	public:
			//
			// Toi events adding/removing
			//

			// Constructs a TOI-event information struct and appends it to the hkContinuousSimulation::m_toievents list.
		void addToiEvent(const hkProcessCollisionOutput& result, const hkAgentNnEntry& entry);

	protected:
			// XXX called by remove entity batch - remove ToiEventsOfEntity is in worldOperationUtil
		void removeToiEventsOfEntities( hkEntity** entities, int numEntities );

		void removeToiEventsOfEntity( hkEntity* entity);

	protected:

		HK_FORCE_INLINE void fireToiEventRemoved( hkToiEvent& event );

			// handle all tois till a given time
		hkStepResult handleAllToisTill( hkTime minTime );

	protected:
			// 
			// Colliding chosen entities of an island -- used in simulateToi (might be slightly faster than hkBackstepSimulation::collideEntitiesContinuous)
			//

			// Internal: Calls broad and narrow phase for selected entities, which are assumed to all belong to the same hkSimulationIsland.
			// This performs island merging in between broad-phase and narrow-phase collision detection.
		HK_FORCE_INLINE void collideEntitiesOfOneIslandContinuous( hkEntity** entities, int numEntities, hkWorld* world, const hkProcessCollisionInput& input ); 

			// Internal: Narrow phase continuous collision detection of a set of hkEntities belonging to one hkSimulationIsland.
			// Implementation info: It uses an internal array of flags to make sure that each agent is processed once only.
			// Note: we keep that in addition to collideEntitiesNarrowPhaseContinuous, as this doesn't use a pointerMap and is therefore slightly faster for our TOI's
			// Note: it's only used for handling TOI events.
			// <todo> this should take the entityState table as input. Be aware of the hack, which prevents backsteppedAndFrozen bodies form being recollided.
		HK_FORCE_INLINE void collideEntitiesOfOneIslandNarrowPhaseContinuous( hkEntity** entities, int numEntities, const hkProcessCollisionInput& input );

			
			// Internal: Removes toi events.
			// Removes those TOI events from m_toiEvents list, which are related to one of the activated bodies.
		HK_FORCE_INLINE void deleteToiEventsForActiveEntitiesInIsland( const hkSimulationIsland* island, const hkFixedArray<hkUchar>& entityState, hkArray<hkToiEvent>& toiEvents );


	protected:

			// This function queries the toiResourceMgr for resources, conducts setup of resource and solver variables.
			// It calls localizedSolveToi, which returns a list of activeEntities. For those entities collision detection is then run,
			// potentially generating new TOI events. Finally the toiResourcesMgr is called to free allocated resources. And
			// any pending operations are run.
		virtual void simulateToi( hkWorld* world, hkToiEvent& event, hkReal physicsDeltaTime );

		//
		//	Predictive Collision Data
		//

	public:
			// Unsorted list of TOI events to be handled.
		hkArray<hkToiEvent>    m_toiEvents;
		hkToiResourceMgr*    m_toiResourceMgr;

		// debug only
		int	m_toiCounter;

};

void hkLs_doSimpleCollisionResponse( hkWorld* world, const hkToiEvent& event, hkReal rotateNormal, hkArray<hkEntity*>& toBeActivated );

#endif // HK_DYNAMICS_CONTINUOUS_SIMULATION_H



/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
