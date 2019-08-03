/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/collide/hkShapeSharingUtil.h>

#include <hkbase/memory/hkLocalArray.h>

#include <hkmath/linear/hkVector4Util.h>
#include <hkmath/basetypes/hkGeometry.h>

#include <hkinternal/preprocess/convexhull/hkGeometryUtility.h>

#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#include <hkcollide/shape/simplemesh/hkSimpleMeshShape.h>

#include <hkdynamics/entity/hkRigidBody.h>

#include <hkbase/htl/hkAlgorithm.h>


hkShapeSharingUtil::Options::Options()
{
	m_equalityThreshold = 1e-6f;
	m_detectPermutedComponents = true;
}


hkShapeSharingUtil::Results::Results()
: m_numSharedShapes(0)
{
	reset();
}

void hkShapeSharingUtil::Results::reset()
{
	m_numSharedShapes=0;

}

void hkShapeSharingUtil::Results::report()
{
	HK_REPORT("Number of shared shapes "<<m_numSharedShapes);
}

static void _getShapeChildren (const hkShape* shape, hkArray<const hkShape*>& childrenOut)
{
	switch (shape->getType())
	{
		case HK_SHAPE_LIST:
			{
				const hkListShape* listshape = static_cast<const hkListShape*> (shape);
				for (int n=0; n<listshape->getNumChildShapes(); n++) 
				{
					const hkShape* listChildShape = listshape->getChildShape(n); 
					childrenOut.pushBack(listChildShape);
				}
				break;
			}

		case HK_SHAPE_TRANSFORM:
			{
				const hkTransformShape* transformShape = static_cast<const hkTransformShape*> (shape);
				childrenOut.pushBack(transformShape->getChildShape());
				break;
			}

		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				const hkConvexTransformShape* convexTransformShape = static_cast<const hkConvexTransformShape*> (shape);
				childrenOut.pushBack(convexTransformShape->getChildShape());
				break;
			}

		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				const hkConvexTranslateShape* convexTranslateShape = static_cast<const hkConvexTranslateShape*> (shape);
				childrenOut.pushBack(convexTranslateShape->getChildShape());
				break;
			}
	}

}

class _ShapeReplacementData
{
	public:

		_ShapeReplacementData (const hkArray<hkRigidBody*>& rigidBodies)
		{
			for (int i=0; i<rigidBodies.getSize(); i++)
			{
				recursivelyParseShapes (rigidBodies[i]->getCollidable()->getShape(),1);
			}
		}

		~_ShapeReplacementData ()
		{
			for (int i=0; i<m_shapeReplacements.getSize(); i++)
			{
				if (m_shapeReplacements[i].m_originalShape)
				{
					m_shapeReplacements[i].m_originalShape->removeReference();
				}
				if (m_shapeReplacements[i].m_replacementShape)
				{
					m_shapeReplacements[i].m_replacementShape->removeReference();
				}
			}
		}

		const hkShape* findReplacement (const hkShape* original) const
		{
			for (int i=0; i<m_shapeReplacements.getSize(); i++)
			{
				if (m_shapeReplacements[i].m_originalShape == original)
				{
					return m_shapeReplacements[i].m_replacementShape;
				}
			}

			return HK_NULL;

		}

		void setReplacement(const hkShape* shape, const hkShape* replacement)
		{
			for (int i=0; i<m_shapeReplacements.getSize(); i++)
			{
				if ( (m_shapeReplacements[i].m_originalShape == shape) ||
					(m_shapeReplacements[i].m_replacementShape == shape) )
				{
					replacement->addReference();

					if (m_shapeReplacements[i].m_replacementShape)
					{
						m_shapeReplacements[i].m_replacementShape->removeReference();
					}

					m_shapeReplacements[i].m_replacementShape = replacement;
				}
			}
		}

		struct ShapeReplacement
		{
			const hkShape* m_originalShape;
			const hkShape* m_replacementShape;
			int m_maxDepth;
		};

		hkArray<ShapeReplacement> m_shapeReplacements;

		int findShapeDataIndex (const hkShape* shape) const
		{
			for (int i=0; i<m_shapeReplacements.getSize(); i++)
			{
				if (m_shapeReplacements[i].m_originalShape == shape)
				{
					return i;
				}
			}
			return -1;
		}


