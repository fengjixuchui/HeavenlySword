/***************************************************************************************************
*
*   $Header:: /game/physicsworld.cpp $
*
*	
*
*	CREATED
*
*	06.12.2002	John	Created
*
***************************************************************************************************/

#include "config.h"
#include "maths_tools.h"

#include "physics/world.h"
#include "Physics/havokincludes.h"
#include "Physics/physicsTools.h"
#include "core/gatso.h"
#include "game/interactioncomponent.h"
#include "game/entity.h"
#include "game/shellconfig.h"
#include "game/entity.inl"
#include "Physics/debugdraw.h"
#include "Physics/collisionfilter.h"
//#include "Physics/physicsTools.h"
#include "physics/system.h"

#ifdef USE_HAVOK_ON_SPUS
#include "exec/ppu/exec_ps3.h"
#endif

#include "game/messagehandler.h"

#ifdef PLATFORM_PC
#include "hair/forcefield.h"
#endif 

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"
#include "effect/psystem_utils.h"
#include "effect/effect_manager.h"

#include "physics/havokthreadutils.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#ifdef PLATFORM_PS3
#include "physics/ntPS3Socket.h"
#endif
#include <hkMath/hkMath.h>
#include <hkbase/debugutil/hkSimpleStatisticsCollector.h>
#include <hkbase/basesystem/hkBaseSystem.h>
#include <hkbase/error/hkDefaultError.h>
#include <hkbase/memory/impl/hkPoolMemory.h>
#include <hkbase/memoryclasses/hkMemoryClassDefinitions.h>
#include <hkutilities/visualdebugger/hkPhysicsContext.h>
#include <hkcollide/dispatch/hkAgentRegisterUtil.h>
#ifndef BINARY_PHYSICS_LOADER
#define HK_CLASSES_FILE <hkserialize/classlist/hkCompleteClasses.h>
#else
#define HK_CLASSES_FILE "physics/TypeRegistry.h"
#endif
#include <hkserialize/util/hkBuiltinTypeRegistry.cxx>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkcollide/collector/bodypaircollector/hkAllCdBodyPairCollector.h>
#include <hkcollide/collector/pointcollector/hkAllCdPointCollector.h>
#include <hkcollide/agent/hkCollidable.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkvisualize/hkVisualDebugger.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\collector\raycollector\hkClosestRayHitCollector.h>
#include <hkcollide\collector\raycollector\hkAllRayHitCollector.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkbase/stream/hkStreambufFactory.h>
#include <hkbase/monitor/hkSpuMonitorCache.h>
#include <hkbase/monitor/hkMonitorStreamAnalyzer.h>
#include <hkbase/stopwatch/hkStopWatch.h>
#include <hkutilities/simulation/hkAsynchronousTimestepper.h>

#include <hkserialize/util/hkFinishLoadedObjectRegistry.h>
#include <hkserialize/util/hkVtableClassRegistry.h>
#include <hkutilities/serialize/hkHavokSnapshot.h>
#include <hkutilities/serialize/hkPhysicsData.h>
#include <hkbase/stream/hkIstream.h>
#include "game/inputcomponent.h"
#include "game/entitymanager.h"

#ifndef _RELEASE
#include <hkdynamics/world/hkSimulationIsland.h>
#endif


#ifdef USE_HAVOK_ON_SPUS
#	include <hkbase/monitor/hkSpuMonitorCache.h>
#if  HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR > 0
#include <hkbase/thread/job/hkJobQueue.h>
#else
#include <hkdynamics/world/simulation/multithreaded/job/hkJobQueue.h>
#endif //HAVOK_SDK_VERSION_MAJOR
#endif

#ifdef PLATFORM_PS3
#	include "physics/ntStreamFactory_ps3.h"
#	include "physics/ntStreamWriter_ps3.h"
#endif // PLATFORM_PS3

#if defined(USE_HAVOK_MULTI_THREADING)
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
#	include "physics/hsMultiThreadingUtil.h"
#else
#	include <hkUtilities/thread/hkMultithreadingUtil.h>
#endif
#endif

#endif // end_PS3_RUN_WITHOUT_HAVOK_BUILD

//#if !defined( _RELEASE )
//#define VISUAL_DBG
//#endif

//#define HAVOK_TIMERS
#ifndef _RELEASE
#define HAVOK_MEMORY_STATISTICS
#endif

#ifndef _RELEASE
//#define DEBUG_ISLAND_ACTIVATION
#endif

#ifdef DEBUG_ISLAND_ACTIVATION 
#include <hkdynamics/world/listener/hkIslandActivationListener.h>
#endif


#ifdef HAVOK_TIMERS
static int frame = 0;
//static int skipFirstFrames = 2;
#endif

#include "physics/advancedcharactercontroller.h"
#include "core/visualdebugger.h"
#include "game/renderablecomponent.h"
#include "gfx/graphicsdevice.h"



#	ifdef USE_HAVOK_ON_SPUS
extern char _binary_hkspursconstraint_elf_start[];   
void* elf = _binary_hkspursconstraint_elf_start;  
#	endif


namespace Physics
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	//--------------------------------------------------
	//!
	//!	CCollisionListener.
	//!	Long Class Description
	//!	Exciting class with lots of stuff to describe
	//!
	//--------------------------------------------------
	#define _USE_COLLLISON_SPARKS

	class ParticleCollisionListener : public hkCollisionListener
	{
	private:
		float m_fLastTriggerTime;
		#define TIMEOUT_DURATION (1.0f/30.0f)

		void* m_pSparksDef,* m_pSparksFlash,* m_pSparkler;

		//--------------------------------------------------
		//!
		//!	TriggerEffects
		//!
		//--------------------------------------------------
		void TriggerEffects( const CEntity* pEnt, hkContactPointConfirmedEvent& event )
		{
			ntAssert( pEnt );

			float fCurr = _R(CTimer::Get().GetGameTime());
			float fSinceLast = fCurr - m_fLastTriggerTime;

			if (fSinceLast >= TIMEOUT_DURATION)
			{
				m_fLastTriggerTime = fCurr;

				if (!m_pSparksDef)
				{
					m_pSparksDef = ObjectDatabase::Get().GetPointerFromName<void*>("Hit_Sparks_Definition");
					ntError_p( m_pSparksDef, ("Missing 'Hit_Sparks_Definition' for Havok ParticleCollisionListener\n") );
				}
				
				if (!m_pSparksFlash)
				{
					m_pSparksFlash = ObjectDatabase::Get().GetPointerFromName<void*>("Hit_Sparks_Flash");
					ntError_p( m_pSparksFlash, ("Missing 'Hit_Sparks_Flash' for Havok ParticleCollisionListener\n") );
				}
				
	//			if (!m_pSparkler)
	//			{
	//				m_pSparkler = ObjectDatabase::Get().GetPointerFromName<void*>("Hit_Sparks_Sparkler");
	//				ntError_p( m_pSparkler, ("Missing 'Hit_Sparks_Sparkler' for Havok ParticleCollisionListener\n") );
	//			}
				
				CMatrix mat = pEnt->GetMatrix();
				mat.SetTranslation( Physics::MathsTools::hkVectorToCPoint(event.m_contactPoint->getPosition()) );

				PSystemUtils::ConstructParticleEffect( m_pSparksDef, mat );
				PSystemUtils::ConstructParticleEffect( m_pSparksFlash, mat );
	//			PSystemUtils::ConstructParticleEffect( m_pSparkler, mat );
			}
		}

	public:

		ParticleCollisionListener ()
		{
			m_fLastTriggerTime = -TIMEOUT_DURATION;
			m_pSparksDef = 0;
			m_pSparksFlash = 0;
			m_pSparkler = 0;
		};

		virtual ~ParticleCollisionListener () {};

		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event)
		{
			hkRigidBody* obRB = hkGetRigidBody(&event.m_collidableA);
			hkRigidBody* obRB2 = hkGetRigidBody(&event.m_collidableB);

			CEntity* pobEntityA = 0;
			CEntity* pobEntityB = 0;

			if(obRB)
				pobEntityA = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();

			if(obRB2)
				pobEntityB = (CEntity*) obRB2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
			if	(
				(obRB) &&
				(obRB->hasProperty(Physics::PROPERTY_SPAWN_PARTICLES_INT)) &&
				(obRB->getLinearVelocity().length3() >= 3.f)
				)
				TriggerEffects(pobEntityA,event);

			if	(
				(obRB2) &&
				(obRB2->hasProperty(Physics::PROPERTY_SPAWN_PARTICLES_INT)) &&
				(obRB2->getLinearVelocity().length3() >= 3.f)
				)
				TriggerEffects(pobEntityB,event);
		};

		virtual void contactPointRemovedCallback (hkContactPointRemovedEvent& event)
		{
		};

		virtual void contactProcessCallback (hkContactProcessEvent& event)
		{
		};


		virtual void contactPointAddedCallback(	hkContactPointAddedEvent& event) 
		{
		};

	};
