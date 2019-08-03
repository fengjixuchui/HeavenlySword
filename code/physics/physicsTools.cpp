#include "config.h"
#include "physicsTools.h"

#include "game/entity.h"

#include <hkcollide/shape/mesh/hkMeshShape.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkbase/memory/hkLocalBuffer.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/phantom/hkPhantom.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkdynamics/world/hkSimulationIsland.h>



#include "physicsloader.h"
#include "havokthreadutils.h"

namespace Physics
{
	/******************************************************
	** SynchronizeTime	
    ** Let's call frame time -> time when frame update is calculated and 
	**            Havok Step time -> time when havok simulation is done 
	** 
	** If those times are very different we get problems with synchronization of full character controllers or 
	** any user detection collision and Havok world. User detection collision is done on the Havok world with 
	** bodies at position, which they take at the time on the Havok simulation step end and no at the current 
	** frame time. Simply detection collision is done on world evolved a bit in the future. Error in time is 
	** the difference between Havok simulation step end time and frame time. Task of this function is to 
	** modify Havok step so that this error will be low.   	
	*******************************************************/


	void Tools::SynchronizeTime(float frameStep, float alreadySimulated, float lastWorldStep, hkArray<hkReal>& worldSteps)
	{
		// How much we can change Havok step in one step. 
		// Havok simulation become unstable if change is too big, on other side to small values 
		// means too slow synchronization and also system will be not able to react to FPS changes. 
		// We use quite low value at the moment, otherwise simulation is really unstable.
		#define MAX_TIME_CHANGE 0.02f

		// We want to Havok step to be calculates just after frame. Ideally immediatelly after frame.
		// This value is distance between frame time and Havok step time we want to keep.
		// If this value is too small and FPS fluctuactes Havok step time could be easely 
		// shifted just before frame what is wrong. On other side, large value means no synchronization.		
		#define DESIRED_POSITION 0.003f

		// Maximal allowed Havok step. If frame step is larger that this value do several step in frame. 
		#define MAX_HAVOK_WORLD_STEP 0.04f

		if (frameStep < alreadySimulated)
			return; 

		float stepWanted = frameStep; 
		if (frameStep > MAX_HAVOK_WORLD_STEP)
		{
			// hmmm simply do several havok steps during one frame
			int multi = (int) (frameStep * ( 1.0f / MAX_HAVOK_WORLD_STEP ));
			multi++;
			stepWanted = frameStep / multi;
		}

		// 1.) make steps on the same lengths...
		if (fabs(stepWanted - lastWorldStep) > MAX_TIME_CHANGE * lastWorldStep)
		{
			float actualTime = alreadySimulated;
			float newStep = lastWorldStep;
			do
			{
				// try to change step to meet the step length
				if (stepWanted > newStep)
					newStep = min(stepWanted, newStep * (1.0f + MAX_TIME_CHANGE));
				else
					newStep = max(stepWanted, newStep * (1.0f - MAX_TIME_CHANGE));

				actualTime += newStep;
				worldSteps.pushBack(newStep);
			} while(actualTime <= frameStep);

			return;
		}

		// try to set up steps so that it will vanish desynchronization
		float actualTime = alreadySimulated;
		float newStep = lastWorldStep;
		float stepWantedMin = stepWanted / (1 + MAX_TIME_CHANGE);
		float stepWantedMax = stepWanted / (1 - MAX_TIME_CHANGE);
		float desiredPosition  = min(DESIRED_POSITION, stepWanted * 0.2f);
		float desiredPositionMaxDiff = desiredPosition * 0.5f;
		do
		{
			// find probable error
			float timeToSim  = frameStep - actualTime + desiredPosition;			
			float error = ((int) (timeToSim / newStep) + 1) * newStep - timeToSim ;

			if (error >= newStep)
				error -= newStep; 

			if (error < (newStep - error))
			{
				if (error > desiredPositionMaxDiff) // hmm let small errors be... 
					newStep = max(stepWantedMin, newStep - min(error, newStep * MAX_TIME_CHANGE));
			}
			else
			{
				error = newStep - error;
				if (error > desiredPositionMaxDiff) // hmm let small errors be... 
					newStep = min(stepWantedMax, newStep + min(error, newStep * MAX_TIME_CHANGE));
			}

			actualTime += newStep;
			worldSteps.pushBack(newStep);
		} while (actualTime <= frameStep);
	}

