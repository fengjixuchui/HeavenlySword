/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/animation/hkAnimationBinding.h>
#include <hkanimation/animation/hkSkeletalAnimation.h>
#include <hkanimation/rig/hkBone.h>
#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>

#include <hkbase/memory/hkLocalArray.h>

void hkSkeletonUtils::transformLocalPoseToWorldPose ( int numTransforms, const hkInt16* parentIndices, const hkQsTransform& worldFromModel, const hkQsTransform* poseLocal, hkQsTransform* poseWorldOut)
{
	for( int i = 0; i < numTransforms; i++ )
	{
		const int parentId = parentIndices[i];
		HK_ASSERT(0x675efb2b,  (parentIndices[i] == -1) || (parentIndices[i] < i) );
		const hkQsTransform& worldFromParent = ( -1 != parentId ) ? poseWorldOut[parentId] : worldFromModel;
		poseWorldOut[i].setMul( worldFromParent, poseLocal[i] );
	}
}

void hkSkeletonUtils::transformWorldPoseToLocalPose ( int numTransforms, const hkInt16* parentIndices, const hkQsTransform& worldFromModel, const hkQsTransform* poseWorld, hkQsTransform* poseLocalOut)
{
	for( int i = 0; i < numTransforms; i++ )
	{
		const int parentId = parentIndices[i];
		const hkQsTransform& parentFromWorld = ( -1 != parentId ) ? poseWorld[parentId] : worldFromModel;
		HK_ASSERT(0x675efb2b,  (parentIndices[i] == -1) || (parentIndices[i] < i) );
		poseLocalOut[i].setMulInverseMul( parentFromWorld, poseWorld[i] );
	}
}

void hkSkeletonUtils::transformLocalPoseToModelPose ( int numTransforms, const hkInt16* parentIndices, const hkQsTransform* poseLocal, hkQsTransform* poseModelOut)
{
	transformLocalPoseToWorldPose( numTransforms, parentIndices, hkQsTransform::getIdentity(), poseLocal, poseModelOut);
}

void hkSkeletonUtils::transformModelPoseToLocalPose ( int numTransforms, const hkInt16* parentIndices, const hkQsTransform* poseModel, hkQsTransform* poseLocalOut)
{
	transformWorldPoseToLocalPose( numTransforms, parentIndices, hkQsTransform::getIdentity(), poseModel, poseLocalOut);
}

void hkSkeletonUtils::transformModelPoseToWorldPose ( int numTransforms, const hkQsTransform& worldFromModel, const hkQsTransform* poseModel, hkQsTransform* poseWorldOut)
{
	for( int i = 0; i < numTransforms; i++ )
	{
		poseWorldOut[i].setMul( worldFromModel, poseModel[i] );
	}
}

void hkSkeletonUtils::transformWorldPoseToModelPose ( int numTransforms, const hkQsTransform& worldFromModel, const hkQsTransform* poseWorld, hkQsTransform* poseModelOut)
{
	for( int i = 0; i < numTransforms; i++ )
	{
		poseModelOut[i].setMulInverseMul( worldFromModel, poseWorld[i] );
	}
}


void hkSkeletonUtils::enforceSkeletonConstraintsLocalSpace (const hkSkeleton& skeleton, hkQsTransform* poseInOut)
{
	const int numBones = skeleton.m_numBones;

	for (int i=0; i< numBones; i++)
	{
		hkBone* bone = skeleton.m_bones[i];

		if (bone->m_lockTranslation)
		{
			poseInOut[i].m_translation = skeleton.m_referencePose[i].m_translation;
		}
	}
}

void hkSkeletonUtils::enforcePoseConstraintsLocalSpace (const hkSkeleton& skeleton, const hkQsTransform* constraintsLocal, hkQsTransform* poseInOut)
{
	const int numBones = skeleton.m_numBones;

	for (int i=0; i< numBones; i++)
	{
		hkBone* bone = skeleton.m_bones[i];

		if (bone->m_lockTranslation)
		{
			poseInOut[i].m_translation = constraintsLocal[i].m_translation;
		}
	}
}

void hkSkeletonUtils::enforceSkeletonConstraintsModelSpace (const hkSkeleton& skeleton, hkQsTransform* poseModelInOut)
{
	const int numBones = skeleton.m_numBones;

	for (int boneIdx=0; boneIdx< numBones; ++boneIdx)
	{
		const hkBone* bone = skeleton.m_bones[boneIdx];
		const hkInt16 parentIdx = skeleton.m_parentIndices[boneIdx];

		if (bone->m_lockTranslation)
		{
			const hkVector4& localTranslation = skeleton.m_referencePose[boneIdx].m_translation;

			if (parentIdx != -1)
			{
				const hkQsTransform& parentModelSpace = poseModelInOut[parentIdx];
				poseModelInOut[boneIdx].m_translation.setRotatedDir(parentModelSpace.getRotation(), localTranslation);
				poseModelInOut[boneIdx].m_translation.add4(parentModelSpace.getTranslation());
			}
			else
			{
				poseModelInOut[boneIdx].m_translation = localTranslation;
			}
		}
	}
}

