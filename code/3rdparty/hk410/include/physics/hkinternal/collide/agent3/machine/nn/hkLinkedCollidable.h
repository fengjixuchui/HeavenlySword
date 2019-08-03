/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_LINKED_COLLIDABLE_H
#define HK_COLLIDE2_LINKED_COLLIDABLE_H

#include <hkcollide/agent/hkCollidable.h>

struct hkAgentNnEntry;

/// A hkLinkedCollidable is a hkCollidable which is referenced by collision agents. If the agent moves
/// it can update all links pointing to it
class hkLinkedCollidable: public hkCollidable
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, hkLinkedCollidable );
		HK_DECLARE_REFLECTION();

			/// Creates a new hkLinkedCollidable.
			/// Note: this constructor sets the owner to be HK_NULL, call setOwner to specify the owner and type
		inline hkLinkedCollidable( const hkShape* shape, const hkMotionState* ms, int type = 0);

	
		inline ~hkLinkedCollidable();

	public:

		/// A list of links to all persistent agent referencing this object
		struct CollisionEntry
		{
			hkAgentNnEntry* m_agentEntry;
			hkLinkedCollidable*	m_partner;
		};

		hkArray<struct CollisionEntry> m_collisionEntries; //+nosave

	public:
	
			// Used by the serialization to discover vtables.
		inline hkLinkedCollidable( class hkFinishLoadedObjectFlag flag ) : hkCollidable(flag) {}
};

#include <hkinternal/collide/agent3/machine/nn/hkLinkedCollidable.inl>

#endif // HK_COLLIDE2_LINKED_COLLIDABLE_H

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
