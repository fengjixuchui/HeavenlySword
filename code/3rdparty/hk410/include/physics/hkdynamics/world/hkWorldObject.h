/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_OBJECT_H
#define HK_DYNAMICS2_WORLD_OBJECT_H

#include <hkinternal/collide/agent3/machine/nn/hkLinkedCollidable.h>
#include <hkbase/thread/util/hkMultiThreadLock.h>
#include <hkcollide/shape/hkShape.h>
#include <hkdynamics/common/hkProperty.h>

class hkWorld;
class hkShape;
class hkPhantom;
class hkRigidBody;

extern const hkClass hkWorldObjectClass;

	// This namespace refers to the operation postponing framework in hkWorldOperationQueue.h
namespace hkWorldOperation
{
		// A result value informing whether a critical operation was performed immediately
		// or postponed ( -- e.g. when requested in a callback during collision detection phase, 
		// when the hkWorld is locked).
	enum Result
	{
		POSTPONED,
		DONE
	};
}

	/// This is the base class for hkEntity and hkPhantom. It contains a list of hkProperty key-value pairs
	/// and also a hkCollidable object.
class hkWorldObject : public hkReferencedObject
{
	public:
	
		HK_DECLARE_REFLECTION();

			/// Adds a property to the entity. Properties must be unique. 
			/// You can use properties to add additional information to an entity - e.g.
			/// for using your own collision filters or backlinks into your game.
		void addProperty( hkUint32 key, hkPropertyValue value );

			/// Removes the property of the specified type. This function returns the property
			/// removed, or 0 if there is no property of the given type.
		hkPropertyValue removeProperty( hkUint32 key );

			/// Updates the property with a new value, the property's key must already
			/// exist. Returns the previous value for the property or HK_NULL if the key is not found.
		hkPropertyValue editProperty( hkUint32 key, hkPropertyValue value );

			/// Returns the property specified, or 0 if there is no property of the given type.
		inline hkPropertyValue getProperty( hkUint32 key ) const;

			/// Returns whether a property of this type has been set for the entity.
		inline hkBool hasProperty( hkUint32 key ) const;

			/// locks the given property field of all entities, allows for
			/// calling editProperty even if you only have 'read only' access for this entity.
			/// Warning: If you use this function in a multi threaded environment, you most likely 
			/// end up with a nonterministic behavior.
		void lockProperty( hkUint32 key );

			/// unlocks a given locked property
		void unlockProperty( hkUint32 key );

			/// Call this function if you wish to add properties to an hkWorldObject
			/// that has been loaded from a packfile.  If you call this, then you
			/// need to call clearAndDeallocateProperties before you deallocate
			/// the packfile memory. See the user guide serialization section for
			/// details on how arrays are handled by packfile loading.
		inline void unlockPropertiesFromLoadedObject();

			/// Clear and deallocate the properties of this world object.
		inline void clearAndDeallocateProperties();


			/// Gets the collidable owned by this world object.
			/// If the object has no collision detection representation, 
			/// the shape in the collidable will be HK_NULL.
		inline hkCollidable* getCollidableRw();

			/// Gets the collidable owned by this world object.
			/// If the object has no collision detection representation, the shape in the collidable will be HK_NULL.
		inline const hkCollidable* getCollidable() const;		

		inline hkLinkedCollidable* getLinkedCollidable();

			/// Returns true if this object is added to a world
		inline hkBool isAddedToWorld() const;

			/// Get the world that owns this object. This is HK_NULL if the object has not been added to the world
		inline hkWorld* getWorld() const;

			/// Get a user data pointer for the world object
		inline void* getUserData() const;

			/// Set a user data pointer for the world object
		inline void setUserData( void* data );

			/// Get the name of this world object.
		inline const char* getName() const;

			/// Set the name of this world object.
			/// IMPORTANT: This data will not be cleaned up by the hkWorldObject destructor. You are required to track it yourself.
		inline void setName( const char* name );

		void calcStatistics( hkStatisticsCollector* collector ) const;

			/// Set the shape of an hkEntity or an hkPhantom.
		virtual hkWorldOperation::Result setShape( hkShape* shape );

			/// Lock this class and all child classes for read only access for this thread
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread locked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void markForRead( );

			/// Lock this class and all child classes for read write access for this thread
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread locked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void markForWrite( );
			
			// implementation of above function
		void markForWriteImpl( );

			/// Undo lockForRead
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread locked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void unmarkForRead( );

			/// Unlock For write
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread locked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void unmarkForWrite();

		/// Check for read write access
		void checkReadWrite();

		/// Check for read only access
		void checkReadOnly() const;

			/// Adds a reference to a world object. This version checks whether the current thread has the read-write rights to the object.
		void addReference();
			/// Removes a reference to a world object. This version checks whether the current thread has the read-write rights to the object.
		void removeReference();
			/// Adds a reference to a world object. This version is a critical operation (unlike the base class' method), and therefore is thread-safe.
		void addReferenceAsCriticalOperation();
			/// Removes a reference to a world object. This version is a critical operation (unlike the base class' method), and therefore is thread-safe.
		void removeReferenceAsCriticalOperation();

	public:

		/// Broadphase types.
		enum BroadPhaseType
		{
			/// 
			BROAD_PHASE_INVALID,
			/// hkEntity.
			BROAD_PHASE_ENTITY,
			/// hkPhantom.
			BROAD_PHASE_PHANTOM,
			/// hkBroadPhaseBorder's objects (aabb phantoms).
			BROAD_PHASE_BORDER,
			/// 
			BROAD_PHASE_MAX_ID
		};

	protected:

		friend class hkWorld;
		friend class hkWorldOperationUtil;
		inline void setWorld( hkWorld* world );

			// gets the motion state 
		virtual class hkMotionState* getMotionState() = 0;

		hkWorldObject( const hkShape* shape, BroadPhaseType type ); 

		inline virtual ~hkWorldObject(); // virtual for the sake of serialization flags

		hkWorld* m_world; //+nosave

		void* m_userData; //+nosave

		const char* m_name;

		class hkMultiThreadLock m_multithreadLock;
	public:

		inline hkMultiThreadLock& getMultiThreadLock();
		inline const hkMultiThreadLock& getMultiThreadLock() const;

		inline void copyProperties( const hkWorldObject* otherObj );

	protected:
			// the collidable, for implementation reasons this is a cdEntity, which 
			// can only be accessed from hkEntity 
		class hkLinkedCollidable m_collidable;
		hkArray<class hkProperty> m_properties;

	public:

		hkWorldObject( class hkFinishLoadedObjectFlag flag );
};

	/// Helper function that returns the hkWorldObject pointer for a hkCollidable
inline hkWorldObject* hkGetWorldObject(const hkCollidable* collidable)
{
	return reinterpret_cast<hkWorldObject*>( const_cast<void*>(collidable->getOwner()) );
}

#include <hkdynamics/world/hkWorldObject.inl>

#endif	//HK_DYNAMICS2_WORLD_OBJECT_H


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
