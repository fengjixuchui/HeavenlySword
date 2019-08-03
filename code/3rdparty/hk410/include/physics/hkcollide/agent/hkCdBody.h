/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CD_BODY_H
#define HK_COLLIDE2_CD_BODY_H

#include <hkcollide/shape/hkShape.h>
#include <hkmath/hkMath.h>

class hkShape;
class hkMotionState;
class hkCollidable;
	
	/// The hkCdBody class is a helper class, which is used to traverse
	/// the shape hierarchy. It is used for all narrow phase collision detection queries.
	/// It contains a shape, and an associated transform. Given two hkCdBody classes, you
	/// can query for the closest distance using the hkCollisionAgentInterface.
	/// Note: You do not instantiate this class directly. Use the hkCollidable, which inherits from
	/// hkCdBody instead.<br>
	/// This class is also created temporarily by the collision detector during queries.
	/// For example, when querying a shape against a landscape, hkCdBody classes are created
	/// for each sub-shape in the landscape. When hkCdBody classes are referenced in callbacks,
	/// their data may only be temporary, and you should not hold references to them.<br>
	/// This class holds either a pointer to a transform or a motionState.
	/// The motion state pointer is used only for the internal processCollision call hierarchy
	/// The transform pointer is used for all other collision detection queries
class hkCdBody
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkCdBody);	

			// ( CdBody is auto reflected, but the XML has some extra settings in it too. )
		HK_DECLARE_REFLECTION();

			///  Get the transform for the current shape.
			///  Note: this transform might be temporary, do not store pointers to this transform
		HK_FORCE_INLINE const hkTransform& getTransform() const;
		
			/// Get the current shape
			/// Note: this pointer might be temporary, do not store pointers to this
		HK_FORCE_INLINE const hkShape* getShape() const;

			/// Get the root collidable. This is the root of the hkCdBody tree.
			/// You can use this call in a callback to get back the collidable which was used to originate the collision query
			/// Note: collidables are persistent.
		inline const hkCollidable* getRootCollidable() const;

			/// Returns the shape key of the current shape with respect to the parent shape.
			/// I.e. if it is not HK_INVALID_SHAPE_KEY then the hkCdBody's parent implements hkShapeContainer and
			/// this->getParent->getContainer()->getChildShape(key) will return the same shape as
			/// this->getShape() (possibly at a different address for temporary shapes).
		inline hkShapeKey getShapeKey() const ;
	
			/// Return the parent hkCdBody
		HK_FORCE_INLINE const hkCdBody* getParent() const;


		//
		//	Internal public section
		//
	public:
			// Constructor which copies parent and motionstate, does not set m_shape and m_shapeKey values
			// This function should only be called internally by a collision agent
		explicit HK_FORCE_INLINE hkCdBody( const hkCdBody* parent );


			// This constructor is used by collision agents to create temporary hkCdBody objects
			// This function should only be called internally by a collision agent
		HK_FORCE_INLINE hkCdBody( const hkCdBody* parent, const hkMotionState* ms );


			// This constructor is used by collision agents to create temporary hkCdBody objects
			// This function should only be called internally by a collision agent
		HK_FORCE_INLINE hkCdBody( const hkCdBody* parent, const hkTransform* t );

			// IMPORTANT: Do not call this function directly if this collidable is in the physics world, 
			// because it is unsafe.
			// Use hkRigidBody or hkPhantom setShape() methods instead.
			// Replace the shape ( implicitly copies the m_shapeKey from the parent )
		HK_FORCE_INLINE void setShape( const hkShape* shape );

			// sets the shape and shapeKey 
			// IMPORTANT: Do not call this function directly if this collidable is in the physics world, 
			// because it is unsafe.
			// Use hkRigidBody or hkPhantom setShape() methods instead.
			// This call is only allowed if m_parent->getShape() implements hkShapeContainer.
			// This function should only be called internally by a collision agent.
		HK_FORCE_INLINE void setShape( const hkShape* shape, hkShapeKey key );

			// Gets the motion state
		HK_FORCE_INLINE const hkMotionState* getMotionState() const;

			// Sets the motion state
		HK_FORCE_INLINE void setMotionState( const hkMotionState* state );

			// Sets the transform
		HK_FORCE_INLINE void setTransform( const hkTransform* t );

			// Empty constructor handly with care;
		HK_FORCE_INLINE hkCdBody(  ){}
		
	protected:

		friend class hkCollidable;

		HK_FORCE_INLINE hkCdBody( const hkShape* shape, const hkMotionState* motionState );

		HK_FORCE_INLINE hkCdBody( const hkShape* shape, const hkTransform* t );

		HK_FORCE_INLINE hkCdBody( const hkCdBody& body ){}	// private copy constructor


		const hkShape*       m_shape;
		hkShapeKey           m_shapeKey; //+overridetype(hkUint32)

			// This is either a hkTransform, or a hkMotionState, and is only accessible through the public get and set methods.
			// This is not serialized, but set upon addition to world etc., as sometimes
			// it is just an offset to a transform (offsets == int == could be serialized) 
			// but sometimes it is a hkMotionState that needs to be a typed ptr, so the two
			// can't coexist in a serialization friendly way. 
		const void* m_motion; //+nosave

		const hkCdBody*		 m_parent;
};

#include <hkcollide/agent/hkCdBody.inl>


#endif // HK_COLLIDE2_CD_BODY_H

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