#endif

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	/***************************************************************************************************
	*
	*	FUNCTION		ntPrintfFunc
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/ 
	static void ntPrintfFunc(const char* str, void* vargs)
	{
		ntPrintf(str, vargs);
// 		ntError(	strstr( str, "Mopp" ) != NULL || strstr( str, "Report" ) != NULL ||
// 					strstr( str, "not fully aligned" ) != NULL || strstr( str, "transform shape as the root shape" ) != NULL ||
// 					strstr( str, "stable thin box" ) != NULL );
	};

	/***************************************************************************************************
	*
	*	FUNCTION		setupVisualDebugger
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/ 
	hkVisualDebugger* setupVisualDebugger(hkPhysicsContext* physicsWorlds)
	{
		if (g_ShellOptions->m_bUseHavokDebugger)
		{
			// Setup the visual debugger                 
			hkArray<hkProcessContext*> contexts;
			contexts.pushBack(physicsWorlds);

			hkVisualDebugger* vdb = HK_NEW hkVisualDebugger(contexts); 
			vdb->serve(); 
#ifdef HAVOK_TIMERS
			hkMonitorStream::getInstance().resize(2 * 1024 * 1024 );
			hkMonitorStream::getInstance().reset();
#endif

			return vdb;
		}
		else
		{
			UNUSED( physicsWorlds );
			return 0;
		}
	};
#endif
//static int wholeAlloc = 0;
static void * HavokMalloc( int size, int align )
{	
	/*wholeAlloc += size;
	if (size > 130000)
	{
		ntPrintf("Big allocation  %d , %d  \n", size, wholeAlloc);
	}
	else
	{
		ntPrintf("Small allocation  %d , %d  \n", size, wholeAlloc);
	}*/
	ScopedCritical sc( CPhysicsWorld::Get().m_CritSec );
	return (void*) NT_MEMALIGN_CHUNK( Mem::MC_HAVOK, size, align );
}
static void HavokFree( void* p )
{
	ScopedCritical sc( CPhysicsWorld::Get().m_CritSec );
	NT_FREE_CHUNK( Mem::MC_HAVOK, (uintptr_t) p );
}


#ifdef USE_HAVOK_MULTI_THREADING
// Callbacks for starting SPURS from the multithreaded util
#ifdef USE_HAVOK_ON_SPUS


void HK_CALL startSpursTask( void* params )
{
	CPhysicsWorld::SpursParams* startSpursCallbackParams = static_cast<CPhysicsWorld::SpursParams*>(params);
	
	for (unsigned int i = 0; i < CPhysicsWorld::Get().NumberOfSpursTasks(); ++i )
	{
		hkSpuParams params;
		params.m_param0 = startSpursCallbackParams->m_dynamicsStepInfo;
		params.m_param1 = startSpursCallbackParams->m_jobQueue;
#ifndef _RELEASE
		//params.m_param2 = startSpursCallbackParams->m_monitorCaches[i];
		//params.m_param3 = (void*)(hkUlong)i;
#endif

		startSpursCallbackParams->m_spuUtil.startSpursTask(params);
	}
}


void HK_CALL startSpurs( void* params )
{
	((hkSpuUtil*)(params))->initSpurs( Exec::GetSpurs(), elf );
}

void HK_CALL waitForSpuCompletion( void* params )
{
	// we used all SPUs  BEWARE what if number change during the processing of task?
	for ( unsigned int i = 0; i < CPhysicsWorld::Get().NumberOfSpursTasks(); ++i )
	{
		static_cast<hkJobQueue*>(params)->m_taskCompletionSemaphore.acquire();
	}
}
#endif //USE_HAVOK_ON_SPUS 
#endif //USE_HAVOK_MULTI_THREADING

#ifdef DEBUG_ISLAND_ACTIVATION 
class CatchActivation : public hkIslandActivationListener
{
public:
	void  islandActivatedCallback (hkSimulationIsland *island) 
	{
		// add here any code for debugging
		int i = 0;
		i++;
	};

	void  islandDeactivatedCallback (hkSimulationIsland *island) 
	{
		// add here any code for debugging
		int i = 0;
		i++;
	};
};
#endif

	void CPhysicsWorld::CreateLocalThreadMem( hkThreadMemory** ppThreadMem, char** ppThreadStack )
	{
		*ppThreadMem = HK_NEW hkThreadMemory( m_memoryManager, 16 );
		hkBaseSystem::initThread( *ppThreadMem );

		int stackSize = 1024*1024;
		*ppThreadStack = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
		(*ppThreadMem)->setStackArea( *ppThreadStack, stackSize);

	}

	void CPhysicsWorld::FreeLocalThreadMem( hkThreadMemory** ppThreadMem, char** ppThreadStack )
	{
		(*ppThreadMem)->setStackArea(0, 0);
		hkDeallocate( *ppThreadStack );
		hkBaseSystem::clearThreadResources();
		HK_DELETE(*ppThreadMem);
		*ppThreadMem = 0; 
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::CPhysicsWorld
	*
	*	DESCRIPTION		This initialises the havok world
	*
	***************************************************************************************************/ 
	CPhysicsWorld::CPhysicsWorld()
	:	m_TimerFrameCount( 0 )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		#ifdef PLATFORM_PS3
		ntPS3Socket::ReplaceSocketImpl();
		#endif

		m_CritSec.Initialise();

		hkSystemMalloc = &HavokMalloc;
		hkSystemFree = &HavokFree;
		
		// Memory ownership os taken away
		m_memoryManager = HK_NEW hkPoolMemory();

		// Debug memory option instead if you want:
		// hkMemory* memoryManager = NT_NEW hkDebugMemory();

		// Initialise the Havok base system.
		m_ThreadMemory = HK_NEW hkThreadMemory( m_memoryManager, 16 );
		hkBaseSystem::init( m_memoryManager, m_ThreadMemory, (hkErrorReportFunction)(ntPrintfFunc) );
		m_memoryManager->removeReference(); // deallocated by Havok

		// Replace the Havok default stream factory under PS3 only (the default works for PC).
#		ifdef PLATFORM_PS3
		{
			hkStreambufFactory::getInstance().replaceInstance( HK_NEW ntStreamFactory() );
		}
#		endif // PLATFORM_PS3

		// We now initialize the stack area (fast temporary memory to be used by the engine).
		int stackSize = 1024*1024;
		m_StackBuffer = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
		m_ThreadMemory->setStackArea( m_StackBuffer, stackSize);
		
		hkWorldCinfo obInfo;
		obInfo.m_gravity								= hkVector4( 0, fGRAVITY, 0 );
		obInfo.m_collisionTolerance						= 0.01f;
		obInfo.m_iterativeLinearCastEarlyOutDistance	= obInfo.m_collisionTolerance / 10.0f;
		obInfo.m_contactPointGeneration					= hkWorldCinfo::CONTACT_POINT_REJECT_MANY;
		obInfo.m_broadPhaseBorderBehaviour				= hkWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY;
		// Can't be enabled yet due to crash bug in Havok
		//obInfo.m_broadPhaseNumMarkers					= 64;
		obInfo.m_minDesiredIslandSize					= 10; // approx number of bodies in ragdolls

		obInfo.setBroadPhaseWorldSize( fBROADPHASE_WORLD_SIZE );
		obInfo.setupSolverInfo( hkWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM );
		//obInfo.setupSolverInfo( hkWorldCinfo::SOLVER_TYPE_8ITERS_HARD );

#		if defined( USE_HAVOK_MULTI_THREADING )
			obInfo.m_simulationType = hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
#		else
			obInfo.m_simulationType = hkWorldCinfo::SIMULATION_TYPE_CONTINUOUS;
#		endif
		
		m_pobHavokWorld = HK_NEW hkWorld( obInfo );		
		m_pobHavokWorld->lock();

		// Collision listener that implements physical materials etc... 
		m_pobCollisionListener = HK_NEW CWorldCollisionListener();
		m_pobHavokWorld->addCollisionListener(m_pobCollisionListener);

#ifdef DEBUG_ISLAND_ACTIVATION 
		m_pobHavokWorld->addIslandActivationListener(HK_NEW CatchActivation()); 
#endif 
		// Initialize the visual debugger so we can connect remotely to the simulation
		hkPhysicsContext *physicsContext = HK_NEW hkPhysicsContext;
		hkPhysicsContext::registerAllPhysicsProcesses(); // all the physics viewers
		physicsContext->addWorld( m_pobHavokWorld ); // add the physics world so the viewers can see it

		if (g_ShellOptions->m_bUseHavokDebugger)
			m_pobDebugger = setupVisualDebugger( physicsContext );

#		if defined( USE_HAVOK_MULTI_THREADING )

		// If we are using SPUs, then we create an SPU Util (our demo wrapper around simple usage of
		// SPURS tasks and SPU threads). 
		// We also set a number of settings for demo purposes: 
		//	We set the multithread config to CPU_CAN_NOT_TAKE_SPU_TASKS - this means the simulation will only
		//		run if SPUs are present.
		//	We set m_maxNumConstraintsSolvedSingleThreaded to be 0 which means that consraint setup is performed
		// on the SPU regardless of how many constraints are present.
		hkMultithreadConfig config;
#ifdef USE_HAVOK_ON_SPUS
		config.m_canCpuTakeSpuTasks = hkMultithreadConfig::CPU_CAN_NOT_TAKE_SPU_TASKS;
#endif
		config.m_maxNumConstraintsSolvedSingleThreaded = 0;
		m_pobHavokWorld->setMultithreadConfig( config );		
		
		// If we are running multiple threads we create an instance of hsMultithreadingUtil.  This will
		// create additional threads to run physics
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
		hsMultithreadingUtilCinfo info;
#else
		hkMultithreadingUtilCinfo info;
#endif
		info.m_world = m_pobHavokWorld;
		info.m_numThreads = 1;
#ifdef HAVOK_TIMERS
		info.m_enableTimers = true;
#else
		info.m_enableTimers = false;
#endif


		// If we are running spurs, we make all calls to spurs from the same thread. We do this by
		// firing callbacks from the primary thread.
#ifdef USE_HAVOK_ON_SPUS				
		// to use to many spu will be ineffective use only 2
		m_spursParams.m_numberOfTasks = Exec::NumberOfSPUsInSpurs() < 2 ? Exec::NumberOfSPUsInSpurs() : 2; 
		m_spursParams.m_dynamicsStepInfo = &m_pobHavokWorld->m_dynamicsStepInfo;
		m_spursParams.m_jobQueue = m_pobHavokWorld->getJobQueue();

		info.m_startSpuTaskFunc = &startSpursTask;
		info.m_startSpuTaskFuncParam = &m_spursParams;
		info.m_startSpuTasksetFunc = &startSpurs;
		info.m_startSpuTasksetFuncParam = &m_spursParams.m_spuUtil;

		info.m_waitForSpuCompletionFunc = &waitForSpuCompletion;
		info.m_waitForSpuCompletionFuncParam = m_pobHavokWorld->getJobQueue();
		
#endif //USE_HAVOK_ON_SPUS
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
		m_ThreadHelper = new hsMultithreadingUtil(info);	
#else
		m_ThreadHelper = new hkMultithreadingUtil(info);
#endif
#endif // USE_HAVOK_MULTI_THREADING

		m_stepInfo.m_deltaTime = 0.0f;
		m_stepInfo.m_invDeltaTime = 0.0f;

		m_stepInfo.m_startTime = m_pobHavokWorld->getCurrentTime();
		m_stepInfo.m_endTime = m_pobHavokWorld->getCurrentTime();
		hkAgentRegisterUtil::registerAllAgents( m_pobHavokWorld->getCollisionDispatcher() );

		// [MUS] - disabled an ntAssert that is not fixable and not crucial
		// - Theses asserts should not stop the execution really...
		//
		// Argh. A bit of documentation would be nice, ffs. How am I supposed
		// to know what's been turned off? [ARV].
		hkError::getInstance().setEnabled( 0x1f0690f0, false );
		hkError::getInstance().setEnabled( 0x1213fa33, false );
		hkError::getInstance().setEnabled( 0x71f7efbf, false );
		hkError::getInstance().setEnabled( 0xffffffff, false );
		hkError::getInstance().setEnabled( 0xf03243ed, false );
		hkError::getInstance().setEnabled( 0xf0764312, false );
		hkError::getInstance().setEnabled( 0x234f224a, false );
		hkError::getInstance().setEnabled( 0x2a1db936, false );

		// All of these asserts are disabled because Havok have cocked up their lock checking for multithreading.
		hkError::getInstance().setEnabled( 0xf02e32df, false );		// Disable "Your object was locked by a different thread" assert. hkmultithreadlock.cpp, line 59.
																	// I've disabled assert 0xf02e32df because the havok locking logic/semantics are currently broken -
																	// we should re-enable when they get it fixed (see ticket #619-2018429). [ARV].
		hkError::getInstance().setEnabled( 0xf02de43e, false );		// Disable "You cannot turn a lockForRead into a lockForWrite" assert. hkmultithreadlock.cpp, line 40.
																	// Havok lock-checking semantics are just plain wrong - it'll let me have a RW lock if I start with no
																	// lock but won't let me have a RW lock if I already own a RO lock. Rubbish. [ARV].
		hkError::getInstance().setEnabled( 0xf02132ff, false );		// Disable "Your object is already locked for write by another thread, you have to create a critical section around the havok object to avoid conflicts".
																	// The check isn't working - Havok are fixing. This is related to the above two. [ARV].
		hkError::getInstance().setEnabled( 0xf043d534, false );
		hkError::getInstance().setEnabled( 0xf02132df, false );

		// Multithreaded asynchronous asserts, disabling it at the request of Havok
		hkError::getInstance().setEnabled( 0xf0ff0080, false );
		hkError::getInstance().setEnabled( 0xad67fa3a, false );
		// Action assert disabled - this is our own ID in our own hkDynamics build
		hkError::getInstance().setEnabled( 0x57d9c254, false );

		hkError::getInstance().setEnabled( 0xabbaefbf, false ); //Disable "Bones A:"<<boneA->m_name<<" and B:"<<boneB->m_name<<" are not fully aligned (error :"<<err<<")."

		hkError::getInstance().setEnabled( 0x38f1276d, false); // Raycast target is outside the broadphase. False miss may be reported.

		hkError::getInstance().setEnabled( 0x54e4323e, false); // Removed reports from hkSkeletonMapperUtils.cpp
		hkError::getInstance().setEnabled( 0x2ff8c16f, false); // Removed warning about transform shape, we cann't collapse them
		hkError::getInstance().setEnabled( 0x9afce65, false); // Removed warning about using hkMultithreaded Util

																
		// End of havok fuck up fix.

		m_pobParticleListener = 0;

#		ifdef _USE_COLLLISON_SPARKS
		{
			m_pobParticleListener = NT_NEW ParticleCollisionListener();
			m_pobHavokWorld->addCollisionListener( m_pobParticleListener );
		}
#		endif

		m_pobHavokWorld->unlock();
#endif
		//#if defined( PLATFORM_PS3 )
		//m_fTimeRemaining = m_fLastFrameChange = 0.0f;
		//#endif
#ifdef HAVOK_TIMERS
		p_obMainStream = HK_NEW hkMonitorStreamAnalyzer( 2 * 1024 * 1024 );
#endif

		m_lastWorldStep = fPHYSICS_WANTED_STEP_SPEED;

		// create auxiliary world
		obInfo.m_simulationType = hkWorldCinfo::SIMULATION_TYPE_CONTINUOUS;
		m_pobAuxiliaryWorld = HK_NEW hkWorld( obInfo );	
		m_pobAuxiliaryWorld->lock();
		hkAgentRegisterUtil::registerAllAgents( m_pobAuxiliaryWorld->getCollisionDispatcher() );
		m_pobAuxiliaryWorld->unlock();		

	}


	/***************************************************************************************************
	*
	*	FUNCTION		~CPhysicsWorld
	*
	*	DESCRIPTION		Cleans up the havok world
	*
	***************************************************************************************************/
	CPhysicsWorld::~CPhysicsWorld()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		m_pobHavokWorld->markForWrite();
		if (g_ShellOptions->m_bUseHavokDebugger)
		{
			NT_DELETE( m_pobDebugger );
			m_pobDebugger = NULL;
		}

