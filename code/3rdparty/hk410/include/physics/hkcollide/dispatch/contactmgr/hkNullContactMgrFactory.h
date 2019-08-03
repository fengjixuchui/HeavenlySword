/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_COLLIDE2_NULL_CONTACT_MGR_FACTORY_H
#define HK_COLLIDE2_NULL_CONTACT_MGR_FACTORY_H

#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkcollide/dispatch/contactmgr/hkNullContactMgr.h>

/// A factory for hkNullContactMgrs
class hkNullContactMgrFactory: public hkContactMgrFactory
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS );

		hkNullContactMgrFactory()
		{
		}

		hkContactMgr*	createContactMgr( const hkCollidable& a, const hkCollidable& b, const hkCollisionInput& env )
		{
			return &m_contactMgr;
		}

		hkNullContactMgr m_contactMgr;
};

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
