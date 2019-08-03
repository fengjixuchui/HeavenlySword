/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkmath/basetypes/hkGeometry.h>
#include <hkutilities/visualdebugger/viewer/hkShapeDisplayBuilder.h>
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

#include <hkinternal/preprocess/convexhull/hkGeometryUtility.h>

#include <hkvisualize/shape/hkDisplayPlane.h>
#include <hkvisualize/shape/hkDisplaySphere.h>
#include <hkvisualize/shape/hkDisplayCylinder.h>
#include <hkvisualize/shape/hkDisplayCapsule.h>
#include <hkvisualize/shape/hkDisplayBox.h>
#include <hkvisualize/shape/hkDisplayConvex.h>
#include <hkvisualize/hkDebugDisplay.h>


hkShapeDisplayBuilder::hkShapeDisplayBuilderEnvironment::hkShapeDisplayBuilderEnvironment()
:	m_spherePhiRes(8),
	m_sphereThetaRes(8)
{
}



hkShapeDisplayBuilder::hkShapeDisplayBuilder(const hkShapeDisplayBuilderEnvironment& env)
:	m_environment(env),
	m_currentGeometry(HK_NULL)
{
}


void hkShapeDisplayBuilder::buildDisplayGeometries(		const hkShape* shape,
														hkArray<hkDisplayGeometry*>& displayGeometries)
{

	hkTransform transform;
	transform.setIdentity();

	resetCurrentRawGeometry();
	displayGeometries.clear();

	buildShapeDisplay(shape, transform, displayGeometries);
}



