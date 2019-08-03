/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkbase/hkBase.h>
#include <hkbase/stream/hkIstream.h>
#include <hkbase/memory/hkLocalBuffer.h>
#include <hkbase/debugutil/hkCheckDeterminismUtil.h>



hkCheckDeterminismUtil g_checkDeterminismUtil;



hkCheckDeterminismUtil::hkCheckDeterminismUtil()
{
	m_inputStream  = HK_NULL;
	m_outputStream = HK_NULL;
}


hkCheckDeterminismUtil::~hkCheckDeterminismUtil()
{
	// Check for hkCheckDeterminismUtil::finish() is called
	HK_ASSERT(0xaf34aee2, !m_inputStream );
	HK_ASSERT(0xaf34aee3, !m_outputStream );
}


void hkCheckDeterminismUtil::startWriteMode(const char* filename)
{
	HK_ASSERT2(0xaf36affe, !m_outputStream, "You cannot switch to READ mode without calling finish() first.");

	m_outputStream = new hkOstream(filename);

	HK_ASSERT2(0xaf36affd, m_outputStream->isOk(), "Output file could not be opened.");
}


void hkCheckDeterminismUtil::startCheckMode(const char* filename)
{
	HK_ASSERT2(0xaf36affe, !m_outputStream, "You cannot switch to READ mode without calling finish() first.");

	m_inputStream = new hkIstream(filename);

	if ( !m_inputStream->isOk() )
	{
		HK_ASSERT2(0xaf36affe, false, "Input file not found.");
		finish();
	}
}


void hkCheckDeterminismUtil::checkImpl(const void* object, int size)
{
	if (!size)
	{
		return;
	}
	const char* sourceObject = (const char*)object;

	if ( m_outputStream )
	{
		m_outputStream->write(sourceObject, size);
		HK_ASSERT( 0xf0323446, m_outputStream->isOk() );
	}
	else if ( m_inputStream )
	{
		hkLocalBuffer<char> readBuffer(size);
		m_inputStream->read(readBuffer.begin(), size);
		HK_ASSERT( 0xf0323445, m_inputStream->isOk() );

			// some variables helping debugging
		const hkReal* of = (const hkReal*)object; of = of;
		const hkReal* cf = (const hkReal*)readBuffer.begin(); cf = cf;
		const void*const* oh = (const void*const*)object; oh = oh;
		const void*const* ch = (const void*const*)readBuffer.begin(); ch = ch;
		{
			for (int i=0; i<size; i++)
			{
				HK_ASSERT(0xaf335142, sourceObject[i] == readBuffer[i]);
			}
		}
	}
}


void hkCheckDeterminismUtil::finish()
{
	if ( m_inputStream )
	{
		// check whether we reached the end of the file
		{
			char tmpBuffer[4];	m_inputStream->read(tmpBuffer, 1);
			HK_ASSERT(0xad87b754, !m_inputStream->isOk());
		}

		delete m_inputStream;
		m_inputStream = HK_NULL;
	}
	else if ( m_outputStream )
	{
		m_outputStream->flush();
		delete m_outputStream;
		m_outputStream = HK_NULL;
	}
}


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