#		ifdef _USE_COLLLISON_SPARKS
		{
			m_pobHavokWorld->removeCollisionListener( m_pobParticleListener );
			NT_DELETE( m_pobParticleListener );
			m_pobParticleListener = NULL;
		}
#		endif

#		if defined( USE_HAVOK_MULTI_THREADING )
		{
			NT_DELETE( m_ThreadHelper );
			m_ThreadHelper = NULL;

#			ifdef USE_HAVOK_ON_SPUS
			{
				m_spursParams.m_spuUtil.quitSpurs();
			}
#			endif
		}
#		endif

		if (m_pobCollisionListener)
		{
			m_pobHavokWorld->removeCollisionListener(m_pobCollisionListener);
			HK_DELETE(m_pobCollisionListener);
		}

		//Should clean itself up as there should be no more references to it		
		m_pobHavokWorld->removeReference();
		m_pobHavokWorld = NULL;

		hkDeallocate( m_StackBuffer );
		m_StackBuffer = NULL;

		hkBaseSystem::quit();

		HK_DELETE( m_ThreadMemory );
		m_ThreadMemory = NULL;

		m_CritSec.Kill();
		RagdollPerformanceManager::Get().Clear();

#endif
	}


	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::SetupCollisionFilter()
	*
	*	DESCRIPTION		I did this - GH - kick me in if its bad
	*
	***************************************************************************************************/
	void CPhysicsWorld::SetupCollisionFilter()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::HSCollisionFilter* pobFilter = HK_NEW Physics::HSCollisionFilter();
		m_pobHavokWorld->lock();
		m_pobAuxiliaryWorld->lock();
		m_pobHavokWorld->setCollisionFilter( pobFilter );
		m_pobAuxiliaryWorld->setCollisionFilter( pobFilter );
		pobFilter->removeReference();
		m_pobHavokWorld->unlock();
		m_pobAuxiliaryWorld->unlock();
#endif
	}

	void CPhysicsWorld::Start_AddingPhysicsObjects()
	{
		m_pobHavokWorld->lock();
	}

	void CPhysicsWorld::End_AddingPhysicsObjects()
	{
		m_pobHavokWorld->unlock();
	}

	/***************************************************************************************************
	*
	*	CLASS			FrameStepSmoother
	*
	*	DESCRIPTION		[Mus] - 2004.02.16
	*					Please, be less harsh with Havok when it comes to the timestep
	*					But really, Havok should be stepped at a fixed rate all the time!
	*
	***************************************************************************************************/
	#define SMOOTH_STEP_OVER_N_FRAME 10
	class FrameStepSmoother {
	public:	

		~FrameStepSmoother() 
		{};

		static float GetSmoothed(float fCurrent)
		{
			static float fLastValue = 0.f;

			if( fabs( fCurrent - fLastValue ) > 0.01f )
			{
				// Reset the smoother in case of massive change recquiered
				m_ArraySize		= 0;
				m_CurrentIndex	= 0;
			};
			fLastValue = 0.f;

			if(m_ArraySize != SMOOTH_STEP_OVER_N_FRAME) {
				m_TimestepArray[m_CurrentIndex] = fCurrent;
				m_CurrentIndex = (m_CurrentIndex+1) % (SMOOTH_STEP_OVER_N_FRAME - 1);
				m_ArraySize++;

				for(int i = 0; i < m_ArraySize; i++)
				{
					fLastValue += m_TimestepArray[i];
				};

				fLastValue /= m_ArraySize;

				return fLastValue;
			} else {

				m_TimestepArray[m_CurrentIndex] = fCurrent;
				m_CurrentIndex = (m_CurrentIndex+1) % (SMOOTH_STEP_OVER_N_FRAME - 1);

				float fLastValue = 0.f;

				for(int i = 0; i < SMOOTH_STEP_OVER_N_FRAME; i++)
				{
					fLastValue += m_TimestepArray[i];
				};

				fLastValue *= m_invSmoothFrame;
			
				return fLastValue;

			};
		};

	private:

		FrameStepSmoother()
		{};

		static float				m_TimestepArray[SMOOTH_STEP_OVER_N_FRAME];
		static int					m_ArraySize;
		static int					m_CurrentIndex;
		static float				m_invSmoothFrame;
	};

	int						FrameStepSmoother::m_ArraySize = 0;
	int						FrameStepSmoother::m_CurrentIndex = 0;
	float					FrameStepSmoother::m_invSmoothFrame = 1.f / SMOOTH_STEP_OVER_N_FRAME;
	float					FrameStepSmoother::m_TimestepArray[SMOOTH_STEP_OVER_N_FRAME];