// This is the alternative to having a buildDisplayGeometry as a virtual function in Shape.
void hkShapeDisplayBuilder::buildShapeDisplay(		const hkShape* shape,
													const hkTransform& transform,
													hkArray<hkDisplayGeometry*>& displayGeometries)
{
	switch (shape->getType())
	{
		case HK_SHAPE_SPHERE:
		{
			const hkSphereShape* sphereShape = static_cast<const hkSphereShape*>(shape);
			hkVector4 zeroVector;
			zeroVector.setZero4();
			hkSphere sphere( zeroVector, sphereShape->getRadius());

			hkDisplaySphere* displaySphere = new hkDisplaySphere(sphere, m_environment.m_sphereThetaRes, m_environment.m_spherePhiRes);

			displaySphere->getTransform() = transform;
			displayGeometries.pushBack(displaySphere);

			break;
		}
		case HK_SHAPE_MULTI_SPHERE:
		{
			const hkMultiSphereShape* s = static_cast<const hkMultiSphereShape*>(shape);
			const hkVector4* v = s->getSpheres();
			for(int i = 0; i < s->getNumSpheres(); ++i)
			{
				hkSphere sphere( hkVector4::getZero(), v[i](3) );
				hkDisplaySphere* displaySphere = new hkDisplaySphere(sphere, m_environment.m_sphereThetaRes, m_environment.m_spherePhiRes);

				displaySphere->getTransform().setIdentity();
				displaySphere->getTransform().setTranslation(v[i]);
				displayGeometries.pushBack(displaySphere);
			}

			break;
		}
		case HK_SHAPE_PLANE:
		{
			const hkPlaneShape* planeShape = static_cast<const hkPlaneShape*>(shape);
			const hkVector4& pEq = planeShape->getPlane();
			hkAabb localAabb;
			planeShape->getAabb(hkTransform::getIdentity(), 0.001f, localAabb);
			
			hkVector4 c; c.setSub4(localAabb.m_max, localAabb.m_min);
			hkVector4 perpToNorm; perpToNorm.setZero4(); perpToNorm(c.getMajorAxis()) = 1;
			c.mul4(0.5f);
			hkVector4 ext = c;
			c.add4(localAabb.m_min);

			hkDisplayPlane* displayPlane = new hkDisplayPlane(pEq, perpToNorm, c, ext);
			displayPlane->getTransform() = transform;
			displayGeometries.pushBack(displayPlane);
			break;
		}
		case HK_SHAPE_CAPSULE:
		{
			const hkCapsuleShape* s = static_cast<const hkCapsuleShape*>(shape);

			hkDisplayCapsule* displayCapsule = new hkDisplayCapsule( s->getVertex(1), s->getVertex(0), s->getRadius() );
			displayCapsule->getTransform() = transform;
			displayGeometries.pushBack( displayCapsule );
 
			break;
		}
		case HK_SHAPE_CYLINDER:
		{
			const hkCylinderShape* s = static_cast<const hkCylinderShape*>(shape);

			hkVector4 top;		top.setTransformedPos( transform, s->getVertex(1) );
			hkVector4 bottom;	bottom.setTransformedPos( transform, s->getVertex(0) );
			hkDisplayCylinder* displayCylinder = new hkDisplayCylinder( top, bottom, s->getCylinderRadius() );
			displayCylinder->getTransform() = transform;
			displayGeometries.pushBack( displayCylinder );
 
			break;
		}

		case HK_SHAPE_MULTI_RAY:
		{
			// TODO
			const hkMultiRayShape* s = static_cast<const hkMultiRayShape*>(shape);

			hkDisplayGeometry* displayGeom = getCurrentRawGeometry(displayGeometries);
			hkGeometry* geom = displayGeom->getGeometry();
			HK_ASSERT(0x142cb874, geom != HK_NULL);

			int vertBase = 0;

			for(int j = 0; j < s->getRays().getSize(); j++)
			{
				hkMultiRayShape::Ray seg = s->getRays()[j];

				hkVector4& start = *geom->m_vertices.expandBy(1);
				start = seg.m_start;
				start.setTransformedPos( transform, start );

				hkVector4& joggle = *geom->m_vertices.expandBy(1);
				joggle = seg.m_start;
				hkVector4 offset; offset.set( 0.01f, 0.f, 0.f );
				joggle.add4( offset );

				hkVector4& end = *geom->m_vertices.expandBy(1);
				end = seg.m_end;
				end.setTransformedPos( transform, end );

				hkGeometry::Triangle& tri = *geom->m_triangles.expandBy(1);
				tri.set(vertBase, vertBase + 1, vertBase + 2);

				vertBase += 3;
			}
			break;
		}
		case HK_SHAPE_BOX:
		{
			const hkBoxShape* boxShape = static_cast<const hkBoxShape*>(shape);
			hkDisplayBox* displayBox = new hkDisplayBox(boxShape->getHalfExtents());
			displayBox->getTransform() = transform;
			displayGeometries.pushBack(displayBox);
			break;
		}
		
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
		/*
		case HK_SHAPE_CONVEX_WELDER:
		{
			const hkConvexWelderShape* cxWeldShape = static_cast<const hkConvexWelderShape*>(shape);
			shape = cxWeldShape->m_compoundShapeToBeWelded;
		}
		*/

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
			const hkShapeContainer* shapeContainer = shape->getContainer();
			
			hkShapeCollection::ShapeBuffer buffer;

			for (hkShapeKey key = shapeContainer->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = shapeContainer->getNextKey( key ) )
			{
				const hkShape* child = shapeContainer->getChildShape(key, buffer );
				buildShapeDisplay(child, transform, displayGeometries);
			}
		
			break;
		}
		case HK_SHAPE_TRIANGLE:
		{
			const hkTriangleShape* triangleShape = static_cast<const hkTriangleShape*>(shape);

			hkDisplayGeometry* displayGeom = getCurrentRawGeometry(displayGeometries);
			hkGeometry* geom = displayGeom->getGeometry();
			HK_ASSERT(0x4b5bb14e, geom != HK_NULL);

				// Convert these vertices to the transformed space.
			int vertBase = geom->m_vertices.getSize();

			geom->m_vertices.expandBy(1)->setTransformedPos(transform, triangleShape->getVertex(0));
			geom->m_vertices.expandBy(1)->setTransformedPos(transform, triangleShape->getVertex(1));
			geom->m_vertices.expandBy(1)->setTransformedPos(transform, triangleShape->getVertex(2));
			hkGeometry::Triangle& tri = *geom->m_triangles.expandBy(1);
			tri.set(vertBase, vertBase + 1, vertBase + 2);

			break;
		}

		case HK_SHAPE_CONVEX_VERTICES:
		{
			const hkConvexVerticesShape* convexVerticesShape = static_cast<const hkConvexVerticesShape*>(shape);

			hkSphereRepShape::hkCollisionSpheresInfo info;
			convexVerticesShape->getCollisionSpheresInfo( info );

			hkArray<hkSphere> vertices;
			if ( info.m_useBuffer )
			{
				vertices.setSize( info.m_numSpheres );
			}
			
			const hkSphere* spheres = convexVerticesShape->getCollisionSpheres(vertices.begin());

				// Convert these vertices to the transformed space.
			hkArray<hkVector4> transformedVertices;
			transformedVertices.setSize( info.m_numSpheres );
			for(int i = 0; i < info.m_numSpheres; i++)
			{
				transformedVertices[i].setTransformedPos(transform, spheres[i].getPosition());

			}

			hkGeometry* outputGeom = new hkGeometry;
			hkArray<hkVector4> usedVertices;
			hkStridedVertices stridedVerts;
			{
				stridedVerts.m_numVertices = transformedVertices.getSize();
				stridedVerts.m_striding = sizeof(hkVector4);
				stridedVerts.m_vertices = &(transformedVertices[0](0));
			}
			hkGeometryUtility::createConvexGeometry( stridedVerts, *outputGeom );

			hkDisplayConvex* displayGeom = new hkDisplayConvex(outputGeom);

			displayGeometries.pushBack(displayGeom);
			break;
		}
		case HK_SHAPE_CONVEX_PIECE:
		{
			const hkConvexPieceShape* triangulatedConvexShape = static_cast<const hkConvexPieceShape*>(shape);

			// Create the geometry
			hkGeometry* outputGeom = new hkGeometry;

			hkShapeCollection::ShapeBuffer buffer2;

			for ( int i = 0 ; i < triangulatedConvexShape->m_numDisplayShapeKeys ; i++ )
			{
				const hkTriangleShape& triangleShape = *( static_cast< const hkTriangleShape* >( 
					triangulatedConvexShape->m_displayMesh->getChildShape( triangulatedConvexShape->m_displayShapeKeys[i], buffer2 ) ));

				// pushback information about this triangle to the new geometry.
				hkGeometry::Triangle& tri = *outputGeom->m_triangles.expandBy(1);

				int vertexSize = outputGeom->m_vertices.getSize();
				tri.set( vertexSize, vertexSize+1, vertexSize+2	);

				for ( int j = 0 ; j < 3 ; j++ )
				{
					hkVector4& transformedVertex = *outputGeom->m_vertices.expandBy(1);
					transformedVertex.setTransformedPos(transform, triangleShape.getVertex( j ));
				}
			}
			
			hkDisplayConvex* displayGeom = new hkDisplayConvex(outputGeom);
			displayGeometries.pushBack(displayGeom);
			
			break;
		}

		case HK_SHAPE_SAMPLED_HEIGHT_FIELD:
		{
			const hkSampledHeightFieldShape* heightField = static_cast<const hkSampledHeightFieldShape*>(shape);

			hkDisplayGeometry* displayGeom = getCurrentRawGeometry(displayGeometries);
			hkGeometry* geom = displayGeom->getGeometry();
			HK_ASSERT(0x34673afe, geom != HK_NULL);

				// Convert these vertices to the transformed space.
			hkVector4 scale = heightField->m_intToFloatScale;

			for (int i = 0; i < heightField->m_xRes-1; i++)
			{
				for (int j = 0; j < heightField->m_zRes-1; j++)
				{
					hkVector4 p00; p00.set( i+0.f, heightField->getHeightAt( i+0, j+0 ), j+0.f ); p00.mul4( scale );
					hkVector4 p01; p01.set( i+0.f, heightField->getHeightAt( i+0, j+1 ), j+1.f ); p01.mul4( scale );
					hkVector4 p10; p10.set( i+1.f, heightField->getHeightAt( i+1, j+0 ), j+0.f ); p10.mul4( scale );
					hkVector4 p11; p11.set( i+1.f, heightField->getHeightAt( i+1, j+1 ), j+1.f ); p11.mul4( scale );

					{
						int vertBase = geom->m_vertices.getSize();

						geom->m_vertices.expandBy(1)->setTransformedPos(transform, p00 );
						geom->m_vertices.expandBy(1)->setTransformedPos(transform, p01 );
						geom->m_vertices.expandBy(1)->setTransformedPos(transform, p10 );
						geom->m_vertices.expandBy(1)->setTransformedPos(transform, p11 );

						if ( heightField->getTriangleFlip())
						{
							geom->m_triangles.expandBy(1)->set(vertBase + 0, vertBase + 1, vertBase + 3);
							geom->m_triangles.expandBy(1)->set(vertBase + 0, vertBase + 3, vertBase + 2);
						}
						else
						{
							geom->m_triangles.expandBy(1)->set(vertBase + 0, vertBase + 1, vertBase + 2);
							geom->m_triangles.expandBy(1)->set(vertBase + 3, vertBase + 2, vertBase + 1);
						}
					}
				}
			}

			break;			
		}

		default:
		{
			for (int i = 0; i < hkUserShapeDisplayBuilder::getInstance().m_userShapeBuilders.getSize(); ++i )
			{
				if ( hkUserShapeDisplayBuilder::getInstance().m_userShapeBuilders[i].type == shape->getType() )
				{
					hkUserShapeDisplayBuilder::getInstance().m_userShapeBuilders[i].f( shape, transform, displayGeometries, this );
					continue;
				}
			}
		}
	}
}