		void recursivelyParseShapes (const hkShape* shape, int currentDepth)
		{
			int index = findShapeDataIndex(shape);

			if (index>=0)
			{
				if (currentDepth > m_shapeReplacements[index].m_maxDepth)
				{
					m_shapeReplacements[index].m_maxDepth=currentDepth;
				}
			}
			else
			{
				ShapeReplacement shapeData;
				shapeData.m_originalShape = shape;
				shapeData.m_replacementShape = HK_NULL;
				shapeData.m_maxDepth = currentDepth;
				m_shapeReplacements.pushBack(shapeData);
				shape->addReference();
			}

			hkArray<const hkShape*> children;
			_getShapeChildren(shape, children);

			for (int i=0; i<children.getSize(); i++)
			{
				recursivelyParseShapes(children[i], currentDepth+1);
			}
		}

};


/*static*/ hkResult hkShapeSharingUtil::shareShapes (hkRigidBody* rigidBody, const Options& options, Results& resultsOut)
{
	hkArray<hkRigidBody*> tempArray;
	tempArray.pushBack(rigidBody);

	return shareShapes(tempArray, options, resultsOut);
}

/*static*/ hkResult hkShapeSharingUtil::shareShapes (hkArray<hkRigidBody*>& rigidBodies, const Options& options, Results& resultsOut)
{
	_ShapeReplacementData shapeReplacementData(rigidBodies);

	resultsOut.reset();

	findIdenticalShapes(options, shapeReplacementData, resultsOut);

	for (int i=0; i<rigidBodies.getSize(); i++)
	{
		hkRigidBody* rb = rigidBodies[i];
		const hkShape* originalShape = rb->getCollidable()->getShape();
		const hkShape* newShape = replaceShapesRecursively(originalShape, shapeReplacementData);

		//remove the previous shape
		originalShape->removeReference();

		rb->getCollidableRw()->setShape((hkShape*)newShape);

	}

	return HK_SUCCESS;
}

/*static*/ const hkShape* hkShapeSharingUtil::replaceShapesRecursively (const hkShape* shape, class _ShapeReplacementData& shapeReplacementData)
{
	const hkShape* replacement = shapeReplacementData.findReplacement(shape);
	if (replacement)
	{
		replacement->addReference();
		return replacement;
	}

	hkArray<const hkShape*> originalChildren;
	_getShapeChildren(shape, originalChildren);

	const int nChildren = originalChildren.getSize();

	if (nChildren==0)
	{
		shape->addReference();
		return shape;
	}

	hkArray<const hkShape*> newChildren;
	bool childrenUnmodified = true;
	for (int i=0; i<nChildren;i++) 
	{
		const hkShape* childShape = originalChildren[i]; 
		const hkShape* newShape = replaceShapesRecursively(childShape, shapeReplacementData);		
		newChildren.pushBack(newShape);

		if (newShape != childShape) childrenUnmodified = false;
	}

	if (childrenUnmodified)
	{
		for (int i=0; i<nChildren; i++) 
		{
			newChildren[i]->removeReference();
		}
		shape->addReference();
		return shape;
	}

	// else - the children have been modified so we need a new shape:

	switch (shape->getType())
	{
		case HK_SHAPE_LIST:
			{
				//otherwise create a new list, and relinquish ownership of the newly created children to the new list
				hkListShape* newListShape = new hkListShape(newChildren.begin(), nChildren);
				for (int n=0; n<nChildren; n++) 
				{
					newChildren[n]->removeReference();		
				}
				
				shapeReplacementData.setReplacement(shape, newListShape);
				return newListShape;
				break;
			}

		case HK_SHAPE_TRANSFORM:
			{
				const hkTransformShape* transformShape = static_cast<const hkTransformShape*> (shape);
				const hkTransformShape* newTransformShape = new hkTransformShape (newChildren[0], transformShape->getTransform());
				
				newChildren[0]->removeReference();
				shapeReplacementData.setReplacement(transformShape, newTransformShape);

				return newTransformShape;
			}					

		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				const hkConvexTransformShape* convexTransformShape = static_cast<const hkConvexTransformShape*> (shape);
				const hkConvexShape* cvxChild = static_cast<const hkConvexShape*> (newChildren[0]);
				const hkConvexTransformShape* newConvexTransformShape = new hkConvexTransformShape (cvxChild, convexTransformShape->getTransform());

				newChildren[0]->removeReference();
				shapeReplacementData.setReplacement(convexTransformShape, newConvexTransformShape);

				return newConvexTransformShape;
			}

		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				const hkConvexTranslateShape* convexTranslateShape = static_cast<const hkConvexTranslateShape*> (shape);
				const hkConvexShape* cvxChild = static_cast<const hkConvexShape*> (newChildren[0]);
				const hkConvexTranslateShape* newConvexTranslateShape = new hkConvexTranslateShape (cvxChild, convexTranslateShape->getTranslation());

				newChildren[0]->removeReference();
				shapeReplacementData.setReplacement(convexTranslateShape, newConvexTranslateShape);

				return newConvexTranslateShape;
			}
	}

	HK_ASSERT(0,0); // We shouldn't get here

	return HK_NULL;
}