//#define REPORT_HAVOK_NUMBERS //uncomment this to get number of entities and constraints simulated by Havok at moment
#ifdef REPORT_HAVOK_NUMBERS
	static void ReportHavokNumbers(hkWorld * world)
	{
		int nActiveEntities = 0; 
		int nActiveConstraints = 0; 
		const hkArray< hkSimulationIsland * >& activIslands = world->getActiveSimulationIslands();
		for (int i = 0; i < activIslands.getSize(); i++)
		{
			const hkArray<hkEntity *>& entities = activIslands[i]->getEntities();
			nActiveEntities += entities.getSize(); 
			for (int j = 0; j < entities.getSize(); j++)
			{
				nActiveConstraints += entities[j]->getNumConstraints();
			}

		}
		nActiveConstraints /= 2; 

		int nInactiveEntities = 0;
		int nInactiveConstraints = 0;

		const hkArray< hkSimulationIsland * >& inactivIslands = world->getInactiveSimulationIslands();
		for (int i = 0; i < inactivIslands.getSize(); i++)
		{		
			const hkArray<hkEntity *>& entities = inactivIslands[i]->getEntities();
			nInactiveEntities += entities.getSize(); 
			for (int j = 0; j < entities.getSize(); j++)
			{
				nInactiveConstraints += entities[j]->getNumConstraints();
			}
		}

		nInactiveConstraints /= 2; 

		int nPhantoms = world->getPhantoms().getSize(); 

		ntPrintf("Active: entities %d, constraint %d \n", nActiveEntities, nActiveConstraints);
		ntPrintf("Inactive: entities %d, constraint %d \n", nInactiveEntities, nInactiveConstraints);
		ntPrintf("nPhantoms %d\n", nPhantoms);
	}
#endif

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::Update()
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	void CPhysicsWorld::Update( float fTimeDelta )
	{		
#ifdef REPORT_HAVOK_NUMBERS
		ReportHavokNumbers(m_pobHavokWorld);
#endif		
#ifdef HAVOK_TIMERS
		const int frames_to_time = 5;
		//skipFirstFrames--;
		if (!frame && /*skipFirstFrames == 0&&*/ CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_P, KEYM_CTRL ))
		{
			frame = frames_to_time;
		}

		if (frame)
		{
			hkMonitorStreamFrameInfo frameInfo;
			frameInfo.m_timerFactor0 = 1e6f / hkReal(hkStopwatch::getTicksPerSecond());
			frameInfo.m_heading = "microseconds";
			frameInfo.m_indexOfTimer0 = 0;
			frameInfo.m_indexOfTimer1 = -1;

			for (int t=0; t<m_ThreadHelper->getNumThreads(); t++)
			{
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
				hsMultithreadingUtil::WorkerThreadData& data = m_ThreadHelper->m_state.m_workerThreads[t];
#else
				hkMultithreadingUtil::WorkerThreadData& data = m_ThreadHelper->m_state.m_workerThreads[t];
#endif
				if (data.m_monitorStreamBegin != data.m_monitorStreamEnd)
				{
					data.m_streamAnalyzer->captureFrameDetails(data.m_monitorStreamBegin, data.m_monitorStreamEnd, frameInfo);
				}
	
			}

			p_obMainStream->captureFrameDetails(hkMonitorStream::getInstance().getStart(), hkMonitorStream::getInstance().getEnd(), frameInfo);

			if (frame == 1)
			{
				hkMonitorStreamAnalyzer monitor(2 * 1024 * 1024);
				hkArray<hkMonitorStreamAnalyzer*> array;
				
				char path[1024*2];
				Util::GetFiosFilePath( "havok_step_timer.txt", path );
				hkOstream out(path);
				out << "Timestep was " << fTimeDelta << "\n";
				ntError(out.isOk());

				for (int t=0; t<m_ThreadHelper->getNumThreads(); t++)
				{
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
					hsMultithreadingUtil::WorkerThreadData& data = m_ThreadHelper->m_state.m_workerThreads[t];
#else
					hkMultithreadingUtil::WorkerThreadData& data = m_ThreadHelper->m_state.m_workerThreads[t];
#endif
					array.pushBack(data.m_streamAnalyzer);
				}

				monitor.writeMultiThreadedStatistics(array, out, hkMonitorStreamAnalyzer::REPORT_SUMMARIES);

				char path2[1024*2];
				Util::GetFiosFilePath( "havok_client_timer.txt", path2 );
				hkOstream client(path2);
				ntError(client.isOk());
				client << "Timestep was " << fTimeDelta << "\n";
				p_obMainStream->writeStatistics(client, hkMonitorStreamAnalyzer::REPORT_SUMMARIES);
			}

			frame--;
		}
#endif // HAVOK_TIMERS

#ifdef HAVOK_MEMORY_STATISTICS
		if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_M, KEYM_SHIFT) )
		{
			char path[1024*2];
			Util::GetFiosFilePath( "havok_memory.txt", path );
			hkOstream client(path);
			ntError(client.isOk());
			m_memoryManager->printStatistics(&client);		
		}
#endif


#			ifdef HAVOK_TIMERS
		hkMonitorStream::getInstance().resize( 2 * 1024 * 1024  );
				hkMonitorStream::getInstance().reset();
#			endif

#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
			if( fTimeDelta > HK_REAL_EPSILON )
#else
			if( fTimeDelta > hkMath::HK_REAL_EPSILON )
