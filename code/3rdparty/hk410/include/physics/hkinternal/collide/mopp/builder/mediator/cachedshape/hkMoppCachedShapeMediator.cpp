/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkinternal/hkInternal.h>


// Primitive Mediator Implementation that utilizes caching of the primitive's maximum/minimum extent
// for all supplied splitting axes

#include <hkinternal/collide/mopp/builder/hkbuilder.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkcollide/shape/sphererep/hkSphereRepShape.h>


hkMoppCachedShapeMediator::hkMoppCachedShapeMediator(const hkShapeContainer* shapeArray)
{
	m_shapeCollection = shapeArray;
	m_numChildShapes = m_shapeCollection->getNumChildShapes();
}


hkMoppCachedShapeMediator::~hkMoppCachedShapeMediator()
{
	m_shapeCollection = HK_NULL;
}


void hkMoppCachedShapeMediator::setSplittingPlaneDirections(const hkMoppSplittingPlaneDirection* directions, int numDirections)
{
	//
	// loop over all shapes in collection
	//
	int numChildShapes = m_shapeCollection->getNumChildShapes();
	{
		hkShapeKey key = m_shapeCollection->getFirstKey();
		for (int i = 0; i < numChildShapes; i++)
		{
			hkShapeCollection::ShapeBuffer buffer;
			const hkShape* shape = m_shapeCollection->getChildShape(key, buffer);

			//
			// only handle convex shapes
			//
			const hkShapeType type = shape->getType();
			switch (type)
			{
				case HK_SHAPE_CONVEX:
				case HK_SHAPE_CONVEX_VERTICES:
				case HK_SHAPE_CONVEX_TRANSFORM:
				case HK_SHAPE_CONVEX_TRANSLATE:
				case HK_SHAPE_BOX:
				case HK_SHAPE_SPHERE:
				case HK_SHAPE_TRIANGLE:
					{
						const hkConvexShape* convexShape = static_cast<const hkConvexShape*>(shape);
						this->addConvexShape(convexShape, key, directions, numDirections);
						break;
					}
				default:
					{
						HK_ASSERT2( 0xaf2132ef, false, "This function requires a shape collection made of only convex objects. As a workaround you can disable hkMoppFitToleranceRequirements::m_cachePrimitiveExtents");
						break;
					}
			}

			key = m_shapeCollection->getNextKey(key);
		}
	}

	return;
}


void hkMoppCachedShapeMediator::addConvexShape(const hkConvexShape* convexShape, hkShapeKey key, const hkMoppSplittingPlaneDirection* directions, int numDirections)
{
	// extract information on the convex shape's vertices/spheres
	hkSphereRepShape::hkCollisionSpheresInfo spheresInfo;
	convexShape->getCollisionSpheresInfo(spheresInfo);

	int numSpheres = spheresInfo.m_numSpheres;

	// allocate memory on stack (if possible)
	hkSphere* sphereBuffer = hkAllocateStack<hkSphere>(numSpheres);

	// extract the convex shape's vertices/spheres themselves
	convexShape->getCollisionSpheres(sphereBuffer);

	hkConvexShapeData& convexShapeData = m_arrayConvexShapeData.expandOne();
	convexShapeData.m_key = key;

	//
	// calculate the shape's minimum/maximum extent on each splitting plane direction
	//
	{
		hkReal minimum;
		hkReal maximum;

		for (int currentDirection = 0; currentDirection < numDirections; currentDirection++)
		{
			if ( numSpheres > 0 )
			{
				// regular case: at least one vertex/sphere is present

				hkVector4 direction = directions[currentDirection].m_direction;

				hkReal sphereRadius;
				hkReal positionOnAxis;
				hkReal positionOnAxisMin;
				hkReal positionOnAxisMax;

				// handle first sphere separately for better performance
				{
					sphereRadius = sphereBuffer[0].getRadius();

					positionOnAxis = hkReal(sphereBuffer[0].getPosition().dot3(direction));
					positionOnAxisMin = positionOnAxis - sphereRadius;
					positionOnAxisMax = positionOnAxis + sphereRadius;
				}

				minimum = positionOnAxisMin;
				maximum = positionOnAxisMax;

				// handle the remaining spheres
				for (int sphereIndex = 1; sphereIndex < numSpheres; sphereIndex++)
				{
					// project vertex onto axis
					positionOnAxis = hkReal(sphereBuffer[sphereIndex].getPosition().dot3(direction));

					// get the sphere's radius (results to 0 if we are dealing with a simple vertex)
					sphereRadius = sphereBuffer[sphereIndex].getRadius();

					// convex shape could be a true sphere -> take radius into account
					positionOnAxisMin = positionOnAxis - sphereRadius;
					positionOnAxisMax = positionOnAxis + sphereRadius;

					// update minimum/maximum values
					if ( positionOnAxisMin < minimum )
					{
						minimum = positionOnAxisMin;
					}
					if ( positionOnAxisMax > maximum )
					{
						maximum = positionOnAxisMax;
					}
				}
			}
			else
			{
				// special case for shapes with no vertices
				minimum = 0;
				maximum = 0;
			}

			// store the shape's minimum/maximum extent on the current axis
			convexShapeData.m_extents[currentDirection].m_min = minimum;
			convexShapeData.m_extents[currentDirection].m_max = maximum;
		}
	}

	hkDeallocateStack<hkSphere>(sphereBuffer);

	return;
}