static hkTransform _getTransform (const hkShape* transformShape)
{
	hkShapeType transformShapeType = transformShape->getType();
	hkTransform parentFromChild;

	switch ( transformShapeType )
	{
		case HK_SHAPE_TRANSFORM:
			{
				const hkTransformShape* tformshape = static_cast<const hkTransformShape*> (transformShape);
				parentFromChild = tformshape->getTransform();
				break;
			} 
		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				const hkConvexTransformShape* ctformshape = static_cast<const hkConvexTransformShape*> (transformShape);
				parentFromChild = ctformshape->getTransform();
				break;
			}
		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				const hkConvexTranslateShape* ctlateshape = static_cast<const hkConvexTranslateShape*> (transformShape);
				parentFromChild.setIdentity();
				parentFromChild.setTranslation( ctlateshape->getTranslation() );
				break;
			}
		default:
			// Shouldn't get here
			HK_ASSERT(0, "Internal error");
			break;
	}

	return parentFromChild;
}

static bool _areShapesIdentical (const hkShape* shape1, const hkShape* shape2, const _ShapeReplacementData& shapeReplacementData, const hkShapeSharingUtil::Options& options);

static bool _areShapeArraysIdentical (const hkArray<const hkShape*>& array1, const hkArray<const hkShape*>& array2, const _ShapeReplacementData& shapeReplacementData, const hkShapeSharingUtil::Options& options)
{
	if (array1.getSize()!=array2.getSize()) return false;

	hkArray<const hkShape*> copy2; copy2 = array2;

	for (int i=0; i<array1.getSize(); i++)
	{
		for (int j=0; j<copy2.getSize(); j++)
		{
			if (_areShapesIdentical(array1[i], copy2[j], shapeReplacementData, options))
			{
				copy2.removeAt(j);
				break;
			}
		}
	}

	return (copy2.getSize()==0);

}

static bool _floatEqual(const float a, const float b, const hkShapeSharingUtil::Options& options)
{
	return (hkMath::fabs(a-b)<=options.m_equalityThreshold);
}

static bool _vectorEqual(const hkVector4& a, const hkVector4& b, const hkShapeSharingUtil::Options& options)
{
	return (a.equals3(b, options.m_equalityThreshold));
}

static bool _vectorArrayEqual(const hkArray<hkVector4>& arrayA, const hkArray<hkVector4>& arrayB, const hkShapeSharingUtil::Options& options)
{
	if (arrayA.getSize()!=arrayB.getSize()) return false;

	if (options.m_detectPermutedComponents)
	{
		hkArray<hkVector4> copyB; copyB = arrayB;

		for (int i=0; i<arrayA.getSize(); i++)
		{
			// find closest match in B
			int closest = -1; 
			float minDistSqr = HK_REAL_MAX;

			for (int j=0; j<copyB.getSize(); j++)
			{
				hkVector4 diff; diff.setSub4(arrayA[i], copyB[j]);
				const hkReal distSqr = diff.lengthSquared3();
				if (distSqr < minDistSqr)
				{
					closest = j;
					minDistSqr = distSqr;
				}
			}

			if (closest>=0)
			{
				if (_vectorEqual(arrayA[i], copyB[closest], options))
				{
					copyB.removeAt(closest);
				}
			}
		}

		return copyB.getSize()==0;
	}
	else
	{

		for (int i=0; i<arrayA.getSize(); i++)
		{
			if (!_vectorEqual(arrayA[i], arrayB[i], options))
			{
				return false;
			}
		}

		return true;
	}

}

