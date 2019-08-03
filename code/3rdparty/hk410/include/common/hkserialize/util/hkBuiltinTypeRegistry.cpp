/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkserialize/hkSerialize.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkserialize/util/hkClassNameRegistry.h>
#include <hkserialize/util/hkFinishLoadedObjectRegistry.h>
#include <hkserialize/util/hkVtableClassRegistry.h>

void hkBuiltinTypeRegistry::addType( hkTypeInfo* info, hkClass* klass )
{
	getClassNameRegistry()->registerClass( klass, klass->getName() );
	getFinishLoadedObjectRegistry()->registerTypeInfo( info );
	getVtableClassRegistry()->registerVtable( info->getVtable(), klass );
}

class hkDefaultBuiltinTypeRegistry : public hkBuiltinTypeRegistry
{
	public:

		hkDefaultBuiltinTypeRegistry()
		{
			m_classnameRegistry = new hkClassNameRegistry();
			m_classnameRegistry->registerList( StaticLinkedClasses );
			m_finishLoadedObjectRegistry = new hkFinishLoadedObjectRegistry();
			m_finishLoadedObjectRegistry->registerList( StaticLinkedTypeInfos );
			m_vtableClassRegistry = new hkVtableClassRegistry();
			m_vtableClassRegistry->registerList( StaticLinkedTypeInfos, StaticLinkedClasses );
		}

		~hkDefaultBuiltinTypeRegistry()
		{
			m_classnameRegistry->removeReference();
			m_finishLoadedObjectRegistry->removeReference();
			m_vtableClassRegistry->removeReference();
		}

		virtual hkFinishLoadedObjectRegistry* getFinishLoadedObjectRegistry()
		{
			return m_finishLoadedObjectRegistry;
		}

		virtual hkClassNameRegistry* getClassNameRegistry()
		{
			return m_classnameRegistry;
		}
		
		virtual hkVtableClassRegistry* getVtableClassRegistry()
		{
			return m_vtableClassRegistry;
		}

	public:

		hkClassNameRegistry* m_classnameRegistry;
		hkFinishLoadedObjectRegistry* m_finishLoadedObjectRegistry;
		hkVtableClassRegistry* m_vtableClassRegistry;
};

HK_SINGLETON_CUSTOM_IMPLEMENTATION(hkBuiltinTypeRegistry, hkDefaultBuiltinTypeRegistry);

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
