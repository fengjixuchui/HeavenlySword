/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkmath/basetypes/hkGeometry.h>
#include <hkutilities/visualdebugger/viewer/hkConvexRadiusBuilder.h>
#include <hkbase/memory/hkLocalArray.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/multisphere/hkMultiSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/bv/hkBvShape.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>
#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldShape.h>
#include <hkcollide/shape/plane/hkPlaneShape.h>

#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/multiray/hkMultiRayShape.h>

#include <hkcollide/shape/convexpiecemesh/hkConvexPieceShape.h>
#include <hkcollide/util/hkTriangleUtil.h>

#include <hkinternal/preprocess/convexhull/hkGeometryUtility.h>

#include <hkvisualize/shape/hkDisplayPlane.h>
#include <hkvisualize/shape/hkDisplaySphere.h>
#include <hkvisualize/shape/hkDisplayCylinder.h>
#include <hkvisualize/shape/hkDisplayCapsule.h>
#include <hkvisualize/shape/hkDisplayBox.h>
#include <hkvisualize/shape/hkDisplayConvex.h>
#include <hkvisualize/hkDebugDisplay.h>


hkConvexRadiusBuilder::hkConvexRadiusBuilderEnvironment::hkConvexRadiusBuilderEnvironment()
:	m_minVisibleRadius(0.001f)
{
}



hkConvexRadiusBuilder::hkConvexRadiusBuilder(const hkConvexRadiusBuilderEnvironment& env)
:	m_environment(env),
	m_currentGeometry(HK_NULL)
{
}


void hkConvexRadiusBuilder::buildDisplayGeometries(		const hkShape* shape,
														hkArray<hkDisplayGeometry*>& displayGeometries)
{

	hkTransform transform;
	transform.setIdentity();

	resetCurrentRawGeometry();
	displayGeometries.clear();

	buildShapeDisplay(shape, transform, displayGeometries);
}

static hkDisplayConvex* _createConvexDisplayFromPlanes( const hkArray<hkVector4>& planeEqs, const hkTransform& transform )
{
	// Create verts from planes
	hkArray<hkVector4> verts;
	hkGeometryUtility::createVerticesFromPlaneEquations(planeEqs, verts);

	if (verts.getSize() < 1)
	{
		return HK_NULL;
	}

	// Transform verts
	hkArray<hkVector4> transformedVerts;
	int numVerts = verts.getSize();
	transformedVerts.setSize(numVerts);
	for (int i=0; i < numVerts; ++i)
	{
		transformedVerts[i].setTransformedPos(transform, verts[i]);
	}

	hkGeometry* outputGeom = new hkGeometry;
	hkStridedVertices stridedVerts;
	{
		stridedVerts.m_numVertices = transformedVerts.getSize();
		stridedVerts.m_striding = sizeof(hkVector4);
		stridedVerts.m_vertices = &(transformedVerts[0](0));
	}

	hkGeometryUtility::createConvexGeometry( stridedVerts, *outputGeom );

	return new hkDisplayConvex(outputGeom);
}

static float _getPlaneToSurfaceDistance(const hkConvexVerticesShape* shape)
{
	const hkArray<hkVector4>& planes = shape->getPlaneEquations();
	hkArray<hkVector4> vertices;
	shape->getOriginalVertices( vertices );

	const int numVerts = vertices.getSize();
	const int numPlanes = planes.getSize();
		
	float closest = -1000;
	for (int v = 0; v < numVerts; v++)
	{
		const hkVector4& vert = vertices[v];
		for (int p = 0; p < numPlanes; p++)
		{
			const hkReal distFromPlane = static_cast<hkReal>(vert.dot3(planes[p])) + planes[p](3);
			if (distFromPlane > closest) // dot is negative if on internal side.
			{
				closest = distFromPlane;
			}
		}
	}
	return closest;
}

static void _addBoundingPlanes(const hkShape* s, float extraRadius, hkArray<hkVector4>& planes)
{
	hkAabb aabb;
	s->getAabb(hkTransform::getIdentity(), extraRadius, aabb);
	planes.expandBy(6);
	
	int numPlanes = planes.getSize();
	planes[ numPlanes - 6 ].set( 1, 0, 0, -aabb.m_max(0) );
	planes[ numPlanes - 5 ].set( 0, 1, 0, -aabb.m_max(1) );
	planes[ numPlanes - 4 ].set( 0, 0, 1, -aabb.m_max(2) );
	planes[ numPlanes - 3 ].set( -1, 0, 0, aabb.m_min(0) );
	planes[ numPlanes - 2 ].set( 0, -1, 0, aabb.m_min(1) );
	planes[ numPlanes - 1 ].set( 0, 0, -1, aabb.m_min(2) );
}