	// Get physics material from hkCdBody
	psPhysicsMaterial * Tools::GetMaterial(const hkCdBody& body)
	{
		if (body.getShape()->getType() != HK_SHAPE_TRIANGLE || !body.getParent()) 
		{
			return (psPhysicsMaterial *) body.getShape()->getUserData();
		}

		// If one of the colliding objects is hkMeshShape, event does not containt pointer to that shape but pointer to 
		// triangle. The parent shape of triangle is then hkMeshShape or hkMoppBvTreeShape.
		const hkShape *parentShape = body.getParent()->getShape();		

		switch (parentShape->getType())
		{ 
		case HK_SHAPE_TRIANGLE_COLLECTION:
			{
				const hkMeshShape *meshShape = static_cast<const hkMeshShape *>(parentShape);
				const hsMeshMaterial * mat = static_cast<const hsMeshMaterial *>(meshShape->getMeshMaterial(body.getShapeKey()));
				if (mat)
				{
					return (psPhysicsMaterial *) mat->GetMaterial();
				}
				else
					return INVALID_MATERIAL;
			}
		case HK_SHAPE_MOPP:
		case HK_SHAPE_BV_TREE:
			{
				parentShape = static_cast<const hkBvTreeShape *>(parentShape)->getShapeCollection(); 
				if (parentShape && parentShape->getType() == HK_SHAPE_TRIANGLE_COLLECTION)
				{
					const hkMeshShape *meshShape = static_cast<const hkMeshShape *>(parentShape);
					const hsMeshMaterial * mat = static_cast<const hsMeshMaterial *>(meshShape->getMeshMaterial(body.getShapeKey()));					
					if (mat)
					{
						return (psPhysicsMaterial *) mat->GetMaterial();					
					}
					else
						return INVALID_MATERIAL;
				}
				else
				{
					return (psPhysicsMaterial *) body.getShape()->getUserData();
				}
			}
		default:
			return (psPhysicsMaterial *) body.getShape()->getUserData();
		}
	}	

	// Get physics material from hkWorldRayCastOutput
	psPhysicsMaterial * Tools::GetMaterial(const hkWorldRayCastOutput& hit)
	{
		if (!hit.hasHit())
			return NULL;

		// we potentially need a buffer for each level of the hierarchy
		hkLocalBuffer<hkShapeContainer::ShapeBuffer> shapeBuffer( hkShapeRayCastOutput::MAX_HIERARCHY_DEPTH );
		const hkShape* shape = hit.m_rootCollidable->getShape();
		for(int keyIndex = 0; shape != HK_NULL && hit.m_shapeKeys[keyIndex] != HK_INVALID_SHAPE_KEY ; ++keyIndex )
		{
			hkShapeType shapeType=shape->getType();

			// if mesh read material and return 
			if (shapeType == HK_SHAPE_TRIANGLE_COLLECTION)
			{
				const hkMeshShape *meshShape = static_cast<const hkMeshShape *>(shape);
				const hsMeshMaterial * mat = static_cast<const hsMeshMaterial *>(meshShape->getMeshMaterial(hit.m_shapeKeys[keyIndex]));
				if (mat)
				{
					return (psPhysicsMaterial *) mat->GetMaterial();
				}
				else
				{
					return INVALID_MATERIAL;
				}
			}
		
			// go to the next level
			shape = shape->getContainer() 
				? shape->getContainer()->getChildShape(hit.m_shapeKeys[keyIndex], shapeBuffer[keyIndex] )
				: HK_NULL;
		}

		if (shape)
			return (psPhysicsMaterial *) shape->getUserData(); // shape is not mesh shape so just return anything... 

		return 0;
	}