struct _Triangle
{
	hkVector4 m_vertices[3];
	hkUint8 m_material;

	bool equals (const _Triangle& other, const hkShapeSharingUtil::Options& options)
	{
		if (m_material!=other.m_material) return false;

		// Allow for triangle indices to be rotated but with the same winding
		for (int shift=0; shift<3; shift++)
		{
			if ( _vectorEqual(m_vertices[0], other.m_vertices[(0+shift)%3], options) && 
				 _vectorEqual(m_vertices[1], other.m_vertices[(1+shift)%3], options) &&
				 _vectorEqual(m_vertices[2], other.m_vertices[(2+shift)%3], options)
				 )
			{
				return true;
			}
		}

		return false;
	}
};

static void _getMeshTriangles (const hkSimpleMeshShape* meshShape, hkArray<_Triangle>& trianglesOut)
{
	for (int tri=0; tri<meshShape->m_triangles.getSize(); tri++)
	{
		_Triangle triangle;
		triangle.m_material = meshShape->m_materialIndices.getSize()>0 ? meshShape->m_materialIndices[tri] : -1;
		triangle.m_vertices[0] = meshShape->m_vertices[meshShape->m_triangles[tri].m_a];
		triangle.m_vertices[1] = meshShape->m_vertices[meshShape->m_triangles[tri].m_b];
		triangle.m_vertices[2] = meshShape->m_vertices[meshShape->m_triangles[tri].m_c];
		trianglesOut.pushBack(triangle);
	}

}


