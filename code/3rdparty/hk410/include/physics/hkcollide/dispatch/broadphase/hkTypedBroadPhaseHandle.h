/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_TYPED_BROAD_PHASE_HANDLE
#define HK_TYPED_BROAD_PHASE_HANDLE

#include <hkinternal/collide/broadphase/hkBroadPhaseHandle.h>
#include <hkcollide/agent/hkCollidableQualityType.h>

/// An hkBroadPhaseHandle with a type. hkCollidable is a subclass of hkTypedBroadPhaseHandle
/// If you use the Havok dynamics lib then
/// type can be hkWorldObject::BROAD_PHASE_ENTITY for entities or hkWorldObject::BROAD_PHASE_PHANTOM for phantoms.
/// Also you can use
///   - hkRigidBody* hkGetRigidBody( hkCollidable* collidable ) 
///   - hkPhantom* hkGetPhantom(hkCollidable* collidable)

class hkTypedBroadPhaseHandle : public hkBroadPhaseHandle
{
	public:

		HK_DECLARE_REFLECTION();
		
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, hkTypedBroadPhaseHandle);

			///Creates a new hkTypedBroadPhaseHandle of the specified type
			///Make sure to call setowner before using this handle.
		HK_FORCE_INLINE hkTypedBroadPhaseHandle( int type );

			///Creates a new hkTypedBroadPhaseHandle of the specified type
		HK_FORCE_INLINE hkTypedBroadPhaseHandle( void* owner, int type );
		
			///Gets the type. The possible types are defined in hkWorldObject::BroadPhaseType.
		HK_FORCE_INLINE int getType() const;
		
			///Gets the owner of this handle
		HK_FORCE_INLINE void setOwner(void* o);

			///Gets the owner of this handle
		HK_FORCE_INLINE void* getOwner() const;

			///Gets the collision filter info. This is an identifying value used by collision filters
			/// - for example, if a group filter is used, this value would encode a collision group and a system group
		HK_FORCE_INLINE hkUint32 getCollisionFilterInfo() const;

			///Sets the collision filter info. This is an identifying value used by collision filters
			/// - for example, if a group filter is used, this value would encode a collision group and a system group
		HK_FORCE_INLINE void setCollisionFilterInfo( hkUint32 info );

		
	protected:

		enum { OFFSET_INVALID = 127 };
		inline void setType( int type );
		friend class hkBroadPhaseBorder; // it overrides the type of its owned phantoms

		hkInt8 m_type;
		hkInt8 m_ownerOffset; // would have padding of 8 here anyway, so keep at 8 and assert if owner more than +/-128 bytes away

	public:

			/// The quality of the object,
			/// You should use the hkCollisionDispatcher to get the hkCollisionQualityInfo

		hkUint16 m_objectQualityType;
		hkUint32 m_collisionFilterInfo;

	public:

		hkTypedBroadPhaseHandle( class hkFinishLoadedObjectFlag flag ) {}
};

#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandle.inl>

#endif // HK_TYPED_BROAD_PHASE_HANDLE

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
