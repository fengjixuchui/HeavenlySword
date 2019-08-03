/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_HKANIMATIONBINDING_HKCLASS_H
#define HKANIMATION_ANIMATION_HKANIMATIONBINDING_HKCLASS_H

/// hkAnimationBinding meta information
extern const class hkClass hkAnimationBindingClass;

/// Describes how an animation is bound to a skeleton
/// This class is useful when we have partial animations which only animate a subset of the skeleton.
class hkAnimationBinding
{
	public:
	
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_DATA, hkAnimationBinding);
	HK_DECLARE_REFLECTION();
		
		//
		// Members
		//
	public:

			/// Default constructor
		hkAnimationBinding();

			/// A handle to the animation bound to the skeleton.
		class hkSkeletalAnimation* m_animation;
		
			/// A mapping from animation track indices to bone indices.
		hkInt16* m_animationTrackToBoneIndices;
		hkInt32 m_numAnimationTrackToBoneIndices;

			/// A hint to indicate how this animation should be blended
			/// See hkAnimatedSkeleton for a description of how this affects animation blending
		enum BlendHint
		{
			NORMAL = 0,
			ADDITIVE = 1
		};

		hkEnum<enum BlendHint, hkInt8> m_blendHint; //+default(0/*hkAnimationBinding::NORMAL*/)

	public:

			// Constructor for initialisation of vtable fixup
		hkAnimationBinding( hkFinishLoadedObjectFlag flag ) {}

};

#include <hkanimation/animation/hkAnimationBinding.inl>

#endif // HKANIMATION_ANIMATION_HKANIMATIONBINDING_HKCLASS_H

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