#endif
			{
				// TODO Deano Forcefields not on PS3 yet
#				if defined( PLATFORM_PC )
				{
					// reset force field result
					if( ForceFieldManager::Exists() )
					{
						ForceFieldManager::Get().ResetForceField();
					}
				}
#				endif // PLATFORM_PC

#				if defined( USE_HAVOK_MULTI_THREADING )
				{
					if( fTimeDelta > EPSILON )
					{
//						m_pobHavokWorld->unlock();

					
						
						/*if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_P, KEYM_CTRL ) )
						{
							char path[1024*2];
							Util::GetFiosFilePath( "scenedumpPreStep.hkx", path );
							
							#ifdef PLATFORM_PS3
							ntBreakpoint();
							ntStreamWriter* pobStream = HK_NEW ntStreamWriter( path );
							bool binary = false; // false will output xml
							hkHavokSnapshot::save( m_pobHavokWorld, pobStream, binary);
							#else
							hkOstream outfile( path );
							ntAssert(outfile.isOk());
							bool binary = false; // false will output xml
							hkHavokSnapshot::save( m_pobHavokWorld, outfile.getStreamWriter(), binary);
							#endif
						}	*/
						
						HK_TIMER_BEGIN("Step World", 0);

						m_pobHavokWorld->resetThreadTokens();
#ifdef USE_ASYCHRONOUS_SIMULATION
						m_ThreadHelper->setFrameTimeMarker(fTimeDelta);	
#ifdef SYNC_ASYNCHRONOUS_SIMULATION
						hkArray<hkReal> stepTimes; 	
						float alreadySimulated = m_pobHavokWorld->getCurrentPsiTime() - m_pobHavokWorld->getCurrentTime(); // -  +  m_lastWorldStep;
						Physics::Tools::SynchronizeTime(fTimeDelta,  alreadySimulated,  m_lastWorldStep, stepTimes);						

						/*Debug::AlwaysOutputString("=========================================\n");						

						for(int i = 0; i < stepTimes.getSize(); i++)
						{
							char text[1024];
							sprintf(text,"%lf %lf %lf %lf\n", fTimeDelta, stepTimes[i], alreadySimulated, min(alreadySimulated, m_lastWorldStep - alreadySimulated));
							Debug::AlwaysOutputString(text);
						}*/

						if (stepTimes.getSize() > 0)
							m_lastWorldStep = stepTimes[stepTimes.getSize() - 1];

						m_ThreadHelper->startStepWorld(stepTimes);
#else
						m_ThreadHelper->startStepWorld(fPHYSICS_WANTED_STEP_SPEED);
#endif

//						ntPrintf("Sumary: %lf %lf %lf\n", m_pobHavokWorld->getCurrentTime(), m_pobHavokWorld->getCurrentPsiTime(), m_timeSimulated);

#else
						m_ThreadHelper->startStepWorld(fTimeDelta);
#endif //ifdef USE_ASYCHRONOUS_SIMULATION 
						m_ThreadHelper->waitForStepWorldFinished();
						HK_TIMER_END();


						// Reset timers after the frame so that we will can time
						// all Havok functions called by the HS code.
						//hkMonitorStream::getInstance().resize(2 * 1024 * 1024 );
						//hkMonitorStream::getInstance().reset();
												
						/*if (CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_P, KEYM_CTRL ))
						{
							hkOstream file("EntireWorld.xml");
							if (!file.isOk())
							{
								ntError(0);
							}
							hkPhysicsData data;
							data.addPhysicsSystem( m_pobHavokWorld->getWorldAsOneSystem() );
							hkHavokSnapshot::save( &data, file.getStreamWriter(), false );

							char path[1024*2];
							Util::GetFiosFilePath( "scenedumpPostStep.hkx", path );
							
							#ifdef PLATFORM_PS3
							ntBreakpoint();
							ntStreamWriter* pobStream = HK_NEW ntStreamWriter( path );
							bool binary = false; // false will output xml
							hkHavokSnapshot::save( m_pobHavokWorld, pobStream, binary);
							#else
							hkOstream outfile( path );
							ntAssert(outfile.isOk());
							bool binary = false; // false will output xml
							hkHavokSnapshot::save( m_pobHavokWorld, outfile.getStreamWriter(), binary);
							#endif
						}*/
						


//						m_pobHavokWorld->lock();
					}
				}
#				else // USE_HAVOK_MULTI_THREADING
				{
					if ( fTimeDelta > EPSILON )
					{
#ifdef USE_ASYCHRONOUS_SIMULATION
						hkAsynchronousTimestepper::stepAsynchronously( m_pobHavokWorld, fTimeDelta, fPHYSICS_WANTED_STEP_SPEED );
#else
						m_pobHavokWorld->stepDeltaTime( fTimeDelta );
#endif //def USE_ASYCHRONOUS_SIMULATION
					}
				}
#				endif // USE_HAVOK_MULTI_THREADING

				// TODO Deano Forcefields not on PS3 yet
#				if defined( PLATFORM_PC )
				{
					// reset force field result
					if ( ForceFieldManager::Exists() )
					{
						ForceFieldManager::Get().ComputeForAll();
					}
				}
#				endif // PLATFORM_PC
			}

		if (g_ShellOptions->m_bUseHavokDebugger)
		{
			m_pobDebugger->step();

#ifdef HAVOK_TIMERS
			hkMonitorStream::getInstance().resize(2 * 1024 * 1024 );
			hkMonitorStream::getInstance().reset();
#endif //HAVOK_TIMERS
		}


		// Cull ragdolls we don't need anymore
		HK_TIMER_BEGIN("UpdateRagdollPerformanceManager", HK_NULL);
		RagdollPerformanceManager::Get().Update();	
		HK_TIMER_END();
				}



	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::GetStepInfo()
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	hkStepInfo CPhysicsWorld::GetStepInfo()
	{
		return m_stepInfo;
	}


	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::GetIntersecting()
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	void CPhysicsWorld::GetIntersecting(const hkCollidable *pobCollision,ntstd::List<CEntity*>& obIntersecting,CastRayFilter *pfnFilter) const
	{
		Physics::ReadAccess mutex;

		//Get all shapes which are penetrating the collidable Note: If you have call this function every step for a given object, use the hkShapePhantom version.
		hkAllCdBodyPairCollector obCollector;

		m_pobHavokWorld->getPenetrations (pobCollision, (hkCollisionInput&)*m_pobHavokWorld->getCollisionInput(), obCollector);
		
		const hkArray<hkRootCdBodyPair>& obHits = obCollector.getHits();

		for(int iHit=0;iHit<obHits.getSize();iHit++)
		{
			const hkRootCdBodyPair* pobCurrentPair = &obHits[iHit];
			const hkCollidable* pobCollided = pobCurrentPair->m_rootCollidableB;
			if(pobCollided->getCollisionFilterInfo())
			{
				hkRigidBody* obRB = hkGetRigidBody(pobCollided);
				CEntity* pobEntity = 0;

				if(obRB)
					pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
					
				else {
					hkPhantom* obPH = hkGetPhantom(pobCollided);
					if(obPH)
						pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
						
				};

				if (pobEntity)
				{
					if (pfnFilter)
					{
						if((*pfnFilter)(pobEntity))
							obIntersecting.push_back( pobEntity );
					}
					else
					{
						obIntersecting.push_back( pobEntity );
					}
				}
			}
		}
	}
#endif
	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::CastRay
	*
	*	DESCRIPTION		Returns info on the first entity found along a line added by JML
	*
	***************************************************************************************************/
	const CEntity *CPhysicsWorld::CastRay(	const CPoint &obSrc, 
											const CPoint &obTarg, 
											Physics::RaycastCollisionFlag obFlag,
											bool bFixedOrKeyframedOnly ) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert((obTarg-obSrc).Length()>EPSILON);

		hkWorldRayCastInput obInput;
		obInput.m_from.set(obSrc.X(), obSrc.Y(), obSrc.Z());
		obInput.m_to.set(obTarg.X(), obTarg.Y(), obTarg.Z());

		obInput.m_filterInfo = obFlag.base;

		if (!bFixedOrKeyframedOnly)
		{
		hkClosestRayHitCollector obOutput;

		{
			Physics::CastRayAccess mutex;
			m_pobHavokWorld->castRay(obInput, obOutput);
		}

	#ifdef _DRAW_RAYCAST_IN_HKVDB
		Physics::HavokDebugDraw::DrawRaycastInVisualiser(obInput, obOutput);
	#endif

		if(!obOutput.hasHit())
		{
			return 0;
		}

		hkRigidBody* obRB = hkGetRigidBody(obOutput.getHit().m_rootCollidable);
		CEntity* pobEntity = 0;

		if(obRB)
			pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
		else {
			hkPhantom* obPH = hkGetPhantom(obOutput.getHit().m_rootCollidable);
			if(obPH)
				pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
				
		}
		
		return pobEntity;
		}
		else
		{
			// Need to get all hits and find the closest fixed
			hkAllRayHitCollector obOutput;

			{
				Physics::CastRayAccess mutex;
				m_pobHavokWorld->castRay(obInput, obOutput);
			}

		#ifdef _DRAW_RAYCAST_IN_HKVDB
			Physics::HavokDebugDraw::DrawRaycastInVisualiser(obInput, obOutput);
		#endif

			float fClosest = 9999999.0f;
			hkRigidBody* pobClosest = 0;
			for ( int i = 0; i < obOutput.getHits().getSize(); i++ )
			{
				if ( obOutput.getHits()[i].m_hitFraction < fClosest )
				{
					hkRigidBody* pobRB = hkGetRigidBody(obOutput.getHits()[i].m_rootCollidable);
					if (pobRB && pobRB->isFixedOrKeyframed())
					{
						pobClosest = pobRB;
					}
				}
			}

			if (pobClosest)
				return (CEntity*)pobClosest->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			else
				return 0;
		}
#else
		UNUSED( obFlag ); UNUSED( obTarg); UNUSED( obSrc );
		return 0;
#endif
	}

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	/***************************************************************************************************
	*
	*	CLASS			hkRayHitCollectorEntityFilter
	*
	*	DESCRIPTION		This class is derived from Havoks basic hit collector used by the cast ray
	*					functions.  Pass it a callback function when constructing and you can choose
	*					which entities to consider.  Added JML.  Where should this class ideally go?
	*
	***************************************************************************************************/
	class hkRayHitCollectorEntityFilter : public hkClosestRayHitCollector
	{
	public:
		hkRayHitCollectorEntityFilter(CastRayFilter *pfnCallback)
		{
			ntAssert(pfnCallback);
			m_pfnFilter = pfnCallback;
		}

		virtual void addRayHit(const hkCdBody &cdBody, const hkShapeRayCastCollectorOutput &hitInfo)
		{
			if(hitInfo.m_hitFraction < m_rayHit.m_hitFraction)
			{
				hkRigidBody* obRB = hkGetRigidBody(cdBody.getRootCollidable());
				CEntity* pobEntity = 0;

				if(obRB)
					pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
					
				else {
					hkPhantom* obPH = hkGetPhantom(cdBody.getRootCollidable());
					if(obPH)
						pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
						
				}
				
				if(pobEntity)
				{
					if((*m_pfnFilter)(pobEntity))
					{
						m_rayHit.m_rootCollidable = cdBody.getRootCollidable();
						m_earlyOutHitFraction = hitInfo.m_hitFraction;
					}
				}
				else
				{
					if((*m_pfnFilter)(0))
					{
						m_rayHit.m_rootCollidable = cdBody.getRootCollidable();
						m_earlyOutHitFraction = hitInfo.m_hitFraction;
					}
				}
			}
		}

	private:
		CastRayFilter *m_pfnFilter;
	};
#endif

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::CastRay
	*
	*	DESCRIPTION		Returns info on the first entity found along a line added by JML
	*
	***************************************************************************************************/
	const CEntity *CPhysicsWorld::CastRayFiltered( const CPoint &obSrc, const CPoint &obTarg, CastRayFilter *pFunc, Physics::RaycastCollisionFlag obFlag) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		//ntPrintf("StartedRayCast %f,%f,%f to %f,%f,%f\n",obSrc.X(), obSrc.Y(), obSrc.Z(),obTarg.X(), obTarg.Y(), obTarg.Z());
		//ntAssert((obTarg-obSrc).Length()>EPSILON);
		hkWorldRayCastInput obInput;
		obInput.m_from.set(obSrc.X(), obSrc.Y(), obSrc.Z());
		obInput.m_to.set(obTarg.X(), obTarg.Y(), obTarg.Z());

		obInput.m_filterInfo = obFlag.base;

		hkRayHitCollectorEntityFilter obOutput(pFunc);

		{
			Physics::CastRayAccess mutex;
			m_pobHavokWorld->castRay(obInput, obOutput);
		}

	#ifdef _DRAW_RAYCAST_IN_HKVDB
		Physics::HavokDebugDraw::DrawRaycastInVisualiser(obInput, obOutput);
	#endif
		//ntPrintf("FinishedRayCast\n");

		if(!obOutput.hasHit()) // We should at always hit something...
		{
			return 0;
		}

		hkRigidBody* obRB = hkGetRigidBody(obOutput.getHit().m_rootCollidable);
		CEntity* pobEntity = 0;

		if(obRB)
			pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
		else {
			hkPhantom* obPH = hkGetPhantom(obOutput.getHit().m_rootCollidable);
			if(obPH)
				pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();			
		};
		
		return pobEntity;
