/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SIMULATION_H
#define HK_DYNAMICS2_SIMULATION_H

#include <hkbase/hkBase.h>

class hkConstraintQueryIn;
class hkSimulationIsland;
struct hkWorldDynamicsStepInfo;
struct hkProcessCollisionInput;
struct hkProcessCollisionOutput;
class hkBroadPhase;
class hkWorld;
class hkStepInfo;
class hkEntity;
struct hkAgentNnEntry;

	/// See hkWorld::stepDeltaTime for details
enum hkStepResult
{
		/// The call to stepDelta time was a processed to the end
	HK_STEP_RESULT_SUCCESS,

		/// The engine predicted that it would run out of memory before doing any work
	HK_STEP_RESULT_MEMORY_FAILURE_BEFORE_INTEGRATION,	

		/// The collide call failed as some collision agents were allocated too much memory
	HK_STEP_RESULT_MEMORY_FAILURE_DURING_COLLIDE,

		/// The advanceTime call failed during TOI solving
	HK_STEP_RESULT_MEMORY_FAILURE_DURING_TOI_SOLVE,

};


	// Base class for simulations, performs simulation at discrete physical time steps.
class hkSimulation: public hkReferencedObject
{
	public:

		hkSimulation( hkWorld* world );
		~hkSimulation();


		//
		// Synchronous interface
		//

			// Implemented method of hkSimulation
		virtual hkStepResult stepDeltaTime( hkReal physicsDeltaTime );


			// Advance the state of bodies in time.
		virtual hkStepResult integrate( hkReal physicsDeltaTime );

			// Perform collision detection.
		virtual hkStepResult collide();

			// Advance the current time
		virtual hkStepResult advanceTime();


		//
		// Asynchronous interface (used in conjunction with the methods above)
		//
			
			// Schedule an asynchronous step, given a frame delta time
		void setFrameTimeMarker( hkReal frameDeltaTime );

			// Returns true if m_currentTime is earlier than the frame time
		bool isSimulationAtMarker();

			// Returns true if m_currentTime is equal to m_currentPsiTime
		bool isSimulationAtPsi() const;


		//
		// Time accessors
		//

		inline hkReal getCurrentTime() { return m_currentTime; }
		inline hkReal getCurrentPsiTime() { return m_currentPsiTime; }
		inline hkReal getSimulateUntilTime() { return m_simulateUntilTime; }
		inline hkReal getPhysicsDeltaTime() { return m_physicsDeltaTime; }


		//
		// Utility methods
		//

		enum FindContacts
		{
			FIND_CONTACTS_DEFAULT = 0,
			FIND_CONTACTS_EXTRA
		};
			/// Calculates contact information by performing discrete collision detection.
			/// It uses hkTransform from hkMotionState and ignores hkSweptTransform, therefore by default it will 
			/// perform collision detection at m_timeOfNextPsi, unless the user changes hkTransforms of bodies.
			/// Note: in case of hkPredGsk, final position will be used always.
			/// This should be only performed for bodies that have just been added, activated, or moved in the world, 
			/// and have a 'stationary' sweptTransform (with the same start & end positions).
		virtual void collideEntitiesDiscrete( hkEntity** entities, int numEntities, hkWorld* world, const hkStepInfo& stepInfo, FindContacts findExtraContacts  ); 

			// Update the broad phase for a set of entities
		static void collideEntitiesBroadPhaseDiscrete( hkEntity** entities, int numEntities, hkWorld* world );

			// Just calls processAgentsOfEntities
		void collideEntitiesNarrowPhaseDiscrete( hkEntity** entities, int numEntities, const hkProcessCollisionInput& input, FindContacts findExtraContacts );


