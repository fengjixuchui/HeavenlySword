/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/htl/hkSmallArray.h>

HK_COMPILE_TIME_ASSERT( sizeof(void*)==sizeof(char*) );

// hack to force this to be out of line
void hkSmallArrayUtil::_reserve(void* array, int numElem, int sizeElem)
{
	HK_ASSERT2(0x3b67c014, numElem >= 0, "Number of elements must not be negative");
	HK_ASSERT2(0x243bf8d1, sizeElem >= 0, "The size of an element must not be negative");

	typedef hkSmallArray<char> hkAnyArray;

	char* p = static_cast<char*>(hkThreadMemory::getInstance().allocateChunk(numElem*sizeElem + hkSizeOf(hkSmallArray<char>::Info), HK_MEMORY_CLASS_ARRAY));
	hkSmallArray<char>::Info* dest = reinterpret_cast<hkSmallArray<char>::Info*>(p);

	hkAnyArray* self = reinterpret_cast< hkAnyArray* >(array);
	dest->m_capacity = hkUint16(numElem);


	if ( self->m_info)
	{
		hkString::memCpy(dest->getData(), self->getData(), self->getSize()*sizeElem);
		dest->m_size     = hkUint16(self->getSize());
		hkThreadMemory::getInstance().deallocateChunk( (char*)self->m_info, self->getCapacity()*sizeElem + hkSizeOf(hkSmallArray<char>::Info), HK_MEMORY_CLASS_ARRAY);
	}
	else
	{
		dest->m_size = 0;
	}
	self->m_info = dest;	
}


// hack to force this to be out of line
void hkSmallArrayUtil::_reserveMore(void* array, int sizeElem)
{
	typedef hkSmallArray<char> hkAnyArray;
	hkAnyArray* self = reinterpret_cast< hkAnyArray* >(array);

	if ( self->m_info )
	{
		_reserve( array, self->m_info->m_size+1, sizeElem );
	}
	else
	{
		_reserve( array, 1, sizeElem );
	}
}

void hkSmallArrayUtil::_reduce(void* array, int sizeElem)
{
	typedef hkSmallArray<char> hkAnyArray;
	hkAnyArray* self = reinterpret_cast< hkAnyArray* >(array);
	if ( self->m_info->m_size == 0)
	{
		hkThreadMemory::getInstance().deallocateChunk( (char*)self->m_info, self->getCapacity()*sizeElem + hkSizeOf(hkSmallArray<char>::Info), HK_MEMORY_CLASS_ARRAY);
		self->m_info = HK_NULL;
		return;
	}

	HK_ASSERT2(0x5828d5f0, sizeElem >= 0, "The size of an element must not be negative");

	int newCapacity = self->getCapacity()/2;

	HK_ASSERT2(0x5828d5f1, newCapacity >= self->m_info->m_size, "Can't reduce array");
	
	char* p = static_cast<char*>(hkThreadMemory::getInstance().allocateChunk(newCapacity*sizeElem + sizeof(hkSmallArray<char>::Info), HK_MEMORY_CLASS_ARRAY));
	hkSmallArray<char>::Info* dest = reinterpret_cast<hkSmallArray<char>::Info*>(p);

	hkString::memCpy(dest->getData(), self->getData(), self->getSize()*sizeElem);
	dest->m_size = hkUint16(self->getSize());
	dest->m_capacity = hkUint16(newCapacity);

	hkThreadMemory::getInstance().deallocateChunk( (char*)self->m_info, self->getCapacity()*sizeElem + hkSizeOf(hkSmallArray<char>::Info), HK_MEMORY_CLASS_ARRAY);
	
	self->m_info = dest;
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