#else
		UNUSED( obFlag ); UNUSED( obSrc); UNUSED( obTarg ); UNUSED( pFunc );
		return 0;
#endif

	}


	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::GetClosestIntersectingSurfaceDetails
	*
	*	DESCRIPTION		This returns details of the first surface that is found to intersect the 
	*					described ray.  It will return true if a surface is found.
	*
	*					The caller of this function needs to pass a reference to a float and direction
	*					to retrieve the point in world space where the intersection occurred and the 
	*					normal of the surface at the point the intersection occurred.
	*
	***************************************************************************************************/
	bool CPhysicsWorld::GetClosestIntersectingSurfaceDetails( const CPoint& obRayStart, const CPoint& obRayEnd, float& fHitFraction, CDirection& obIntersectNormal, Physics::RaycastCollisionFlag obFlag ) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// Removed the below test - let havok handle it - gets caught when the game runs really slow
		// ntAssert((obRayEnd-obRayStart).Length()>EPSILON);

		// Convert our input into Havok types - may be useful to write some reusable code to do this kind of stuff
		hkVector4 obHavokRayStart( obRayStart.X(), obRayStart.Y(), obRayStart.Z() );
		hkVector4 obHavokRayEnd( obRayEnd.X(), obRayEnd.Y(), obRayEnd.Z() );

		// Build up a input struct to describe the ray
		hkWorldRayCastInput strRayCastInput;
		strRayCastInput.m_from = obHavokRayStart;
		strRayCastInput.m_to = obHavokRayEnd;
		strRayCastInput.m_filterInfo = obFlag.base;

		// Create an item with which to store out output
		hkClosestRayHitCollector obResults;

		// Dip in our tackle, see what bites
		{
			Physics::CastRayAccess mutex;
			m_pobHavokWorld->castRay( strRayCastInput, obResults );
		}

	#ifdef _DRAW_RAYCAST_IN_HKVDB
		Physics::HavokDebugDraw::DrawRaycastInVisualiser(strRayCastInput, obResults);
	#endif

		// Now get the details of the closest surface if any and return them in a non Havok form
		if ( obResults.hasHit() )
		{
			// Get the useful data from the results - the surface normal
			hkVector4 obSurfaceNormal = obResults.getHit().m_normal;
			obIntersectNormal = CDirection( obSurfaceNormal.getSimdAt( 0 ), obSurfaceNormal.getSimdAt( 1 ), obSurfaceNormal.getSimdAt( 2 ) );

			// And the point on the surface at which the collision took place
			// WD changed this to the hit fraction, as it's more useful to me, and im the
			// only person using this. (10.01.2005)

			fHitFraction = obResults.getHit().m_hitFraction;
			// obIntersectPosition = CPoint::Lerp( obRayStart, obRayEnd, fHitFraction );

			// An item has been found succesfully
			return true;
		}
#else
		UNUSED( obRayStart ); UNUSED( obRayEnd ); UNUSED( fHitFraction ); UNUSED( obIntersectNormal ); UNUSED( obFlag );
