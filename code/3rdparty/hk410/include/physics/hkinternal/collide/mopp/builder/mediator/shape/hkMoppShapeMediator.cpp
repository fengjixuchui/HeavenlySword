/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkinternal/hkInternal.h>


// Primitive Mediator Implementation

// include all Shape Mopp headers
#include <hkinternal/collide/mopp/builder/hkbuilder.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

hkMoppShapeMediator::hkMoppShapeMediator(const hkShapeContainer* shapeArray)
{
	m_shape = shapeArray;
	m_numChildShapes = m_shape->getNumChildShapes();
}

hkMoppShapeMediator::~hkMoppShapeMediator()
{
	m_shape = HK_NULL;
}


void hkMoppShapeMediator::setSplittingPlaneDirections(const hkMoppSplittingPlaneDirection* directions, int numDirections)
{
	return;
}


int hkMoppShapeMediator::getPrimitiveProperties( const hkMoppCompilerPrimitive &primitiveIn, hkPrimitiveProperty cid[hkMoppCode::MAX_PRIMITIVE_PROPERTIES])
{
	hkUint32 ShapeId = primitiveIn.m_primitiveID2;
	cid[0] = ShapeId;

	return 1;
}


void hkMoppShapeMediator::projectPrimitive( const hkMoppCompilerPrimitive &primitiveIn, const hkVector4 &direction, int directionIndex, hkReal* minimum, hkReal* maximum )
{

	/*
	hkUint32 shapeId = primitiveIn.m_primitiveID;
	hkUint32 meshId = primitiveIn.m_primitiveID2;
	if ( ShapeId >= HK_MOPP_SHAPE_MEDIATOR_MAX_SHAPES )
	{
		ShapeId -= HK_MOPP_SHAPE_MEDIATOR_MAX_SHAPES;
		const hkMoppShapeMediatorFace* face = m_faces[ShapeId];
		
		hkReal mn = face->m_vertices[0].dot3( direction );
		hkReal mx = mn;

		for (int v = 1; v < face->m_numVertices; v++)
		{
			const hkVector4 &vertex = face->m_vertices[v];
			const hkReal dot = vertex.dot3( direction );
			if (dot > mx)
			{
				mx = dot;
			}
			if ( dot < mn )
			{
				mn = dot;
			}
		}
		minimum[0] = mn;
		maximum[0] = mx;

		return;
	}
	*/

	{
		hkShapeKey key;
		key = primitiveIn.m_primitiveID;

		hkShapeCollection::ShapeBuffer buffer;

		const hkShape* shape = m_shape->getChildShape(key, buffer );

		if (shape != HK_NULL)
		{
			*maximum = shape->getMaximumProjection( direction );
			hkVector4 negDirection; negDirection.setNeg4( direction );
			*minimum = -hkReal(shape->getMaximumProjection( negDirection ));
		}
		else
		{
			*minimum = 0; // HK_REAL_MAX;
			*maximum = 0; //-HK_REAL_MAX;
		}
	}
}


//
// projectPrimitives
// retrieves the projected (unsorted) primitives from the mediator given a list of primitives
//
void hkMoppShapeMediator::projectPrimitives(const hkVector4 &direction, int directionIndex, hkMoppCompilerPrimitive* primitives, int numPrimitives,
														  hkReal* absMinOut, hkReal* absMaxOut)
{
	HK_ASSERT(0xaf539457, numPrimitives > 0 );

	hkReal maximum;
	hkReal minimum;
	
	projectPrimitive(primitives[0], direction, directionIndex, &minimum, &maximum);

	primitives[0].m_extent.m_min = minimum;
	primitives[0].m_extent.m_max = maximum;

	hkReal absMin = minimum;
	hkReal absMax = maximum;

	for (int i = 1; i < numPrimitives; i++)
	{
		// find the maximum and minimum projected vertices of the primitive
		projectPrimitive(primitives[i], direction, directionIndex, &minimum, &maximum);

		// add the minimum and maximum vertices to our (unsorted) array of distances
		primitives[i].m_extent.m_min = minimum;
		primitives[i].m_extent.m_max = maximum;
		
		// check absolute minimum and maximum extends
		if ( minimum < absMin )
		{
			absMin = minimum;
		}
		if ( maximum > absMax )
		{
			absMax = maximum;
		}
	}

	*absMinOut = absMin;
	*absMaxOut = absMax;
}

