/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Allocate a job off of a job list to this SPU
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_ALLOCATE_JOB_H
#define WWS_JOB_ALLOCATE_JOB_H

//--------------------------------------------------------------------------------------------------

//TEMP: this struct is for a local variable return value used only on the SPU
union SpuJobHeader
{
	struct
	{
		JobHeader	m_jobHeader;
		U64			m_jobIndex;
	};
	U64 m_u64[2];
	U32	m_u32[4];
	U16	m_u16[8];
	U8	m_u8[16];
} WWSJOB_ALIGNED( 16 );

//This same atomic buffer can be re-used by different systems.
//Data left in the buffer after a function returns cannot be assumed to still be present when
//the function is next called.  This should be fine for these usages of 128 byte atomic buffers.
union SharedTempAtomicBuffer
{
	qword				m_qwords[8];
	JobListHeader		m_jobListHeader;
	DependencyCounter	m_depCounters[8][4];
	JobHeader			m_jobHeader;
} WWSJOB_ALIGNED(128);

extern "C" SharedTempAtomicBuffer g_tempUsageAtomicBuffer WWSJOB_ALIGNED(128);	//May be shared by various systems, but data cannot be left in the buffer

//--------------------------------------------------------------------------------------------------

extern "C" SpuJobHeader WwsJob_AllocateJob( U32 eaWorkload, U32 spuNum );

//--------------------------------------------------------------------------------------------------

extern "C" void DecrementDependency( U32 mmaDependencyCounter );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_ALLOCATE_JOB_H */
