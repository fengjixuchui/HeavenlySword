/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE_BUILTINTYPEREGISTRY_H
#define HK_SERIALIZE_BUILTINTYPEREGISTRY_H

class hkClass;
class hkTypeInfo;
class hkClassNameRegistry;
class hkVtableClassRegistry;
class hkFinishLoadedObjectRegistry;

/// Registry of all compiled in types.
/// All typeinfos and classes in the StaticLinkedTypeInfos and
/// StaticLinkedClasses lists are added to the registry when 
/// this singleton is created.
class hkBuiltinTypeRegistry : public hkSingleton<hkBuiltinTypeRegistry>
{
	public:

			/// Access the hkFinishLoadedObjectRegistry.
		virtual hkFinishLoadedObjectRegistry* getFinishLoadedObjectRegistry() = 0;

			/// Access the hkClassNameRegistry.
		virtual hkClassNameRegistry* getClassNameRegistry() = 0;

			/// Access the hkClassNameRegistry.
		virtual hkVtableClassRegistry* getVtableClassRegistry() = 0;

			/// Shortcut for adding to each of the registries.
		virtual void addType( hkTypeInfo* info, hkClass* klass );
	
	public:

			/// List of typeinfos which are added to the default registry.
			/// See demos/classes.h and demos/classes.cpp to see how to
			/// set this up.
		static const hkTypeInfo* const StaticLinkedTypeInfos[];

			/// List of classes which are added to the default registry.
			/// See demos/classes.h and demos/classes.cpp to see how to
			/// set this up.
		static const hkClass* const StaticLinkedClasses[];
};

#endif // HK_SERIALIZE_BUILTINTYPEREGISTRY_H

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
