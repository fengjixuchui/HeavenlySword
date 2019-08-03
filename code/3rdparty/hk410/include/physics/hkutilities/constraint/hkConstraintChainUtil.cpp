/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkbase/htl/hkAlgorithm.h>
#include <hkbase/htl/hkPointerMap.h>

#include <hkutilities/constraint/hkConstraintChainUtil.h>
#include <hkutilities/constraint/hkConstraintUtils.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>
#include <hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h>

#include <hkdynamics/constraint/chain/ballsocket/hkBallSocketChainData.h>
#include <hkdynamics/constraint/chain/powered/hkPoweredChainData.h>

#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/ballsocket/hkBallSocketChainData.h>

#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>
//#include <hkdynamics/constraint/motor/position/hkPositionConstraintMotor.h>

#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkdynamics/constraint/chain/hingelimits/hkHingeLimitsData.h>
#include <hkinternal/dynamics/constraint/chain/ragdolllimits/hkRagdollLimitsData.h>


//////////////////////////////////////////////////////////////////////////
//
//  Utils for constraint chains
//
//////////////////////////////////////////////////////////////////////////



void hkConstraintChainUtil::addConstraintToBallSocketChain( hkConstraintChainInstance* instance, hkEntity* entityToAppend, const hkVector4& pivotWs )
{
	HK_ASSERT2(0x4c040392, instance->getNumConstraints() == instance->getData()->getNumConstraintInfos(), "Warning: Number of constraints and constraint info in a chain should be equal when calling hkConstraintChainUtil::addConstraintToBallSocketChain().");
	HK_ASSERT2(0xad6788d5, instance->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_BALL_SOCKET_CHAIN, "Incompatible constraint passed to hkConstraintChainUtil::addConstraintToBallSocketChain().");

	hkBallSocketChainData* data = static_cast<hkBallSocketChainData*>(instance->getData());
	hkBallSocketChainData::ConstraintInfo info;

	hkRigidBody* body[2] = { hkGetRigidBody( instance->m_chainedEntities.back()->getCollidable() ),
		hkGetRigidBody( entityToAppend->getCollidable() ) };

	HK_ASSERT2(0xad67888d, body[0] && body[1], "Only rigid bodies can be processed by hkConstraintChainUtil::addConstraintToBallSocketChain.");

	info.m_pivotInA.setTransformedInversePos( body[0]->getTransform(), pivotWs );
	info.m_pivotInB.setTransformedInversePos( body[1]->getTransform(), pivotWs );

	data->addConstraintInfoInBodySpace( info.m_pivotInA, info.m_pivotInB );
	instance->addEntity( entityToAppend );
}

void hkConstraintChainUtil::addConstraintToPoweredChain( hkConstraintChainInstance* instance, hkEntity* entityToAppend, const hkVector4& pivotWs, hkConstraintMotor* motor0, hkConstraintMotor* motor1, hkConstraintMotor* motor2 )
{
	HK_ASSERT2(0x73776c0b, instance->getNumConstraints() == instance->getData()->getNumConstraintInfos(), "Warning: Number of constraints and constraint info in a chain should be equal when calling hkConstraintChainUtil::addConstraintToBallSocketChain().");
	HK_ASSERT2(0xad6788d5, instance->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_POWERED_CHAIN, "Incompatible constraint passed to hkConstraintChainUtil::addConstraintToPoweredChain().");

	hkPoweredChainData* data = static_cast<hkPoweredChainData*>(instance->getData());
	hkPoweredChainData::ConstraintInfo info;

	hkRigidBody* body[2] = { hkGetRigidBody( instance->m_chainedEntities.back()->getCollidable() ),
		hkGetRigidBody( entityToAppend->getCollidable() ) };

	HK_ASSERT2(0xad67888e, body[0] && body[1], "Only rigid bodies can be processed by hkConstraintChainUtil::addConstraintToPoweredChain.");

	info.m_pivotInA.setTransformedInversePos( body[0]->getTransform(), pivotWs );
	info.m_pivotInB.setTransformedInversePos( body[1]->getTransform(), pivotWs );
	info.m_aTc.setMulInverse( body[1]->getRotation(), body[0]->getRotation() );

	HK_ASSERT2(0xad67db88, motor0 && motor1 && motor2, "Powered chain requires non-null motors.");
	info.m_motors[0] = motor0;
	info.m_motors[1] = motor1;
	info.m_motors[2] = motor2;

	data->addConstraintInfoInBodySpace( info.m_pivotInA, info.m_pivotInB, info.m_aTc, info.m_motors[0], info.m_motors[1], info.m_motors[2] );
	instance->addEntity( entityToAppend );
}




