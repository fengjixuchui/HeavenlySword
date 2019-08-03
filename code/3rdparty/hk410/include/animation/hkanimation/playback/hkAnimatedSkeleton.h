/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ANIMATED_SKELETON_H
#define HK_ANIMATED_SKELETON_H

#include <hkbase/hkBase.h>
#include <hkanimation/playback/control/hkAnimationControlListener.h>
#include <hkanimation/animation/hkAnnotationTrack.h>
#include <hkanimation/animation/hkSkeletalAnimation.h>

class hkSkeleton;
class hkChunkCache;
class hkAnimationControl;
class hkPose;

/// This class manages the runtime instance of each skeleton.
/// It is responsible for maintaining the currently active animation controls.
/// It also performs animation blending and mixing.
///  In general controls are sampled and blended by adding their contributions and renormalizing.
///  If an animation binding has a blend hint of additive then this is layered on after other controls
///  have been accumulated and renormalized.
class hkAnimatedSkeleton : public hkReferencedObject, protected hkAnimationControlListener
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ANIM_RUNTIME);

			/// The skeleton below is not reference counted and must persist for the lifetime of this hkAnimatedSkeleton instance
		hkAnimatedSkeleton( const hkSkeleton* skeleton );

		~hkAnimatedSkeleton();

			/// Iterates through all controls and advances each of their local clocks
		virtual void stepDeltaTime( hkReal time );

			/// This method is the core of the blending and binding.
			/// It queries all active controls and samples their animations by
			/// iterating over the skeleton extracting bound animation track info.
			/// It then accumulates or blends this data across tracks into a single local
			/// pose. An optional cache can be passed to improve decompression performance.
		virtual void sampleAndCombineAnimations( hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache = HK_NULL ) const;

			/// This method is the core of the blending and binding.
			/// It queries all active controls and samples their animations by
			/// iterating over the skeleton extracting bound animation track info.
			/// It then accumulates or blends this data across tracks into a single local
			/// pose. An optional cache can be passed to improve decompression performance.
			/// This method will sample only the first maxBone bones in the skeleton, to assist in
			/// LODing animations.
		virtual void sampleAndCombinePartialAnimations( hkQsTransform* poseLocalSpaceOut, hkUint32 maxBone, hkChunkCache* cache = HK_NULL ) const;

			/// Access to the original skeleton 
		inline const hkSkeleton* getSkeleton() const;

			/// If the total weight on any bone falls below the reference pose weight threshold
			/// we begin to blend in the reference pose.
		inline hkReal getReferencePoseWeightThreshold() const;

			/// If the total weight on any bone falls below the reference pose threshold
			/// we begin to blend in the reference pose.
		void setReferencePoseWeightThreshold( hkReal val );

			/// Calculates the change in root position and orientation for a given timestep.
			/// This combines the weighted contributions from each of then playing animations.
		void getDeltaReferenceFrame(hkReal deltaTimestep, hkQsTransform& deltaMotionOut) const;

			/*
			 * Animation controls
			 */

			/// Add an animation control.
			/// If the weight on this control is > 0 then the associated animation
			/// will be blended and combined.
		void addAnimationControl( hkAnimationControl*  animation );

			/// Remove an animation control
		void removeAnimationControl( hkAnimationControl* animation );

			/// Return the number of added controls
		inline int getNumAnimationControls() const;

			/// Access to an animation control.
		inline hkAnimationControl* getAnimationControl( int i ) const ;


			/*
			 * Annotation support
			 */

			/// A structure to hold annotation results
			/// The ID is remaped from an animation track ID to a skelton bone ID
		struct BoneAnnotation {
			hkUint16 m_boneID;
			const hkAnnotationTrack::Annotation* m_annotation;
		};

			/// Examine all animation controls and counts the annotations that occur
			/// in the timestep. Usually you query this in advance of stepping the controls
		hkUint32 getNumAnnotations( hkReal deltaTime ) const;

			/// Examine all animation controls and counts the annotations
			/// that occur in the timestep. These are passed back in the preallocated
		void getAnnotations(hkReal deltaTime, BoneAnnotation* annotations) const;

	protected:

		HK_ALIGN16( hkArray<hkAnimationControl*> m_animationControls );

		const hkSkeleton* m_skeleton;

		hkReal m_referencePoseWeightThreshold;

			// Animation Control listener interface
		virtual void controlDeletedCallback(hkAnimationControl* control);

		void sampleAndCombineInternal( hkQsTransform* poseLocalSpaceOut, hkUint32 maxBone, hkChunkCache* cache, bool partial ) const;

};

#include <hkanimation/playback/hkAnimatedSkeleton.inl>

#endif // HK_ANIMATED_SKELETON_H

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
