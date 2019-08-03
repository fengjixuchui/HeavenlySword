/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkragdoll/hkRagdoll.h>
#include <hkragdoll/utils/hkRagdollUtils.h>
#include <hkragdoll/instance/hkRagdollInstance.h>
#include <hkutilities/inertia/hkInertiaTensorComputer.h>
#include <hkutilities/collide/hkTransformCollapseUtil.h>
#include <hkutilities/constraint/hkConstraintUtils.h>

#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>
#include <hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h>
#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>
#include <hkdynamics/constraint/bilateral/prismatic/hkPrismaticConstraintData.h>
#include <hkdynamics/constraint/bilateral/stiffspring/hkStiffSpringConstraintData.h>
#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>
#include <hkdynamics/constraint/motor/position/hkPositionConstraintMotor.h>

#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/htl/hkAlgorithm.h>

#include <hkanimation/rig/hkSkeleton.h>
//#include <hkanimation/rig/hkPose.h>

/*
** Static functions
*/

static hkResult moveRigidBodyPivot (hkRigidBody* rigidBody, const hkQsTransform& localFromBone)
{
	// if they are already aligned, nothing to do
	if (localFromBone.isApproximatelyEqual(hkQsTransform::getIdentity()))
	{
		return HK_SUCCESS;
	}

	hkQsTransform worldFromLocal; // current RB transform
	worldFromLocal.setFromTransformNoScale(rigidBody->getTransform());

	hkQsTransform worldFromBone; // new RB transform
	worldFromBone.setMul(worldFromLocal, localFromBone);

	hkQsTransform boneFromLocal; // used to transform shapes
	boneFromLocal.setInverse(localFromBone);

	// Store the original center of mass
	hkVector4 originalCenterOfMassWS = rigidBody->getCenterOfMassInWorld();

	// Put an extra transform shape
	{
		// Temporarily disable warning
		const hkBool wasEnabled = hkError::getInstance().isEnabled(0x2ff8c16f);
		hkError::getInstance().setEnabled(0x2ff8c16f, false);

		const hkShape* currentShape = rigidBody->getCollidable()->getShape();

		hkTransform theTransform;
		boneFromLocal.copyToTransformNoScale(theTransform);

		hkTransformShape* newShape = new hkTransformShape(currentShape, theTransform);

		// This should just really be : 
		//   rigidBody->setShape(newShape);
		//   newShape->removeReference();
		// But you can't call setShape() on an hkRigidBody (yet)
		currentShape->removeReference();
		rigidBody->setShape(newShape);

		// Reduce the number of transform shapes, if possible
		hkTransformCollapseUtil::Options options;
		hkTransformCollapseUtil::Results results;
		hkTransformCollapseUtil::collapseTransforms(rigidBody, options, results);

		// Switch warning back on/off
		hkError::getInstance().setEnabled(0x2ff8c16f, wasEnabled);
	}

	// And change the rigid body transform
	{
		hkTransform newTransform;
		worldFromBone.copyToTransformNoScale(newTransform);

		rigidBody->setTransform(newTransform);

		// Restore center of mass
		if (rigidBody->getMotionType() != hkMotion::MOTION_FIXED)
		{
			hkVector4 newCenterOfMassLS; newCenterOfMassLS.setTransformedInversePos(newTransform, originalCenterOfMassWS);
			rigidBody->setCenterOfMassLocal(newCenterOfMassLS);
		}	
	}

	return HK_SUCCESS;
}

static bool _isConstraintSupported (const hkConstraintInstance* constraint, bool warnIfNot = true)
{
	switch (constraint->getData()->getType())
	{
		case hkConstraintData::CONSTRAINT_TYPE_BALLANDSOCKET:
		case hkConstraintData::CONSTRAINT_TYPE_HINGE:
		case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		case hkConstraintData::CONSTRAINT_TYPE_PRISMATIC:
		case hkConstraintData::CONSTRAINT_TYPE_STIFFSPRING:
			return true;
	}

	if (warnIfNot)
	{
		HK_WARN_ALWAYS (0xabbabf3b, "Unsupported type of constraint in rag doll");
	}

	return false;
}


