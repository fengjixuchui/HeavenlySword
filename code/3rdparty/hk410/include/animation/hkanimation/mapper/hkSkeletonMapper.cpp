/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/mapper/hkSkeletonMapper.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkanimation/rig/hkPose.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/monitor/hkMonitorStream.h>

#include <hkbase/class/hkTypeInfo.h>
HK_REFLECTION_DEFINE_VIRTUAL(hkSkeletonMapper);

hkSkeletonMapper::hkSkeletonMapper(const hkSkeletonMapperData& mapping)
{
	m_mapping = mapping;

#ifdef HK_DEBUG
	checkMapping(mapping);
#endif
}

// Destructor
hkSkeletonMapper::~hkSkeletonMapper()
{
}


void hkSkeletonMapper::mapPose ( const hkQsTransform* poseAModelSpace, const hkQsTransform* originalPoseBLocalSpace, hkQsTransform* poseBModelSpace, ConstraintSource source  ) const
{
	HK_TIMER_BEGIN_LIST("MapPose", "init");
#ifdef HK_DEBUG
	hkLocalArray<int> outputIntialized(m_mapping.m_skeletonB->m_numBones);
	{ 
		outputIntialized.setSizeUnchecked( m_mapping.m_skeletonB->m_numBones );
		// clear flags 
		for (int i = 0; i < m_mapping.m_skeletonB->m_numBones; i++ )
		{
			outputIntialized[i] = false;
		}
	}
#endif

	/*
	** Direct access arrays, for optimized access
	*/
	HK_TIMER_SPLIT_LIST("simpleMappings");

	// First start with the simple mappings
	{
		const int numSimpleMappings = m_mapping.m_simpleMappings.getSize();
		for (int i = 0; i <numSimpleMappings; ++i)
		{
			const hkSkeletonMapperData::SimpleMapping& simpleMapping = m_mapping.m_simpleMappings[i];
			HK_ON_DEBUG( outputIntialized[simpleMapping.m_boneB] = 1);
			poseBModelSpace[simpleMapping.m_boneB].setMul(poseAModelSpace[simpleMapping.m_boneA], simpleMapping.m_aFromBTransform );
		}
	}

	HK_TIMER_SPLIT_LIST("chainMappings");

	// Then do the chains
	{

		// Start by positioning them to world
		const int numChainMappings = m_mapping.m_chainMappings.getSize();
		for (int i = 0; i <numChainMappings; i++)
		{
			const hkSkeletonMapperData::ChainMapping& chainMapping = m_mapping.m_chainMappings[i];

			const hkInt16 startBoneA = chainMapping.m_startBoneA;
			const hkInt16 endBoneA = chainMapping.m_endBoneA;
			const hkInt16 startBoneB = chainMapping.m_startBoneB;
			const hkInt16 endBoneB = chainMapping.m_endBoneB;

			// Aligning the start of the chain with A
			// (this is probably done already by simple m_mapping)
			HK_ON_DEBUG( outputIntialized[startBoneB] = 1 );
			poseBModelSpace[startBoneB].setMul(poseAModelSpace[startBoneA], chainMapping.m_startAFromBTransform );

			// Get the bones in the chain
			hkLocalArray<hkInt16> chainBones (endBoneB - startBoneB + 1); // maximum size
			hkSkeletonUtils::getBoneChain(*m_mapping.m_skeletonB, startBoneB, endBoneB, chainBones);

			// Calculate the current position of the end of chain (based on start of chain)
			hkVector4 currentEndModelSpacePos;
			{
				// modelFromEnd = modelFromStart * startFromNext * .... * parentFromEnd
				currentEndModelSpacePos = originalPoseBLocalSpace[endBoneB].getTranslation();

				for (int b=chainBones.getSize()-2; b>=1; b--)
				{
					hkInt16 bone = chainBones[b];

					currentEndModelSpacePos.setTransformedPos(originalPoseBLocalSpace[bone], currentEndModelSpacePos );
				}

				currentEndModelSpacePos.setTransformedPos(poseBModelSpace[startBoneB], currentEndModelSpacePos);
			}

			// Rotate the start of the chain to the proper direction
			{
				const hkVector4& startModelSpacePos = poseBModelSpace[startBoneB].getTranslation();
				//const hkVector4& currentEndModelSpacePos = poseBModelSpace[endBoneB].getTranslation();
				// worldfromB = worldFromA * afromB
				hkQsTransform desiredEndModelSpace; desiredEndModelSpace.setMul(poseAModelSpace[endBoneA], chainMapping.m_endAFromBTransform );
				const hkVector4& desiredEndModelSpacePos = desiredEndModelSpace.getTranslation();

				// Calculate the difference and reorientate the chain (start bone) so it points to the same direction (no change in shape)
				hkVector4 currentDirection;
				currentDirection.setSub4(currentEndModelSpacePos, startModelSpacePos);
				currentDirection.normalize3();

				hkVector4 desiredDirection;
				desiredDirection.setSub4(desiredEndModelSpacePos, startModelSpacePos);
				desiredDirection.normalize3();

				hkQuaternion rotationQuaternion;
				rotationQuaternion.setShortestRotation( currentDirection, desiredDirection );

				// Rotate start of chain(s)
				const hkQuaternion currentRot = poseBModelSpace[startBoneB].getRotation();

				hkQuaternion temp;
				temp.setMul(rotationQuaternion, currentRot);
				HK_ON_DEBUG( outputIntialized[startBoneB] = 1);
				poseBModelSpace[startBoneB].setRotation(temp);
			}

			// Rotate the rest of the bones in the chain (keep them local)
			{
				for (int b=1; b<chainBones.getSize()-1; b++)
				{
					hkInt16 childIdx = chainBones[b];
					hkInt16 parentIdx = chainBones[b-1];

					// modelFromChild = modelFromParent * parentFromChild
					const hkQsTransform& parentFromChild = originalPoseBLocalSpace[childIdx];
					const hkQsTransform& modelFromParent = poseBModelSpace[parentIdx];
					HK_ASSERT2( 0xf032de23, outputIntialized[parentIdx], "Your chain m_mapping is not ordered correctly" );
					HK_ON_DEBUG( outputIntialized[childIdx] = 1);
					poseBModelSpace[childIdx].setMul(modelFromParent, parentFromChild);
				}
			}

		}
	}

	HK_TIMER_SPLIT_LIST("unmappedBones");
	// Fix any unmmapped bones if so requested
	{
		const int numUnmapped = m_mapping.m_unmappedBones.getSize();
		if (m_mapping.m_keepUnmappedLocal)
		{
			for (int i=0; i<numUnmapped; ++i)
			{
				const hkInt16 childIdx = m_mapping.m_unmappedBones[i];
				const hkInt16 parentIdx = m_mapping.m_skeletonB->m_parentIndices[childIdx];

				// If the root was unmapped there is nothing we can do
				if (parentIdx != -1)
				{
					// modelFromChild = modelFromParent * parentFromChild
					const hkQsTransform& parentFromChild  = originalPoseBLocalSpace[childIdx];
					const hkQsTransform& modelFromParent  = poseBModelSpace[parentIdx];
					HK_ASSERT2( 0xf032de24, outputIntialized[parentIdx], "Your m_mapping is not ordered correctly" );
					poseBModelSpace[childIdx].setMul(modelFromParent , parentFromChild);
				}
				else
				{
					poseBModelSpace[childIdx] = originalPoseBLocalSpace[childIdx];
				}
				HK_ON_DEBUG( outputIntialized[childIdx] = 1);
			}
		}
	}

	HK_TIMER_SPLIT_LIST("enforceConstraints");
	{
		switch (source)
		{
		case hkSkeletonMapper::REFERENCE_POSE: 
			hkSkeletonUtils::enforceSkeletonConstraintsModelSpace( *m_mapping.m_skeletonB, poseBModelSpace);
			break;
		case hkSkeletonMapper::CURRENT_POSE: 
			hkSkeletonUtils::enforcePoseConstraintsModelSpace( *m_mapping.m_skeletonB, originalPoseBLocalSpace, poseBModelSpace);
			break;
		default:
			/* no constraints */
			break;
		}
	}
	HK_TIMER_END_LIST();
}