hkDisplayGeometry* hkShapeDisplayBuilder::getCurrentRawGeometry(hkArray<hkDisplayGeometry*>& displayGeometries)
{
	if (m_currentGeometry == HK_NULL)
	{
		hkGeometry* geom = new hkGeometry;
		m_currentGeometry = new hkDisplayConvex(geom);
		displayGeometries.pushBack(m_currentGeometry);
	}
	return m_currentGeometry;
}


void hkShapeDisplayBuilder::resetCurrentRawGeometry()
{
	m_currentGeometry = HK_NULL;
}


HK_SINGLETON_IMPLEMENTATION(hkUserShapeDisplayBuilder);


void hkUserShapeDisplayBuilder::registerUserShapeDisplayBuilder( ShapeBuilderFunction f, hkShapeType type )
{
	for (int i = 0; i < m_userShapeBuilders.getSize(); ++i )
	{
		if ( m_userShapeBuilders[i].type == type )
		{
			HK_WARN(0x7bbfa3c4, "You have registered two shape display builders for user type" << type << ". Do you have two different shapes with this type?");
			return;
		}
	}
	UserShapeBuilder b;
	b.f = f;
	b.type = type;

	m_userShapeBuilders.pushBack(b);
}

void HK_CALL hkShapeDisplayBuilder::addObjectToDebugDisplay( const hkShape* shape, hkTransform& t, hkUlong id )
{
	hkShapeDisplayBuilder::hkShapeDisplayBuilderEnvironment env;
	hkShapeDisplayBuilder builder(env);


	hkArray<hkDisplayGeometry*> displayGeometries;

	builder.buildDisplayGeometries( shape, displayGeometries );
	hkDebugDisplay::getInstance().addGeometry( displayGeometries, t, id, 0 );

	while( displayGeometries.getSize() )
	{
		delete displayGeometries[0];
		displayGeometries.removeAt(0);
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
