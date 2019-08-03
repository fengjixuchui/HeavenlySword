/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_DEFORM_SKINNING_HKMESHBINDING_HKCLASS_H
#define HKANIMATION_DEFORM_SKINNING_HKMESHBINDING_HKCLASS_H

class hkxMesh;
class hkBone;
class hkSkeleton;

/// hkMeshBinding meta information
extern const class hkClass hkMeshBindingClass;

/// A link between a set of bones and an index mesh (the indices referring to the
/// bones, in order).
class hkMeshBinding
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_DATA, hkMeshBinding);
		HK_DECLARE_REFLECTION();

		struct Mapping
		{
			HK_DECLARE_REFLECTION();

			hkInt16* m_mapping;
			hkInt32 m_numMapping;
		};

		//
		// Members
		//
	public:
		
			/// The mesh (hopefully indexed)
		hkxMesh* m_mesh;
		
			/// The skeleton that holds the bones.
		hkSkeleton* m_skeleton;
		
			/// The bone indices that apply to this mesh, as seen by the skeleton.
			/// If there is no mapping, then the mapping is trivial in that the 
			/// indices in the mesh represent the bone number directly.
			/// If there is only one mapping then all sections in the mesh use the 
			/// same indexed pallete.
			/// If there is more than one mapping then it implies that there
			/// is a mapping per mesh section's index buffer, in order. This can happen 
			/// due to breaking up exported sections and reindexing to keep within bone 
			/// influence limits imposed by hardware skinning for instance.
		struct Mapping* m_mappings; 
		hkInt32 m_numMappings;
		
			/// This array contains a transformation for each bone in the skeleton. The
			/// transformation is the inverse of the original bone transform in its 
			/// T-pose or bind pose, multiplied by the initial world transform of the skin. 
			/// By storing this we only have to store vertices once in skin space, rather than 
			/// multiple times in the local space of each bone.
			/// Therefore, and despite the name of the member, the transformations stored are 
			/// bone-from-skinMesh rather than bone-from-World transformations.
		hkTransform* m_inverseWorldBindPose;
		hkInt32 m_numInverseWorldBindPose;
};

#endif // HKANIMATION_DEFORM_SKINNING_HKMESHBINDING_HKCLASS_H

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
