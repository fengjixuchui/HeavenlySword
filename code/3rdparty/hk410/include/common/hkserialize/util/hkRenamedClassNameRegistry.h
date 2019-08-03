/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE_RENAMED_CLASSNAME_REGISTRY_H
#define HK_SERIALIZE_RENAMED_CLASSNAME_REGISTRY_H

#include <hkserialize/util/hkClassNameRegistry.h>
#include <hkserialize/version/hkVersionUtil.h>

class hkRenamedClassNameRegistry : public hkClassNameRegistry
{
	public:

		hkRenamedClassNameRegistry( const hkVersionUtil::ClassRename* renames, hkClass*const* classes )
		{
			registerList( classes );
			if( renames )
			{
				for( const hkVersionUtil::ClassRename* r = renames; r->oldName != HK_NULL; ++r )
				{
					m_renames.insert( r->oldName, r->newName );
				}
			}		
		}

		const hkClass* getClassByName( const char* oldname )
		{
			const char* name = m_renames.getWithDefault( oldname, oldname );
			return hkClassNameRegistry::getClassByName( name );
		}

		hkStringMap<const char*> m_renames;
};

#endif // HK_SERIALIZE_RENAMED_CLASSNAME_REGISTRY_H

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
