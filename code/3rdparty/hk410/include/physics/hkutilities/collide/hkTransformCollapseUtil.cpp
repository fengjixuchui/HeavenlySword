/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/collide/hkTransformCollapseUtil.h>

#include <hkbase/memory/hkLocalArray.h>

#include <hkmath/linear/hkVector4Util.h>
#include <hkmath/basetypes/hkGeometry.h>

#include <hkinternal/preprocess/convexhull/hkGeometryUtility.h>

#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>

#include <hkdynamics/entity/hkRigidBody.h>


hkTransformCollapseUtil::Options::Options () : 
	m_sharedShapeBehaviour(ALWAYS_COLLAPSE), 
	m_sharedShapeThreshold(0), 
	m_propageTransformInList(true) 
{

}

hkTransformCollapseUtil::Results::Results() : 
	m_numCollapsedShapes(0), 
	m_numIdentityTransformsRemoved(0), 
	m_numSpecializedTransformShapes(0), 
	m_numPropagatedTransformsToLists(0)
{
	reset();
}

void hkTransformCollapseUtil::Results::reset()
{
	m_numCollapsedShapes = 0;
    m_numIdentityTransformsRemoved = 0;
    m_numSpecializedTransformShapes = 0;
    m_numPropagatedTransformsToLists =0;
}