// This is the alternative to having a buildDisplayGeometry as a virtual function in Shape.
void hkConvexRadiusBuilder::buildShapeDisplay(		const hkShape* shape,
													const hkTransform& transform,
													hkArray<hkDisplayGeometry*>& displayGeometries)
{
	switch (shape->getType())
	{
		//
		// These do not use convex radius:
		//
		case HK_SHAPE_SPHERE:
		case HK_SHAPE_MULTI_SPHERE:
		case HK_SHAPE_PLANE:
		case HK_SHAPE_CAPSULE:
		case HK_SHAPE_MULTI_RAY:
		case HK_SHAPE_SAMPLED_HEIGHT_FIELD:
			break;
		
		//
		// Shape wrapper types
		//

		case HK_SHAPE_CONVEX_TRANSLATE:
		{
			const hkConvexTranslateShape* ts = static_cast<const hkConvexTranslateShape*>( shape );
			hkTransform tst; tst.setIdentity();	tst.setTranslation( ts->getTranslation());
			hkTransform T; T.setMul( transform, tst );
			buildShapeDisplay( ts->getChildShape(), T, displayGeometries);
			break;
		}

		case HK_SHAPE_CONVEX_TRANSFORM:
		{
			const hkConvexTransformShape* ts = static_cast<const hkConvexTransformShape*>( shape );
			hkTransform T; T.setMul( transform, ts->getTransform() );
			buildShapeDisplay( ts->getChildShape(), T, displayGeometries);
			break;
		}	

		case HK_SHAPE_TRANSFORM:
		{
			const hkTransformShape* ts = static_cast<const hkTransformShape*>( shape );
			hkTransform T; T.setMul( transform, ts->getTransform() );
			buildShapeDisplay( ts->getChildShape(), T, displayGeometries);
			break;
		}
		case HK_SHAPE_BV:
		{
			const hkBvShape* bvShape = static_cast<const hkBvShape*>(shape);
			buildShapeDisplay( bvShape->getBoundingVolumeShape(), transform, displayGeometries);
			break;
		}

		case HK_SHAPE_BV_TREE:
		case HK_SHAPE_MOPP:
		{
			const hkBvTreeShape* bvShape = static_cast<const hkBvTreeShape*>(shape);
			buildShapeDisplay(bvShape->getShapeCollection(), transform, displayGeometries);
			break;
		}
		
		case HK_SHAPE_CONVEX_LIST:
		case HK_SHAPE_LIST:
		case HK_SHAPE_COLLECTION:
		case HK_SHAPE_TRI_PATCH:
		case HK_SHAPE_TRIANGLE_COLLECTION:
		{
			const hkShapeCollection* shapeCollection = static_cast<const hkShapeCollection*>(shape);
			hkShapeCollection::ShapeBuffer buffer;
			for (hkShapeKey key = shapeCollection->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = shapeCollection->getNextKey( key ) )
			{
				const hkShape* child = shapeCollection->getChildShape(key, buffer );
				buildShapeDisplay(child, transform, displayGeometries);
			}
			break;
		}

		case HK_SHAPE_CONVEX_PIECE:
		{
			const hkConvexPieceShape* triangulatedConvexShape = static_cast<const hkConvexPieceShape*>(shape);
			if (triangulatedConvexShape->getRadius() > m_environment.m_minVisibleRadius)
			{
				hkShapeCollection::ShapeBuffer buffer2;
				for ( int i = 0 ; i < triangulatedConvexShape->m_numDisplayShapeKeys ; i++ )
				{
					const hkTriangleShape& triangleShape = *( static_cast< const hkTriangleShape* >( 
						triangulatedConvexShape->m_displayMesh->getChildShape( triangulatedConvexShape->m_displayShapeKeys[i], buffer2 ) ));
					buildShapeDisplay(&triangleShape, transform, displayGeometries);	
				}
			}
			break;
		}

		//
		// shapes that use a radius
		//

		case HK_SHAPE_CYLINDER:
		{
			const hkCylinderShape* s = static_cast<const hkCylinderShape*>(shape);
			float cylRadius = s->getCylinderRadius();
			float convexRadius = s->getRadius(); // cyl code has this as the 'padding' radius
			if (convexRadius > m_environment.m_minVisibleRadius)
			{
				hkVector4 top;		top.setTransformedPos( transform, s->getVertex(1) );
				hkVector4 bottom;	bottom.setTransformedPos( transform, s->getVertex(0) );

				// add in the top and bottom radius
				// the radius on the sides will be added to the cyl radius
				hkVector4 axis; axis.setSub4(top, bottom); axis.normalize3();
				axis.mul4(convexRadius);
				top.add4(axis);
				bottom.sub4(axis);

				float totalRadius = cylRadius + convexRadius;
				hkDisplayCylinder* displayCylinder = new hkDisplayCylinder( top, bottom, totalRadius );
				displayCylinder->getTransform() = transform;
				displayGeometries.pushBack( displayCylinder );
			}
			break;
		}

		case HK_SHAPE_BOX:
		{
			const hkBoxShape* boxShape = static_cast<const hkBoxShape*>(shape);
			float convexRadius = boxShape->getRadius();
			if (convexRadius > m_environment.m_minVisibleRadius)
			{
				hkVector4 trueExtents; trueExtents.setAll(convexRadius);
				trueExtents.add4( boxShape->getHalfExtents() );

				hkDisplayBox* displayBox = new hkDisplayBox(trueExtents);
				displayBox->getTransform() = transform;
				displayGeometries.pushBack(displayBox);
			}
			break;
		}

		case HK_SHAPE_TRIANGLE:
		{
			const hkTriangleShape* triangleShape = static_cast<const hkTriangleShape*>(shape);
			float convexRadius = triangleShape->getRadius();
			if (convexRadius > m_environment.m_minVisibleRadius)
			{
				// A triangle with radius is a convex object made of 5 planes (3 edges, 2 faces)
				// but the radius is distance from the verts so really the pointy edges are rounded
				// so we will add 3 planes at each end point too. If we don't thin triangled get very pointy
				// ends that look like a bug to the unknowing user. 

				// For a large landscape this may be too much to show (as each tri is now up to around 12 new tris (10 normal + flatened pointy ends)
				// but most landscapes have 0 radius.
				const hkVector4& p0 = triangleShape->getVertex(0);
				const hkVector4& p1 = triangleShape->getVertex(1);
				const hkVector4& p2 = triangleShape->getVertex(2);

				if ( hkTriangleUtil::isDegenerate(p0, p1, p2, 0.001f) )
					break; // too small..

				hkVector4 edge01; edge01.setSub4(p1, p0);
				edge01.normalize3();
				hkVector4 edge12; edge12.setSub4(p2, p1);
				edge12.normalize3();
				hkVector4 edge02; edge02.setSub4(p2, p0);
				edge02.normalize3();
				hkVector4 normal; normal.setCross(edge01, edge02); 

				hkArray<hkVector4> planes(8);
				planes.setSize(8);				
				hkVector4 convexRadiusNormal; 
				hkVector4 planePoint; 

				// face planes
				planes[0] = normal; 
				convexRadiusNormal.setMul4(convexRadius, planes[0]);
				planePoint.setAdd4(convexRadiusNormal, p0);
				planes[0](3) = -planes[0].dot3( planePoint );

				planes[1].setNeg4(normal); 
				convexRadiusNormal.setMul4(convexRadius, planes[1]);
				planePoint.setAdd4(convexRadiusNormal, p0);
				planes[1](3) = -planes[1].dot3( planePoint );

				// edge planes
				planes[2].setCross(edge01, normal); 
				convexRadiusNormal.setMul4(convexRadius, planes[2]);
				planePoint.setAdd4(convexRadiusNormal, p0);
				planes[2](3) = -planes[2].dot3( planePoint );
				
				planes[3].setCross(edge12, normal); 
				convexRadiusNormal.setMul4(convexRadius, planes[3]);
				planePoint.setAdd4(convexRadiusNormal, p1);
				planes[3](3) = -planes[3].dot3( planePoint );
				
				planes[4].setCross(normal, edge02); 
				convexRadiusNormal.setMul4(convexRadius, planes[4]);
				planePoint.setAdd4(convexRadiusNormal, p2);
				planes[4](3) = -planes[4].dot3( planePoint );

				// extra edges and end points to tighten it up.
				planes[5].setAdd4(planes[2], planes[3]); planes[5].normalize3();
				convexRadiusNormal.setMul4(convexRadius, planes[5]);
				planePoint.setAdd4(convexRadiusNormal, p1);
				planes[5](3) = -planes[5].dot3( planePoint );

				planes[6].setAdd4(planes[3], planes[4]); planes[6].normalize3();
				convexRadiusNormal.setMul4(convexRadius, planes[6]);
				planePoint.setAdd4(convexRadiusNormal, p2);
				planes[6](3) = -planes[6].dot3( planePoint );

				planes[7].setAdd4(planes[4], planes[2]); planes[7].normalize3();
				convexRadiusNormal.setMul4(convexRadius, planes[7]);
				planePoint.setAdd4(convexRadiusNormal, p0);
				planes[7](3) = -planes[7].dot3( planePoint );

				// create vertices based on planes intersection points
				hkDisplayConvex* displayGeom = _createConvexDisplayFromPlanes( planes, transform  );
				if (displayGeom)
				{
					displayGeometries.pushBack(displayGeom);
				}
				else
				{		
					HK_WARN(0x3236452A, "Could not create shape representing the convex radius around a triangle!");
				}
			}
			break;
		}

		case HK_SHAPE_CONVEX_VERTICES:
		{
			const hkConvexVerticesShape* convexVerticesShape = static_cast<const hkConvexVerticesShape*>(shape);
			float convexRadius = convexVerticesShape->getRadius();
			if (convexRadius > m_environment.m_minVisibleRadius)
			{
				// copy plane equations and expand
				const hkArray<hkVector4>& planeEqs = convexVerticesShape->getPlaneEquations();

				// We don't know if the planes are from the verts or are pre expanded by the 
				// convex radius (as happens with the shrink filter)
				// Once there is agreement on whether the planes should
				// always be a radius distance from the verts then 
				hkReal currentDist = _getPlaneToSurfaceDistance(convexVerticesShape); // may be negative
				
				hkDisplayConvex* displayGeom = HK_NULL;
				const hkReal RADIUS_EPSILON = 0.005f;
				hkArray<hkVector4> newPlanesEqs; newPlanesEqs = planeEqs;
				if ( currentDist > (RADIUS_EPSILON - convexRadius) ) // then not already pushed out (vert within radius of a plane), assume the planes are close to 0 distance from the real object surface 
				{
					int numPlanes = planeEqs.getSize();
					for (int i=0; i < numPlanes; ++i)
					{
						newPlanesEqs[i](3) -= convexRadius;
					}
				}
				
				_addBoundingPlanes(convexVerticesShape, convexRadius, newPlanesEqs);
				displayGeom = _createConvexDisplayFromPlanes( newPlanesEqs, transform );
				
				if (displayGeom)
				{
					displayGeometries.pushBack(displayGeom);
				}
				else
				{		
					HK_WARN(0x3236452A, "Could not create shape representing the convex radius around a convex shape!");
				}
			}			
			break;
		}
	
		default:
			for (int i = 0; i < hkUserConvexRadiusBuilder::getInstance().m_userConvexRadiusBuilders.getSize(); ++i )
			{
				if ( hkUserConvexRadiusBuilder::getInstance().m_userConvexRadiusBuilders[i].type == shape->getType() )
				{
					hkUserConvexRadiusBuilder::getInstance().m_userConvexRadiusBuilders[i].f( shape, transform, displayGeometries, this );
					continue;
				}
			}
	}
}