			// Invalidates TIMs and removes contact points from manifolds.
		virtual void resetCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world, hkBool skipReinitializationOfAgents = false );

			// Checks that there is no simulation-scope information relating to the entities.  
			// Checks for Toi Events in hkContinuousSimulation.
		virtual void assertThereIsNoCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world ) {}
	
			// Removes simluation's collision information related to the agent. This is empty here, as no simulation-scope information 
			// is stored in hkSimluation. (This is overridden for hkContinuousSimulation.)
		virtual void removeCollisionInformationForAgent( hkAgentNnEntry* agent ) {}

			// Asserts when collision information related to the agent is found.
		virtual void assertThereIsNoCollisionInformationForAgent( hkAgentNnEntry* agent ) {}


			// Reintegrates bodies immediately. Only useful in asynchronous simulations.
			// When physics and framerate run at different rates (especially when the physics
			// is run at a much lower rate) velocity changes of rigid bodies will only be used
			// at normal physical integration steps. That means setting a velocity of a body
			// will not change the visible velocity immediately. This can lead to very small
			// delays between a player action and the expected reaction. With this function
			// you can force the reintegration of a set of entities at any time between two
			// physics integration steps, avoiding any delays in the system at the expense of some
			// extra CPU.
			// Tech info:
			//       This does not perform backstepping -- it only integrates the the position from their current
			//       position1.
		virtual void reintegrateAndRecollideEntities( hkEntity** entityBatch, int numEntities, hkWorld* world );



	protected:

		//
		// Helper step methods
		//
			// Asserts if the time step changes dramatically from step to step
		void checkDeltaTimeIsOk( hkReal deltaTime );

			// Utility function
		hkStepResult reCollideAfterStepFailure();

			// Collide discrete
		virtual void collideInternal( const hkStepInfo& stepInfoIn );

			// Integrate discrete
		void integrateInternal( const hkStepInfo& stepInfoIn );

			// Apply all actions of the world
		void applyActions();

			// Called from integrate()
		static HK_FORCE_INLINE void integrateIsland( hkSimulationIsland* isle, const hkWorldDynamicsStepInfo& stepInfo, hkConstraintQueryIn& constraintQueryIn );

			// Called from collide()
		static HK_FORCE_INLINE void collideIslandNarrowPhaseDiscrete( hkSimulationIsland* isle, const hkProcessCollisionInput& input);


			// Helper called from advanceTime()
		hkReal snapSimulateTimeAndGetTimeToAdvanceTo();

		//
		// Agent processing sub methods
		//
			// Defines a callback function type to be called by processAgentsOfEntities (below)
		typedef void (hkSimulation::*AgentEntryProcessFunction)(hkAgentNnEntry*, const hkProcessCollisionInput&, hkProcessCollisionOutput&);

			// Iterates through agents, processes each only once, and calls processingFunction for it.
		void processAgentsOfEntities( hkEntity** entities, int numEntities, const hkProcessCollisionInput& input, AgentEntryProcessFunction processingFunction, FindContacts findExtraContacts );


			// The standard implementation of the callback function for hkSimulation. Runs discrete collision detection for an agent entry.
		void processAgentCollideDiscrete(hkAgentNnEntry* entry, const hkProcessCollisionInput& processInput, hkProcessCollisionOutput& processOutput);

			// Callback function which invalidates TIM's and contact points in the contact manfold.
		void processAgentResetCollisionInformation(hkAgentNnEntry* entry, const hkProcessCollisionInput& processInput, hkProcessCollisionOutput& processOutput);

	public:

			/// Adds deltaTime's value to whatever time-related variables are stored in hkSimulation.
		virtual void warpTime( hkReal deltaTime ) {}

	public:

			// Internal use only
		void setCurrentTime( hkTime time ) { m_currentTime = time; }

			// Internal use only
		void setCurrentPsiTime( hkTime time ) { m_currentPsiTime = time; }

			// Internal use only
		void setSimulateUntilTime ( hkTime time ) { m_simulateUntilTime = time; }

		hkWorld* getWorld(){ return m_world; }
	
	protected:

		friend class hkWorld;
		class hkWorld* m_world;

		enum LastProcessingStep
		{
			INTEGRATE,
			COLLIDE
		};

			// Used for debug checks that integrate and collide are called consecutively
		LastProcessingStep m_lastProcessingStep;

			// Current time of the simluation. (Not the same as the time of the last simulation step.)
			// All TOIs up to this time have been processed.
			// Note: that the simulation's global time is not the absolute time from the beginning of the simulation. The time is
			// reset in the whole simulation every once in a while to avoid floating-point precision problems.
		hkTime m_currentTime;

			// Current "PSI" time of the simulation. This is always greater than or equal to m_currentTime.
			// PSIs are stepped "ahead" and then verified by solving TOIs up to that time.
		hkTime m_currentPsiTime;

			// Current delta time being simulated
		hkReal m_physicsDeltaTime;

			// Current asynchronous marker (-1 if not set)
		hkTime m_simulateUntilTime;

		hkReal m_frameMarkerPsiSnap;

		hkStepResult m_previousStepResult;

};

#endif // HK_DYNAMICS2_SIMULATION_H



/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