#endif
		// We did not find a surface
		return false;

	}

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	/***************************************************************************************************
	*
	*	CLASS			hkClosestRayHitCollectorEntityFilter
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	class hkClosestRayHitCollectorEntityFilter : public hkClosestRayHitCollector
	{
		public:

			hkClosestRayHitCollectorEntityFilter (const CEntity* pobIgnoreEntity, CastRayFilter *pfnFilter)
			{
				m_pobIgnoreEntity=pobIgnoreEntity;
				m_pfnFilter = pfnFilter;
			}

		protected:
			virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& output )
			{
				if ( output.m_hitFraction < m_rayHit.m_hitFraction)
				{
					// Check to see if the ray hit the ignore entity

					hkRigidBody* obRB = hkGetRigidBody(cdBody.getRootCollidable());
					CEntity* pobEntity = 0;

					if(obRB)
						pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
						
					else {
						hkPhantom* obPH = hkGetPhantom(cdBody.getRootCollidable());
						if(obPH)
							pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
							
					};
					
					if (pobEntity)
					{
						if (pobEntity==m_pobIgnoreEntity)
							return;

						if (m_pfnFilter) 
						{
							if (!((*m_pfnFilter)(pobEntity)))
								return;
						}
					};

					// Otherwise, register this hit						
					hkShapeRayCastCollectorOutput& dest = m_rayHit;
					dest = output;
					shapeKeysFromCdBody( m_rayHit.m_shapeKeys, hkShapeRayCastOutput::MAX_HIERARCHY_DEPTH, cdBody );
					m_rayHit.m_rootCollidable = cdBody.getRootCollidable();
					m_earlyOutHitFraction = output.m_hitFraction;
				}
			}
		
			const CEntity* m_pobIgnoreEntity;
			CastRayFilter* m_pfnFilter;
	};


	/***************************************************************************************************
	*
	*	CLASS			hkClosestRayHitCollectorEntityGroupFilter
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	class hkClosestRayHitCollectorEntityGroupFilter : public hkClosestRayHitCollector
	{
		public:

			hkClosestRayHitCollectorEntityGroupFilter (ntstd::List<CEntity*>& obIgnoreEntityList, CastRayFilter *pfnFilter)
			{
				m_pobIgnoreEntityList=&obIgnoreEntityList;
				m_pfnFilter = pfnFilter;
			}

		protected:
			virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& output )
			{
				if ( output.m_hitFraction < m_rayHit.m_hitFraction)
				{
					// Check to see if the ray hit the ignore entity

					hkRigidBody* obRB = hkGetRigidBody(cdBody.getRootCollidable());
					CEntity* pobEntity = 0;

					if(obRB)
						pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
						
					else {
						hkPhantom* obPH = hkGetPhantom(cdBody.getRootCollidable());
						if(obPH)
							pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
							
					};
					
					if (pobEntity)
					{
						if (m_pobIgnoreEntityList)
						{
							for(ntstd::List<CEntity*>::const_iterator obIt=(*m_pobIgnoreEntityList).begin(); obIt!=(*m_pobIgnoreEntityList).end(); ++obIt)
							{
								if (pobEntity == (*obIt))
									return;
							}
						}

						if (m_pfnFilter) 
						{
							if (!((*m_pfnFilter)(pobEntity)))
								return;
						}
					};

					// Otherwise, register this hit	
					hkShapeRayCastCollectorOutput& dest = m_rayHit;
					dest = output;
					shapeKeysFromCdBody( m_rayHit.m_shapeKeys, hkShapeRayCastOutput::MAX_HIERARCHY_DEPTH, cdBody );
					m_rayHit.m_rootCollidable = cdBody.getRootCollidable();
					m_earlyOutHitFraction = output.m_hitFraction;
				}
			}
		
			ntstd::List<CEntity*>* m_pobIgnoreEntityList;
			CastRayFilter* m_pfnFilter;
	};
#endif

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::TraceLine
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	bool CPhysicsWorld::TraceLine (	const CPoint& obStart,
									const CPoint& obEnd,
									const CEntity* pobIgnoreEntity,	
									TRACE_LINE_QUERY& stQuery,
									Physics::RaycastCollisionFlag obFlag,
									CastRayFilter *pfnFilter, 
									hkAabbPhantom * phantom) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		//ntAssert((obEnd-obStart).Length()>EPSILON);

		// Build up a input struct to describe the ray
		hkWorldRayCastInput strRayCastInput;
		strRayCastInput.m_from(0) = obStart.X();
		strRayCastInput.m_from(1) = obStart.Y();
		strRayCastInput.m_from(2) = obStart.Z();
		strRayCastInput.m_to(0) = obEnd.X();
		strRayCastInput.m_to(1) = obEnd.Y();
		strRayCastInput.m_to(2) = obEnd.Z();
		strRayCastInput.m_filterInfo = obFlag.base;

		// Create an item with which to store out output
		hkClosestRayHitCollectorEntityFilter obResults(pobIgnoreEntity,pfnFilter);

		// Dip in our tackle, see what bites
		{
			Physics::CastRayAccess mutex;
			if (phantom)
				phantom->castRay( strRayCastInput, obResults );
			else
			m_pobHavokWorld->castRay( strRayCastInput, obResults );
		}

	#ifdef _DRAW_RAYCAST_IN_HKVDB
		Physics::HavokDebugDraw::DrawRaycastInVisualiser(strRayCastInput, obResults);
	#endif

		// Now get the details of the closest surface if any and return them in a non Havok form
		if ( obResults.hasHit() )
		{
			// Get the useful data from the results - the surface normal
			const hkVector4& obSurfaceNormal = obResults.getHit().m_normal;
			float fHitFraction = obResults.getHit().m_hitFraction;

			// Fill in information for the query
			stQuery.fFraction=fHitFraction;
			stQuery.obIntersect=CPoint::Lerp(obStart,obEnd,fHitFraction);
			stQuery.obNormal=CDirection(obSurfaceNormal.getSimdAt( 0 ), obSurfaceNormal.getSimdAt( 1 ), obSurfaceNormal.getSimdAt( 2 ) );

			stQuery.obCollidedFlag.base = obResults.getHit().m_rootCollidable->getCollisionFilterInfo();

			hkRigidBody* obRB = hkGetRigidBody(obResults.getHit().m_rootCollidable);
			stQuery.pobEntity = 0;

			if(obRB) {
				stQuery.pobEntity			= (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			} else {
				hkPhantom* obPH = hkGetPhantom(obResults.getHit().m_rootCollidable);
				if(obPH)
					stQuery.pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();					

			};

			if (stQuery.ppobPhysicsMaterial)
			{
				*stQuery.ppobPhysicsMaterial = Tools::GetMaterial(obResults.getHit());
			};
			
			// An item has been found succesfully
			return true;
		}


		stQuery.fFraction=0;
		stQuery.obIntersect.Clear();
		stQuery.obNormal.Clear();

		stQuery.pobEntity = 0;
#else
		UNUSED( obStart ); UNUSED( obEnd ); UNUSED( pobIgnoreEntity ); UNUSED( stQuery ); UNUSED( obFlag ); UNUSED( pfnFilter );
#endif
		// We did not find a surface
		return false;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::TraceLine
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	bool CPhysicsWorld::TraceLine (const CPoint& obStart,
								   const CPoint& obEnd,
								   ntstd::List<CEntity*>& obIgnoreList,
								   TRACE_LINE_QUERY& stQuery, 
								   Physics::RaycastCollisionFlag obFlag, 
								   CastRayFilter *pfnFilter,								 
								   hkAabbPhantom * phantom) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert((obEnd-obStart).Length()>EPSILON);

		// Build up a input struct to describe the ray
		hkWorldRayCastInput strRayCastInput;
		strRayCastInput.m_from(0) = obStart.X();
		strRayCastInput.m_from(1) = obStart.Y();
		strRayCastInput.m_from(2) = obStart.Z();
		strRayCastInput.m_to(0) = obEnd.X();
		strRayCastInput.m_to(1) = obEnd.Y();
		strRayCastInput.m_to(2) = obEnd.Z();
		strRayCastInput.m_filterInfo = obFlag.base;

		// Create an item with which to store out output
		hkClosestRayHitCollectorEntityGroupFilter obResults(obIgnoreList,pfnFilter);

		// Dip in our tackle, see what bites
		{
			Physics::CastRayAccess mutex;
			if (phantom)
				phantom->castRay( strRayCastInput, obResults );
			else
			m_pobHavokWorld->castRay( strRayCastInput, obResults );
		}

	#ifdef _DRAW_RAYCAST_IN_HKVDB
		Physics::HavokDebugDraw::DrawRaycastInVisualiser(strRayCastInput, obResults);
	#endif

		// Now get the details of the closest surface if any and return them in a non Havok form
		if ( obResults.hasHit() )
		{
			// Get the useful data from the results - the surface normal
			const hkVector4& obSurfaceNormal = obResults.getHit().m_normal;
			float fHitFraction = obResults.getHit().m_hitFraction;

			// Fill in information for the query
			stQuery.fFraction=fHitFraction;
			stQuery.obIntersect=CPoint::Lerp(obStart,obEnd,fHitFraction);
			stQuery.obNormal=CDirection(obSurfaceNormal.getSimdAt( 0 ), obSurfaceNormal.getSimdAt( 1 ), obSurfaceNormal.getSimdAt( 2 ) );

			stQuery.obCollidedFlag.base = obResults.getHit().m_rootCollidable->getCollisionFilterInfo();

			hkRigidBody* obRB = hkGetRigidBody(obResults.getHit().m_rootCollidable);
			stQuery.pobEntity = 0;

			if(obRB) {
				stQuery.pobEntity			= (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
				
			} else {
				hkPhantom* obPH = hkGetPhantom(obResults.getHit().m_rootCollidable);
				if(obPH)
					stQuery.pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
					
			};

			if (stQuery.ppobPhysicsMaterial)
			{
				*stQuery.ppobPhysicsMaterial = Tools::GetMaterial(obResults.getHit());
			};
			
			// An item has been found succesfully
			return true;
		}


		stQuery.fFraction=0;
		stQuery.obIntersect.Clear();
		stQuery.obNormal.Clear();

		stQuery.pobEntity = 0;
#else
		UNUSED( obStart ); UNUSED( obEnd ); UNUSED( obIgnoreList ); UNUSED( stQuery ); UNUSED( obFlag ); UNUSED( pfnFilter );
#endif
		// We did not find a surface
		return false;
	}

	// filter to avoid character in character controllers state... 
	class AvoidCharacterControllersForCastRay : public CastRayFilter
	{
	public:		
		bool operator() (CEntity *pobEntity) const
		{
			// no character controllers
            if (pobEntity->IsCharacter())
			{
				Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*)
					pobEntity->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

				return pobCharacterState->IsCharacterControllerActive() ? false : true;
			}
			return true;
		}
	};

	bool CPhysicsWorld::TraceLineExactCharacters( const CPoint& obStart, const CPoint& obEnd, float characterOverlap, 
		TRACE_LINE_QUERY& stQuery, Physics::RaycastCollisionFlag obFlag, CastRayFilter *pfnFilter) const
	{
		// ----- Collision test -----

		// We want to do collision between the ray and scene. Character in scene could
		// be represented even like a ragdoll or like a capsule (character controller representation). 
		// Capsule representation of character is quite unexact: It doesn't cover the whole character and further 
		// it is impossible to determine which part of character was hit from intersection with capsule. 
		// That's why we will replace characters capsule by characters ragdoll and do collision 
		// on it. 
		// 
		// Steps: 
		// * Find every object in proximity of the ray (even if trajectory is not
        //		intersecting character capsule it can still intersect ragdoll) 
		// * Make raycast on found objects
		// * For all characters represented by capsule, do raycast on their ragdolls
		// * Report found contact
				
		// Find colliding bodies, use hkAabbPhantom for that... 
		CPoint minP( obStart);
		minP = minP.Min(obEnd);
		minP -= (CPoint(characterOverlap, characterOverlap, characterOverlap)); 

		CPoint maxP( obStart);
		maxP = maxP.Max(obEnd);
		maxP += (CPoint(characterOverlap, characterOverlap, characterOverlap));

		hkAabb overlapArea(MathsTools::CPointTohkVector(minP), MathsTools::CPointTohkVector(maxP));

		Physics::EntityCollisionFlag obPhantomFlag; obPhantomFlag.base = 0;
		obPhantomFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT; // I do not have proper bit for this one, 
		                                                     // use LARGE_INTERACTABLE_BIT, because every body collide 
		                                                     // with this one
		obPhantomFlag.flags.i_collide_with = obFlag.flags.i_collide_with;

		hkAabbPhantom * overlapPhantom = HK_NEW hkAabbPhantom(overlapArea, obPhantomFlag.base); 
		CPhysicsWorld::Get().AddPhantom(overlapPhantom);
		overlapPhantom->removeReference(); 

		// raycast in phantom... we need to avoid cast with character controller
		AvoidCharacterControllersForCastRay filter; 

		Physics::RaycastCollisionFlag dummy; // it is not used in function in fact
		dummy.base = 0; 

		bool collide; 
		if (pfnFilter)
		{
			GroupCastRayFilter grpFilter;
			grpFilter.AddFilter(pfnFilter);
			grpFilter.AddFilter(&filter);

			collide = CPhysicsWorld::Get().TraceLine(obStart,obEnd,NULL,stQuery,dummy, &grpFilter, overlapPhantom);
		}
		else
			collide = CPhysicsWorld::Get().TraceLine(obStart,obEnd,NULL,stQuery,dummy, &filter, overlapPhantom);
	
		// aditional raycast on ragdolls for character controllers 
		hkArray< hkCollidable * > & overlaps = overlapPhantom->getOverlappingCollidables();

		// collect all characters in character controller starte
		ntstd::Set< CEntity* > characterControllers; 
		for(int i = 0; i < overlaps.getSize(); i++)
		{
			Physics::EntityCollisionFlag obInfo;
			obInfo.base = overlaps[i]->getCollisionFilterInfo();

			if ((obInfo.flags.i_am == CHARACTER_CONTROLLER_PLAYER_BIT) ||
				(obInfo.flags.i_am == CHARACTER_CONTROLLER_ENEMY_BIT))
			{
				// get entity
				CEntity * ent = Physics::Tools::GetEntity(*overlaps[i]);
				if (ent && (!pfnFilter || (*pfnFilter)(ent)))
					characterControllers.insert(ent);
			}
		}

		CPhysicsWorld::Get().RemovePhantom(overlapPhantom);

	    // redo raycast for character controllers
		for(ntstd::Set< CEntity* >::iterator it = characterControllers.begin(); it != characterControllers.end(); it++)
		{
			Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) 
					(*it)->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

			TRACE_LINE_QUERY ragQuery;
			Physics::RaycastCollisionFlag obFlag;
			obFlag.base = 0;
			obFlag.flags.i_am = COLLISION_ENVIRONMENT_BIT;
			obFlag.flags.i_collide_with = Physics::RAGDOLL_BIT;

			bool bCollided = pobCharacterState->CastRayOnRagdoll( obStart, obEnd, obFlag, ragQuery );
			if (bCollided && (!collide || ragQuery.fFraction < stQuery.fFraction))
			{
				stQuery = ragQuery;
				collide = true; 
			}
		}

		return collide;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsWorld::FindIntersectingEntities
	*
	*	DESCRIPTION		Find all the entities that are intersected within a radius from a point - 
	*					returns true if there is anything in the list
	*
	***************************************************************************************************/
	bool CPhysicsWorld::FindIntersectingRigidEntities(	const CPoint&		obCentre, 
														float				fRadius, 
														ntstd::List<CEntity*>&	obIntersecting ) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// Quick check on our data
		if ( fRadius <= 0.0f )
			return false;

		// Make sure the list is empty
		ntAssert( obIntersecting.size() == 0 );

		// Translate our centre point to havok data
		hkVector4 obPosition( obCentre.X(), obCentre.Y(), obCentre.Z() );
		
		// Create a havok collidable sphere at this point
		hkSphereShape obSphereShape( fRadius );
		hkTransform obLocalTransform( hkQuaternion::getIdentity(), obPosition );
		hkCollidable obCollidable( &obSphereShape, &obLocalTransform );

		// Get all shapes which are penetrating the collidable
		hkAllCdBodyPairCollector obCollector;
		Physics::ReadAccess mutex;
		m_pobHavokWorld->getPenetrations( &obCollidable, ( hkCollisionInput& )*m_pobHavokWorld->getCollisionInput(), obCollector );
		const hkArray<hkRootCdBodyPair>& obHits = obCollector.getHits();

		// Loop through all the items that penetrated
		for ( int iHit=0; iHit < obHits.getSize(); iHit++ )
		{
			// Get the shape that collided
			const hkRootCdBodyPair* pobCurrentPair = &obHits[iHit];
			const hkCollidable* pobCollided = pobCurrentPair->m_rootCollidableB;

			// If we have some data then...
			if ( pobCollided->getCollisionFilterInfo() )
			{
				// If we have a rigid body...
				hkRigidBody* obRB = hkGetRigidBody( pobCollided );
				if ( obRB )
				{
					// Try to get an entity pointer
					CEntity* pobEntity = static_cast<CEntity*>( obRB->getProperty( Physics::PROPERTY_ENTITY_PTR ).getPtr() );

					// If we can get an entity pointer then add it to the list
					if ( pobEntity )
						obIntersecting.push_back( pobEntity );
				}
			}
		}

		// Indicate whether we have useful results
		return ( obIntersecting.size() > 0 );