hkDisplayGeometry* hkConvexRadiusBuilder::getCurrentRawGeometry(hkArray<hkDisplayGeometry*>& displayGeometries)
{
	if (m_currentGeometry == HK_NULL)
	{
		hkGeometry* geom = new hkGeometry;
		m_currentGeometry = new hkDisplayConvex(geom);
		displayGeometries.pushBack(m_currentGeometry);
	}
	return m_currentGeometry;
}


void hkConvexRadiusBuilder::resetCurrentRawGeometry()
{
	m_currentGeometry = HK_NULL;
}


HK_SINGLETON_IMPLEMENTATION(hkUserConvexRadiusBuilder);


void hkUserConvexRadiusBuilder::registerUserConvexRadiusDisplayBuilder( ConvexRadiusBuilderFunction f, hkShapeType type )
{
	for (int i = 0; i < m_userConvexRadiusBuilders.getSize(); ++i )
	{
		if ( m_userConvexRadiusBuilders[i].type == type )
		{
			HK_WARN(0x7bbfa3c4, "You have registered two convex shape display builders for user type" << type << ". Do you have two different shapes with this type?");
			return;
		}
	}
	UserShapeBuilder b;
	b.f = f;
	b.type = type;

	m_userConvexRadiusBuilders.pushBack(b);
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
