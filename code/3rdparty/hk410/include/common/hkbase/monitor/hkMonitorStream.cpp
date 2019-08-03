/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/monitor/hkMonitorStream.h>

#ifdef HK_CONFIG_THREAD_USE_TLS
	HK_THREAD_LOCAL_IMPL( hkMonitorStream* ) hkMonitorStream::m_instance;
#else
	HK_THREAD_LOCAL_IMPL( hkMonitorStream ) hkMonitorStream::m_instance;
#endif

bool hkMonitorStream::m_debugTimersEnabled = true;

#if defined(HK_XBOX_USE_PERFLIB) && defined(HK_PLATFORM_XBOX360)
hkUlong g_hkXbox360PerfSampleRegAddr = 0x8FFF1230; // 0x30 = 6*8 == reg 6 == LHS cycles in a PB0T0 setup
#endif 

#if !defined(HK_PLATFORM_PS3SPU)
void hkMonitorStream::init() 
{
	HK_THREAD_LOCAL_INIT( hkMonitorStream, m_instance );
	hkMonitorStream& instance = getInstance();
	instance.m_isBufferAllocatedOnTheHeap = false;
	instance.m_start = HK_NULL;
	instance.m_capacity = HK_NULL;
	instance.m_end = HK_NULL;
	instance.m_capacityMinus16 = HK_NULL;
}

void hkMonitorStream::quit()
{
	if ( getStart() && isBufferAllocatedOnTheHeap() )
	{
		hkDeallocate(getStart());
	}

	// free the tls entry
	HK_THREAD_LOCAL_QUIT( m_instance );
}
#endif


#if !defined(HK_PLATFORM_PS3SPU)
void HK_CALL hkMonitorStream::resize( int newSize )
{

	if ( newSize == getCapacity() - getStart() )
	{
		return;
	}

	if (newSize > 0)
	{
		if ( getStart() && isBufferAllocatedOnTheHeap() )
		{
			hkDeallocate(getStart());
		}
		
		m_isBufferAllocatedOnTheHeap = true;
		m_start = hkAllocate<char>(newSize, HK_MEMORY_CLASS_MONITOR);
		m_end = m_start;
		m_capacity = m_start + newSize;
		m_capacityMinus16 = m_capacity - 32;
	}
	else
	{
		quit();
	}
}
#endif

void HK_CALL hkMonitorStream::setStaticBuffer( char* buffer, int bufferSize )
{
#if !defined(HK_PLATFORM_PS3SPU)
	if ( isBufferAllocatedOnTheHeap() )
	{
		resize(0);
	}
#endif

	m_isBufferAllocatedOnTheHeap = false;
	m_start = buffer ;
	m_end = buffer;
	m_capacity = m_start + bufferSize;
	m_capacityMinus16 = m_capacity - 32 ;
}

void HK_CALL hkMonitorStream::reset()
{
	m_end = m_start;
}


#if defined(HK_COMPILER_MWERKS)
#	pragma force_active on
#endif

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
