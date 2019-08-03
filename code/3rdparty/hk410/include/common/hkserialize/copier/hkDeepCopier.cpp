/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkserialize/copier/hkDeepCopier.h>
#include <hkserialize/copier/hkObjectCopier.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkbase/htl/hkPointerMap.h>

void* hkDeepCopier::deepCopy(const void* dataIn, const hkClass& klassIn, hkDeepCopier::CopyFromOriginal* previousCopies )
{
	hkArray<char> buffer;
	hkOstream out(buffer);

	hkRelocationInfo relocations;
	hkArray<hkVariant> todo;
	{
		hkVariant v;
		v.m_object = const_cast<void*>(dataIn);
		v.m_class =  &klassIn;
		todo.pushBack(v);
	}

	hkPointerMap<void*, int> currentCopies; // copies in the current buffer

	hkObjectCopier copier( hkStructureLayout::HostLayoutRules, hkStructureLayout::HostLayoutRules );

	for( int todoIndex = 0; todoIndex < todo.getSize(); ++todoIndex )
	{
		currentCopies.insert( todo[todoIndex].m_object, buffer.getSize() );
		int origNumGlobals = relocations.m_global.getSize();
		copier.copyObject(todo[todoIndex].m_object, *todo[todoIndex].m_class,
			out.getStreamWriter(), *todo[todoIndex].m_class,
			relocations );
		for( int i = origNumGlobals; i < relocations.m_global.getSize(); ++i )
		{
			hkVariant v;
			v.m_object = relocations.m_global[i].m_toAddress;
			v.m_class =  relocations.m_global[i].m_toClass;

			void* previous = previousCopies ? previousCopies->getWithDefault(v.m_object, HK_NULL) : HK_NULL;
			if( previous )
			{
				relocations.m_global[i].m_toAddress = previous;
			}
			else if( currentCopies.hasKey(v.m_object) == false )
			{
				todo.pushBack( v );
			}
		}
	}

	if( buffer.getSize() )
	{
		char* versioned = hkAllocate<char>( buffer.getSize(), HK_MEMORY_CLASS_EXPORT );
		hkString::memCpy( versioned, buffer.begin(), buffer.getSize() );

		hkArray<hkRelocationInfo::Local>& local = relocations.m_local;
		for( int localIndex = 0; localIndex < local.getSize(); ++localIndex )
		{
			*(void**)(versioned + local[localIndex].m_fromOffset) = versioned + local[localIndex].m_toOffset;
		}
		hkArray<hkRelocationInfo::Global>& global = relocations.m_global;
		for( int globalIndex = 0; globalIndex < global.getSize(); ++globalIndex )
		{
			void* porig = global[globalIndex].m_toAddress;
			void* pnew  = porig;
			int off = currentCopies.getWithDefault(porig,-1);
			if( off != -1 )
			{
				pnew = versioned + off;
				if( previousCopies )
				{
					previousCopies->insert( porig, pnew );
				}
			}
			void* from = versioned + global[globalIndex].m_fromOffset;
			*(void**)(from) = pnew;
		}
		return versioned;
	}
	return HK_NULL;
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
