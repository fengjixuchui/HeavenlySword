/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/storagemesh/hkStorageMeshShape.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/storagemesh/hkStorageMeshShape.h>


// External pointer and enum types
extern const hkClass hkStorageMeshShapeSubpartStorageClass;

//
// Class hkStorageMeshShape::SubpartStorage
//
static const hkInternalClassMember hkStorageMeshShape_SubpartStorageClass_Members[] =
{
	{ "vertices", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_REAL, 0, 0, HK_OFFSET_OF(hkStorageMeshShape::SubpartStorage,m_vertices) },
	{ "indices16", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT16, 0, 0, HK_OFFSET_OF(hkStorageMeshShape::SubpartStorage,m_indices16) },
	{ "indices32", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT32, 0, 0, HK_OFFSET_OF(hkStorageMeshShape::SubpartStorage,m_indices32) },
	{ "materialIndices", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT8, 0, 0, HK_OFFSET_OF(hkStorageMeshShape::SubpartStorage,m_materialIndices) },
	{ "materials", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT32, 0, 0, HK_OFFSET_OF(hkStorageMeshShape::SubpartStorage,m_materials) },
	{ "materialIndices16", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT16, 0, 0, HK_OFFSET_OF(hkStorageMeshShape::SubpartStorage,m_materialIndices16) }
};
extern const hkClass hkReferencedObjectClass;

const hkClass hkStorageMeshShapeSubpartStorageClass(
	"hkStorageMeshShapeSubpartStorage",
	&hkReferencedObjectClass, // parent
	sizeof(hkStorageMeshShape::SubpartStorage),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkStorageMeshShape_SubpartStorageClass_Members),
	int(sizeof(hkStorageMeshShape_SubpartStorageClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkStorageMeshShape
//
const hkInternalClassMember hkStorageMeshShape::Members[] =
{
	{ "storage", &hkStorageMeshShapeSubpartStorageClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkStorageMeshShape,m_storage) }
};
extern const hkClass hkMeshShapeClass;

extern const hkClass hkStorageMeshShapeClass;
const hkClass hkStorageMeshShapeClass(
	"hkStorageMeshShape",
	&hkMeshShapeClass, // parent
	sizeof(hkStorageMeshShape),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkStorageMeshShape::Members),
	int(sizeof(hkStorageMeshShape::Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

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