void hkTransformCollapseUtil::Results::report() const
{
	HK_REPORT("Number of collapsed shapes :"<<m_numCollapsedShapes);
	HK_REPORT("Number of identity transforms removed : "<<m_numIdentityTransformsRemoved);
	HK_REPORT("Number of specialized transform shapes : "<<m_numSpecializedTransformShapes);
	HK_REPORT("Number of propagated transforms to lists : "<<m_numPropagatedTransformsToLists);
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

class _SharedShapeData
{
	public:

		_SharedShapeData (const hkArray<hkRigidBody*>& rigidBodies)
		{
			for (int i=0; i<rigidBodies.getSize(); i++)
			{
				recursivelyParseShapeReferences (rigidBodies[i]->getCollidable()->getShape());
			}

			removeSingleReferencedShapeData();
		}

		~_SharedShapeData ()
		{
			for (int i=0; i<m_sharedShapes.getSize(); i++)
			{
				if (m_sharedShapes[i].m_originalShape)
				{
					m_sharedShapes[i].m_originalShape->removeReference();
				}
				if (m_sharedShapes[i].m_replacementShape)
				{
					m_sharedShapes[i].m_replacementShape->removeReference();
				}
			}
		}

		const hkShape* findReplacement (const hkShape* original) const
		{
			for (int i=0; i<m_sharedShapes.getSize(); i++)
			{
				if (m_sharedShapes[i].m_originalShape == original)
				{
					return m_sharedShapes[i].m_replacementShape;
				}
			}

			return HK_NULL;

		}

		const int numberOfReferences (const hkShape* shape) const
		{
			for (int i=0; i<m_sharedShapes.getSize(); i++)
			{
				if ( (m_sharedShapes[i].m_originalShape == shape) ||
					 (m_sharedShapes[i].m_replacementShape == shape) )
				{
					return m_sharedShapes[i].m_numReferences;
				}
			}

			return 0;
		}

		hkResult setReplacement(const hkShape* shape, const hkShape* replacement)
		{
			for (int i=0; i<m_sharedShapes.getSize(); i++)
			{
				if ( (m_sharedShapes[i].m_originalShape == shape) ||
					(m_sharedShapes[i].m_replacementShape == shape) )
				{
					replacement->addReference();

					if (m_sharedShapes[i].m_replacementShape)
					{
						m_sharedShapes[i].m_replacementShape->removeReference();
					}

					m_sharedShapes[i].m_replacementShape = replacement;
					return HK_SUCCESS;
				}
			}

			return HK_FAILURE;
		}

	private:

		struct SharedShape
		{
			const hkShape* m_originalShape;
			const hkShape* m_replacementShape;
			int m_numReferences;
		};

		hkArray<SharedShape> m_sharedShapes;

		int findShapeDataIndex (const hkShape* shape) const
		{
			for (int i=0; i<m_sharedShapes.getSize(); i++)
			{
				if (m_sharedShapes[i].m_originalShape == shape)
				{
					return i;
				}
			}
			return -1;
		}


		void recursivelyParseShapeReferences (const hkShape* shape)
		{
			int index = findShapeDataIndex(shape);

			if (index>=0)
			{
				m_sharedShapes[index].m_numReferences++;
				// we don't recurse once we find a shared shape as somebody else already has
				return;
			}
			else
			{
				SharedShape sharedShape;
				sharedShape.m_originalShape = shape;
				sharedShape.m_replacementShape = HK_NULL;
				sharedShape.m_numReferences = 1;
				m_sharedShapes.pushBack(sharedShape);
				shape->addReference();
			}

			hkArray<const hkShape*> children;
			_getShapeChildren(shape, children);

			for (int i=0; i<children.getSize(); i++)
			{
				recursivelyParseShapeReferences(children[i]);
			}
		}

		void removeSingleReferencedShapeData()
		{
			for (int i=m_sharedShapes.getSize()-1; i>=0; --i)
			{
				if (m_sharedShapes[i].m_numReferences<2)
				{
					if (m_sharedShapes[i].m_originalShape)
					{
						m_sharedShapes[i].m_originalShape->removeReference();
					}
					if (m_sharedShapes[i].m_replacementShape)
					{
						m_sharedShapes[i].m_replacementShape->removeReference();
					}
					m_sharedShapes.removeAt(i);
				}
			}
		}


};


/*static*/ hkResult hkTransformCollapseUtil::collapseTransforms (hkRigidBody* rigidBody, const Options& options, Results& resultsOut)
{
	hkArray<hkRigidBody*> tempArray;
	tempArray.pushBack(rigidBody);

	return collapseTransforms(tempArray, options, resultsOut);
}


/*static*/ hkResult hkTransformCollapseUtil::collapseTransforms (hkArray<hkRigidBody*>& rigidBodies, const Options& options, Results& resultsOut)
{
	resultsOut.reset();

	_SharedShapeData sharedShapeData (rigidBodies);

	for (int i=0; i<rigidBodies.getSize(); i++)
	{
		hkRigidBody* rb = rigidBodies[i];
		const hkShape* originalShape = rb->getCollidable()->getShape();
		const hkShape* newShape = collapseShapesRecursively (originalShape, options, sharedShapeData, resultsOut);

		//remove the previous shape
		originalShape->removeReference();

		rb->getCollidableRw()->setShape((hkShape*)newShape);

	}

	return HK_SUCCESS;
}

/*static*/ const hkShape* hkTransformCollapseUtil::collapseShapesRecursively (const hkShape* shape, const Options& options, class _SharedShapeData& sharedShapeData, Results& resultsOut)
{
	const hkShape* replacement = sharedShapeData.findReplacement(shape);
	if (replacement)
	{
		replacement->addReference();
		return replacement;
	}

	// else

	switch (shape->getType())
	{
		case HK_SHAPE_LIST:
			{
				// TODO : handle single child list shapes

				const hkListShape* listShape = static_cast<const hkListShape*> (shape);
				int nChildren = listShape->getNumChildShapes();

				hkArray<const hkShape*> newShapes;

				bool childrenUnmodified = true;
				for (int n=0; n<nChildren; n++) 
				{
					const hkShape* childShape = listShape->getChildShape(n); 
					const hkShape* newShape = collapseShapesRecursively(childShape, options, sharedShapeData, resultsOut);		
					newShapes.pushBack(newShape);

					if (newShape != childShape) childrenUnmodified = false;
				}

				//if none of the children were modified, remove a reference from each child shape
				//and return the original list plus a reference
				if (childrenUnmodified) 
				{
					for (int n=0; n<nChildren; n++) 
					{
						newShapes[n]->removeReference();
					}
					listShape->addReference();
					return listShape;
				}

				//otherwise create a new list, and relinquish ownership of the newly created children to the new list
				hkListShape* newListShape = new hkListShape(newShapes.begin(), nChildren);
				for (int n=0; n<nChildren; n++) 
				{
					newShapes[n]->removeReference();		
				}
				
				sharedShapeData.setReplacement(listShape, newListShape);

				replacement = newListShape;
				break;
			}

		case HK_SHAPE_TRANSFORM:
			{
				const hkTransformShape* transformShape = static_cast<const hkTransformShape*> (shape);
				const hkShape* originalChild = transformShape->getChildShape();
				const hkShape* newChild = collapseShapesRecursively(originalChild, options, sharedShapeData, resultsOut);

				const hkTransformShape* newTransformShape;
				{
					if (originalChild==newChild)
					{
						newChild->removeReference();
						newTransformShape = transformShape;
						transformShape->addReference();
					}
					else
					{
						newTransformShape = new hkTransformShape (newChild, transformShape->getTransform());
						newChild->removeReference();
						sharedShapeData.setReplacement(transformShape, newTransformShape);
					}					
				}

				// The child has been collapsed, we now need to possibly collapse this guy into the child
				const hkShape* collapsedShape = collapseTransformShape (newTransformShape, options, sharedShapeData, resultsOut);
				newTransformShape->removeReference();

				replacement = collapsedShape;
				break;
			}

		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				const hkConvexTransformShape* convexTransformShape = static_cast<const hkConvexTransformShape*> (shape);
				const hkConvexShape* originalChild = convexTransformShape->getChildShape();
				const hkConvexShape* newChild = static_cast<const hkConvexShape*> (collapseShapesRecursively(originalChild, options, sharedShapeData, resultsOut));

				const hkConvexTransformShape* newConvexTransformShape;
				{
					if (originalChild==newChild)
					{
						newChild->removeReference();
						newConvexTransformShape = convexTransformShape;
						convexTransformShape->addReference();
					}
					else
					{
						newConvexTransformShape = new hkConvexTransformShape (newChild, convexTransformShape->getTransform());
						newChild->removeReference();
						sharedShapeData.setReplacement(convexTransformShape, newConvexTransformShape);
					}					
				}

				// The child has been collapsed, we now need to possibly collapse this guy into the child
				const hkShape* collapsedShape = collapseTransformShape (newConvexTransformShape, options, sharedShapeData, resultsOut);
				newConvexTransformShape->removeReference();
				
				replacement = collapsedShape;
				break;
			}

		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				const hkConvexTranslateShape* convexTranslateShape = static_cast<const hkConvexTranslateShape*> (shape);
				const hkConvexShape* originalChild = convexTranslateShape->getChildShape();
				const hkConvexShape* newChild = static_cast<const hkConvexShape*> (collapseShapesRecursively(originalChild, options, sharedShapeData, resultsOut));

				const hkConvexTranslateShape* newConvexTranslateShape;
				{
					if (originalChild==newChild)
					{
						newChild->removeReference();
						newConvexTranslateShape = convexTranslateShape;
						convexTranslateShape->addReference();
					}
					else
					{
						newConvexTranslateShape = new hkConvexTranslateShape (newChild, convexTranslateShape->getTranslation());
						newChild->removeReference();
						sharedShapeData.setReplacement(convexTranslateShape, newConvexTranslateShape);
					}					
				}

				// The child has been collapsed, we now need to possibly collapse this guy into the child
				const hkShape* collapsedShape = collapseTransformShape (newConvexTranslateShape, options, sharedShapeData, resultsOut);
				newConvexTranslateShape->removeReference();

				replacement = collapsedShape;
				break;
			}

		default:
			{
				// Nothing to do, pretend we created a new shape
				shape->addReference();
				replacement = shape;
			}
	}
	return replacement;
}

