/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcollide/hkCollide.h>
#include <hkutilities/collide/hkShapeShrinker.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/bv/hkBvShape.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/convexlist/hkConvexListShape.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>
#include <hkinternal/preprocess/convexhull/hkGeometryUtility.h>


static hkShape* _shrinkShape(hkShape* shape, hkArray<hkShapeShrinker::ShapeCache>& doneShapes)
{
	// To support shared shapes in a hierarchy, we check if we have done this one before.
	for (int dsi=0; dsi < doneShapes.getSize(); ++dsi)
	{
		if (doneShapes[dsi].originalShape == shape)
		{
			return doneShapes[dsi].newShape;
		}
	}

	hkShapeShrinker::ShapeCache& ds = doneShapes.expandOne();
	ds.originalShape = shape;
	ds.newShape = HK_NULL;

	switch (shape->getType())
	{
		// types that require no shrink (radius == proper radius or no convex shape radius used)
	case HK_SHAPE_SPHERE: 
	case HK_SHAPE_MULTI_SPHERE:
	case HK_SHAPE_PLANE:
	case HK_SHAPE_CAPSULE: 
			break;

		// Unshrinkable (2D)
	case HK_SHAPE_TRIANGLE:
	case HK_SHAPE_TRIANGLE_COLLECTION:
	case HK_SHAPE_TRI_PATCH:
			break;

		// Collections or shape wrappers
	case HK_SHAPE_CONVEX_TRANSLATE:
		{
			hkConvexTranslateShape* ts = static_cast<hkConvexTranslateShape*>( shape );
			
			// Grab the child shape and shrink it
			hkConvexShape* shrunkenChild = static_cast<hkConvexShape*>(_shrinkShape(const_cast<hkConvexShape*>(ts->getChildShape()), doneShapes));

			// EXP-685 : It can be NULL if there were no changes
			if (shrunkenChild)
			{
				// Create a new translate shape with the newly shrunken child
				hkConvexTranslateShape* shrunkenTranslateShape = new hkConvexTranslateShape(shrunkenChild, ts->getTranslation());

				ds.newShape = shrunkenTranslateShape;
			}
			break;
		}
	case HK_SHAPE_CONVEX_TRANSFORM:
		{
			hkConvexTransformShape* ts = static_cast<hkConvexTransformShape*>( shape );

			// Grab the child shape and shrink it
			hkConvexShape* shrunkenChild = static_cast<hkConvexShape*>(_shrinkShape(const_cast<hkConvexShape*>(ts->getChildShape()), doneShapes));

			// EXP-685 : It can be NULL if there were no changes
			if (shrunkenChild)
			{
				// Create a new transform shape with the newly shrunken child
				hkConvexTransformShape* shrunkenTransformShape = new hkConvexTransformShape(shrunkenChild, ts->getTransform());

				ds.newShape = shrunkenTransformShape;
			}

			break;
		}	
	case HK_SHAPE_TRANSFORM:
		{
			hkTransformShape* ts = static_cast<hkTransformShape*>( shape );

			// Grab the child shape and shrink it
			hkShape* shrunkenChild = static_cast<hkShape*>(_shrinkShape(const_cast<hkShape*>(ts->getChildShape()), doneShapes));

			// EXP-685 : It can be NULL if there were no changes
			if (shrunkenChild)
			{
				// Create a new transform shape with the newly shrunken child
				hkTransformShape* shrunkenTransformShape = new hkTransformShape(shrunkenChild, ts->getTransform());

				ds.newShape = shrunkenTransformShape;
			}

			break;
		}
	case HK_SHAPE_BV:
		{
			hkBvShape* bvShape = static_cast<hkBvShape*>(shape);
			ds.newShape = _shrinkShape( const_cast<hkShape*>(bvShape->getChildShape()), doneShapes );
			break;
		}

	case HK_SHAPE_BV_TREE:
	case HK_SHAPE_MOPP:
		{
			// const hkBvTreeShape* bvShape = static_cast<const hkBvTreeShape*>(shape);
			// TODO: chould add an option to reduced the landscape radius. (can't 
			//       really add to radius as the mopp is created? )

			break;
		}
	case HK_SHAPE_CONVEX_LIST:
	case HK_SHAPE_LIST:
	case HK_SHAPE_COLLECTION:
		{
			const hkShapeCollection* shapeCollection = static_cast<const hkShapeCollection*>(shape);
			hkShapeCollection::ShapeBuffer buffer;

			bool isMutable = ((shape->getType() == HK_SHAPE_LIST) || (shape->getType() == HK_SHAPE_CONVEX_LIST));
			hkArray<hkShape*> newShapes;
			newShapes.reserveExactly(shapeCollection->getNumChildShapes());
			bool foundNewOnes = false;
			for (hkShapeKey key = shapeCollection->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = shapeCollection->getNextKey( key ) )
			{
				const hkShape* child = shapeCollection->getChildShape(key, buffer );
				hkShape* newShape = _shrinkShape(const_cast<hkShape*>(child), doneShapes);
				if (newShape)
				{
					// ehh.. not in charge of the shapes in the collecton so can't change them
					if (!isMutable)
					{
						HK_WARN( 0x0, "Found a shape collection with children that required changing. Not processing.");
					}
					foundNewOnes = true;
				}
				newShapes.pushBack(newShape? newShape : const_cast<hkShape*>(child) );
			}

			hkShape* newS = HK_NULL;
			if (foundNewOnes && isMutable)
			{
				if ( shape->getType() == HK_SHAPE_LIST ) 
					newS = new hkListShape( newShapes.begin(), newShapes.getSize() );
				else if ( shape->getType() == HK_SHAPE_CONVEX_LIST )
					newS = new hkConvexListShape( (const hkConvexShape**)newShapes.begin(), newShapes.getSize() );
			}

			ds.newShape = newS;
			break;
		}

	case HK_SHAPE_CYLINDER: // created with two radii. One is the correct one, the other (padding) left at default, so if we reduce the normal 
		// radius by the padding, and the length by the padding too it will be visually correct
		{
			hkCylinderShape* cylShape = static_cast<hkCylinderShape*>(shape);
			float rP = cylShape->getRadius();

			float rC = cylShape->getCylinderRadius();
			rC -= rP;
			cylShape->setCylinderRadius(rC);

			// shift the end points down by radius;
			hkVector4 dir; dir.setSub4( cylShape->getVertex(1), cylShape->getVertex(0) );
			dir.normalize3();
			dir.mul4( rP );

			hkVector4 newV0; newV0.setAdd4(cylShape->getVertex(0), dir);
			hkVector4 newV1; newV1.setSub4(cylShape->getVertex(1), dir );
			cylShape->setVertex(0, newV0);
			cylShape->setVertex(1, newV1);

			break;
		}

	case HK_SHAPE_BOX: // exported from max etc with default radius so definitly needs a shrink
		{
			hkBoxShape* boxShape = static_cast<hkBoxShape*>(shape);
			// reduce the extents by the radius
			float r = boxShape->getRadius();
			hkVector4 ext = boxShape->getHalfExtents();

			if (ext(0) > r)
				ext(0) -= r;
			else if (ext(0) < -r)
				ext(0) += r;
			// else large radius, small object..

			if (ext(1) > r)
				ext(1) -= r;
			else if (ext(1) < -r)
				ext(1) += r;

			if (ext(2) > r)
				ext(2) -= r;
			else if (ext(2) < -r)
				ext(2) += r;

			boxShape->setHalfExtents(ext);
			break;
		}

	case HK_SHAPE_CONVEX_VERTICES:
		{
			hkConvexVerticesShape* convexVerticesShape = static_cast<hkConvexVerticesShape*>(shape);
			const hkArray<hkVector4>& planeEq = convexVerticesShape->getPlaneEquations();
			const float radius = convexVerticesShape->getRadius();
			const int numPlanes = planeEq.getSize();
			hkArray<hkVector4> newVerts;

			// reduce the planes distance by the padding radius
			hkArray<hkVector4> newPlaneEq(numPlanes);

			for (int pidx = 0; pidx < numPlanes; ++pidx)
			{
				newPlaneEq[pidx] = planeEq[pidx];
				newPlaneEq[pidx](3) += radius;
			}

			// create new vertices based on the shrunk verts and weld
			hkGeometryUtility::createVerticesFromPlaneEquations(newPlaneEq, newVerts);

			// easy to shrink a shape too far and make no verts
			// so check if that has happened.
			if (newVerts.getSize() < 1)
			{
				HK_WARN_ALWAYS(0x447f32f6,
					"hkShapeShrinker has attempted to shrink a hkConvexVerticesShape too far,"
					" and the resulting shape is invalid. The shape has not been modified.");
				break;
			}
			
			hkStridedVertices newSverts;
			newSverts.m_numVertices = newVerts.getSize();
			newSverts.m_striding = sizeof(hkVector4);
			newSverts.m_vertices = (const float*)( newVerts.begin() );

			// new verts, orig planes (for casts)
			hkConvexVerticesShape* newShape = new hkConvexVerticesShape(newSverts, planeEq, radius);
			ds.newShape = newShape;
			break;
		}

		//
		// Unhandled at this time
		//
	case HK_SHAPE_CONVEX_PIECE:
	case HK_SHAPE_SAMPLED_HEIGHT_FIELD:	
	default:
		break;
	}

	return ds.newShape; // new shape, null if not new
}

hkShape* hkShapeShrinker::shrinkByConvexRadius( hkShape* s, hkArray<ShapeCache>* doneShapes )
{
	if (doneShapes)
	{
		return _shrinkShape(s, *doneShapes);
	}
	else
	{
		// make a temp one
		hkArray<ShapeCache> shapeCache;
		return _shrinkShape(s, shapeCache);
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
