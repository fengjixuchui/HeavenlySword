/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_HKANIMATIONCONTAINER_HKCLASS_H
#define HKANIMATION_HKANIMATIONCONTAINER_HKCLASS_H

/// hkAnimationContainer meta information
extern const class hkClass hkAnimationContainerClass;

/// This is the top level container class for animation data
class hkAnimationContainer
{
	public:
		
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_DATA, hkAnimationContainer );
	HK_DECLARE_REFLECTION();
		
		//
		// Members
		//
	public:
		
			/// The rigs in the scene
		class hkSkeleton** m_skeletons;
		hkInt32 m_numSkeletons;
		
			/// The animations in the scene
		class hkSkeletalAnimation** m_animations;
		hkInt32 m_numAnimations;
		
			/// The bindings from an animations to a set of bone numbers. Check the skeleton you
			/// apply them to fits.
		class hkAnimationBinding** m_bindings;
		hkInt32 m_numBindings;
		
			/// The attachments in the scene
		class hkBoneAttachment** m_attachments;
		hkInt32 m_numAttachments;
		
			/// The skins in the scene that reference the above skeletons.
		class hkMeshBinding** m_skins;
		hkInt32 m_numSkins;

		//
		// Utility methods
		//

			/// Finds the given skeleton in the animation container
		hkSkeleton* findSkeletonByName (const char* name) const;

			/// Finds the given bone attachment in the animation container
		hkBoneAttachment* findBoneAttachmentByName (const char* name) const;


};

#endif // HKANIMATION_HKANIMATIONCONTAINER_HKCLASS_H

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