#else
		UNUSED( obCentre ); UNUSED( fRadius ); UNUSED( obIntersecting ); 
		return false;
#endif
	}

	const hkVector4 &CPhysicsWorld::GetGravity() const
	{
		Physics::ReadAccess mutex;
		return m_pobHavokWorld->getGravity();
	}

	hkTime CPhysicsWorld::GetFrameTime() const
	{
		Physics::ReadAccess mutex;
		//return m_pobHavokWorld->getFrameTime();
		return m_pobHavokWorld->getCurrentTime();
	}

	void CPhysicsWorld::UpdateCollisionFilterOnEntity( hkEntity *entity, hkUpdateCollisionFilterOnEntityMode updateMode, hkUpdateCollectionFilterMode updateShapeCollectionFilter )
	{
		Physics::WriteAccess mutex;
		m_pobHavokWorld->updateCollisionFilterOnEntity( entity, updateMode, updateShapeCollectionFilter );
	}

	void CPhysicsWorld::UpdateCollisionFilterOnPhantom( hkPhantom *phantom, hkUpdateCollectionFilterMode updateShapeCollectionFilter )
	{
		Physics::WriteAccess mutex;
		m_pobHavokWorld->updateCollisionFilterOnPhantom( phantom, updateShapeCollectionFilter );
	}

	void CPhysicsWorld::AddEntity( hkEntity *entity, hkEntityActivation initialActivationState )
	{
		Physics::WriteAccess mutex;
		m_pobHavokWorld->addEntity( entity, initialActivationState );
	}

	void CPhysicsWorld::AddEntityBatch( hkEntity * const *entityBatch, int numEntities, hkEntityActivation initialActivationState )
	{
		Physics::WriteAccess mutex;
		m_pobHavokWorld->addEntityBatch( entityBatch, numEntities, initialActivationState );
	}

	hkPhantom *CPhysicsWorld::AddPhantom( hkPhantom *phantom )
	{
		Physics::WriteAccess mutex;
		return m_pobHavokWorld->addPhantom( phantom );
	}

	void CPhysicsWorld::RemovePhantom( hkPhantom *phantom )
	{
		Physics::WriteAccess mutex;
		m_pobHavokWorld->removePhantom( phantom );
	}

	hkAction *CPhysicsWorld::AddAction( hkAction *action )
	{
		Physics::WriteAccess mutex;
		return m_pobHavokWorld->addAction( action );
	}

	void CPhysicsWorld::RemoveAction( hkAction *action )
	{
		Physics::WriteAccess mutex;
		m_pobHavokWorld->removeAction( action );
	}

	hkBool CPhysicsWorld::RemoveConstraint( hkConstraintInstance *constraint )
	{
		Physics::WriteAccess mutex;
		return m_pobHavokWorld->removeConstraint( constraint );
	}

	hkConstraintInstance *CPhysicsWorld::AddConstraint( hkConstraintInstance *constraint )
	{
		Physics::WriteAccess mutex;
		return m_pobHavokWorld->addConstraint( constraint );
	}

	void CPhysicsWorld::GetPenetrations( const hkCollidable *collA, const hkCollisionInput &input, hkCdBodyPairCollector &collector )
	{
		Physics::ReadAccess mutex;
		m_pobHavokWorld->getPenetrations( collA, input, collector );
	}

	void CPhysicsWorld::GetClosestPoints( const hkCollidable *collA, const hkCollisionInput &input, hkCdPointCollector &collector )
	{
		Physics::ReadAccess mutex;
		m_pobHavokWorld->getClosestPoints( collA, input, collector );
	}

	hkProcessCollisionInput *CPhysicsWorld::GetCollisionInput() const
	{
		Physics::ReadAccess mutex;
		return m_pobHavokWorld->getCollisionInput();
	}

	bool CPhysicsWorld::IsBoxIntersectingStaticGeometry(const CPoint& obPosition, const CPoint& obHalfExtents, float fYRotation)
	{
		#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		
		// Create a havok collidable sphere at this point
		hkBoxShape obBoxShape( hkVector4(obHalfExtents.X(),obHalfExtents.Y(),obHalfExtents.Z()) );
		hkRotation obRotation;
		obRotation.setAxisAngle(hkVector4(0.0,1.0,0.0,0.0),fYRotation);
		hkTransform obLocalTransform( hkQuaternion(obRotation), Physics::MathsTools::CPointTohkVector(obPosition) );
		hkCollidable obCollidable( &obBoxShape, &obLocalTransform );

		EntityCollisionFlag obFlag;
		obFlag.base = 0; 
		obFlag.flags.i_am = LARGE_INTERACTABLE_BIT; // hmmm I need something with what everybody collides
		                                            // LARGE_INTERACTABLE_BIT has such a property at the moment
		                                            // better will be to have some flag for it in future... 
		obFlag.flags.i_collide_with = LARGE_INTERACTABLE_BIT | SMALL_INTERACTABLE_BIT;

		obCollidable.setCollisionFilterInfo(obFlag.base);

		// Get all shapes which are penetrating the collidable
		hkAllCdBodyPairCollector obCollector;
		Physics::ReadAccess mutex;
		m_pobHavokWorld->getPenetrations( &obCollidable, ( hkCollisionInput& )*m_pobHavokWorld->getCollisionInput(), obCollector );
		const hkArray<hkRootCdBodyPair>& obHits = obCollector.getHits();

		// Loop through all the items that penetrated
		for ( int iHit=0; iHit < obHits.getSize(); iHit++ )
		{
			// Get the shape that collided
			const hkRootCdBodyPair* pobCurrentPair = &obHits[iHit];
			const hkCollidable* pobCollided = pobCurrentPair->m_rootCollidableB;
			
			// If we have a rigid body...
			hkRigidBody* pobRB = hkGetRigidBody( pobCollided );		
			if (pobRB && pobRB->isFixedOrKeyframed())
			{
				//CEntity * pobEntity = (CEntity*) pobRB->getProperty(PROPERTY_ENTITY_PTR).getPtr();
				//ntPrintf("IsBoxIntersecting hit %s, type %i.\n", pobEntity->GetName().c_str(),pobEntity->GetEntType());
				return true; 				
			}			
		}

		return false;
#else
		UNUSED( obCentre ); UNUSED( fRadius );  
		return false;
#endif
	}

	int CPhysicsWorld::GetNumberOfStepsWorldWillTakeNextFrame( const hkWorld* world, hkReal frameDeltaTime, hkReal physicsDeltaTime )
	{
		// Copy all world timing values
		hkReal timeOfNextFrame = world->getCurrentTime();
		hkReal timeOfNextPsi = world->getCurrentPsiTime();

		// Calculate the number of frames the world will step based on code within stepAsyncronously
		hkInt32 numFrames = 0;
		{
			timeOfNextFrame += frameDeltaTime;

			while( 1 )
			{
				// Sync physics time
				if( hkMath::fabs(timeOfNextFrame - timeOfNextPsi) < 0.01f * physicsDeltaTime && frameDeltaTime / physicsDeltaTime > 0.1f )
				{
					timeOfNextFrame = timeOfNextPsi;
				}

				if( timeOfNextPsi >= timeOfNextFrame )
				{
					break;
				}

				//timeOfLastPsi = timeOfNextPsi;
				timeOfNextPsi += physicsDeltaTime;

				//currentTime = timeOfLastPsi;

				numFrames++;
			}
		}

		return numFrames;
	}
}
