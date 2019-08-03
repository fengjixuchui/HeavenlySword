/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CONSTRAINT_OWNER_H
#define HK_DYNAMICS2_CONSTRAINT_OWNER_H

#include <hkbase/thread/util/hkMultiThreadLock.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/hkConstraintData.h>

class hkConstraintOwner: public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_DYNAMICS);

		inline void addConstraintInfo( hkConstraintInstance* constraint, hkConstraintInfo& delta )
		{
			HK_ON_DEBUG_MULTI_THREADING( checkAccessRw() );
			if ( constraint->m_internal )
			{
				m_constraintInfo.add( delta );
				constraint->m_internal->addConstraintInfo( delta );
			}
		}

		inline void subConstraintInfo( hkConstraintInstance* constraint, hkConstraintInfo& delta )
		{
			HK_ON_DEBUG_MULTI_THREADING(checkAccessRw());
			if ( constraint->m_internal )
			{
				m_constraintInfo.sub( delta );
				constraint->m_internal->subConstraintInfo( delta );
			}
		}

	public:

		virtual void addConstraintToCriticalLockedIsland(      hkConstraintInstance* constraint ){}
		virtual void removeConstraintFromCriticalLockedIsland( hkConstraintInstance* constraint ){}
		virtual void addCallbackRequest( hkConstraintInstance* constraint, int request ){}
		virtual void checkAccessRw() {}

	public:
		hkConstraintInfo m_constraintInfo;
		
};




#endif // HK_DYNAMICS2_CONSTRAINT_OWNER_H

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