hkResult hkConstraintChainUtil::addConstraintToChain(hkConstraintInstance* newChildInstance, hkConstraintChainInstance* chainInstance, hkBallSocketChainData* chainData)
{
	hkArray<hkEntity*>& chainedEntities = chainInstance->m_chainedEntities;

	hkVector4 pivotA;
	hkVector4 pivotB;
	if ( HK_SUCCESS != hkConstraintUtils::getConstraintPivots(newChildInstance, pivotA, pivotB) )
	{
		HK_ASSERT2(0xad677788, false, "Cannot extract pivots from the supplied constraint.");
		return HK_FAILURE;
	}

	if( !chainedEntities.getSize() )
	{
		chainInstance->addEntity( newChildInstance->getEntityA() );
	}

	HK_ASSERT2(0xad7877de, chainedEntities.getSize() == 1 + chainData->getNumConstraintInfos(), "Error constructing a constraint chain. Pivot count not in synch with entity count.");

	if( chainedEntities.back() == newChildInstance->getEntityA() )
	{
		chainData->addConstraintInfoInBodySpace(pivotA, pivotB); // do NOT reverse
		chainInstance->addEntity( newChildInstance->getEntityB() );
	}
	else if( chainedEntities.back() == newChildInstance->getEntityB() )
	{
		chainData->addConstraintInfoInBodySpace(pivotB, pivotA); // DO reverse
		chainInstance->addEntity( newChildInstance->getEntityA() );
	}
	else if( chainedEntities[0] == newChildInstance->getEntityA() )
	{
		// Add in front of the list
		hkBallSocketChainData::ConstraintInfo info;
		info.m_pivotInA = pivotB;
		info.m_pivotInB = pivotA;
		chainData->m_infos.insertAt(0, info); // DO reverse

		chainInstance->insertEntityAtFront( newChildInstance->getEntityB() );
	}
	else if( chainedEntities[0] == newChildInstance->getEntityB() )
	{
		// Add in front of the list
		hkBallSocketChainData::ConstraintInfo info;
		info.m_pivotInA = pivotA;
		info.m_pivotInB = pivotB;
		chainData->m_infos.insertAt(0, info); // do NOT reverse

		chainInstance->insertEntityAtFront( newChildInstance->getEntityA() );
	}
	else
	{
		HK_ASSERT2(0xad675555, false, "Neither of the new child hkConstraintInstance's entities is the last or first element of the chain. Cannot add." );
		return HK_FAILURE;
	}

	return HK_SUCCESS;
}

namespace 
{
		// This body info has the same index in its array as the body has in the island
	struct hkEntityInfo
	{
		int m_side;
		int m_prevEntityIndex;
		hkConstraintInstance* m_linkingConstraint;
		int m_dist;
	};
}

