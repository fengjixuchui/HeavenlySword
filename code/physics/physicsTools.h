#ifndef	_PHYSICSTOOLS_H
#define	_PHYSICSTOOLS_H

#include "physicsmaterial.h"
#include "havokIncludes.h"
#include <hkbase/config/hkConfigVersion.h>
#include <hkcollide/agent/hkCollidable.h>
#include <hkcollide/castutil/hkWorldRayCastOutput.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>

class CEntity;

namespace Physics
{
	class Tools
	{
	public:
		static void SynchronizeTime(float frameStep, float alreadySimulated, float lastWorldStep, hkArray<hkReal>& worldSteps);

		// return material in contact
		static psPhysicsMaterial * GetMaterial(const hkCdBody& body);
		static psPhysicsMaterial * GetMaterial(const hkWorldRayCastOutput& hit);
		static psPhysicsMaterial * GetMaterial(const hkCollidable& collidable, hkShapeKey key);

		// get entity from collidable
		static CEntity * GetEntity(const hkCollidable& collidable);

		// recursively travers shapes and set their user data
		static void SetShapeUserData(hkShape * shape, hkUlong userData);

		// calculate normal of triangle
		static hkVector4 CalcTriangleNormal(const hkVector4 * vertexes);

		// debugging func
		static void CheckRigidBody(hkRigidBody * body);	
		static void CheckRigidBodies(hkWorld * world);
	};
}

#endif //_PHYSICSTOOLS_H 
