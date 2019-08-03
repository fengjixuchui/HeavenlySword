/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_CODE_H
#define HK_COLLIDE2_MOPP_CODE_H

typedef hkUint32 hkPrimitiveProperty;
extern const hkClass hkMoppCodeClass;

/// The MOPP code simulates a hierarchical bounding volume tree. Note that hkMoppCodes are reference counted.

class hkMoppCode : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MOPP );
		HK_DECLARE_REFLECTION();

		virtual ~hkMoppCode() {} // so that the serialization knows it is virtual.

		struct CodeInfo
		{
			HK_DECLARE_REFLECTION();
			
			hkVector4 m_offset;		

			inline hkReal getScale() const;

			inline void setScale(hkReal inVal);
		};

		struct CodeInfo m_info;
		hkArray<hkUint8> m_data;

		inline hkInt32 getCodeSize() const;

	public:

		// range of property values
		enum
		{
			MIN_PROPERTY_VALUE = 0,
			MAX_PROPERTY_VALUE = 0xffffffff
		};

		// number of property values
		enum
		{
			MAX_PRIMITIVE_PROPERTIES = 1
		};

	public:

			/// Create empty mopp code.
		hkMoppCode( ) { m_info.m_offset.setZero4(); }

			// Used for serialization.
		hkMoppCode( hkFinishLoadedObjectFlag f ) : m_data(f) { }

			/// Create from existing data. "moppData" is used inplace.
		inline hkMoppCode( const CodeInfo& info, const hkUint8* moppData, int moppBytes );
};

#include <hkinternal/collide/mopp/code/hkMoppCode.inl>

#endif // HK_COLLIDE2_MOPP_CODE_H

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