hkConstraintChainInstance* hkConstraintChainUtil::buildPoweredChain(hkArray<hkConstraintInstance*>& constraints, hkBool cloneMotors)
{
	hkPoweredChainData* chainData = new hkPoweredChainData();
	hkConstraintChainInstance* chainInstance = new hkConstraintChainInstance( chainData );

	{
		hkEntity* firstEntity = constraints[0]->getEntityA();
		if (constraints.getSize() > 1 && (firstEntity == constraints[1]->getEntityA() || firstEntity == constraints[1]->getEntityB()))
		{
			firstEntity = constraints[0]->getEntityB();
		}
		chainInstance->addEntity( firstEntity );
	}

	for (int c = 0; c < constraints.getSize(); c++)
	{
		hkVector4 pivotA, pivotB;

		if ( HK_FAILURE == hkConstraintUtils::getConstraintPivots(constraints[c], pivotA, pivotB)  )
		{
			HK_WARN_ALWAYS(0xabbad88d, "Not supported types of constraints used to build a chain!");
			chainData->removeReference();
			chainInstance->removeReference();
			return HK_NULL;
		}
		if (constraints[c]->getEntityA() != chainInstance->m_chainedEntities.back())
		{
			hkAlgorithm::swap( pivotA, pivotB );

			if (constraints[c]->getEntityB() != chainInstance->m_chainedEntities.back())
			{
				HK_WARN_ALWAYS(0xabbad88d, "Constraints are not ordered properly ! Two consecutive constraint share no common hkEntity.");
				chainData->removeReference();
				chainInstance->removeReference();
				return HK_NULL;
			}
		}

		{
			hkQuaternion aTb; aTb.setInverseMul( static_cast<hkRigidBody*>(constraints[c]->getEntityA())->getRotation(), static_cast<hkRigidBody*>(constraints[c]->getEntityB())->getRotation() );
			hkConstraintMotor* motors[3];
			hkConstraintUtils::getConstraintMotors(constraints[c]->getData(), motors[0], motors[1], motors[2]);
			if (motors[0]==HK_NULL)
			{
				HK_WARN_ALWAYS(0xabba88d3, "No motors extracted from the original constraint.");
				chainData->removeReference();
				chainInstance->removeReference();
				return HK_NULL;
			}

			if (!motors[1]) { motors[1] = motors[0]; }
			if (!motors[2]) { motors[2] = motors[0]; }

			if (cloneMotors)
			{
				motors[0] = motors[0]->clone();
				motors[1] = motors[1]->clone();
				motors[2] = motors[2]->clone();
			}

			chainData->addConstraintInfoInBodySpace( pivotA, pivotB, aTb, motors[0], motors[1], motors[2] );
		}

		//
		//  Todo: determine now m_bTc should be oriented.
		//

		if (constraints[c]->getEntityA() == chainInstance->m_chainedEntities.back())
		{
			chainData->m_infos.back().m_switchBodies = true;
			chainInstance->addEntity( constraints[c]->getEntityB() );
		}
		else
		{
			chainInstance->addEntity( constraints[c]->getEntityA() );
		}
	}

	chainData->removeReference();

	return chainInstance;
}

