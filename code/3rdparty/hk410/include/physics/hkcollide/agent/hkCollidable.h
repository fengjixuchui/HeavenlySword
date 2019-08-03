/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLIDABLE_H
#define HK_COLLIDE2_COLLIDABLE_H

#include <hkcollide/agent/hkCdBody.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandle.h>

class hkShape;
class hkTransform;

/// An hkCollidable can collide with other hkCollidables. It has an hkShape and a hkMotionState (or hkTransform) for that shape.
/// It also has void* owner pointer.
/// If you wish to make a collision query, you must create two hkCollidable objects, and use the hkCollisionDispatcher
/// to access the correct hkCollisionAgent for the query.
/// hkCollidable objects are automatically created by the hkdynamics system, and the owner points to either the hkEntity
/// or hkPhantom object in the world. See these classes for further information.
/// The collidable used to inherit from broad phase handle too, but serialization
/// can handle the multiple inheritance (of classes with data members)
class hkCollidable : public hkCdBody
{
	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, hkCollidable);

			/// Creates a new hkCollidable.
			/// Note: this constructor sets the owner to be HK_NULL, call setOwner to specify the owner and type
			/// Note: this collidable can't be used for normal collision detection (processCollision)
		inline hkCollidable( const hkShape* shape, const hkTransform* ms, int type = 0);

			/// Creates a new hkCollidable.
			/// Note: this constructor sets the owner to be HK_NULL, call setOwner to specify the owner and type
		inline hkCollidable( const hkShape* shape, const hkMotionState* ms, int type = 0);


		inline ~hkCollidable();

			///Gets the hkShape.
		inline const hkShape* getShape() const;

			/// Sets the hkShape. Note: you should be careful about setting the shape of a hkCollidable at runtime, as if there
			/// are any collision agents that depend on it they will no longer work and crash
		inline void setShape(hkShape* shape);

			/// Sets the entity that owns this hkCollidable.
			/// and the type, see hkWorldObject::BroadPhaseType for possible types.
		inline void setOwner( void* owner );

			///Gets the entity that owns this hkCollidable.
		inline void* getOwner() const;

		void checkPerformance();

		//
		// TypedBroadPhase handle i/f:
		//
		inline const hkTypedBroadPhaseHandle* getBroadPhaseHandle() const;
		inline       hkTypedBroadPhaseHandle* getBroadPhaseHandle();


		HK_FORCE_INLINE hkCollidableQualityType getQualityType() const;

			/// Sets the quality type of this collidable.
			///
			/// Note: Having two non-continuously colliding dynamic objects and fixing one of them
			/// doesn't result in continuous collision detection between the objects
			/// if the agent is already created. The current agent will be replaced
			/// by its continuous version only after the bodies separate and lose their broadphase
			/// overlap (when their agent is destroyed) and then come into proximity again
			/// (creating a new agent of type conforming to their current qualityType settings).
		HK_FORCE_INLINE void setQualityType(hkCollidableQualityType type);

			/// Gets the current allowed penetration depth.
			/// This is a hint to the continuous physics to allow some penetration for this object
			/// to reduce CPU load. Note: this is not a hard limit but more a guideline to the engine.
			/// Depending on the qualityType, this allowed penetration can be breached sooner or later.
			/// See user guide on continuous physics for details.
		HK_FORCE_INLINE hkReal getAllowedPenetrationDepth() const;

			/// Sets the current allowed penetration depth. See getAllowedPenetrationDepth for details.
		HK_FORCE_INLINE void setAllowedPenetrationDepth( hkReal val );


	public:
			///Gets the collision filter info. This is an identifying value used by collision filters
			/// - for example, if a group filter is used, this value would encode a collision group and a system group
		HK_FORCE_INLINE hkUint32 getCollisionFilterInfo() const;

			///Sets the collision filter info. This is an identifying value used by collision filters
			/// - for example, if a group filter is used, this value would encode a collision group and a system group
		HK_FORCE_INLINE void setCollisionFilterInfo( hkUint32 info );

			///Gets the type. The possible types are defined in hkWorldObject::BroadPhaseType.
		HK_FORCE_INLINE int getType() const;

	protected:

			/// This is used by hkDynamics to point to the hkRigidBody.
			/// Check the type before doing casts.
			/// Note: You can use hkGetRigidBody(hkCollidable*) to get a type checked owner
			/// (defined in file <hkdynamics/entity/hkRigidBody.h> )
			/// It is an offset from 'this' to the owner as it assumes that if
			/// a collidable has an owner, the collidable is a member of that owner.
			/// Stored as an offset so that the serialization can handle it 'as is'.
		hkInt32 m_ownerOffset;

		class hkTypedBroadPhaseHandle m_broadPhaseHandle;

	public:
			// Should be set to the allowed penetration depth
		hkReal m_allowedPenetrationDepth;

	public:

		hkCollidable( class hkFinishLoadedObjectFlag flag );

	private:

		inline const hkCollidable* getRootCollidable() const { return this; }
};

#include <hkcollide/agent/hkCollidable.inl>

#endif // HK_COLLIDE2_COLLIDABLE_H

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
