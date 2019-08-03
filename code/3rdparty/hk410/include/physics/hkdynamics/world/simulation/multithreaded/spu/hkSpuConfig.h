/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SPU_CONFIG_H
#define HK_SPU_CONFIG_H



namespace hkSpuBuildAccumulatorsJobDmaGroups
{
	enum
	{
		GET_ENTITY_BATCH_DMA_GROUP			= 0,
		GET_TASK_HEADER_DMA_GROUP			= 1,
		READ_MOTIONS_BASE_DMA_GROUP			= 2, // we are using double buffering, so we need to reserve 2 dma groups for this!
		WRITE_ACCUMULATORS_BASE_DMA_GROUP	= 4, // we are using double buffering, so we need to reserve 2 dma groups for this!
		// next free dma group to use: 6
	};
}

namespace hkSpuBuildJacobiansJobDmaGroups
{
	enum
	{
		NUM_ACCUMULATORS_DMA_GROUPS = 4
	};

	enum
	{
		GET_QUERYIN_DMA_GROUP							= 0,
		ACCUMULATORS_CACHE_BASE_DMA_GROUP				= 1, // reserve NUM_ACCUMULATORS_DMA_GROUPS groups!
		READ_RUNTIME_AND_ATOMS_DMA_GROUP				= 5, // we are using double buffering, so we need to reserve 2 dma groups for this!
		WRITE_BACK_CONTACT_CONSTRAINT_ATOM_DMA_GROUP	= 7,
		WRITE_BACK_RUNTIME_DMA_GROUP					= 8,
		// next free dma group to use: 9
	};
}

namespace hkSpuSolveJobDmaGroups
{
	enum
	{
		GET_TASK_HEADER_DMA_GROUP	= 0,
		// next free dma group to use: 1
	};
}

namespace hkSpuIntegrateMotionJobDmaGroups
{
	enum
	{
			// important: make sure to not share dma groups between the solveJob and the integrateMotionJob as the integrateMotionJob's actual
			//            implementation is also called from the solveJob!
		GET_TASK_HEADER_DMA_GROUP			= 1,
		GET_ENTITY_BATCH_DMA_GROUP			= 2,
		READ_WRITE_MOTIONS_BASE_DMA_GROUP	= 3, // we are using triple buffering, so we need to reserve 3 dma groups for this!
		// next free dma group to use: 6
	};
}



#define HK_SPU_TOTAL_BUFFER_SIZE 0x1c800

#define HK_MAX_NUM_CONTACT_POINTS_IN_CONSTRAINT_ON_SPU 128

#define HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE 4

// buffers size 
// - for jacobian writer
#define HK_SPU_JACOBIAN_WRITER_BASE_BUFFER_SIZE 512 
#define HK_SPU_JACOBIAN_WRITER_OVERFLOW_BUFFER_SIZE ( 48 * HK_MAX_NUM_CONTACT_POINTS_IN_CONSTRAINT_ON_SPU + 512)	// HK_SIZE_OF_JACOBIAN_LAA = 48 

// - for schemas writer
#define HK_SPU_SCHEMA_WRITER_BASE_BUFFER_SIZE 512
#define HK_SPU_SCHEMA_WRITER_OVERFLOW_BUFFER_SIZE (256 + 4 * HK_MAX_NUM_CONTACT_POINTS_IN_CONSTRAINT_ON_SPU) // only if we check after every contact point. Max num of consecutive schemas for a cp is 3. Max size of a schema is 48.

// cache size
// - for accumulator fetching (sizeof(hkVelocityAccumulator) == 128, cacheRow = 2* cacheLine, cacheLine = hkVelocityAccumulator + 2*int?)
#if defined (HK_DEBUG)
#	define HK_SPU_ACCUMULATOR_CACHE_NUM_CACHE_ROWS 16
#else
#	define HK_SPU_ACCUMULATOR_CACHE_NUM_CACHE_ROWS 64 // got memory ?? gimme more rows.
#endif
#define HK_SPU_ACCUMULATOR_CACHE_NUM_OVERFLOW_LINES 2 // we need min two overflow cache lines: because we keep 4 accumulators locked at a time; two may fit into one cache row, two may fit into the overflow buffer.

// dma buffer sizes
// - for constraintRuntime
#define HK_SPU_CONSTRAINT_RUNTIME_BUFFER_SIZE (sizeof(float) * 2 * 15) // sizeof(float) * 2 == sizeof(hkSolverResults)

// - for constraintAtoms
#define HK_SPU_CONSTRAINT_ATOM_BUFFER_SIZE ( 2 * HK_MAX_NUM_CONTACT_POINTS_IN_CONSTRAINT_ON_SPU * (sizeof(hkContactPointProperties) + sizeof(hkContactPoint)) + 256) // == max num cp's * size of (cp+cpp) + size of contactConstraint.m_info.  <todo> 0xbee



// info:
// sizeof(hkBuildJacobianTask) ~= 5100

#endif // HK_SPU_CONFIG_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20061017)
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