void hkMoppShapeMediator::findExtents(const hkVector4 &direction, int directionIndex, const hkMoppCompilerPrimitive* primitives, int numPrimitives,
														  hkReal* absMinOut, hkReal* absMaxOut)
{
	HK_ASSERT(0xaf539457, numPrimitives > 0 );

	hkReal maximum;
	hkReal minimum;

	projectPrimitive(primitives[0], direction, directionIndex, &minimum, &maximum);

	hkReal absMin = minimum;
	hkReal absMax = maximum;

	for (int i = 1; i < numPrimitives; i++)
	{
		// find the maximum and minimum projected vertices of the primitive
		projectPrimitive(primitives[i], direction, directionIndex, &minimum, &maximum);

		// check absolute minimum and maximum extends
		if ( minimum < absMin )
		{
			absMin = minimum;
		}
		if ( maximum > absMax )
		{
			absMax = maximum;
		}
	}

	*absMinOut = absMin;
	*absMaxOut = absMax;
}

//
// assignNewPrimitives
// assign the argument primitives to those which were added to the mediator
// the caller has to make sure enough space is allocated to hold the entire list of primitives
//
void hkMoppShapeMediator::getPrimitives(hkMoppCompilerPrimitive* primitives)
{
	int nChildren = m_numChildShapes;
	hkShapeKey key = m_shape->getFirstKey();
	for( int s = 0; s < nChildren; s++)
	{
		//TODO: this does not allow for split triangles RonanOS 2002-04-16
		primitives->m_primitiveID	= key;
		primitives->m_primitiveID2	= 0;
		primitives++;
		key = m_shape->getNextKey( key );
	}
}

void hkMoppShapeMediator::splitPrimitive( const hkMoppCompilerPrimitive &primitiveIn,
										const hkVector4 &direction, hkReal planeOffset, int depth,
										hkMoppCompilerPrimitive* primitiveOut )
{
	
	primitiveOut[0] = primitiveIn;
	/*hkMoppShapeMediatorFace* newFace;

	int ShapeId = primitiveIn.m_primitiveID2;
	if ( ShapeId >= HK_MOPP_SHAPE_MEDIATOR_MAX_SHAPES )
	{
		ShapeId -= HK_MOPP_SHAPE_MEDIATOR_MAX_SHAPES;
		const hkMoppShapeMediatorFace* face = m_faces[ShapeId];

		//newFace = new hkMoppShapeMediatorFace( direction, planeOffset, face->m_vertices, face->m_numVertices, face->m_originalShape );
	}
	else
	{
		const hkShape* Shape = m_SHAPES[ShapeId];

		Havok::Triangle triangle;
		Shape->getTriangle( primitiveIn.m_primitiveID, &triangle );

		hkVector4 vertices[3];
		vertices[0] = triangle.getVertex(0);
		vertices[1] = triangle.getVertex(1);
		vertices[2] = triangle.getVertex(2);

		newFace = new hkMoppShapeMediatorFace( direction, planeOffset, vertices, 3, ShapeId );
	}
	primitiveOut[0].m_primitiveID2 = HK_MOPP_SHAPE_MEDIATOR_MAX_SHAPES + m_faces.size();
	m_faces.push_back( newFace );
	*/
}


//
// getNumPrimitives
// retries the current number of Mopp primitives in the system
//
int hkMoppShapeMediator::getNumPrimitives()
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
