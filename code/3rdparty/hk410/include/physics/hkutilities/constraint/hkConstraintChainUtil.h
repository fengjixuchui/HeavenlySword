/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES_CONSTRAINT_CHIAN_UTIL_H
#define HK_UTILITIES_CONSTRAINT_CHIAN_UTIL_H

#include <hkbase/hkBase.h>

class hkConstraintInstance;
class hkConstraintMotor;
class hkPositionConstraintMotor;
class hkConstraintChainInstance;
class hkBallSocketChainData;
class hkEntity;
class hkVector4;


class hkConstraintInstance;
class hkConstraintMotor;
class hkPositionConstraintMotor;
class hkConstraintChainInstance;
class hkBallSocketChainData;
class hkEntity;
class hkVector4;
class hkQuaternion;

	/// Constraint chain utility functions.
class hkConstraintChainUtil
{
public:
		//
		// Chain construction functions
		//

		/// This function builds a constraint chain instance, and a powered chain data.
		/// These are build from the ordered constraints list.
		/// All the constraints are assumed to be powered ragdoll/hinge constraints. 
		/// The function triggers an assert otherwise.
	static hkConstraintChainInstance* HK_CALL buildPoweredChain(hkArray<hkConstraintInstance*>& constraints, hkBool cloneMotors);

		/// This function adds a new constraint to a ball-socket constraint chain.
	static void HK_CALL addConstraintToBallSocketChain( hkConstraintChainInstance* instance, hkEntity* entityToAppend, const hkVector4& pivotWs );

		/// This function adds a new constraint to a powered constraint chain.
	static void HK_CALL addConstraintToPoweredChain( hkConstraintChainInstance* instance, hkEntity* entityToAppend, const hkVector4& pivotWs, hkConstraintMotor* motor0, hkConstraintMotor* motor1, hkConstraintMotor* motor2  );

		/// This function adds a new constraint to a ball-socket constraint chain.
	static hkResult HK_CALL addConstraintToChain(hkConstraintInstance* newChildInstance, hkConstraintChainInstance* chainInstance, hkBallSocketChainData* chainData);

		//
		// Helper functions
		//

		/// This attempts to find a connection from entityA to entityB along the constraints in the allConstraint list.
		/// Upon success, the function return HK_SUCCESS value and fills the entitiesOut and constraintsOut with ordered lists
		/// of entities and constraints visited on the shortes path. 
		/// The function returns HK_FAILURE when no path connects entityA and entityB.
 	static hkResult HK_CALL findConstraintLinkBetweenEntities(const hkArray<hkConstraintInstance*>& allConstraints, hkEntity* entityA, hkEntity* entityB, hkArray<hkEntity*>& entitiesOut, hkArray<hkConstraintInstance*>& constraintsOut);

		/// This returns a list of hkEntities linked to the specified entity by any of the constraints in the allConstraints list.
		/// The entities found are placed in the entitiesOut array, and the constraintsOut array contains the corresponding constraints, which lead to those entities.
	static void HK_CALL getAdjointEntities(const hkArray<hkConstraintInstance*>& allConstraints, const hkEntity* entity, hkArray<hkEntity*>& entitiesOut, hkArray<hkConstraintInstance*>& constraintsOut);

		
};

#endif // HK_UTILITIES_CONSTRAINT_CHIAN_UTIL_H

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
