/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE_SHAPE_CONTAINER_H
#define HK_COLLIDE_SHAPE_CONTAINER_H

/// Interface to shapes which have one or more children, accessible through a shapekey.
/// See hkShape::getCollection()
class hkShapeContainer
{		
	public:

		HK_DECLARE_REFLECTION();

			///
		virtual ~hkShapeContainer() { }

			/// The number of child shapes. The default implementation just iterates over all keys and is really slow
		virtual int getNumChildShapes() const;

			/// Get the first child shape key
			/// see getChildShape() for extra details
		virtual hkShapeKey getFirstKey() const = 0;

			/// Get the next child shape key
			/// If the the "oldKey" parameter is the last key in the shape collection, this function
			/// returns HK_INVALID_SHAPE_KEY
			/// see getChildShape() for extra details
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const = 0;

			/// Return the collision filter info for a given child shape
		virtual hkUint32 getCollisionFilterInfo( hkShapeKey key ) const;

			/// Attributes of the buffer used in getChildShape.
		enum {  HK_SHAPE_BUFFER_ALIGNMENT = 16,	HK_SHAPE_BUFFER_SIZE = 512 };

			/// A buffer type, allocated locally on the stack by calling functions,
			/// to be passed to getChildShape.
		typedef HK_ALIGN16( char ShapeBuffer[HK_SHAPE_BUFFER_SIZE] );

			/// Gets a child shape using a shape key.
			/// This function must return a child shape pointer. This is only called internally by
			/// the collision detection system after having called getFirstKey() or getNextKey().
			/// If you have shape keys that are invalid, you must implement getNextKey() in such
			/// a way that it skips over these shapes.
			/// Important Note: It is assumed by the system that a shape key, if valid (i.e. returned by
			/// getNextkey()) will always remain valid.<br>
			/// Notes:
			/// - You can return a pointer to a shape
			/// - our you can construct a shape in place in the buffer and return a pointer to that buffer.
			///	   e.g. hkMeshShape uses this buffer for temporarily created triangles.
			///	   hkListShape does not use the buffer as it already has shape instances.
			///    Attention: When the buffer gets erased, no destructor will be called.
			/// - The buffer must be 16 byte aligned.
		virtual const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const = 0;

			/// Return whether welding should be disabled for this shape container
		virtual bool disableWelding() const { return false; }
};

/// Utility class for a shape which has a single child.
class hkSingleShapeContainer : public hkShapeContainer
{
	public:

		HK_DECLARE_REFLECTION();

		void* operator new(hk_size_t, void* p) { return p; }
		void  operator delete(void* p) { }

			/// Create a single shape collection.
		hkSingleShapeContainer( const hkShape* s ) : m_childShape(s)
		{
			m_childShape->addReference();
		}

		~hkSingleShapeContainer()
		{
			m_childShape->removeReference();
		}

			// Implmented method of hkShapeContainer
		virtual int getNumChildShapes() const { return 1; }

			// Implmented method of hkShapeContainer
		virtual hkShapeKey getFirstKey() const { return 0; }

			// Implmented method of hkShapeContainer
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const { return HK_INVALID_SHAPE_KEY; }

			// Implmented method of hkShapeContainer
		virtual const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const { return m_childShape; }

			/// Get the child shape.
		inline const hkShape* getChild() const { return m_childShape; }

			/// 
		inline const hkShape* operator->() const { return m_childShape; }

		hkSingleShapeContainer(hkFinishLoadedObjectFlag) {}

	protected:

		const hkShape* m_childShape;
};

#endif // HK_COLLIDE_SHAPE_CONTAINER_H

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