hkResult hkConstraintChainUtil::findConstraintLinkBetweenEntities(const hkArray<hkConstraintInstance*>& allConstraints, hkEntity* entityA, hkEntity* entityB, hkArray<hkEntity*>& entitiesOut, hkArray<hkConstraintInstance*>& constraintsOut)
{
	if (entityA == entityB)
	{
		HK_WARN_ALWAYS(0xabba3bb3, "Specify two different end bodies.");
		return HK_FAILURE;
	}

	// Create list of all entities
	hkArray<hkEntity*> allEntities;
	// Create a map, which returns entity's index.
	hkPointerMap<hkEntity*, int> m_entityIndexLookup;

	// Verify that all entities referenced by constraints on allConstraints are in the allEntities list.
	{
		for (int c = 0; c < allConstraints.getSize(); c++)
		{
			const hkConstraintInstance* instance = allConstraints[c];

			hkPointerMap<hkEntity*, int>::Iterator itA = m_entityIndexLookup.findKey( instance->getEntityA() );
			if (! m_entityIndexLookup.isValid(itA))
			{
				m_entityIndexLookup.insert(instance->getEntityA(), allEntities.getSize());
				allEntities.pushBack(instance->getEntityA());
			}
			hkPointerMap<hkEntity*, int>::Iterator itB = m_entityIndexLookup.findKey( instance->getEntityB() );
			if (! m_entityIndexLookup.isValid(itB))
			{
				m_entityIndexLookup.insert(instance->getEntityB(), allEntities.getSize());
				allEntities.pushBack(instance->getEntityB());
			}
		}
	}

	// The system is traversed from both ends.
	hkInplaceArray<hkEntityInfo, 32> infos;
	infos.setSize(allEntities.getSize());

	for (int e = 0; e < infos.getSize(); e++)
	{
		infos[e].m_side = -1;
	}

	// The key is the current constraint, the value is the previous constraint leading to the current one.
	hkInplaceArray<int, 32> indexQueue[2];
	int currQueueElem[2] = { -1, -1 };

	{
		int entityAidx = m_entityIndexLookup.getValue( m_entityIndexLookup.findKey(entityA) );
		int entityBidx = m_entityIndexLookup.getValue( m_entityIndexLookup.findKey(entityB) );

		infos[entityAidx].m_side = 0;
		infos[entityAidx].m_prevEntityIndex = -1;
		infos[entityAidx].m_linkingConstraint = HK_NULL;
		infos[entityAidx].m_dist = 0;

		infos[entityBidx].m_side = 1;
		infos[entityBidx].m_prevEntityIndex = -1;
		infos[entityBidx].m_linkingConstraint = HK_NULL;
		infos[entityBidx].m_dist = 0;

		indexQueue[0].pushBack( entityAidx );
		indexQueue[1].pushBack( entityBidx );
	}

	int listSize[2];
	int endIdx[2];
	hkConstraintInstance* lastConstraint = HK_NULL;
	int activeSide = 0;

	hkInplaceArray<hkEntity*, 32> m_adjointEntities;
	hkInplaceArray<hkConstraintInstance*, 32> m_adjointConstraints;

	int linkFound = false;
	for ( ;!linkFound; activeSide = 1 - activeSide)
	{
		int otherSide = 1 - activeSide;
		int queueIdx = ++currQueueElem[activeSide];
		if (queueIdx >= indexQueue[activeSide].getSize())
		{
			return HK_FAILURE;
		}
		int entityIdx = indexQueue[activeSide][queueIdx];
		hkEntity* entity = allEntities[entityIdx];

		// fill adjoint entities
		m_adjointEntities.clear();
		m_adjointConstraints.clear();
		getAdjointEntities(allConstraints, entity, m_adjointEntities, m_adjointConstraints);


		for (int a = 0; (a < m_adjointEntities.getSize()) && (!linkFound); a++)
		{
			//HK_ASSERT2(0xad6777dd, m_adjointEntities[a]->getSimulationIsland() == island, "One of traversed entities does not belong to the same island. This can happen when the world is locked and this function is called in a callback.");
			const int adIdx = m_entityIndexLookup.getValue( m_entityIndexLookup.findKey(m_adjointEntities[a]) );
			const int side = infos[adIdx].m_side;// adjoint entity index
			//check if is in the current list
			if (side == activeSide)
			{
				continue;
			}

			if (side == otherSide)
			{
				// connection found -- do something
				linkFound = true;
				listSize[activeSide] = infos[entityIdx].m_dist + 1;
				listSize[otherSide]  = infos[adIdx].m_dist + 1;
				endIdx[activeSide] = entityIdx;
				endIdx[otherSide]  = adIdx;
				lastConstraint = m_adjointConstraints[a];
				break;
			}

			// new element -- add to queue
			indexQueue[activeSide].pushBack(adIdx);
			infos[adIdx].m_side = activeSide;
			infos[adIdx].m_prevEntityIndex = indexQueue[activeSide][currQueueElem[activeSide]];
			infos[adIdx].m_linkingConstraint = m_adjointConstraints[a];
			infos[adIdx].m_dist = infos[entityIdx].m_dist + 1;

		}

	}

	//
	// construct entity and constraint lists
	//

	entitiesOut.clear();
	entitiesOut.setSize(listSize[0] + listSize[1]);
	constraintsOut.clear();
	constraintsOut.setSize(listSize[0] + listSize[1] - 1 + 1); // add one, it's going to be removed later

	int idx = endIdx[0];
	{
		for (int k = listSize[0] - 1; k > 0; k--)
		{
			entitiesOut[k] = allEntities[idx];
			constraintsOut[ k - 1 ] = infos[idx].m_linkingConstraint;
			idx = infos[idx].m_prevEntityIndex;
		}
	}

	entitiesOut[0] = allEntities[idx];
	HK_ASSERT2(0xad6777dd, infos[idx].m_prevEntityIndex == -1, "error.");

	constraintsOut[listSize[0] - 1] = lastConstraint;

	idx = endIdx[1];
	{
		int base = listSize[0];
		for (int k = 0; k < listSize[1]; k++ )
		{
			entitiesOut[ base + k] = allEntities[idx];
			constraintsOut[ base + k ] = infos[idx].m_linkingConstraint;
			idx = infos[idx].m_prevEntityIndex;
		}
	}

	HK_ASSERT2(0xad6777dd, idx == -1 && constraintsOut.back() == HK_NULL, "error.");
	constraintsOut.popBack();

	return HK_SUCCESS;
}


void hkConstraintChainUtil::getAdjointEntities(const hkArray<hkConstraintInstance*>& allConstraints, const hkEntity* entity, hkArray<hkEntity*>& entitiesOut, hkArray<hkConstraintInstance*>& constraintsOut)
{
	// Iterate through allConstraints
	//  Check if they connect to the specified entity
	//   Assume that the other entity is on the allEntities list
	//   Append constraint and entity on the output queues

	for (int c = 0; c < allConstraints.getSize(); c++)
	{
		hkConstraintInstance* instance = allConstraints[c];

		if (instance->getEntityA() == entity || instance->getEntityB() == entity)
		{
			entitiesOut.pushBack( instance->getOtherEntity(entity) );
			constraintsOut.pushBack( instance );
		}
	}
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
