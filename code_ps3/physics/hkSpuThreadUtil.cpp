/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//*********************************************************************************************
//
//	NOTE:	I've changed this file substantially but it was originally a helper file
//			from the Havok SDK. [ARV].
//
//*********************************************************************************************
#include <hkbase\config\hkConfigVersion.h>

#if HAVOK_SDK_VERSION_MAJOR != 4
#include "physics/config.h"
#ifdef USE_HAVOK_ON_SPUS

#include <hkbase/hkBase.h>

#include "physics/hkSpuThreadUtil.h"

#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConstraint.h>
#include <hkbase/thread/hkSpuUtils.h>
#include <hkbase/hkBase.h>
#include <hkdynamics/world/simulation/multithreaded/job/hkJobQueue.h>
#include <hkconstraintsolver/solve/hkSolverInfo.h>
#include <hkbase/thread/hkThread.h>
#include "physics/hkPhysicsThreads.h"

#include <sys/spu_initialize.h>
#include <sys/spu_thread.h>
#include <sys/spu_thread_group.h>
#include <sys/spu_utility.h>
#include <sys/spu_image.h>
#include <sys/event.h>
#include <cell/atomic.h>

#include <sys/sys_time.h>

#define PRIORITY 100
static sys_spu_segment_t* segments = HK_NULL;	// The SPU segments 
static sys_spu_thread_attribute_t thread_attr;	// SPU thread attribute 
static sys_spu_image img;

#define INVALID_GROUP_ID 0xffffffff

hkSpuThreadUtil::hkSpuThreadUtil( char *spuProg )
{
	m_groupId = INVALID_GROUP_ID;
	//
	// Load the SPU ELF image, and construct the SPU segment information
	//

	int32_t ret = sys_spu_image_open( &img, spuProg );
	
	ntError_p( ret == CELL_OK, ("Failed to open Havok SPU elf %s - error reported as %i", spuProg, ret) );
	UNUSED( ret );
}

void hkSpuThreadUtil::startSpuThreads( int32_t numSpus, void *arg1, void *arg2, void *arg3, void *arg4 )
{
	// Destroy the old thread group if there
	if ( m_groupId != INVALID_GROUP_ID )
	{
		sys_spu_thread_group_terminate( m_groupId, 0 );
		sys_spu_thread_group_destroy( m_groupId );
		m_groupId = INVALID_GROUP_ID;
	}

	ntError( numSpus <= MAX_NUM_SPU_THREADS );
	m_numSpus = numSpus;

	// Need to comment these out as appropriate if more SPUs are reserved for other threading purposes.
	const char *thread_names[ MAX_NUM_SPU_THREADS ] = 
	{
//		"SPU Thread 0",		 Thread 0 is used by the ice job manager for music stuff.
		"SPU Thread 4",
		"SPU Thread 3",
		"SPU Thread 2",
		"SPU Thread 1"
	};
	
	//
	// Create an SPU thread m_groupId
	// 
	// The SPU thread m_groupId is initially in the NOT INITIALIZED state.
	//
	{
		const char* group_name = "HavokGroup";
		sys_spu_thread_group_attribute_t group_attr;	// SPU thread m_groupId attribute

		group_attr.name = group_name;
		group_attr.nsize = hkString::strLen( group_attr.name ) + 1;
		group_attr.type = SYS_SPU_THREAD_GROUP_TYPE_NORMAL;
		int32_t ret = sys_spu_thread_group_create( &m_groupId, numSpus, PRIORITY, &group_attr );

		ntError_p( ret == CELL_OK, ("Failed to create thread group for Havok - error code: %i", ret) );
		UNUSED( ret );
	}

	//
	// Initialize SPU threads in the SPU thread m_groupId.
	// This sample loads the same image to all SPU threads.
	//
	for ( int32_t i = 0; i < numSpus; i++ )
	{
		sys_spu_thread_argument_t thread_args;
		static int spu_num[ MAX_NUM_SPU_THREADS ] = { 4, 3, 2, 1 };	// Don't use SPU0.

		//
		// nsegs, segs and entry_point have already been initialized by 
		// sys_spu_thread_elf_loader().
		//
		thread_attr.name = thread_names[ i ];
		thread_attr.nsize = hkString::strLen( thread_names[ i ] ) + 1;
		thread_attr.option = SYS_SPU_THREAD_OPTION_NONE;

		thread_args.arg1 = SYS_SPU_THREAD_ARGUMENT_LET_64( (uint64_t)(uintptr_t)arg1 );
		thread_args.arg2 = SYS_SPU_THREAD_ARGUMENT_LET_64( (uint64_t)(uintptr_t)arg2 );
		thread_args.arg3 = SYS_SPU_THREAD_ARGUMENT_LET_64( (uint64_t)(uintptr_t)arg3 );
		thread_args.arg4 = SYS_SPU_THREAD_ARGUMENT_LET_64( (uint64_t)(uintptr_t)arg4 );

		int32_t ret = sys_spu_thread_initialize( &threads[ i ], m_groupId, spu_num[ i ], &img, &thread_attr, &thread_args );
		ntError_p( ret == CELL_OK, ("Failed to initialise Havok thread group with error %i", ret) );
		UNUSED( ret );
	}


	//
	// Start the SPU thread m_groupId
	//
	int32_t ret = sys_spu_thread_group_start( m_groupId );
	ntError_p( ret == CELL_OK, ("Failed to start Havok thread group with error %i", ret) );
	UNUSED( ret );
}

hkSpuThreadUtil::~hkSpuThreadUtil()
{
	if ( m_groupId != INVALID_GROUP_ID )
	{
		sys_spu_thread_group_terminate( m_groupId, 0 );
		sys_spu_thread_group_destroy( m_groupId );
		m_groupId = INVALID_GROUP_ID;
	}
	// The SPU segment information can be safely freed by now
	free( segments );
}

#endif
#endif

