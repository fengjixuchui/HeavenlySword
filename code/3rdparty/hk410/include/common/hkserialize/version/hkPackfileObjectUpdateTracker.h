/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/version/hkObjectUpdateTracker.h>
#include <hkserialize/util/hkPointerMultiMap.h>

/// A tracker which understands the layout of packfiles.
class hkPackfileObjectUpdateTracker : public hkObjectUpdateTracker
{
	public:

		hkPackfileObjectUpdateTracker( hkPackfileData* data )
			: m_packfileData(data)
		{
			m_packfileData->addReference();
			m_topLevelObject = HK_NULL;
			m_topLevelClassName = HK_NULL;
		}

		~hkPackfileObjectUpdateTracker()
		{
			m_packfileData->removeReference();
		}

			//
			// Memory
			//

		virtual void addAllocation(void* p)
		{
			m_packfileData->addAllocation(p);
		}
		virtual void addChunk(void* p, int n, HK_MEMORY_CLASS c)
		{
			m_packfileData->addChunk(p,n,c);
		}

			//
			// Pointers
		//

		void objectPointedBy( void* newObject, void* fromWhere )
		{
			void* oldObject = *static_cast<void**>(fromWhere);
			if (oldObject)
			{
				for( int i = m_pointers.getFirstIndex(oldObject);
					i != -1;
					i = m_pointers.getNextIndex(i) )
				{
					if( m_pointers.getValue(i) == fromWhere )
					{
						if( newObject == oldObject )
						{
							return;
						}
						m_pointers.removeByIndex(oldObject, i);
						break;
					}
				}
			}
			if( newObject )
			{
				m_pointers.insert( newObject, fromWhere );
			}
			*static_cast<void**>(fromWhere) = newObject;
		}

		virtual void replaceObject( void* oldObject, void* newObject, const hkClass* newClass )
		{
			if( oldObject == m_topLevelObject )
			{
				m_topLevelObject = newObject;
				m_topLevelClassName = newClass->getName();
			}
			// replace pointers to old object with pointers to new one
			int index = m_pointers.getFirstIndex(oldObject);
			if( newObject )
			{
				m_pointers.m_indexMap.insert( newObject, index );
			}
			while( index != -1 )
			{
				void* ptrOldObject = m_pointers.getValue(index);
				HK_ASSERT(0x7fe24edd, *static_cast<void**>(ptrOldObject) == oldObject );
				*static_cast<void**>(ptrOldObject) = newObject;
				index = m_pointers.getNextIndex(index);
			}
			// keep exports valid
			for( int i = 0; i < m_packfileData->m_exports.getSize(); ++i )
			{
				if( m_packfileData->m_exports[i].data == oldObject)
				{
					m_packfileData->m_exports[i].data = newObject;
				}
			}
			// replace object in the finish list
			removeFinish(oldObject);
			if( newClass )
			{
				addFinish(newObject, newClass->getName());
			}
		}

			//
			// Finish
			//

		void addFinish( void* newObject, const char* className )
		{
			HK_ASSERT( 0x567037f2, m_finish.hasKey(newObject) == false );
			m_finish.insert( newObject, className );
		}

		void removeFinish( void* oldObject )
		{
			m_finish.remove(oldObject);
		}

		hkPackfileData* m_packfileData;
		hkPointerMultiMap<void*> m_pointers;
		hkPointerMap<void*, const char*> m_finish;
		void* m_topLevelObject;
		const char* m_topLevelClassName;
};

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