static hkVector4* _accessConstraintPivotA (const hkConstraintInstance* constraint)
{
	hkConstraintData *constraintData = constraint->getData();

	switch (constraintData->getType())
	{
		case hkConstraintData::CONSTRAINT_TYPE_BALLANDSOCKET:
		{
			hkBallAndSocketConstraintData* bsConstraint = static_cast<hkBallAndSocketConstraintData*> (constraintData);
			return &bsConstraint->m_atoms.m_pivots.m_translationA;
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_HINGE:
		{
			hkHingeConstraintData* hConstraint = static_cast<hkHingeConstraintData*> (constraintData);
			return &hConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		{
			hkLimitedHingeConstraintData* hConstraint = static_cast<hkLimitedHingeConstraintData*> (constraintData);
			return &hConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		{
			hkRagdollConstraintData* rConstraint = static_cast<hkRagdollConstraintData*> (constraintData);
			return &rConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_PRISMATIC:
		{
			hkPrismaticConstraintData* pConstraint = static_cast<hkPrismaticConstraintData*> (constraintData);
			return &pConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_STIFFSPRING:
		{
			hkStiffSpringConstraintData* ssConstraint = static_cast<hkStiffSpringConstraintData*> (constraintData);
			return &ssConstraint->m_atoms.m_pivots.m_translationA;
		}
		break;

	default:
		{
			HK_WARN_ALWAYS (0xabbabf3b, "Unsupported type of constraint in rag doll");
			return HK_NULL;
			break;
		}
	}
}

static hkVector4* _accessConstraintPivotB (const hkConstraintInstance* constraint)
{
	hkConstraintData *constraintData = constraint->getData();

	switch (constraintData->getType())
	{
		case hkConstraintData::CONSTRAINT_TYPE_BALLANDSOCKET:
		{
			hkBallAndSocketConstraintData* bsConstraint = static_cast<hkBallAndSocketConstraintData*> (constraintData);
			return &bsConstraint->m_atoms.m_pivots.m_translationB;
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_HINGE:
		{
			hkHingeConstraintData* hConstraint = static_cast<hkHingeConstraintData*> (constraintData);
			return &hConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		{
			hkLimitedHingeConstraintData* hConstraint = static_cast<hkLimitedHingeConstraintData*> (constraintData);
			return &hConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		{
			hkRagdollConstraintData* rConstraint = static_cast<hkRagdollConstraintData*> (constraintData);
			return &rConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_PRISMATIC:
		{
			hkPrismaticConstraintData* pConstraint = static_cast<hkPrismaticConstraintData*> (constraintData);
			return &pConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_STIFFSPRING:
		{
			hkStiffSpringConstraintData* ssConstraint = static_cast<hkStiffSpringConstraintData*> (constraintData);
			return &ssConstraint->m_atoms.m_pivots.m_translationA;
		}
		break;

	default:
		{
			HK_WARN_ALWAYS(0xabbabf3b, "Unsupported type of constraint in rag doll.");
			return HK_NULL;
			break;
		}
	}
}


static hkResult alignChildrenToConstraints (const hkArray<hkConstraintInstance*>& constraints, hkBool warnOnAlignment)
{
	const int numConstraints = constraints.getSize();

	hkResult finalResult = HK_SUCCESS;

	for (int c=0; c < numConstraints; c++)
	{
		const hkConstraintInstance* constraint = constraints[c];

		if (_isConstraintSupported(constraint, true))
		{
			const hkVector4 pivotInA = *_accessConstraintPivotA(constraint);; 

			hkRigidBody* child = constraint->getRigidBodyA();

			hkQsTransform childFromConstraint (hkQsTransform::IDENTITY);
			childFromConstraint.setTranslation(pivotInA);

			if (pivotInA.lengthSquared3()>1e-6f)
			{
				if (warnOnAlignment)
				{
					HK_WARN(0xabbaaff3, "Changing the pivot of rigid body (\""<<child->getName()<<"\") to align it to its constraint.");
				}

				const hkResult res = moveRigidBodyPivot(child, childFromConstraint);

				if (res!=HK_SUCCESS)
				{
					finalResult = HK_FAILURE;
				}
			}
		}
		else
		{
			finalResult = HK_FAILURE;
		}

	}

	return finalResult;
}

static hkResult alignConstraintSpacesToChildren (const hkArray<hkConstraintInstance*>& constraints)
{
	const int numConstraints = constraints.getSize();

	for (int c=0; c < numConstraints; c++)
	{
		const hkConstraintInstance* constraint = constraints[c];

		if (_isConstraintSupported(constraint, true))
		{
			hkRigidBody* child = constraint->getRigidBodyA();
			hkRigidBody* parent = constraint->getRigidBodyB();

			hkVector4 pivotInWorld = child->getTransform().getTranslation();

			hkVector4& ref_pivotInA = *_accessConstraintPivotA(constraint);
			ref_pivotInA.setZero4();

			hkVector4& ref_pivotInB = *_accessConstraintPivotB(constraint); 
			// pB =  BfromW * pW = inv(WfromB) * pW
			ref_pivotInB.setTransformedInversePos(parent->getTransform(), pivotInWorld);
		}

		// EXP-844 : Warn when aligning spring ends as usually this is not desired
		{
			if (constraint->getData()->getType()==hkConstraintData::CONSTRAINT_TYPE_STIFFSPRING)
			{
				HK_WARN_ALWAYS(0xabba876d, "Unusual : Aligning constraint spaces of a stiff spring.");
			}
		}
	}

	return HK_SUCCESS;

}

hkResult hkRagdollUtils::reorderForRagdoll(hkArray<hkRigidBody*>& rigidBodies, hkArray<hkConstraintInstance*>& constraints)
{
	const int numRigidBodies = rigidBodies.getSize();
	const int numConstraints = constraints.getSize();
	if (numConstraints != numRigidBodies-1 )
	{
		// Too many or too few constraints
		HK_WARN_ALWAYS(0xabba54bb, "Expecting "<<numRigidBodies-1<<" constraints, not "<<numConstraints<<". Can't create rag doll");
		return HK_FAILURE;
	}

	hkLocalArray<int> associatedConstraints(numRigidBodies);
	associatedConstraints.setSize(numRigidBodies, HK_NULL);
	hkLocalArray<int> newToOriginal (numRigidBodies);
	newToOriginal.setSizeUnchecked (numRigidBodies);

	hkLocalArray<int> hierarchy(numRigidBodies);
	hierarchy.setSize(numRigidBodies,-1);
	// Look for parents and associated constraints
	// Go through the constraints, associate them with rigid bodies, and check the parent
	{
		for (int c=0; c<numConstraints; c++)
		{
			const hkConstraintInstance* constraint = constraints[c];
			hkRigidBody* child = static_cast<hkRigidBody*> (constraint->getEntityA());
			hkRigidBody* parent = static_cast<hkRigidBody*> (constraint->getEntityB());

			const int childId = rigidBodies.indexOf(child);
			const int parentId = rigidBodies.indexOf(parent);
			if (childId<0 || parentId<0)
			{
				// One of the constraints refers to an unknown rigid body
				return HK_FAILURE;
			}

			if (hierarchy[childId]!=-1 || associatedConstraints[childId]!=HK_NULL)
			{
				// Another constraint already had this object as a child
				return HK_FAILURE;
			}

			associatedConstraints[childId] = c;
			hierarchy[childId] = parentId;

		}
	}


	// Now, we should have the associatedConstraints array full (except one HK_NULL) and the hierarchy array full (except one -1)

	// Reorder based on hierarchy
	{
		// We can't do a simple sort based on the hierarchy array since it's self-referencing
		// Build a depth array, and sort based on that
		hkLocalArray<int> depths(numRigidBodies);
		depths.setSize(numRigidBodies, 0);
		{
			for (int r=0; r<numRigidBodies; r++)
			{
				for (int current = hierarchy [r] ; current !=-1; current = hierarchy[current])
				{
					depths[r]++;
				}
			}
		}

		// Now, based on depth, reorder a map to the original rigid bodies
			{	
			for (int i=0; i<numRigidBodies; i++)
			{
				newToOriginal[i] = i;
			}

			// We do a bubble sort of the map based on depth
			{
				for (int i=0; i<numRigidBodies-1; i++)
				{
					for (int j=i+1; j<numRigidBodies; j++)
					{
						if (depths[i] > depths[j])
						{
							// swap
							hkAlgorithm::swap(depths[i], depths[j]);
							hkAlgorithm::swap(newToOriginal[i], newToOriginal[j]);
						}
					}
				}
			}
		}
	}

	// Finally, use this map to reconstruct the rigid bodies and constraint arrays
	hkLocalArray<hkRigidBody*> newRigidBodies(numRigidBodies);
	newRigidBodies.setSizeUnchecked(numRigidBodies);

	hkLocalArray<hkConstraintInstance*> newConstraints(numConstraints);
	newConstraints.setSizeUnchecked(numConstraints);

	for (int i=0; i<numRigidBodies; i++)
	{
		const int oldRigidBodyId = newToOriginal[i];
		newRigidBodies[i] = rigidBodies[oldRigidBodyId];

		if (i>0)
		{
			const int oldConstraintId = associatedConstraints[oldRigidBodyId];
			if (oldConstraintId >= 0)
				newConstraints [i-1] = constraints[oldConstraintId];
			else
				newConstraints [i-1] = HK_NULL;
		}
	}

	// copy over old array and return
	rigidBodies = newRigidBodies;
	constraints = newConstraints;

	return HK_SUCCESS;
}

hkResult hkRagdollUtils::reorderAndAlignForRagdoll (hkArray<hkRigidBody*> &rigidBodiesInOut, hkArray<hkConstraintInstance*> &constraintsInOut, hkBool warnOnAlignment)
{

	// Check that the system has the right amount of bodies and constraints, and order them properly
	{
		hkResult res = reorderForRagdoll (rigidBodiesInOut, constraintsInOut);

		if (res != HK_SUCCESS)
		{
			return HK_FAILURE;
		}
	}

	// Now we have a system with the right amount and order of rigid bodies and constraints
	{

		// Align child rigid bodies to their constraint pivot
		{
			hkResult res = alignChildrenToConstraints (constraintsInOut, warnOnAlignment);
			if (res!=HK_SUCCESS)
			{
				return HK_FAILURE;
			}
		}

		// Reset constraint spaces
		{
			hkResult res = alignConstraintSpacesToChildren (constraintsInOut);
			if (res!=HK_SUCCESS)
			{
				return HK_FAILURE;
			}
		}

	}

	return HK_SUCCESS;
}


/*
** CONSTRUCT SKELETON
*/

hkSkeleton* hkRagdollUtils::constructSkeletonForRagdoll (const hkArray<hkRigidBody*> &rigidBodies, const hkArray<hkConstraintInstance*> &constraints)
{
	hkSkeleton* skeleton = new hkSkeleton();

	const int numBones = rigidBodies.getSize();

	skeleton->m_numBones = numBones;
	skeleton->m_bones = hkAllocate<hkBone*> (numBones, HK_MEMORY_CLASS_UTILITIES );

	skeleton->m_numParentIndices = numBones;
	skeleton->m_parentIndices = hkAllocate<hkInt16> (numBones, HK_MEMORY_CLASS_UTILITIES );

	skeleton->m_numReferencePose = numBones;
	skeleton->m_referencePose = hkAllocate<hkQsTransform> (numBones, HK_MEMORY_CLASS_UTILITIES );

	for (int b=0; b<numBones; b++)
	{	
		int parentId = -1;

		if ( b != 0 )
		{
			hkConstraintInstance* constraint = constraints[b-1];

			hkRigidBody* parentRb = static_cast<hkRigidBody*> (constraint->getEntityB());

			// Look for the ID of the parent RB
			for (int p=0; p<b; p++)
			{
				if (rigidBodies[p] == parentRb)
				{
					parentId = p;
					break;
				}
			}
			HK_ASSERT2(0x54366cce, parentId!=-1, "Rigid Bodies in physic system are not properly ordered (parent-first)");

		}

		skeleton->m_parentIndices[b] = static_cast<hkInt16>(parentId);

		hkRigidBody* rbody = rigidBodies[b];

		hkBone* newBone = new  hkBone;

		// Calculate initial transform
		{
			if (parentId>=0)
			{
				hkRigidBody* parentRb = rigidBodies[parentId];

				// Normal child
				hkQsTransform boneWorldTransform;
				boneWorldTransform.setFromTransformNoScale(rbody->getTransform());
				hkQsTransform parentWorldTransform;
				parentWorldTransform.setFromTransformNoScale(parentRb->getTransform());

				// worldFromChild = worldFromParent * parentFromChild
				// parentFromChild = inv(worldParent) * worldFromChild
				// bW = pW * local
				// local = inv(pw) * bw
				skeleton->m_referencePose[b].setMulInverseMul(parentWorldTransform, boneWorldTransform);
			}
			else
			{
				// Root
				skeleton->m_referencePose[b].setFromTransformNoScale(rbody->getTransform());
			}

		}


		// Calculate name
		{
			const char* rbodyName = rbody->getName();


			if (rbodyName)
			{
				// !!!
				newBone->m_name = hkString::strDup(rbodyName);
			}
			else
			{
				HK_WARN_ALWAYS (0xabbacc14, "Rigid Body name could not be found");
				newBone->m_name = HK_NULL;
			}
		}

		// Calculate bone constraints
		{
			// This really just locks translation for all bones except root
			newBone->m_lockTranslation = (b != 0);
		}

		skeleton->m_bones[b] = newBone;
	}

	// name (just first bone name for now)
	if (skeleton->m_numBones > 0)
		skeleton->m_name = skeleton->m_bones[0]->m_name;
	else
		skeleton->m_name = HK_NULL;

	return skeleton;
}

/*
** DESTROY SKELETON
*/

// Deallocates any memory allocated by constructSkeletonFromRagdoll()
void hkRagdollUtils::destroySkeleton (hkSkeleton* skeleton)
{
	const int numBones = skeleton->m_numBones;
	hkDeallocate(skeleton->m_parentIndices);

	for (int b=0; b<numBones; b++)
	{
		hkBone* bone = skeleton->m_bones[b];
		if (bone->m_name)
		{
			hkDeallocate<char> (bone->m_name);
		}
		delete bone;
	}
	hkDeallocate(skeleton->m_bones);
	hkDeallocate(skeleton->m_referencePose);
	delete skeleton;
}


hkRagdollInstance* hkRagdollUtils::createRagdollInstanceFromSkeleton (const hkSkeleton* skeleton, const hkArray<hkRigidBody*>& candidateRBs, const hkArray<hkConstraintInstance*>& candidateConstraints)
{
	const int numBones = skeleton->m_numBones;

	// We'll fill these two arrays as we go through the bones in the skeleton
	hkLocalArray<hkRigidBody*> theRigidBodies(numBones);
	hkLocalArray<hkConstraintInstance*> theConstraints(numBones-1);

	// For each bone, look for the rigid body and the constraint
	for (int bi=0; bi < numBones; ++bi)
	{
		hkBone* bone = skeleton->m_bones[bi];
		hkInt16 parentIdx = skeleton->m_parentIndices[bi];
		if (bone->m_name)
		{
			hkRigidBody* theRigidBody= HK_NULL;
			{
				for (int rbi=0; rbi < candidateRBs.getSize(); ++rbi)
				{
					hkRigidBody* rb = candidateRBs[rbi];
					const char* rbName = rb->getName();
					if ( rbName && (hkString::strCmp(rb->getName(), bone->m_name) == 0) )
					{
						if (theRigidBody==HK_NULL)
						{
							theRigidBody = rb;
						}
						else
						{
							HK_WARN_ALWAYS(0xabba98aa, "Some RBS have duplicated names : "<<rbName);
						}
					}
				}
			}

			if (!theRigidBody)
			{
				HK_WARN_ALWAYS(0xabbab8dd, "Couldn't find RB for bone : "<<bone->m_name);
				return HK_NULL;
			}

			theRigidBodies.pushBack(theRigidBody);

			// If it's not the root, look for the constraint
			if (parentIdx != -1)
			{
				const hkRigidBody* desiredChild = theRigidBody;
				const hkRigidBody* desiredParent = theRigidBodies[parentIdx];

				hkConstraintInstance* theConstraint = HK_NULL;

				// find the matching constraint which has this body as the child
				for (int c=0; c<candidateConstraints.getSize(); c++)
				{
					hkConstraintInstance* constraint = candidateConstraints[c];
					const hkRigidBody* child = static_cast<hkRigidBody*> (constraint->getEntityA());
					const hkRigidBody* parent = static_cast<hkRigidBody*> (constraint->getEntityB());

					if ((child == desiredChild) && ( parent == desiredParent))
					{
						if (theConstraint == HK_NULL)
						{
							theConstraint = constraint;
						}
						else
						{
							HK_WARN_ALWAYS(0xabba82ff, "Found more than one constraint between rigid bodies "<<child->getName()<<" and "<<parent->getName());
						}
					}
				}

				if (!theConstraint)
				{
					HK_WARN_ALWAYS(0xabba9f9f, "Couldn't find constraint for bone : "<<bone->m_name);
					return HK_NULL;
				}

				theConstraints.pushBack(theConstraint);
			}
		}
		else
		{
			HK_WARN_ALWAYS(0xabbae6a2, "Some bones have no name - can't create rag doll");
			return HK_NULL;
		}
	}

	HK_ASSERT2(0xabba9134, theRigidBodies.getSize()==numBones, "Internal logic error");
	HK_ASSERT2(0xabba9134, theConstraints.getSize()==numBones-1, "Internal logic error");

	// We have everything we need, create the ragdoll instance
	hkRagdollInstance* theRagdollInstance = new hkRagdollInstance(theRigidBodies, theConstraints, skeleton);

	return theRagdollInstance;
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
