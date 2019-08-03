/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClassEnum.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkbase/stream/impl/hkCrc32StreamWriter.h>
#include <hkbase/stream/hkOArchive.h>

hkResult hkClassEnum::getNameOfValue( int val, const char** name ) const
{
	for(int i = 0; i < m_numItems; ++i )
	{
		if( m_items[i].getValue() == val )
		{
			*name = m_items[i].getName();
			return HK_SUCCESS;
		}
	}
	return HK_FAILURE;
}

hkResult hkClassEnum::getValueOfName( const char* name, int* val ) const
{
	for(int i = 0; i < m_numItems; ++i )
	{
		if( hkString::strCasecmp(name, m_items[i].getName()) == 0 )
		{
			*val = m_items[i].getValue();
			return HK_SUCCESS;
		}
	}
	return HK_FAILURE;
}

hkUint32 hkClassEnum::getSignature() const
{
	hkCrc32StreamWriter crc;
	writeSignature(&crc);
	return crc.getCrc32();
}

void hkClassEnum::writeSignature( hkStreamWriter* w ) const
{
    hkOArchive oa(w);
	oa.writeRaw( getName(), hkString::strLen(getName()) );
	int numItem = getNumItems();

	for( int j = 0; j < numItem; ++j )
	{
		const Item& item = getItem(j);
		oa.writeRaw( item.getName(), hkString::strLen(item.getName()));
		int val = item.getValue();
		oa.write32( val );
	}
	oa.write32( numItem );
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