/*static*/ const hkShape* hkTransformCollapseUtil::collapseTransformShape (const hkShape* transformShape, const Options& options, class _SharedShapeData& sharedShapeData, Results& resultsOut)
{
	hkShapeType transformShapeType = transformShape->getType();
	hkTransform parentFromChild;
	
	const hkShape* childShape = HK_NULL;

	switch ( transformShapeType )
	{
		case HK_SHAPE_TRANSFORM:
			{
				const hkTransformShape* tformshape = static_cast<const hkTransformShape*> (transformShape);
				parentFromChild = tformshape->getTransform();
				childShape = tformshape->getChildShape();
				break;
			} 
		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				const hkConvexTransformShape* ctformshape = static_cast<const hkConvexTransformShape*> (transformShape);
				parentFromChild = ctformshape->getTransform();
				childShape = ctformshape->getChildShape();
				break;
			}
		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				const hkConvexTranslateShape* ctlateshape = static_cast<const hkConvexTranslateShape*> (transformShape);
				parentFromChild.setIdentity();
				parentFromChild.setTranslation( ctlateshape->getTranslation() );
				childShape = ctlateshape->getChildShape();
				break;
			}
		default:
			// Shouldn't get here
			HK_ASSERT(0, "Internal error");
			break;
	}

	// Otherwise : do different stuff depending on the shape type
	const hkShapeType childShapeType = childShape->getType();

	// If the child is a sphere, ignore any rotations
	if (childShapeType == HK_SHAPE_SPHERE)
	{
		parentFromChild.setRotation(hkTransform::getIdentity().getRotation());
	}

	// First case : transform is identity -> return child shape
	if (parentFromChild.isApproximatelyEqual(hkTransform::getIdentity()))
	{
		resultsOut.m_numIdentityTransformsRemoved++;
		childShape->addReference();
		sharedShapeData.setReplacement(transformShape, childShape);
		return childShape;
	}

	// Possibly do not collapse
	bool allowChangesToChildShape = true;
	{
		switch (options.m_sharedShapeBehaviour)
		{
			case ALWAYS_COLLAPSE:
				allowChangesToChildShape = true;
				break;
			case NEVER_COLLAPSE:
				allowChangesToChildShape = false;
				break;
			case COLLAPSE_IF_LESS_THAN_THRESHOLD:
				allowChangesToChildShape = sharedShapeData.numberOfReferences(childShape)<options.m_sharedShapeThreshold;
				break;
		}
	}

	// Handle list shape case (T->L->x,y,z..)
	if (childShapeType==HK_SHAPE_LIST)
	{
		const hkListShape* listShape = static_cast<const hkListShape*> (childShape);
		if (!options.m_propageTransformInList && allowChangesToChildShape)
		{
			resultsOut.m_numPropagatedTransformsToLists++;

			// Create L'->(T1->x),(T2->y),(T3->z)
			hkArray<const hkShape*> newChildren;
			for (int i=0; i<listShape->getNumChildShapes(); i++)
			{
				const hkShape* grandChildShape = listShape->getChildShape(i);
				hkTransformShape* childTransformShape = new hkTransformShape (grandChildShape, parentFromChild);
				const hkShape* collapsedGrandChild = collapseTransformShape(childTransformShape, options, sharedShapeData, resultsOut);
				newChildren.pushBack(collapsedGrandChild);
				childTransformShape->removeReference();				
			}

			const hkListShape* newListShape = new hkListShape(newChildren.begin(), newChildren.getSize());
			for (int n=0; n<newChildren.getSize(); n++) 
			{
				newChildren[n]->removeReference();		
			}

			sharedShapeData.setReplacement(transformShape, newListShape);
			return newListShape;
		}
		else
		{
			transformShape->addReference();
			return transformShape;
		}
	}

	// Possibly, collapse into the child shape
	if (allowChangesToChildShape)
	{
		switch (childShapeType)
		{
			case HK_SHAPE_CAPSULE:
				{
					// Capsule : Transform the vertices of the capsule (same as cylinder)
					const hkCapsuleShape* childCapsuleShape = static_cast<const hkCapsuleShape*> (childShape);

					const hkVector4& childA = childCapsuleShape->getVertices()[0];
					const hkVector4& childB = childCapsuleShape->getVertices()[1];
					const hkReal radius = childCapsuleShape->getRadius();

					hkVector4 parentA;
					parentA.setTransformedPos ( parentFromChild, childA );
					hkVector4 parentB;
					parentB.setTransformedPos ( parentFromChild, childB	);

					const hkCapsuleShape* newCapsule = new hkCapsuleShape(parentA, parentB, radius);

					sharedShapeData.setReplacement(transformShape, newCapsule);

					resultsOut.m_numCollapsedShapes++;
					return newCapsule;
				}

			case HK_SHAPE_CYLINDER:
				{
					// Cylinder : Transform both vertices (same as capsule)
					const hkCylinderShape* childCylinderShape = static_cast<const hkCylinderShape*> (childShape);

					const hkVector4& childA = childCylinderShape->getVertices()[0];
					const hkVector4& childB = childCylinderShape->getVertices()[1];
					const hkReal cylRadius = childCylinderShape->getCylinderRadius();
					const hkReal radius = childCylinderShape->getRadius();

					hkVector4 parentA; 
					parentA.setTransformedPos ( parentFromChild, childA );
					hkVector4 parentB;
					parentB.setTransformedPos ( parentFromChild, childB	);

					const hkCylinderShape* newCylinder = new hkCylinderShape(parentA, parentB, cylRadius, radius);

					sharedShapeData.setReplacement(transformShape, newCylinder);

					resultsOut.m_numCollapsedShapes++;
					return newCylinder;
				}

			case HK_SHAPE_CONVEX_VERTICES:
				{
					// Convex Vertices : Transform the vertices and planes
					const hkConvexVerticesShape* childCVShape = static_cast<const hkConvexVerticesShape*> (childShape);

					//transform vertices
					hkArray<hkVector4> vertices;
					childCVShape->getOriginalVertices(vertices);
					hkVector4Util::transformPoints(parentFromChild, vertices.begin(), vertices.getSize(), vertices.begin());
					hkStridedVertices newverts;
					newverts.m_numVertices = vertices.getSize();
					newverts.m_striding = sizeof(hkVector4);
					newverts.m_vertices = &(vertices[0](0));

					//transform plane equations
					const hkArray<hkVector4>& planes = childCVShape->getPlaneEquations();
					hkArray<hkVector4> newplanes; newplanes.setSize(planes.getSize());
					hkVector4 pivotShift = parentFromChild.getTranslation();
					for (int p = 0; p < planes.getSize(); p++)
					{
						hkVector4 plane(planes[p]);
						hkReal newDist = plane(3) - hkReal(pivotShift.dot3( plane ));
						plane.setRotatedDir(parentFromChild.getRotation(), plane);
						plane(3) = newDist;
						newplanes[p] = plane;
					}

					const hkConvexVerticesShape* newCVShape = new hkConvexVerticesShape(newverts, newplanes);

					sharedShapeData.setReplacement(transformShape, newCVShape);

					resultsOut.m_numCollapsedShapes++;
					return newCVShape;
				}

			case HK_SHAPE_BOX:
				{
					// Box : if transform consists of 90 degree rotations (plus any translation),
					//       collapse by returning a box with modified half-extents, 
					//       wrapped in an hkConvexTranslateShape if the translation is non-zero.
	
					hkReal tol(1.0e-4f), sum;
					int map[3];
					bool failedA, failedB;
					bool testA0, testA1, passedA1; 
					
					//check if the rotation matrix is valid first
					if ( !(parentFromChild.getRotation().isOrthonormal() && parentFromChild.getRotation().isOk()) )
					{
						break;
					}

					//check if transform is just a translation, in which case the extents won't be modified
					if ( parentFromChild.getRotation().isApproximatelyEqual(hkTransform::getIdentity().getRotation()) )
					{
						break;
					}

					// If the rotated coordinate system lines up with the axes, we can replace the transform with modified box extents
					// (otherwise just proceed below to try to replace transform type with convex transform or translate).
					
					failedA = false;
					failedB = false;

					for (int i=0; i<3; i++)
					{
						//columns of the rotation matrix are the transformed basis vectors
						hkVector4& axisRot = parentFromChild.getRotation().getColumn(i);

						//rotation takes box to box iff a) && b), where:

						// a) each element of each column has magnitude zero or one, within tolerance
						passedA1 = false;
						
						for (int j=0; j<3; j++)
						{ 
							//to pass, at least one of these must be true
							testA0 = hkMath::fabs( axisRot(j) ) < tol;
							testA1 = hkMath::fabs( hkMath::fabs(axisRot(j))-1.0f ) < tol; 
							
							if (!(testA0 || testA1)) 
					 		{
								failedA = true;
								break;
							}

							if (testA1) 
							{
								//jth element is 1, thus the new j-axis corresponds to the old i-axis 
								map[j] = i;
								passedA1 = true;
							}
						}
						if (!passedA1) failedA = true; //one of the elements must be 1.0, within tolerance
							
						if (failedA) break; 
						
						// b) the sum of the magnitudes of the elements in each column is one, within tolerance 
						sum = 0.0f;
						for (int j=0; j<3; j++)
						{
							sum += hkMath::fabs(axisRot(j));
						}
						failedB = !( sum-1.0f < tol );
						
						if (failedB) break;
					}
		
					if (failedA || failedB) break; //proceed to switch below

					//transform is of right type, now modify extents
					const hkBoxShape* childBoxShape = static_cast<const hkBoxShape*> (childShape);
					const hkVector4& halfExtents = childBoxShape->getHalfExtents();
					hkVector4 newHalfExtents( halfExtents(map[0]), halfExtents(map[1]), halfExtents(map[2]) );
					
					const hkBoxShape* newBoxShape = new hkBoxShape( newHalfExtents );

					// if translation is zero, just return the new box shape
					if ( parentFromChild.getTranslation().length3() < tol )
					{						
						sharedShapeData.setReplacement(transformShape, newBoxShape);
						resultsOut.m_numCollapsedShapes++;
						return newBoxShape;
					}

					// otherwise, wrap new box in an hkConvexTranslateShape
					const hkConvexTranslateShape* newShape = new hkConvexTranslateShape(newBoxShape, parentFromChild.getTranslation());
					newBoxShape->removeReference();
					sharedShapeData.setReplacement(transformShape, newShape);
					resultsOut.m_numSpecializedTransformShapes++;
					return newShape;
					break;
				}

		}
	}

	// else (do not collapse, but possibly replace transform type)
	switch (childShapeType)
	{
		case HK_SHAPE_CAPSULE:
		case HK_SHAPE_CYLINDER:
		case HK_SHAPE_CONVEX_VERTICES:
		case HK_SHAPE_CONVEX:
		case HK_SHAPE_SPHERE:
		case HK_SHAPE_BOX:
			{
				// Convex shapes : Use hkConvexTranslate or hkConvexTransform shapes
				const hkConvexShape* childConvexShape = static_cast<const hkConvexShape*> (childShape);

				// Is is just translation? (sphere's rotation can safely be ignored)
				const hkBool translateOnly = parentFromChild.getRotation().isApproximatelyEqual(hkTransform::getIdentity().getRotation());

				if (translateOnly && (transformShapeType != HK_SHAPE_CONVEX_TRANSLATE))
				{
					const hkConvexTranslateShape* newShape = new hkConvexTranslateShape(childConvexShape, parentFromChild.getTranslation());
					
					sharedShapeData.setReplacement(transformShape, newShape);

					resultsOut.m_numSpecializedTransformShapes++;
					return newShape;
				}
				else if (transformShapeType == HK_SHAPE_TRANSFORM)
				{
					const hkConvexTransformShape* newShape = new hkConvexTransformShape(childConvexShape, parentFromChild);

					sharedShapeData.setReplacement(transformShape, newShape);

					resultsOut.m_numSpecializedTransformShapes++;
					return newShape;
				}

				break;
			}
	}

	// We couldn't collapse anything, return the same shape
	transformShape->addReference();
	return transformShape;
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
