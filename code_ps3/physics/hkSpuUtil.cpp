/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include "physics/havokincludes.h"
#include "hkSpuUtil.h"

#define MAX_NUM_SPU_THREADS 6

#ifdef USE_HAVOK_ON_SPUS

	#include <sys/spu_initialize.h>
	#include <sys/spu_thread.h>
	#include <sys/ppu_thread.h>
	#include <sys/event.h>
	#include <sys/timer.h>
	#include <cell/spurs.h>
	#include <sys/spu_initialize.h>
	#include <sys/spu_thread.h>
	#include <sys/spu_thread_group.h>
	#include <sys/spu_utility.h>
	#include <sys/event.h>
	#include <cell/atomic.h>


	#include <hkbase/thread/hkSpuUtils.h>
	#include <sys/spu_image.h>


	//#include <demos/Utilities/Thread/spu_printf_util.h>
	//#include <demos/Utilities/Thread/spurs_printf_util.h>


	static inline int get_secondary_ppu_thread_priority(int *prio)
	{
		int ret;
		sys_ppu_thread_t my_ppu_thred_id;
		ret = sys_ppu_thread_get_id(&my_ppu_thred_id);
		if(ret){ return ret; }
		ret = sys_ppu_thread_get_priority(my_ppu_thred_id, prio);
		if(ret){ return ret; }

		*prio = *prio - 1;
		return 0;
	}

	#define	SPURS_THREAD_GROUP_PRIORITY 250
	#define SPURS_HANDLER_THREAD_PRIORITY 1

	#define INVALID_GROUP_ID 0xffffffff
	#define THREAD_GROUP_PRIORITY 100



	hkSpuUtil::hkSpuUtil()
	{
		// SPURS
		m_spursElf = HK_NULL;

		// Thread groups
		m_groupId = INVALID_GROUP_ID;
		m_spuThreadCounter = 0;
		m_numSpuThreads = -1;
	}

	hkSpuUtil::~hkSpuUtil()
	{
		if ( m_spursElf != HK_NULL )
		{
			quitSpurs();
		}
		else if (m_groupId != INVALID_GROUP_ID )
		{
			quitSpuThreadGroup();
		}
	}

	void hkSpuUtil::initSpurs(CellSpurs * spurs, void* elf)
	{
		HK_ASSERT2( 0, m_groupId == INVALID_GROUP_ID, "You cannot initialize spurs if you have initialized thread groups");	
		HK_ASSERT( 0, m_spursElf == HK_NULL );
		HK_ASSERT( 0, elf != HK_NULL );

		m_spursElf = elf;

		int ret;

		// Spurs is allready created. Use one created by Exec
		/*int ppu_thr_prio;

		ret = get_secondary_ppu_thread_priority(&ppu_thr_prio);
		if (ret)
		{
			HK_ERROR(0x73e432a1, "get PPU thread priority failed :" << ret);
		}

		//
		// Create spurs
		//
		m_spurs = (CellSpurs*)hkAlignedAllocate<char> ( CELL_SPURS_ALIGN, CELL_SPURS_SIZE , HK_MEMORY_CLASS_DEMO );
		ret = cellSpursInitialize (m_spurs, numSpus, SPURS_THREAD_GROUP_PRIORITY, ppu_thr_prio, SPURS_HANDLER_THREAD_PRIORITY);
		if (ret)
		{
			HK_ERROR(0x73e432a2, "cellSpursInitialize failed :" << ret);
		}*/

		//
		// Register taskset
		//
		//hkSpuParams param __attribute__((aligned(128)));
		

		m_taskset = (CellSpursTaskset*)hkAlignedAllocate<char> ( CELL_SPURS_TASKSET_ALIGN, CELL_SPURS_TASKSET_SIZE , HK_MEMORY_CLASS_DEMO );
		uint8_t prios[8] =  { 1, 1, 1, 1, 1, 1, 1, 1 };
		//ret = cellSpursCreateTaskset( m_spurs, m_taskset, (uint64_t)&param, prios, 1 );
		ret = cellSpursCreateTaskset( spurs, m_taskset, 0, prios, 1 );
		if (ret)
		{
			HK_ERROR(0x73e432a3, "cellSpursCreateTaskset failed :" << ret);
		}

	}

	void hkSpuUtil::startSpursTask( hkSpuParams& params )
	{
		HK_ASSERT2(0x9545ff34, m_spursElf != HK_NULL, "You must call startSpurs() before calling startTask()" );
		// create task (without context)
		CellSpursTaskId tid;
		int ret = cellSpursCreateTask(m_taskset, &tid, m_spursElf,
								(void*)0, 0, (CellSpursTaskLsPattern*)0, (CellSpursTaskArgument*)(&params) );
		if (ret)
		{
			HK_ERROR(0x73e432a5, "cellSpursCreateTask failed :" << ret);
		}
	}


	void hkSpuUtil::quitSpurs()
	{
		if (m_spursElf == HK_NULL)
			return;

		HK_ASSERT2(0x9545fe34, m_spursElf != HK_NULL, "You must call initSpurs() before calling quitSpurs()" );

		// Shutdown taskset
		int ret = cellSpursShutdownTaskset( m_taskset );
		if (ret)
		{
			HK_ERROR(0x73e432b4, "cellSpursShutdownTaskset failed :" << ret);
		}

		// Join taskset
		ret = cellSpursJoinTaskset( m_taskset );
		if (ret)
		{
			HK_ERROR(0x73e432b2, "cellSpursJoinTaskset failed :" << ret);
		}

		// free resources if necessary
		/*ret = cellSpursFinalize ( m_spurs );
		if (ret)
		{
			HK_ERROR(0x73e432b3, "cellSpursFinalize failed :" << ret);
		}*/
		m_spursElf = HK_NULL;
	}


	//
	// SPU threads
	//


	void hkSpuUtil::initSpuThreadGroup( int numSpus, char* spuProg )
	{
		HK_ASSERT(0, m_groupId == INVALID_GROUP_ID );
		HK_ASSERT(0, m_spursElf == HK_NULL );
		HK_ASSERT(0, numSpus <= MAX_NUM_SPU_THREADS);


		HK_ON_DEBUG(int ret = )sys_spu_image_open(&m_spuThreadImg, spuProg);
		HK_ASSERT2(0x102346fa, ret == CELL_OK, "sys_spu_thread_elf_loader failed:" << ret );

		m_numSpuThreads	= numSpus;


		//
		// Create an SPU thread with id m_groupId
		// 
		{
			const char* group_name = "HavokGroup";
			sys_spu_thread_group_attribute_t group_attr;	// SPU thread m_groupId attribute

			group_attr.name = group_name;
			group_attr.nsize = hkString::strLen(group_attr.name) + 1;
			group_attr.type = SYS_SPU_THREAD_GROUP_TYPE_NORMAL;
			HK_ON_DEBUG(int ret = )sys_spu_thread_group_create(&m_groupId, numSpus, THREAD_GROUP_PRIORITY, &group_attr);

			HK_ASSERT2(0x10294fa6,ret == CELL_OK, "sys_spu_thread_group_create failed:" << ret );
		}



	#if defined(HK_ENABLED_SPU_DEBUG_PRINTFS)
			//ret = spu_printf_service_initialize (10, numSpus);
			HK_ASSERT2(0xf2345612, ret == CELL_OK, "spu_printf_service_initialize failed:" << ret );
	#endif

	}

	void hkSpuUtil::startSpuThread( hkSpuParams& params )
	{
		HK_ASSERT(0, m_groupId != INVALID_GROUP_ID );
		HK_ASSERT(0, m_spuThreadCounter < m_numSpuThreads );


	//	static int spu_num[MAX_NUM_SPU_THREADS] = { 0, 1, 2, 3, 4, 5 };

		char threadName[100];
		hkString::sprintf( threadName, "SPU Thread %d", m_spuThreadCounter );


		sys_spu_thread_attribute_t threadAttr;	// SPU thread attribute 

		threadAttr.name = threadName;
		threadAttr.nsize = hkString::strLen(threadName) + 1;
		threadAttr.option = SYS_SPU_THREAD_OPTION_NONE;

		sys_spu_thread_argument_t threadParams;

		threadParams.arg1 = SYS_SPU_THREAD_ARGUMENT_LET_64((uint64_t)params.m_param0 );
		threadParams.arg2 = SYS_SPU_THREAD_ARGUMENT_LET_64((uint64_t)params.m_param1 );
		threadParams.arg3 = SYS_SPU_THREAD_ARGUMENT_LET_64((uint64_t)params.m_param2 );
		threadParams.arg4 = SYS_SPU_THREAD_ARGUMENT_LET_64((uint64_t)params.m_param3 );

		sys_spu_thread_t thread;

		HK_ON_DEBUG(int ret = )sys_spu_thread_initialize(&thread, m_groupId, m_spuThreadCounter, &m_spuThreadImg, &threadAttr, &threadParams);
		HK_ASSERT2(0x675857ed,  ret == CELL_OK, "sys_spu_thread_initialize failed: " << ret);

		// If this is the last thread, start the thread group

		m_spuThreadCounter++;
		if (m_spuThreadCounter == m_numSpuThreads )
		{
			HK_ON_DEBUG(int ret = )sys_spu_thread_group_start(m_groupId);
			HK_ASSERT2(0x67b8a7ed,  ret == CELL_OK, "sys_spu_thread_group_start failed: " << ret);
		}

	}

	void hkSpuUtil::quitSpuThreadGroup()
	{
		HK_ASSERT(0, m_groupId != INVALID_GROUP_ID );
		HK_ASSERT(0, m_spuThreadCounter == m_numSpuThreads );

		int ret;

		ret = sys_spu_thread_group_terminate( m_groupId, 0 );
		HK_ASSERT2(0x67b8a7e1,  ret == CELL_OK, "sys_spu_thread_group_terminate failed: " << ret);

		int cause, status;
		ret = sys_spu_thread_group_join(m_groupId, &cause, &status);
		HK_ASSERT2(0x67b8a7e2,  ret == CELL_OK, "sys_spu_thread_group_join failed: " << ret);

		ret = sys_spu_thread_group_destroy(m_groupId);
		HK_ASSERT2(0x67b8a7e2,  ret == CELL_OK, "sys_spu_thread_group_destroy failed: " << ret);

		m_groupId = INVALID_GROUP_ID;
	}




#endif

/*
* Havok SDK - DEMO RELEASE, BUILD(#20060630)
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
