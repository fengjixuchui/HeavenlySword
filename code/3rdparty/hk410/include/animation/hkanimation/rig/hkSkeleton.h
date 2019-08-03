/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_RIG_HKSKELETON_HKCLASS_H
#define HKANIMATION_RIG_HKSKELETON_HKCLASS_H

#include <hkanimation/rig/hkBone.h>

/// hkSkeleton meta information
extern const class hkClass hkSkeletonClass;

/// The class that represents a character rig.
/// Note that m_numBones==m_numParentIndices==m_numReferencePose. This redundancy
/// is required in order to properly serialize m_bones, m_parentIndices and m_referencePose 
/// as arrays. Check the Serialization documentation for details.
class hkSkeleton
{
	public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RIG, hkSkeleton );
	HK_DECLARE_REFLECTION();	
		
		//
		// Members
		//
	public:

			/// A user name to aid in identifying the skeleton
		const char* m_name;
		
			/// Parent relationship
		hkInt16* m_parentIndices;

			/// Number of parent indices, has to be identical to m_numBones
		hkInt32 m_numParentIndices;
		
			/// Bones for this skeleton
		hkBone** m_bones;
			/// Number of bones in the skeleton
		hkInt32 m_numBones;

			/// The reference pose for this skeleton. This pose is stored in local space.
		hkQsTransform* m_referencePose;
			/// Number of elements in the reference pose array
		int m_numReferencePose;

};

#endif // HKANIMATION_RIG_HKSKELETON_HKCLASS_H

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
