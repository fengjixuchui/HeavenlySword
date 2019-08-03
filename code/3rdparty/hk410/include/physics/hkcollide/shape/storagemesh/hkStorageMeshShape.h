/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_STORAGEMESHSHAPE_H
#define HK_STORAGEMESHSHAPE_H

#include <hkcollide/shape/mesh/hkMeshShape.h>

extern const hkClass hkStorageMeshShapeClass;
extern const hkClass hkStorageMeshShapeSubpartStorageClass;

/// A mesh shape which stores its data. See also hkSimpleMeshShape.
/// NOTE: it is very error prone to modify the subparts of an
/// hkStorageMeshShape directly because the subpart pointers need to
/// be updated if the storage is resized.
class hkStorageMeshShape : public hkMeshShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// Default constructor.
			/// The data for this shape is public, so simply fill in the
			/// member data after construction.
		hkStorageMeshShape( hkReal radius = hkConvexShapeDefaultRadius, int numBitsForSubpartIndex = 12 );

			/// Copy the mesh into this mesh.
		hkStorageMeshShape( const hkMeshShape* mesh );

			///
		~hkStorageMeshShape();

			/// Add the part and copy its data internally.
			/// NOTE: it is not recommended to modify a subpart after it
			/// has been added.
			/// NOTE: materials are not copied.
		virtual void addSubpart( const Subpart& part );

	public:

		hkStorageMeshShape( hkFinishLoadedObjectFlag flag );

		struct SubpartStorage : public hkReferencedObject
		{
			public:

				HK_DECLARE_REFLECTION();
				HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SHAPE);

				SubpartStorage() {}
				~SubpartStorage() {}

				hkArray<hkReal> m_vertices;
				hkArray<hkUint16> m_indices16;
				hkArray<hkUint32> m_indices32;
				hkArray<hkUint8> m_materialIndices; //materialIndices8
				hkArray<hkUint32> m_materials;
				hkArray<hkUint16> m_materialIndices16;

			public:

				SubpartStorage( hkFinishLoadedObjectFlag flag );
		};

	protected:

		hkArray<struct SubpartStorage*> m_storage;
};

#endif //HK_STORAGEMESHSHAPE_H

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
