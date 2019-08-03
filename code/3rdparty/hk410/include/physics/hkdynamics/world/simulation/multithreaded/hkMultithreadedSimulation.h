/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS_MULTITHREADED_SIMULATION_H
#define HK_DYNAMICS_MULTITHREADED_SIMULATION_H

#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>
#include <hkbase/thread/job/hkJobQueue.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>
#include <hkbase/thread/hkSemaphore.h>

	// A class which supports multithreaded physics 
class hkMultiThreadedSimulation : public hkContinuousSimulation
{
	public: 
		
			// Initializes locks and changes the worlds broad phase dispatcher
		hkMultiThreadedSimulation( hkWorld* world );

			// 
		~hkMultiThreadedSimulation();


		//
		// Main loop functions
		//

		virtual hkStepResult stepDeltaTime( hkReal physicsDeltaTime );

		//
		// Internal
		//

			// Checks that there is no Toi Events relating to the entities.  
			// This overrides the hkContinuousSimuation's version as it must use a critical section to access the m_toiEvents array.
		virtual void assertThereIsNoCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world );

			// Removes Toi Events created by the agent.
			// This overrides the hkContinuousSimuation's version as it must use a critical section to access the m_toiEvents array.
		virtual void assertThereIsNoCollisionInformationForAgent( hkAgentNnEntry* agent );


		//
		// Additional multithreading interface.  The world must upcast to the hkMultiThreadedSimulation to call these.
		//

		hkThreadToken getThreadToken();
		void resetThreadTokens();

		void stepBeginSt( hkReal physicsDeltaTime );
		void stepProcessMt( const hkThreadToken& token );
		void stepEndSt();


		//
		// Interface implementation
		//


		virtual hkJobQueue* getJobQueue() { return &m_jobQueue; }

		virtual void getMultithreadConfig( hkMultithreadConfig& config );

		virtual void setMultithreadConfig( const hkMultithreadConfig& config );

			
			// All subsequent threads to call step delta time execute this function
		void processNextJob( hkJobQueue::ThreadType threadType );

	public:

				// This class is used internally by hkWorld to dispatch broad phase pairs to the relevant phantoms.
		class MtEntityEntityBroadPhaseListener : public hkBroadPhaseListener
		{
			public:

				MtEntityEntityBroadPhaseListener(  )
					: m_simulation(HK_NULL) {}

					// Delays addition of pairs between islands if the world is locked
				virtual void addCollisionPair( hkTypedBroadPhaseHandlePair& pair );

					// Delays removal of pairs between islands if the world is locked
				virtual void removeCollisionPair( hkTypedBroadPhaseHandlePair& pair );

			public:
				hkMultiThreadedSimulation* m_simulation;
		};

		class MtPhantomBroadPhaseListener : public hkBroadPhaseListener
		{
			public:
				MtPhantomBroadPhaseListener(  ): m_criticalSection(HK_NULL) {}

					// Adds the collision pair elements A and B if they are phantoms
				virtual void addCollisionPair( hkTypedBroadPhaseHandlePair& pair );

					// Removes the collision pair elements A and B if they are phantoms
				virtual void removeCollisionPair( hkTypedBroadPhaseHandlePair& pair );

			public:
				hkCriticalSection* m_criticalSection;
		};
		class MtBroadPhaseBorderListener : public hkBroadPhaseListener
		{
			public:
				MtBroadPhaseBorderListener(  ): m_criticalSection(HK_NULL) {}

				// Adds the collision pair elements A and B if they are phantoms
				virtual void addCollisionPair( hkTypedBroadPhaseHandlePair& pair );

				// Removes the collision pair elements A and B if they are phantoms
				virtual void removeCollisionPair( hkTypedBroadPhaseHandlePair& pair );

			public:
				hkCriticalSection* m_criticalSection;
		};

		//
		// Helper functions
		//
	public:

		//
		// Data shared between all threads during a step
		//

		MtEntityEntityBroadPhaseListener m_entityEntityBroadPhaseListener;
		MtPhantomBroadPhaseListener      m_phantomBroadPhaseListener;
		MtBroadPhaseBorderListener       m_broadPhaseBorderListener;
		
			// if this flag is set to true, new and deleted collidablepairs between different
			// islands are added to the m_addedCrossIslandPairs and m_removedCrossIslandPairs arrays
		hkBool							 m_crossIslandPairsCollectingActive;

		hkArray<hkTypedBroadPhaseHandlePair> m_addedCrossIslandPairs;
		hkCriticalSection m_addCrossIslandPairCriticalSection;

		hkArray<hkTypedBroadPhaseHandlePair> m_removedCrossIslandPairs;
		hkCriticalSection m_removeCrossIslandPairCriticalSection;

		hkMultithreadConfig m_multithreadConfig;


		// The number of active threads currently running the simulation
		int m_numActiveThreads;

		hkSemaphore m_stepInitSemaphore;

		HK_ALIGN(hkCriticalSection m_threadTokenCriticalSection, 64);


			// Job queue and associated data and locks
		HK_ALIGN( hkJobQueue m_jobQueue, 64 );

		HK_ALIGN(hkCriticalSection m_toiQueueCriticalSection, 64);

		HK_ALIGN(hkCriticalSection m_phantomCriticalSection, 64 );
};


#endif // HK_DYNAMICS_MULTITHREADED_SIMULATION_H

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
