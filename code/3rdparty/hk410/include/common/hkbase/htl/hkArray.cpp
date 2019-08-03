/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>

HK_COMPILE_TIME_ASSERT( sizeof(void*)==sizeof(char*) );

// hack to force this to be out of line
void hkArrayUtil::_reserve(void* array, int numElem, int sizeElem)
{
	HK_ASSERT2(0x3b67c014, numElem >= 0, "Number of elements must not be negative");
	HK_ASSERT2(0x243bf8d1, sizeElem >= 0, "The size of an element must not be negative");

	typedef hkArray<char> hkAnyArray;

	char* p = static_cast<char*>(hkThreadMemory::getInstance().allocateChunk(numElem*sizeElem, HK_MEMORY_CLASS_ARRAY));
	hkAnyArray* self = reinterpret_cast< hkAnyArray* >(array);
	hkString::memCpy(p, self->m_data, self->m_size*sizeElem);
	//cannot call self->releaseMemory(), because we're lying about the size of each element.
	if( (self->m_capacityAndFlags & hkAnyArray::DONT_DEALLOCATE_FLAG) == 0)
	{
		hkThreadMemory::getInstance().deallocateChunk( self->m_data, self->getCapacity()*sizeElem, HK_MEMORY_CLASS_ARRAY);
	}
	
	self->m_data = p;
	self->m_capacityAndFlags = numElem | (self->m_capacityAndFlags & hkAnyArray::LOCKED_FLAG); // prop the locked flag? (even though we have realloced..)
}


// hack to force this to be out of line
void hkArrayUtil::_reserveMore(void* array, int sizeElem)
{
	HK_ASSERT2(0x5828d5cf, sizeElem >= 0, "The size of an element must not be negative");

	typedef hkArray<char> hkAnyArray;
	hkAnyArray* self = reinterpret_cast< hkAnyArray* >(array);
	int numElem = (self->m_size ? self->m_size*2 : 1);


	char* p = static_cast<char*>(hkThreadMemory::getInstance().allocateChunk(numElem*sizeElem, HK_MEMORY_CLASS_ARRAY));
	hkString::memCpy(p, self->m_data, self->m_size*sizeElem);
	//cannot call self->releaseMemory(), because we're lying about the size of each element.
	if( (self->m_capacityAndFlags & hkAnyArray::DONT_DEALLOCATE_FLAG) == 0)
	{
		hkThreadMemory::getInstance().deallocateChunk( self->m_data, self->getCapacity()*sizeElem, HK_MEMORY_CLASS_ARRAY);
	}
	
	self->m_data = p;
	self->m_capacityAndFlags = numElem | (self->m_capacityAndFlags & hkAnyArray::LOCKED_FLAG); // prop the locked flag? (even though we have realloced..)
}

// hack to force this to be out of line
void hkArrayUtil::_reduce(void* array, int sizeElem, char* inplaceMem, int numElemInplaceMem)
{
	typedef hkArray<char> hkAnyArray;
	hkAnyArray* self = reinterpret_cast< hkAnyArray* >(array);

	HK_ASSERT2(0x5828d5f0, sizeElem >= 0, "The size of an element must not be negative");
	HK_ASSERT2(0x5828d5f1, (self->m_capacityAndFlags & hkAnyArray::DONT_DEALLOCATE_FLAG) == 0, "Don't call _reduce if the array is already optimal or preallocated.");

	int newCapacity;
	int deallocateFlag; 
	char* p;	

	if ( (HK_NULL != inplaceMem) && (self->m_size < numElemInplaceMem) )
	{
		p = inplaceMem;
		newCapacity = numElemInplaceMem;
		deallocateFlag = hkAnyArray::DONT_DEALLOCATE_FLAG;
	}
	else
	{
		newCapacity = self->getCapacity()/2;
		HK_ASSERT2(0x5828d5f1, newCapacity >= self->m_size, "Can't reduce array");
		p = static_cast<char*>(hkThreadMemory::getInstance().allocateChunk(newCapacity*sizeElem, HK_MEMORY_CLASS_ARRAY));
		deallocateFlag = 0;
	}

	hkString::memCpy(p, self->m_data, self->m_size*sizeElem);

	hkThreadMemory::getInstance().deallocateChunk( self->m_data, self->getCapacity()*sizeElem, HK_MEMORY_CLASS_ARRAY);
	
	self->m_data = p;
	self->m_capacityAndFlags = newCapacity | deallocateFlag | (self->m_capacityAndFlags & hkAnyArray::LOCKED_FLAG); // prop the locked flag? 
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