void hkSkeletonUtils::enforcePoseConstraintsModelSpace (const hkSkeleton& skeleton, const hkQsTransform* constraintsLocal, hkQsTransform* poseModelInOut)
{
	const int numBones = skeleton.m_numBones;

	for (int boneIdx=0; boneIdx< numBones; ++boneIdx)
	{
		const hkBone* bone = skeleton.m_bones[boneIdx];
		const hkInt16 parentIdx = skeleton.m_parentIndices[boneIdx];

		if (bone->m_lockTranslation)
		{
			const hkVector4& localTranslation = constraintsLocal[boneIdx].m_translation;

			if (parentIdx != -1)
			{
				const hkQsTransform& parentModelSpace = poseModelInOut[parentIdx];
				poseModelInOut[boneIdx].m_translation.setRotatedDir(parentModelSpace.getRotation(), localTranslation);
				poseModelInOut[boneIdx].m_translation.add4(parentModelSpace.getTranslation());
			}
			else
			{
				poseModelInOut[boneIdx].m_translation = localTranslation;
			}
		}
	}
}

void hkSkeletonUtils::lockTranslations (const hkSkeleton& skeleton, hkBool exceptRoots)
{
	const int numBones = skeleton.m_numBones;

	for (int i=0; i< numBones; i++)
	{
		hkBone* bone = skeleton.m_bones[i];
		
		if ((skeleton.m_parentIndices[i]>0) || (!exceptRoots))
		{
			bone->m_lockTranslation = true;
		}
	}
}

// This one does allocate space in the array (since length is unknown)
hkBool hkSkeletonUtils::getBoneChain(const hkSkeleton& skeleton, hkInt16 startBone, hkInt16 endBone, hkArray<hkInt16>& bonesInOut)
{
	// Go backwards from the end of the chain to the start
	hkInt16 boneId;
	for (boneId = endBone; (boneId > 0) && (boneId != startBone); boneId = skeleton.m_parentIndices[boneId])
	{
		bonesInOut.insertAt(0, boneId);
	}
	bonesInOut.insertAt(0, startBone);

	const hkBool ok =  (boneId>0);

	return ok;
}

hkInt16 hkSkeletonUtils::findBoneWithName (const hkSkeleton& skeleton, const char* name, hkStringCompareFunc compare)
{
	const int numBones = skeleton.m_numBones;

	if (compare == HK_NULL)
		compare = hkString::strCasecmp;

	for (hkInt16 boneId=0; boneId < numBones; boneId++)
	{
		if ( compare(name, skeleton.m_bones[boneId]->m_name) == 0 )
		{
			return boneId;
		}
	}

	return -1;
}




// Unlocks all the translations for the given skeleton
void hkSkeletonUtils::unlockTranslations (const hkSkeleton& skeleton)
{
	const int numBones = skeleton.m_numBones;

	for (int i=0; i< numBones; i++)
	{
		hkBone* bone = skeleton.m_bones[i];

		bone->m_lockTranslation = false;
	}

}


void hkSkeletonUtils::getDescendants(const hkSkeleton& skeleton, hkInt16 startBone, hkArray<hkInt16>& bonesOut)
{
	hkLocalArray<hkBool> included(skeleton.m_numBones);
	included.setSize(skeleton.m_numBones, false);

	included[startBone] = true;
	for (hkInt16 b=startBone; b<skeleton.m_numBones; b++)
	{
		const hkInt16 parent = skeleton.m_parentIndices[b];
		if (included[parent])
		{
			included[b] = true;
			bonesOut.pushBack(b);
		}
	}
}

inline void blendTwoTransforms (const hkQsTransform& transformA, const hkQsTransform& transformB, const hkReal weight, hkQsTransform& transformOut)
{
	const hkSimdReal sWeight(weight);

	transformOut.m_translation.setInterpolate4(
		transformA.m_translation,
		transformB.m_translation, sWeight);

	// Make sure we blend the closest representation of the quaternion; flip if necessary
	hkVector4 bRot = transformB.m_rotation.m_vec;
	if ( transformA.m_rotation.m_vec.dot4(transformB.m_rotation.m_vec) < 0.0f )
		bRot.mul4(-1.0f);

	transformOut.m_rotation.m_vec.setInterpolate4(
		transformA.m_rotation.m_vec,
		bRot, sWeight);

	transformOut.m_scale.setInterpolate4(
		transformA.m_scale,
		transformB.m_scale, sWeight);

	// Check Rotation
	{
		const hkReal quatNorm = transformOut.m_rotation.m_vec.lengthSquared4();
		if (quatNorm < HK_REAL_EPSILON)
		{
			// no rotations blended (or cancelled each other) -> set to identity
			transformOut.m_rotation.setIdentity();
		}
		else
		{
			// normalize
			transformOut.m_rotation.normalize();
		}
	}
}