static bool _areShapesIdentical (const hkShape* shape1, const hkShape* shape2, const _ShapeReplacementData& shapeReplacementData, const hkShapeSharingUtil::Options& options)
{
	// Shapes with different user data shouldn't be shared
	if (shape1->getUserData() != shape2->getUserData())
	{
		return false;
	}

	const hkShape* replacement1 = shapeReplacementData.findReplacement(shape1);
	if (replacement1 ==shape2) 
	{
		return true;
	}
	
	const hkShape* replacement2 = shapeReplacementData.findReplacement(shape2);
	if (replacement2==shape1)
	{
		return true;
	}

	if (replacement1 && (replacement1==replacement2))
	{
		return true;
	}

	// Ignore mopps, go to the collection inside it
	if (shape1->getType() == HK_SHAPE_MOPP)
	{
		const hkMoppBvTreeShape* mopp1 = static_cast<const hkMoppBvTreeShape*> (shape1);
		const hkShapeCollection* actualShape1 = mopp1->getShapeCollection();

		return _areShapesIdentical(actualShape1, shape2, shapeReplacementData, options);
	}

	if (shape2->getType() == HK_SHAPE_MOPP)
	{
		const hkMoppBvTreeShape* mopp2 = static_cast<const hkMoppBvTreeShape*> (shape2);
		const hkShapeCollection* actualShape2 = mopp2->getShapeCollection();

		return _areShapesIdentical(shape1, actualShape2, shapeReplacementData, options);
	}

	hkArray<const hkShape*> children1;
	_getShapeChildren(shape1, children1);
	hkArray<const hkShape*> children2;
	_getShapeChildren(shape2, children2);

	if (! _areShapeArraysIdentical (children1, children2, shapeReplacementData, options) )
	{
		return false;
	}

	switch (shape1->getType())
	{
		case HK_SHAPE_TRANSFORM:
		case HK_SHAPE_CONVEX_TRANSFORM:
		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				switch (shape2->getType())
				{
					case HK_SHAPE_TRANSFORM:
					case HK_SHAPE_CONVEX_TRANSFORM:
					case HK_SHAPE_CONVEX_TRANSLATE:
						{
							hkTransform trans1 = _getTransform(shape1);
							hkTransform trans2 = _getTransform(shape2);

							if (trans1.isApproximatelyEqual(trans2))
							{
								return true;
							}
							else
							{
								return false;
							}
						}			
					default:
						return false;
				}
			}
		case HK_SHAPE_LIST:
			{
				if (shape2->getType()!=HK_SHAPE_LIST) return false;

				// The children are identical so we are identical
				return true; 				
			}

		case HK_SHAPE_SPHERE:
			{
				if (shape2->getType()!=HK_SHAPE_SPHERE) return false;

				const hkSphereShape* sphere1 = static_cast<const hkSphereShape*> (shape1);
				const hkSphereShape* sphere2 = static_cast<const hkSphereShape*> (shape2);

				if (!_floatEqual(sphere1->getRadius(), sphere2->getRadius(), options))
				{
					return false;
				}

				return true;
			}

		case HK_SHAPE_BOX:
			{
				if (shape2->getType()!=HK_SHAPE_BOX) return false;

				const hkBoxShape* box1 = static_cast<const hkBoxShape*> (shape1);
				const hkBoxShape* box2 = static_cast<const hkBoxShape*> (shape2);

				if (!_floatEqual(box1->getRadius(), box2->getRadius(), options))
				{
					return false;
				}

				if (!_vectorEqual(box1->getHalfExtents(), box2->getHalfExtents(), options))
				{
					return false;
				}

				return true;
			}
		case HK_SHAPE_CAPSULE:
			{
				if (shape2->getType()!=HK_SHAPE_CAPSULE) return false;

				const hkCapsuleShape* capsule1 = static_cast<const hkCapsuleShape*> (shape1);
				const hkCapsuleShape* capsule2 = static_cast<const hkCapsuleShape*> (shape2);

				if (!_floatEqual(capsule1->getRadius(), capsule2->getRadius(), options))
				{
					return false;
				}

				if ( _vectorEqual(capsule1->getVertex(0), capsule2->getVertex(0), options) &&
					 _vectorEqual(capsule1->getVertex(1), capsule2->getVertex(1), options) 
					)
				{
					return true;
				}

				if ( _vectorEqual(capsule1->getVertex(0), capsule2->getVertex(1), options) &&
					_vectorEqual(capsule1->getVertex(1), capsule2->getVertex(0), options) 
					)
				{
					return true;
				}

				return false;
			}

		case HK_SHAPE_CYLINDER:
			{
				if (shape2->getType()!=HK_SHAPE_CYLINDER) return false;

				const hkCylinderShape* cylinder1 = static_cast<const hkCylinderShape*> (shape1);
				const hkCylinderShape* cylinder2 = static_cast<const hkCylinderShape*> (shape2);

				if (!_floatEqual(cylinder1->getRadius(), cylinder2->getRadius(), options))
				{
					return false;
				}

				if (!_floatEqual(cylinder1->getCylinderRadius(), cylinder2->getCylinderRadius(), options))
				{
					return false;
				}

				if ( _vectorEqual(cylinder1->getVertex(0), cylinder2->getVertex(0), options) &&
					_vectorEqual(cylinder1->getVertex(1), cylinder2->getVertex(1), options) 
					)
				{
					return true;
				}

				if ( _vectorEqual(cylinder1->getVertex(0), cylinder2->getVertex(1), options) &&
					_vectorEqual(cylinder1->getVertex(1), cylinder2->getVertex(0), options) 
					)
				{
					return true;
				}

				return false;
			}
		case HK_SHAPE_CONVEX_VERTICES:
			{
				if (shape2->getType()!=HK_SHAPE_CONVEX_VERTICES) return false;

				const hkConvexVerticesShape* cvs1 = static_cast<const hkConvexVerticesShape*> (shape1);
				const hkConvexVerticesShape* cvs2 = static_cast<const hkConvexVerticesShape*> (shape2);

				if (!_floatEqual(cvs1->getRadius(), cvs2->getRadius(), options))
				{
					return false;
				}

				hkArray<hkVector4> vertices1; cvs1->getOriginalVertices(vertices1);
				hkArray<hkVector4> vertices2; cvs2->getOriginalVertices(vertices2);

				if (!_vectorArrayEqual(vertices1, vertices2, options))
				{
					return false;
				}

				const hkArray<hkVector4>& planes1 = cvs1->getPlaneEquations();
				const hkArray<hkVector4>& planes2 = cvs2->getPlaneEquations();

				if (!_vectorArrayEqual(planes1, planes2, options))
				{
					return false;
				}

				return true;
			}


		case HK_SHAPE_TRIANGLE_COLLECTION:
			{
				if (shape2->getType()!=HK_SHAPE_TRIANGLE_COLLECTION) return false;

				const hkSimpleMeshShape* meshShape1 = static_cast<const hkSimpleMeshShape*> (shape1);
				const hkSimpleMeshShape* meshShape2 = static_cast<const hkSimpleMeshShape*> (shape2);

				if (!_floatEqual(meshShape1->getRadius(), meshShape2->getRadius(), options))
				{
					return false;
				}

				if (meshShape1->m_vertices.getSize()!=meshShape2->m_vertices.getSize())
				{
					return false;
				}

				if (meshShape1->m_triangles.getSize()!=meshShape2->m_triangles.getSize())
				{
					return false;
				}

				if (meshShape1->m_materialIndices.getSize()!=meshShape2->m_materialIndices.getSize())
				{
					return false;
				}

				if (options.m_detectPermutedComponents)
				{
					// construct array of explicit triangles
					hkArray<_Triangle> triangles1; _getMeshTriangles(meshShape1, triangles1);
					hkArray<_Triangle> triangles2; _getMeshTriangles(meshShape2, triangles2);

					if (triangles1.getSize()!=triangles2.getSize()) return false;

					hkArray<_Triangle> copy2; copy2 = triangles2;

					for (int i=0; i<triangles1.getSize(); i++)
					{
						for (int j=0; j<copy2.getSize(); j++)
						{
							if (triangles1[i].equals(copy2[j], options))
							{
								copy2.removeAt(j);
								break;
							}
						}
					}

					return (copy2.getSize()==0);

				}
				else
				{
					// compare triangle indices
					for (int i=0; i<meshShape1->m_triangles.getSize(); i++)
					{
						const hkSimpleMeshShape::Triangle& tri1 = meshShape1->m_triangles[i];
						const hkSimpleMeshShape::Triangle& tri2 = meshShape2->m_triangles[i];
						if ( (tri1.m_a != tri2.m_a) || (tri1.m_b != tri2.m_b) || (tri1.m_c != tri2.m_c))
						{
							return false;
						}
					}

					// compare vector positions
					if (!_vectorArrayEqual(meshShape1->m_vertices, meshShape2->m_vertices, options))
					{
						return false;
					}

					// compare material indices
					for (int j=0; j<meshShape1->m_materialIndices.getSize(); j++)
					{
						if (meshShape1->m_materialIndices[j]!=meshShape2->m_materialIndices[j])
						{
							return false;
						}
					}

					return true;

				}

			}

	}

	return false;
}

