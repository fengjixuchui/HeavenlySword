/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_MULTITHREADING_UTIL_H
#define HK_UTILITIES2_MULTITHREADING_UTIL_H


#include <hkbase/thread/hkSemaphore.h>
#include <hkbase/thread/hkThread.h>
#include <hkdynamics/world/hkWorld.h>

typedef void (HK_CALL *StartSpuTasksetFunc)( void* userParam );
typedef void (HK_CALL *StartSpuTaskFunc)( void* userParam );
typedef void (HK_CALL *WaitForSpuCompletionFunc )( void* userParam );


/// The construction info for the hkMultithreadingUtil
struct hkMultithreadingUtilCinfo
{
		/// The world, must be of simulation type multithreaded.
	hkWorld* m_world;

		/// The initial number of threads (defaults to 1)
	int m_numThreads;

		/// The local stack size for each thread (defaults to 4 Mb)
	int m_localStackSize;

		/// If timers are enabled, the util currently allocates 12 Mb in each thread for collecting timer
		/// information. They are enabled by default.
	bool m_enableTimers;


		/// This function is a callback called from the primary thread to enable you to start a SPURS taskset
		/// when the primary thread starts
	StartSpuTasksetFunc m_startSpuTasksetFunc;
		/// User info for the m_startSpuTasksetFunc callback function
	void* m_startSpuTasksetFuncParam;

		/// This function is a callback called from the primary thread to enable you to start a SPURS task
		/// from the primary thread after it calls stepBeginSt().
	StartSpuTaskFunc m_startSpuTaskFunc;
		/// User info for the m_startSpuTaskFunc callback function
	void* m_startSpuTaskFuncParam;

		/// This function is a callback called from the primary thread to enable you to wait for completion 
		/// on a set of SPURS tasks from the primary thread, prior to calling stepEndSt()
	WaitForSpuCompletionFunc m_waitForSpuCompletionFunc;
		/// User info for the m_waitForSpuCompletionFunc callback function
	void* m_waitForSpuCompletionFuncParam;

	hkMultithreadingUtilCinfo();
};

	/// Utility class for running Havok Physics in multiple threads.
	/// The utility creates a pool of threads, and suspends them with a semaphore. On each call to
	/// startStepWorld the main thread resumes/releases all physics threads.
	/// Before the main thread references hkWorld again and before the next startStepWorld() call
	/// it must call waitForStepWorldFinish() which suspends the main thread until all physics threads
	/// finish their calculations.
	/// Note that this class is intended as an example for how to use multithreaded stepping with Havok.
	/// It is intended that you look at the code, particularly in WorkerThreadFunc, and use it as
	/// as example to integrate multithreaded stepping into your game code.
class hkMultithreadingUtil
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES, hkMultithreadingUtil);

		enum { MAX_NUM_PHYSICS_THREADS = 6 };

			/// Initialize multithreading state and create threads.
		hkMultithreadingUtil( const hkMultithreadingUtilCinfo& cinfo );

			/// Destroy threads and delete state.
		~hkMultithreadingUtil();

			/// This function may be used to step the world asynchronously.  
		void setFrameTimeMarker( hkReal frameDeltaTime );

			/// Step world by deltaTime.
			/// NOTE: You must call hkWorld::resetThreadTokens before calling this function
			/// This function simply calls startStepWorld and waitForStepWorldFinish.
		void stepWorld( hkReal physicsDeltaTime );

			/// Initialize stepping the world by deltaTime. 
			/// NOTE: You must call hkWorld::resetThreadTokens before calling this function
			/// This fires off the physics threads
			/// and allows you to perform other calculations in your main thread, e.g. graphics.
			/// All information synchronization, e.g. transforms of entities, should be done before 
			/// this call.
			/// It is important not to reference the hkWorld object before the next call 
			/// to waitForStepWorldFinished(), as the hkWorld may be locked and being modified by the 
			/// physics threads.
		void startStepWorld( hkReal physicsDeltaTime );

			/// Wait till steppingWorld is finished. This pauses the current thread until all physics threads
			/// successfully finish their physics computations, and until the hkWorld is safe to use.
		void waitForStepWorldFinished();

			/// Get number of threads currently assigned for physics.
		int getNumThreads();

			/// Set the number of threads to be used for physics. Returns the number of threads actually set.
		int setNumThreads(int numThreads);

			/// use this helper function to attach a new world to an already existing multithreading utility;
			/// make sure to detach the old world first!
		void attachWorld(hkWorld* world);

			/// use this helper function to detach the current world from this multithreading utility
		void detachWorld(hkWorld* world);


	public:

		struct State;

			/// Thread state data. Used only by one physics thread and the main thread.
		struct WorkerThreadData
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ENTITY, WorkerThreadData);

			WorkerThreadData();

				/// Pointer to the multithreading state structure shared between physics threads.
			State* m_state;

				/// System handle to the thread.
			hkThread m_thread;

				/// Arbitrary thread Id
			int m_threadId;

				/// Flag is set to true when the thread is requested to close.
			bool m_killThread;

				/// Semaphore used to pause a physics thread after it's done its calculations.
				/// This semaphore is released by the main thread on every simulation step.
			hkSemaphore m_semaphore;

			hkThreadToken m_threadToken;

				// Internal buffer used for collecting and copying Havok's timer information from a
				// physics thread back to the main thread.
			char* m_monitorStreamBegin;
			char* m_monitorStreamEnd;

			const char* m_localMonitorStreamBegin;
			const char* m_localMonitorStreamEnd;

			class hkMonitorStreamAnalyzer* m_streamAnalyzer;
		};

			/// Multithreading state data. Shared by all threads.
		struct State
		{
			State();

				/// The world simulated by the utility.
			class hkWorld* m_world;

				/// Simulation time step.
			hkReal m_deltaTime;

				/// Frame time step
			hkReal m_frameDeltaTime;

				/// Semaphore used to pause the main thread when it waits for physics threads 	
				/// to finish their calculations.
			hkSemaphore m_workerThreadFinished;

			hkSemaphore m_stepInitSemaphore;

				/// Semaphore only used in asynchronous stepping, when looping many PSIs, to prevent secondary threads
				/// looping before the primary thread is finished calling advanceTime()
			hkSemaphore m_stepEndSemaphore;

				/// An additional semaphore used for asynchronous stepping.
			hkSemaphore m_asyncSetupSemaphore;

				/// An additional semaphore used for asynchronous stepping.
			hkSemaphore m_asyncFinishSemaphore;

				/// Debugging flag set to true when physics threads may be running.
			hkBool m_physicsRunning;

				/// Data local to each physics thread.
			WorkerThreadData m_workerThreads[MAX_NUM_PHYSICS_THREADS];

			bool m_enableTimers;

				/// Num of physics threads.
			int m_numThreads;

			int m_localStackSize;

			StartSpuTasksetFunc m_startSpuTasksetFunc;
			void* m_startSpuTasksetFuncParam;

			StartSpuTaskFunc m_startSpuTaskFunc;
			void* m_startSpuTaskFuncParam;
			WaitForSpuCompletionFunc m_waitForSpuCompletionFunc;
			void* m_waitForSpuCompletionFuncParam;
		};

	protected:

			// Initialize multithreading state and create threads.
		void initMultithreading(class hkWorld* world, int numThreads);

			// Destroy threads and delete state.
		void quitMultithreading();

			// Add a physics thread.
		hkResult addThread();

			// Remove a physics thread.
		hkResult removeThread();

		void reassignThreadTokens();

	public:
		State m_state;
};

#endif // HK_UTILITIES2_MULTITHREADING_UTIL_H

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