void hkSkeletonUtils::blendPoses( hkUint32 numBones, const hkQsTransform* poseA, const hkQsTransform* poseB, const hkReal* weights, hkQsTransform* poseOut )
{
	for (hkUint32 i=0; i < numBones; i++)
	{
		blendTwoTransforms (poseA[i], poseB[i], weights[i], poseOut[i]);
	}
}

void hkSkeletonUtils::blendPoses( hkUint32 numBones, const hkQsTransform* poseA, const hkQsTransform* poseB, const hkReal weight, hkQsTransform* poseOut )
{
	for (hkUint32 i=0; i < numBones; i++)
	{
		blendTwoTransforms (poseA[i], poseB[i], weight, poseOut[i]);
	}
}

void hkSkeletonUtils::blendPartialPoses( hkUint32 numBones, hkInt16* bones, const hkQsTransform* poseA, const hkQsTransform* poseB, const hkReal weight, hkQsTransform* poseOut )
{
	for( hkUint32 i = 0; i < numBones; i++ )
	{
		blendTwoTransforms( poseA[ bones[i] ], poseB[ bones[i] ], weight, poseOut[ bones[i] ] );
	}
}

hkBool hkSkeletonUtils::isBindingOk( const hkSkeleton& skeleton, const class hkAnimationBinding& binding )
{
	for (int i=0; i < binding.m_numAnimationTrackToBoneIndices; i++)
	{
		if (( binding.m_animationTrackToBoneIndices[i] < 0) || ( binding.m_animationTrackToBoneIndices[i] >= skeleton.m_numBones ))
			return false;
	}

	return true;

}

void hkSkeletonUtils::getModelSpaceScale (const hkSkeleton& skeleton, const hkQsTransform* poseLocal, int boneId, hkVector4& scaleOut)
{
	scaleOut.set(1.0f,1.0f,1.0f);
	while (boneId!=-1)
	{
		scaleOut.mul4(poseLocal[boneId].getScale());
		boneId = skeleton.m_parentIndices[boneId];
	}
}

void hkSkeletonUtils::normalizeRotations(hkQsTransform* transformsInOut, int numTransforms )
{
	for (int i=0; i < numTransforms; i++)
	{
		transformsInOut[i].m_rotation.normalize();
	}
}

hkResult hkSkeletonUtils::bindByName(const hkSkeleton& skeleton, const hkSkeletalAnimation& anim, hkStringCompareFunc compare, class hkAnimationBinding& bindingOut)
{
	// Check binding is initialized
	if ((!bindingOut.m_animationTrackToBoneIndices) || (bindingOut.m_numAnimationTrackToBoneIndices != anim.m_numberOfTracks))
	{
		HK_ERROR(  0x654e3435, "Binding is not initiialized" );
		return HK_FAILURE;
	}

	// Check anim has track names
	if (anim.m_numAnnotationTracks != anim.m_numberOfTracks )
	{
		HK_ERROR(  0x654e3437, "No track names present in animation" );
		return HK_FAILURE;
	}

	// Check skeleton has bone names
	for (int b=0; b < skeleton.m_numBones; b++)
	{
		const char* boneName = skeleton.m_bones[b]->m_name;
		if (boneName == HK_NULL)
		{
			HK_ERROR(  0x654e3437, "No bone name for bone " << b );
			return HK_FAILURE;
		}
	}

	for (int t=0; t < anim.m_numberOfTracks; t++)
	{
		bool foundMatch = false;
		const char* trackName = anim.m_annotationTracks[t]->m_name;

		for (hkInt16 b=0; b < skeleton.m_numBones; b++)
		{
			const char* boneName = skeleton.m_bones[b]->m_name;
			if ( compare(boneName, trackName ) == 0)
			{
				foundMatch = true;
				bindingOut.m_animationTrackToBoneIndices[t] = b;
			}
		}

		if (!foundMatch)
		{
			HK_ERROR(  0x654e3438, "No match for track " << trackName );
			return HK_FAILURE;
		}
	}

	return HK_SUCCESS;
}

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
