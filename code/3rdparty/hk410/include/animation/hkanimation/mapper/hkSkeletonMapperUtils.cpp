/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/mapper/hkSkeletonMapperUtils.h>
#include <hkanimation/rig/hkSkeleton.h>

#include <hkbase/memory/hkLocalArray.h>

struct Match
{
	// Index of the match in A
	int m_indexA; 
	// Index of the match in B
	int m_indexB;
};

void hkSkeletonMapperUtils::createMapping (const Params& originalParams, hkSkeletonMapperData& originalAToB, hkSkeletonMapperData& originalBToA)
{
	HK_REPORT_SECTION_BEGIN(0x54e4323e, "hkSkeletonMapperUtils::createMapping");

	// EXP-396
	bool switchedOrder = false;
	Params params (originalParams);
	{
		if (originalParams.m_skeletonA->m_numBones > originalParams.m_skeletonB->m_numBones)
		{
			// Flip
			params.m_skeletonA = originalParams.m_skeletonB;
			params.m_skeletonB = originalParams.m_skeletonA;

			params.m_userMappingsAtoB = originalParams.m_userMappingsBtoA;
			params.m_userMappingsBtoA = originalParams.m_userMappingsAtoB;

			switchedOrder = true;
		}
	}

	// EXP-396
	hkSkeletonMapperData& aToB = switchedOrder ? originalBToA : originalAToB;
	hkSkeletonMapperData& bToA = switchedOrder ? originalAToB : originalBToA;

	const int numBonesA = params.m_skeletonA->m_numBones;
	const int numBonesB = params.m_skeletonB->m_numBones;

	hkLocalArray<hkQsTransform> modelPoseA (numBonesA);
	modelPoseA.setSizeUnchecked(numBonesA);

	hkSkeletonUtils::transformLocalPoseToModelPose( numBonesA, params.m_skeletonA->m_parentIndices, 
													params.m_skeletonA->m_referencePose, modelPoseA.begin());

	hkLocalArray<hkQsTransform> modelPoseB (numBonesB);
	modelPoseB.setSizeUnchecked(numBonesB);

	hkSkeletonUtils::transformLocalPoseToModelPose( numBonesB, params.m_skeletonB->m_parentIndices, 
													params.m_skeletonB->m_referencePose, modelPoseB.begin());

	const int UNMATCHED = -1;
	const int INCHAIN = -2;

	// We keep track of which bones have been mapped, either by simple or by chain mappings
	hkLocalArray<hkInt16> map_AtoB (numBonesA); map_AtoB.setSize(numBonesA, UNMATCHED);
	hkLocalArray<hkInt16> map_BtoA (numBonesB); map_BtoA.setSize(numBonesB, UNMATCHED);

	// General info (skeletons)
	{
		aToB.m_skeletonA = params.m_skeletonA;
		aToB.m_skeletonB = params.m_skeletonB;

		bToA.m_skeletonA = params.m_skeletonB;
		bToA.m_skeletonB = params.m_skeletonA;
	}

	// 2 - User mappings enforced by user (always done by name)
	{
		// A to B
		{
			const int numMappings = params.m_userMappingsAtoB.getSize();
			for (int i=0; i<numMappings; i++)
			{
				const hkSkeletonMapperUtils::UserMapping& mapping = params.m_userMappingsAtoB[i];

				const hkInt16 boneIn = hkSkeletonUtils::findBoneWithName(*params.m_skeletonA, mapping.m_boneIn, params.m_compareNames );
				const hkInt16 boneOut = hkSkeletonUtils::findBoneWithName(*params.m_skeletonB, mapping.m_boneOut, params.m_compareNames );

				if ((boneIn!=-1) && (boneOut!=-1))
				{
					const hkQsTransform& transformIn = modelPoseA[boneIn];
					const hkQsTransform& transformOut = modelPoseB[boneOut];

					hkSkeletonMapperData::SimpleMapping simpleMapping;
					simpleMapping.m_boneA = boneIn;
					simpleMapping.m_boneB = boneOut;
					simpleMapping.m_aFromBTransform.setMulInverseMul(transformIn, transformOut);

					aToB.m_simpleMappings.pushBack(simpleMapping);

					map_BtoA[boneOut] = boneIn;

				}
				else
				{
					HK_WARN_ALWAYS(0xabba1c64, "Couldn't find bones for mapping ("<<mapping.m_boneIn<<" - "<<mapping.m_boneOut<<")");
				}
			}
		}

		// B to A
		{
			const int numMappings = params.m_userMappingsBtoA.getSize();
			for (int i=0; i<numMappings; i++)
			{
				const hkSkeletonMapperUtils::UserMapping& mapping = params.m_userMappingsBtoA[i];

				const hkInt16 boneIn = hkSkeletonUtils::findBoneWithName(*params.m_skeletonB, mapping.m_boneIn, params.m_compareNames);
				const hkInt16 boneOut = hkSkeletonUtils::findBoneWithName(*params.m_skeletonA, mapping.m_boneOut, params.m_compareNames);

				if ((boneIn!=-1) && (boneOut!=-1))
				{
					const hkQsTransform& transformIn = modelPoseB[boneIn];
					const hkQsTransform& transformOut = modelPoseA[boneOut];

					hkSkeletonMapperData::SimpleMapping simpleMapping;
					simpleMapping.m_boneA = boneIn;
					simpleMapping.m_boneB = boneOut;
					simpleMapping.m_aFromBTransform.setMulInverseMul(transformIn, transformOut);

					bToA.m_simpleMappings.pushBack(simpleMapping);

					map_AtoB[boneOut] = boneIn;

				}
				else
				{
					HK_WARN_ALWAYS(0xabba1c64, "Couldn't find bones for mapping ("<<mapping.m_boneIn<<" - "<<mapping.m_boneOut<<")");
				}
			}
		}


	}

	// 3 - User chains
	{
		const int numChains = params.m_userChains.getSize();
		for (int i=0; i<numChains; i++)
		{
			const hkSkeletonMapperUtils::UserChain& chain = params.m_userChains[i];

			// We look for the bones in both skeletons
			hkInt16 startChainA = hkSkeletonUtils::findBoneWithName(*params.m_skeletonA, chain.m_start, params.m_compareNames);
			hkInt16 endChainA = hkSkeletonUtils::findBoneWithName(*params.m_skeletonA, chain.m_end, params.m_compareNames);

			hkInt16 startChainB = hkSkeletonUtils::findBoneWithName(*params.m_skeletonB, chain.m_start, params.m_compareNames);
			hkInt16 endChainB = hkSkeletonUtils::findBoneWithName(*params.m_skeletonB, chain.m_end, params.m_compareNames);

			if ((startChainA<0) && (startChainB<0))
			{
				HK_WARN_ALWAYS(0x38521c64, "Couldn't make a chain ("<<chain.m_start<<" - "<<chain.m_end<<"). Start bone not found in either skeleton.");
				continue;			
			}

			if ((endChainA<0) && (endChainB<0))
			{
				HK_WARN_ALWAYS(0x38521c64, "Couldn't make a chain ("<<chain.m_start<<" - "<<chain.m_end<<"). End bone not found in either skeleton.");
				continue;			
			}

			// Fill the gaps by using simple mappings
			if (startChainA<0) startChainA = map_BtoA[startChainB];
			if (endChainA<0) endChainA = map_BtoA[endChainB];
			if (startChainB<0) startChainB = map_AtoB[startChainA];
			if (endChainB<0) endChainB = map_AtoB[endChainA];

			// Now look for the actual chains
			// Now that have two chains, make sure at least one has length>2
			hkArray<hkInt16> chainA;
			hkArray<hkInt16> chainB;
			const hkBool chainAok = hkSkeletonUtils::getBoneChain(*params.m_skeletonA, startChainA, endChainA, chainA);
			const hkBool chainBok = hkSkeletonUtils::getBoneChain(*params.m_skeletonB, startChainB, endChainB, chainB);

			if (!chainAok || !chainBok)
			{
				HK_WARN_ALWAYS(0xabba1c63, "Couldn't make a chain ("<<chain.m_start<<" - "<<chain.m_end<<")");
				continue;
			}

			const int chainLengthA = chainA.getSize();
			const int chainLengthB = chainB.getSize();

			// Ignore chains of 1
			if (chainLengthA<2 || chainLengthB<2)
			{
				HK_WARN_ALWAYS(0xabba1c64, "Couldn't make a chain ("<<chain.m_start<<" - "<<chain.m_end<<")");
				continue;
			}

			// Make sure the in-between links in the chain are free
			{
				for (int a=1; a<chainLengthA-1; a++)
				{
					if (map_AtoB[chainA[a]] != UNMATCHED)
					{
						HK_WARN_ALWAYS(0xabba1c65, "Couldn't make a chain ("<<chain.m_start<<" - "<<chain.m_end<<")");
						continue;
					}
				}

				for (int b=1; b<chainLengthB-1; b++)
				{
					if (map_BtoA[chainB[b]] != UNMATCHED)
					{
						HK_WARN_ALWAYS(0xabba1c66, "Couldn't make a chain ("<<chain.m_start<<" - "<<chain.m_end<<")");
						continue;
					}
				}
			}

			// Ok, everything seems good, go ahead and make a chain
			{
				HK_REPORT("Creating chain " <<
					params.m_skeletonA->m_bones[startChainA]->m_name << " - " <<
					params.m_skeletonA->m_bones[endChainA]->m_name);

				const hkQsTransform& transformStartA = modelPoseA[startChainA];
				const hkQsTransform& transformStartB = modelPoseB[startChainB];
				const hkQsTransform& transformEndA = modelPoseA[endChainA];
				const hkQsTransform& transformEndB = modelPoseB[endChainB];

				// A to B
				{
					hkSkeletonMapperData::ChainMapping chainMappingAtoB;

					chainMappingAtoB.m_startBoneA = startChainA;
					chainMappingAtoB.m_endBoneA = endChainA;
					chainMappingAtoB.m_startBoneB = startChainB;
					chainMappingAtoB.m_endBoneB = endChainB;
					chainMappingAtoB.m_endAFromBTransform.setMulInverseMul(transformEndA, transformEndB);
					chainMappingAtoB.m_startAFromBTransform.setMulInverseMul(transformStartA, transformStartB);

					aToB.m_chainMappings.pushBack(chainMappingAtoB);
				}


				// B to A
				{
					hkSkeletonMapperData::ChainMapping chainMappingBtoA;

					chainMappingBtoA.m_startBoneA = startChainB;
					chainMappingBtoA.m_endBoneA = endChainB;
					chainMappingBtoA.m_startBoneB = startChainA;
					chainMappingBtoA.m_endBoneB = endChainA;
					chainMappingBtoA.m_endAFromBTransform.setMulInverseMul(transformEndB, transformEndA);
					chainMappingBtoA.m_startAFromBTransform.setMulInverseMul(transformStartB, transformStartA);

					bToA.m_chainMappings.pushBack(chainMappingBtoA);
				}

				// Mark the links in between as INCHAIN
				// (the start and end will be matched as simple mappings)
				{
					for (int a=1; a<chainLengthA-1; a++)
					{
						map_AtoB[chainA[a]] = INCHAIN;
					}
					for (int b=1; b<chainLengthB-1; b++)
					{
						map_BtoA[chainB[b]] = INCHAIN;
					}
				}

			}

		}
	}

	// 4 - Non User Simple mappings
	if (params.m_autodetectSimple)
	{
		hkArray<Match> matches;

		// Check names ( if (compare || (no compare & no tolerance)) )
		hkStringCompareFunc compare = (params.m_compareNames == HK_NULL) ? (params.m_positionMatchTolerance > 0.0f? HK_NULL : hkString::strCasecmp) : params.m_compareNames;
		hkSimdReal tolSqrd = (params.m_positionMatchTolerance * params.m_positionMatchTolerance);
	
		for (int idxA = 0; idxA < numBonesA; idxA++)
		{
			if (compare) // find any strings that 'match'
			{
				for (int idxB = 0; idxB < numBonesB; idxB++)
				{
					const char* nameA =  params.m_skeletonA->m_bones[idxA]->m_name;
					const char* nameB =  params.m_skeletonB->m_bones[idxB]->m_name;
					
					// are the names the 'same'
					if ( compare( nameA, nameB ) == 0)
					{
						Match& match = matches.expandOne();
						match.m_indexA = idxA;
						match.m_indexB = idxB;
					}
				}
			}
			else // positional, find closest bone (so that tolerance can be larger)
			{
				const hkQsTransform& transformA = modelPoseA[idxA];

				float shortest = 100000.0f;
				int shortestIndex = -1;
				for (int idxB = 0; idxB < numBonesB; idxB++)
				{
					const hkQsTransform& transformB = modelPoseB[idxB];
					
					// are transform A and B similar enough to simple match? Just use the translation
					hkVector4 dif; dif.setSub4( transformA.getTranslation(), transformB.getTranslation() );
					float ls = dif.lengthSquared3();
					if (ls < shortest)
					{
						shortest = ls;
						shortestIndex = idxB;
					}
				}
				if (shortest < tolSqrd)
				{
					Match& match = matches.expandOne();
					match.m_indexA = idxA;
					match.m_indexB = shortestIndex;
				}
			}
		}

		const int numMatches = matches.getSize();

		// Go through the names that matched and create simple mappings for them
		{
			for (int i = 0; i<numMatches; i++)
			{

				const hkInt16 boneA = static_cast<hkInt16> (matches[i].m_indexA);
				const hkInt16 boneB = static_cast<hkInt16> (matches[i].m_indexB);

				// If the bones are already matched (or inside a chain), ignore them
				if ( map_AtoB[boneA] != UNMATCHED || map_BtoA[boneB] != UNMATCHED)
				{
					continue;
				}

				const hkQsTransform& transformA = modelPoseA[boneA];
				const hkQsTransform& transformB = modelPoseB[boneB];

				// A to B
				{
					hkSkeletonMapperData::SimpleMapping simpleMappingAtoB;
					simpleMappingAtoB.m_boneA = boneA;
					simpleMappingAtoB.m_boneB = boneB;
					simpleMappingAtoB.m_aFromBTransform.setMulInverseMul(transformA, transformB);

					aToB.m_simpleMappings.pushBack(simpleMappingAtoB);
				}

				// B to A
				{
					hkSkeletonMapperData::SimpleMapping simpleMappingBtoA;
					simpleMappingBtoA.m_boneA = boneB;
					simpleMappingBtoA.m_boneB = boneA;
					simpleMappingBtoA.m_aFromBTransform.setMulInverseMul(transformB, transformA);

					bToA.m_simpleMappings.pushBack(simpleMappingBtoA);
				}

				// Keep track that this bones are already mapped
				map_AtoB[boneA] = boneB;
				map_BtoA[boneB] = boneA;

				hkString msg = hkString("Matching ") + params.m_skeletonA->m_bones[boneA]->m_name;
				HK_REPORT(msg.cString());
			}
		}
	}

	
	// 5 - Non User Chains
	// Revise the bones that haven't been mapped, check if they could belong to a chain
	if (params.m_autodetectChains)
	{
		for (int boneA = 0; boneA <numBonesA; boneA++)
		{
			if (map_AtoB[boneA]<0) // there was no match
			{
				continue;
			}

			const hkInt16 endChainA = static_cast<hkInt16> (boneA);
			const hkInt16 endChainB = map_AtoB[boneA];

			// Now, look for the closest parent that was matched against B
			hkInt16 startChainA = params.m_skeletonA->m_parentIndices[boneA];

			while ((startChainA != -1) && ( UNMATCHED == map_AtoB[startChainA]) )
			{
				startChainA = params.m_skeletonA->m_parentIndices[startChainA];
			}

			// We couldn't find the start of the chain (or we crosses another chain)
			if ( (-1 == startChainA) || (INCHAIN == map_AtoB[startChainA]) )
			{
				continue;
			}

			const hkInt16 startChainB = map_AtoB[startChainA];

			// Now that have two chains, make sure at least one has length>2
			hkArray<hkInt16> chainA;
			hkArray<hkInt16> chainB;
			const hkBool chainAok = hkSkeletonUtils::getBoneChain(*params.m_skeletonA, startChainA, endChainA, chainA);
			const hkBool chainBok = hkSkeletonUtils::getBoneChain(*params.m_skeletonB, startChainB, endChainB, chainB);

			// One of the chains (the second, really) is not really a chain (bones are not connected)
			if (!chainAok || !chainBok)
			{
				continue;
			}

			const int chainLengthA = chainA.getSize();
			const int chainLengthB = chainB.getSize();

			// Ignore maps of length 2 (we handle them as simple mappings)
			if (chainLengthA==2 && chainLengthB==2)
			{
				continue;
			}

			// Make sure the in-between links in the other chain are also free
			{
				bool ok = true;
				for (int b=1; b<chainLengthB-1; b++)
				{
					if (map_BtoA[chainB[b]] != UNMATCHED)
					{
						ok = false;
						continue;
					}
				}
				if (!ok) continue;
			}

			// Ok, we can go ahead - we have a proper chain mapping

			{
				HK_REPORT("Creating chain " <<
					params.m_skeletonA->m_bones[startChainA]->m_name << " - " <<
					params.m_skeletonA->m_bones[endChainA]->m_name);

				const hkQsTransform& transformStartA = modelPoseA[startChainA];
				const hkQsTransform& transformStartB = modelPoseB[startChainB];
				const hkQsTransform& transformEndA = modelPoseA[endChainA];
				const hkQsTransform& transformEndB = modelPoseB[endChainB];
				
				// A to B
				{
					hkSkeletonMapperData::ChainMapping chainMappingAtoB;

					chainMappingAtoB.m_startBoneA = startChainA;
					chainMappingAtoB.m_endBoneA = endChainA;
					chainMappingAtoB.m_startBoneB = startChainB;
					chainMappingAtoB.m_endBoneB = endChainB;
					chainMappingAtoB.m_endAFromBTransform.setMulInverseMul(transformEndA, transformEndB);
					chainMappingAtoB.m_startAFromBTransform.setMulInverseMul(transformStartA, transformStartB);

					aToB.m_chainMappings.pushBack(chainMappingAtoB);
				}


				// B to A
				{
					hkSkeletonMapperData::ChainMapping chainMappingBtoA;

					chainMappingBtoA.m_startBoneA = startChainB;
					chainMappingBtoA.m_endBoneA = endChainB;
					chainMappingBtoA.m_startBoneB = startChainA;
					chainMappingBtoA.m_endBoneB = endChainA;
					chainMappingBtoA.m_endAFromBTransform.setMulInverseMul(transformEndB, transformEndA);
					chainMappingBtoA.m_startAFromBTransform.setMulInverseMul(transformStartB, transformStartA);

					bToA.m_chainMappings.pushBack(chainMappingBtoA);
				}

				// Mark the links in between as INCHAIN
				// (the start and end were already matched)
				{
					for (int a=1; a<chainLengthA-1; a++)
					{
						map_AtoB[chainA[a]] = INCHAIN;
					}
					for (int b=1; b<chainLengthB-1; b++)
					{
						map_BtoA[chainB[b]] = INCHAIN;
					}
				}
			}


		}
	}

	// 6 - Finally, look for unmapped bones
	// Always list unmapped bones
	{

		// Bones in B that A cannot map
		for (int b = 0; b<numBonesB; b++)
		{
			if (UNMATCHED == map_BtoA[b])
			{
				aToB.m_unmappedBones.pushBack(static_cast<hkInt16>(b));
			}
		}

		//Bone in A that B cannot map
		for (int a = 0; a<numBonesA; a++)
		{
			if (UNMATCHED == map_AtoB[a])
			{
				bToA.m_unmappedBones.pushBack(static_cast<hkInt16>(a));
			}
		}


		aToB.m_keepUnmappedLocal = true;
		bToA.m_keepUnmappedLocal = true;

	}
	HK_REPORT_SECTION_END();
}