const bool _goesFirst (const _ShapeReplacementData::ShapeReplacement& a, const _ShapeReplacementData::ShapeReplacement& b)
{
	return (a.m_maxDepth>b.m_maxDepth);
}

/*static*/ hkResult hkShapeSharingUtil::findIdenticalShapes (const Options& options, class _ShapeReplacementData& shapeReplacementData, Results& resultsOut)
{
	hkArray<_ShapeReplacementData::ShapeReplacement>& shapeReplacements = shapeReplacementData.m_shapeReplacements;

	// Reorder 
	hkAlgorithm::quickSort(shapeReplacements.begin(), shapeReplacements.getSize(), _goesFirst);

	for (int i=0; i<shapeReplacements.getSize(); i++)
	{
		const hkShape* shape1 = shapeReplacements[i].m_originalShape;

		for (int j=i+1; j<shapeReplacements.getSize(); j++)
		{
			if (shapeReplacements[j].m_replacementShape!=HK_NULL) continue; // already replaced

			const hkShape* shape2 = shapeReplacements[j].m_originalShape;

			if (_areShapesIdentical(shape1, shape2, shapeReplacementData, options))
			{
				shapeReplacementData.setReplacement(shape2, shape1);
				shapeReplacementData.setReplacement(shape1, shape1); // mark shape for no more replacements
				resultsOut.m_numSharedShapes++;
			}
		}
	}

	return HK_SUCCESS;
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