void hkSkeletonMapper::mapPose(const hkPose& poseAIn, hkPose& poseBInOut, ConstraintSource source ) const
{
	const hkQsTransform* modelAIn = poseAIn.getPoseModelSpace().begin();
	const hkQsTransform* localBIn  = poseBInOut.getPoseLocalSpace().begin();
	hkQsTransform*       modelBInOut = poseBInOut.accessPoseModelSpace().begin();

	mapPose(modelAIn, localBIn, modelBInOut, source);
}


void hkSkeletonMapper::checkMapping(const hkSkeletonMapperData& m_mapping)
{
	// Check simple mappings, make sure there are no translations
	{
		for (int i=0; i<m_mapping.m_simpleMappings.getSize(); i++)
		{
			const hkSkeletonMapperData::SimpleMapping& simpleMapping = m_mapping.m_simpleMappings[i];
			const hkInt16 boneIdA = simpleMapping.m_boneA;
			const hkInt16 boneIdB = simpleMapping.m_boneB;
			
			const hkBone* boneA = m_mapping.m_skeletonA->m_bones[boneIdA];
			const hkBone* boneB = m_mapping.m_skeletonB->m_bones[boneIdB];

			if ((boneB->m_lockTranslation))
			{
				// If the target skeletons locks translation for this bone, make sure there is no translation in the A-B transform
				const hkVector4& trans = simpleMapping.m_aFromBTransform.getTranslation();
				const hkReal err = trans.length3();
				if ( err > 0.01f)
				{
					HK_WARN_ALWAYS(0xabbaefbf, "Bones A:"<<boneA->m_name<<" and B:"<<boneB->m_name<<" are not fully aligned (error :"<<err<<").");
				}
			}
		}
	}

	// Check that not all the bones have translations enabled; if so, m_mapping is not really useful
	{
		hkBool atLeastOne = false;
		for (int i=0; i<m_mapping.m_skeletonB->m_numBones; i++)
		{
			const hkBone* bone = m_mapping.m_skeletonB->m_bones[i];
			if (bone->m_lockTranslation)
			{
				atLeastOne = true;
				break;
			}
		}

		if (!atLeastOne)
		{
			HK_WARN_ALWAYS(0xabba77a5, "None of the bones in skeleton have translations locked - did you forget to lock translations ?.");
		}
	}
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