	// get material from collidable and shapeKey... if under root shape is some hierarchy of shapes 
	// we do not have enough info to find a shape... 
	psPhysicsMaterial * Tools::GetMaterial(const hkCollidable& collidable, hkShapeKey key)
	{
		// If one of the colliding objects is hkMeshShape, event does not containt pointer to that shape but pointer to 
		// triangle. The parent shape of triangle is then hkMeshShape or hkMoppBvTreeShape.
		const hkShape *shape = collidable.getShape();		

		switch (shape->getType())
		{ 
		case HK_SHAPE_TRIANGLE_COLLECTION:
			{
				const hkMeshShape *meshShape = static_cast<const hkMeshShape *>(shape);
				const hsMeshMaterial * mat = static_cast<const hsMeshMaterial *>(meshShape->getMeshMaterial(key));
				if (mat)
				{
					return (psPhysicsMaterial *) mat->GetMaterial();
				}
				else
					return INVALID_MATERIAL;
			}
		case HK_SHAPE_MOPP:
		case HK_SHAPE_BV_TREE:
			{
				shape = static_cast<const hkBvTreeShape *>(shape)->getShapeCollection(); 
				if (shape && shape->getType() == HK_SHAPE_TRIANGLE_COLLECTION)
				{
					const hkMeshShape *meshShape = static_cast<const hkMeshShape *>(shape);
					const hsMeshMaterial * mat = static_cast<const hsMeshMaterial *>(meshShape->getMeshMaterial(key));					
					if (mat)
					{
						return (psPhysicsMaterial *) mat->GetMaterial();					
					}
					else
						return INVALID_MATERIAL;
				}
				else
				{
					return (psPhysicsMaterial *) shape->getUserData();
				}
			}
		default:
			return (psPhysicsMaterial *) shape->getUserData();
		}
	}

	
	/** Returns entity from collidable */
	CEntity * Tools::GetEntity(const hkCollidable& collidable)
	{		
		hkRigidBody* obRB = hkGetRigidBody(&collidable);		
		if(obRB) 
		{
			if (obRB->hasProperty(Physics::PROPERTY_ENTITY_PTR))
				return (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();			
			else
				return 0; 
		}
		else
		{
			hkPhantom * obPhantom = hkGetPhantom(&collidable);	
			if (obPhantom && obPhantom->hasProperty(Physics::PROPERTY_ENTITY_PTR))
				return (CEntity*) obPhantom->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();			
			else
				return 0; 
		}
	}

	/** Sets user data to shape and alse recursively to all child shapes */
	void Tools::SetShapeUserData(hkShape * shape, hkUlong userData) 
	{
#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
			shape->setUserData(userData);
#else
			shape->setUserData((void *) userData);
#endif //HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1

		switch (shape->getType())
		{
		case HK_SHAPE_LIST:
		case HK_SHAPE_CONVEX_LIST:
			{
				hkListShape * list = static_cast<hkListShape *>(shape);
				for(int i = 0; i < list->getNumChildShapes(); i++)
				{
					SetShapeUserData(const_cast<hkShape *>(list->getChildShape(i)), userData);
				}
				break;
			}
		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				hkConvexTranslateShape * transShape = static_cast<hkConvexTranslateShape *>(shape);
				SetShapeUserData(const_cast<hkConvexShape *>(transShape->getChildShape()), userData);
				break;
			}
		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				hkConvexTransformShape * transShape = static_cast<hkConvexTransformShape *>(shape);
				SetShapeUserData(const_cast<hkConvexShape *>(transShape->getChildShape()), userData);
				break;
			}
		case HK_SHAPE_TRANSFORM :
			{
				hkTransformShape * transShape = static_cast<hkTransformShape *>(shape);
				SetShapeUserData(const_cast<hkShape *>(transShape->getChildShape()), userData);
				break;
			}
		case HK_SHAPE_MOPP:
		case HK_SHAPE_BV_TREE:
			{
				hkBvTreeShape * bvShape = static_cast<hkBvTreeShape *>(shape);
				SetShapeUserData(const_cast<hkShapeCollection *>(bvShape->getShapeCollection()), userData);
				break;
			}		
		default:;
		}
	}

	hkVector4 Tools::CalcTriangleNormal(const hkVector4 * vertexes)
	{
		hkVector4 sideA;
		sideA.setSub4(vertexes[1], vertexes[0]);
		hkVector4 sideB;
		sideB.setSub4(vertexes[2], vertexes[0]);
		hkVector4 normal;
		normal.setCross(sideA, sideB);
		return normal;
	}

	///////
	// Auxiliary functions for debugging
	//////
#ifndef _GOLD_MASTER
	void Tools::CheckRigidBody(hkRigidBody * body)
	{
		if (!body) 
			return;
		if (!body->hasProperty(Physics::PROPERTY_ENTITY_PTR))
		{
			ntPrintf("%s %p no entity pointer in properties\n", body->getName(), body->getCollidable()->getShape());
		}

		if (body->hasProperty(Physics::PROPERTY_ENTITY_PTR) && body->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr() == NULL)
		{
			ntPrintf("%s %p NULL entity pointer in properties\n", body->getName(), body->getCollidable()->getShape());
		}

		ntAssert(body && body->hasProperty(Physics::PROPERTY_ENTITY_PTR) && body->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr() != NULL);
	}

	void Tools::CheckRigidBodies(hkWorld * world)
	{
		ReadAccess lock; 
		const hkArray< hkSimulationIsland * >& activeIslands = world->getActiveSimulationIslands();
		for(int i = 0; i < activeIslands.getSize(); i++)
		{
			const hkArray<hkEntity *>& entities = activeIslands[i]->getEntities();
			for(int j = 0; j < entities.getSize(); j++)
			{
				hkEntity * ent = entities[j];
				hkRigidBody	* rigid = static_cast<hkRigidBody *>(ent);
				CheckRigidBody(rigid);
			}	
		}

		const hkArray< hkSimulationIsland * >& inactiveIslands = world->getInactiveSimulationIslands();
		for(int i = 0; i < inactiveIslands.getSize(); i++)
		{
			const hkArray<hkEntity *>& entities = inactiveIslands[i]->getEntities();
			for(int j = 0; j < entities.getSize(); j++)
			{
				hkEntity * ent = entities[j];
				hkRigidBody	 * rigid = static_cast<hkRigidBody *>(ent);
				CheckRigidBody(rigid);
			}	
		}

		const hkSimulationIsland * fixedIsland = world->getFixedIsland();	
		const hkArray<hkEntity *>& entities = fixedIsland->getEntities();
		for(int j = 0; j < entities.getSize(); j++)
		{
			hkEntity * ent = entities[j];
			hkRigidBody	 * rigid = static_cast<hkRigidBody *>(ent);
			if (rigid != world->getFixedRigidBody())
				CheckRigidBody(rigid);
		}			
	}
#endif
}