int hkMoppCachedShapeMediator::getPrimitiveProperties(const hkMoppCompilerPrimitive& primitiveIn, hkPrimitiveProperty cid[hkMoppCode::MAX_PRIMITIVE_PROPERTIES])
{
	cid[0] = 0;

	return 1;
}


void hkMoppCachedShapeMediator::projectPrimitive(const hkMoppCompilerPrimitive& primitiveIn, int directionIndex, hkReal* minimum, hkReal* maximum)
{
	*minimum = m_arrayConvexShapeData[primitiveIn.m_primitiveID2].m_extents[directionIndex].m_min;
	*maximum = m_arrayConvexShapeData[primitiveIn.m_primitiveID2].m_extents[directionIndex].m_max;
}


void hkMoppCachedShapeMediator::projectPrimitives(const hkVector4& direction, int directionIndex, hkMoppCompilerPrimitive* primitives, int numPrimitives,
														  hkReal* absMinOut, hkReal* absMaxOut)
{
	HK_ASSERT(0xaf539457, numPrimitives > 0 );

	hkReal maximum;
	hkReal minimum;

	// handle first primitive separately for better performance
	{
		projectPrimitive(primitives[0], directionIndex, &minimum, &maximum);

		primitives[0].m_extent.m_min = minimum;
		primitives[0].m_extent.m_max = maximum;
	}

	hkReal absMin = minimum;
	hkReal absMax = maximum;

	// handle the remaining primitives
	for (int i = 1; i < numPrimitives; i++)
	{
		// project the primitive and find its minimum and maximum extend on the supplied axis
		projectPrimitive(primitives[i], directionIndex, &minimum, &maximum);

		// store the primitive's extent values (used by e.g. quicksort algorithm)
		primitives[i].m_extent.m_min = minimum;
		primitives[i].m_extent.m_max = maximum;

		// update minimum/maximum values
		absMin = hkMath::min2( absMin, minimum );
		absMax = hkMath::max2( absMax, maximum );
	}

	*absMinOut = absMin;
	*absMaxOut = absMax;
}

void hkMoppCachedShapeMediator::findExtents(const hkVector4& direction, int directionIndex, const hkMoppCompilerPrimitive* primitives, int numPrimitives,
														  hkReal* absMinOut, hkReal* absMaxOut)
{
	HK_ASSERT(0xaf539456, numPrimitives > 0 );

	hkReal maximum;
	hkReal minimum;
	
	// handle first primitive separately for better performance
	{
		projectPrimitive(primitives[0], directionIndex, &minimum, &maximum);
	}

	hkReal absMin = minimum;
	hkReal absMax = maximum;

	// handle the remaining primitives
	for (int i = 1; i < numPrimitives; i++)
	{
		// project the primitive and find its minimum and maximum extend on the supplied axis
		projectPrimitive(primitives[i], directionIndex, &minimum, &maximum);

		// update minimum/maximum values
		absMin = hkMath::min2( absMin, minimum );
		absMax = hkMath::max2( absMax, maximum );
	}

	*absMinOut = absMin;
	*absMaxOut = absMax;
}

void hkMoppCachedShapeMediator::getPrimitives(hkMoppCompilerPrimitive* primitives)
{
	int nChildren = m_numChildShapes;
	hkShapeKey shapeKey = m_shapeCollection->getFirstKey();
	for (int shapeIndex = 0; shapeIndex < nChildren; shapeIndex++)
	{
		//TODO: this does not allow for split triangles RonanOS 2002-04-16
		primitives->m_primitiveID	= shapeKey;
		primitives->m_primitiveID2	= shapeIndex;
		primitives++;
		shapeKey = m_shapeCollection->getNextKey(shapeKey);
	}
}

void hkMoppCachedShapeMediator::splitPrimitive(const hkMoppCompilerPrimitive& primitiveIn,
										const hkVector4& direction, hkReal planeOffset, int depth,
										hkMoppCompilerPrimitive* primitiveOut )
{
	primitiveOut[0] = primitiveIn;
}


int hkMoppCachedShapeMediator::getNumPrimitives()
{
	return m_numChildShapes;
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
