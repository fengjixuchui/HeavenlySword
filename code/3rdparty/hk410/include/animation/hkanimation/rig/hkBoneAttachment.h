/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_RIG_HKBONEATTACHMENT_HKCLASS_H
#define HKANIMATION_RIG_HKBONEATTACHMENT_HKCLASS_H

#include <hkanimation/rig/hkBone.h>
#include <hkanimation/rig/hkSkeleton.h>

/// hkBoneAttachment meta information
extern const class hkClass hkBoneAttachmentClass;

/// A link between a bone and a reflected object (mesh, light, camera, etc.).
class hkBoneAttachment
{
	public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RIG, hkBoneAttachment );
	HK_DECLARE_REFLECTION();
		
		//
		// Members
		//
	public:
		
			/// The transform from the local space of the mesh to the space of the attach point
		hkMatrix4 m_boneFromAttachment;

			/// The object attached. Check the variant class and object pointer to find out what it is.
		hkVariant m_attachment;

			/// A name that can be used to recognize the attachment
		const char* m_name;

			/// The bone in that skeleton.
		hkInt16 m_boneIndex;
};

#endif // HKANIMATION_RIG_HKBONEATTACHMENT_HKCLASS_H

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