const hkSkeletonMapperData::SimpleMapping* hkSkeletonMapperUtils::findSimpleMapping(hkInt16 boneA, const hkSkeletonMapperData& mapperdata )
{
	for (int i=0; i < mapperdata.m_simpleMappings.getSize(); ++i)
	{
		if (mapperdata.m_simpleMappings[i].m_boneA == boneA)
			return &mapperdata.m_simpleMappings[i];
	}
	
	return HK_NULL;
}

hkResult hkSkeletonMapperUtils::lockTranslationsAutomatically (const hkSkeletonMapperData& mapperData)
{
	const hkSkeleton& skeletonA = *mapperData.m_skeletonA;
	const hkSkeleton& skeletonB = *mapperData.m_skeletonB;

	// Look for the first bones in A and B that were mapped
	int firstMapA = skeletonA.m_numBones;
	int firstMapB = skeletonB.m_numBones;

	for (int m=0; m<mapperData.m_simpleMappings.getSize(); m++)
	{
		const hkSkeletonMapperData::SimpleMapping& sm = mapperData.m_simpleMappings[m];
		if (sm.m_boneA < firstMapA)
		{
			if (sm.m_boneB < firstMapB)
			{
				firstMapA = sm.m_boneA;
				firstMapB = sm.m_boneB;
			}
			else
			{
				HK_WARN_ALWAYS (0xabba782a, "Invalid mapping - cannot lock translation based on it");
				return HK_FAILURE;
			}
			break;
		}
	}

	if (firstMapA == skeletonA.m_numBones)
	{
		HK_WARN_ALWAYS (0xabba782a, "Invalid mapping - cannot lock translations based on it");
		return HK_FAILURE;
	}

	HK_REPORT_SECTION_BEGIN(0x87e5654e, "lockTranslationsAutomatically");

	// Lock translations in A
	{
		HK_REPORT("Locking translations in skeleton "<<skeletonA.m_name<<" from bone "<<skeletonA.m_bones[firstMapA]->m_name);
		for (int i=0; i<skeletonA.m_numBones; i++)
		{
			skeletonA.m_bones[i]->m_lockTranslation = (i > firstMapA);
		}
	}

	// Lock translations in B
	{
		HK_REPORT("Locking translations in skeleton "<<skeletonB.m_name<<" from bone "<<skeletonB.m_bones[firstMapB]->m_name);
		for (int i=0; i<skeletonB.m_numBones; i++)
		{
			skeletonB.m_bones[i]->m_lockTranslation = (i > firstMapB);
		}
	}

	HK_REPORT_SECTION_END();

	return HK_SUCCESS;
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
