/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/collide/hkCollapseTransformsDeprecated.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkmath/linear/hkVector4Util.h>

#include <hkutilities/inertia/hkInertiaTensorComputer.h>

#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>

#include <hkdynamics/entity/hkRigidBody.h>

hkResult hkCollapseTransformsDeprecated::collapseAllTransformShapes(hkRigidBody* rigidBody)
{
	const hkShape* originalShape = rigidBody->getCollidable()->getShape();

	const hkShape* currentShape = originalShape;

	// We always act as if "currentShape" is a new shape (so we always remove a reference at the end)
	currentShape->addReference();

	// We do a while loop in case there is more than one chained transform shape
	while (currentShape->getType() == HK_SHAPE_TRANSFORM)
	{
		const hkTransformShape* tshape = static_cast<const hkTransformShape*> (currentShape);

		const hkShape* newShape = collapseTransformShape(tshape);

		currentShape->removeReference();
		
		if (newShape == currentShape)
		{
			// collapseTransformShape couldn't collapse anymore - leave 
			break;
		}
		currentShape = newShape;
	}

	if (currentShape == originalShape)
	{
		// We haven't done really anything
		currentShape->removeReference();

		return HK_FAILURE;
	}
	else
	{
			// This should just really be : 
			//   rigidBody->setShape(currentShape);
			//   currentShape->removeReference();
			// But you can't call setShape() on an hkRigidBody (yet)
			rigidBody->getCollidable()->getShape()->removeReference();
			rigidBody->getCollidableRw()->setShape( const_cast<hkShape*> (currentShape) );	

			return HK_SUCCESS;
	}

}


const hkShape* hkCollapseTransformsDeprecated::collapseTransformShape(const hkTransformShape* transformShape)
{
	const hkTransform& parentFromChild = transformShape->getTransform();
	const hkShape* childShape = transformShape->getChildShape();
	const hkShape* shape = collapseTransformShape( parentFromChild, childShape );
	if ( shape )
	{
		return shape;
	}
	transformShape->addReference();
	return transformShape;
}

const hkShape* hkCollapseTransformsDeprecated::collapseConvexTranslate(const hkConvexTranslateShape* tls)
{
	hkTransform t; 	t.setIdentity();
	t.setTranslation( tls->getTranslation() );
	const hkShape* childShape = tls->getChildShape();
	const hkShape* shape = collapseTransformShape( t, childShape );
	if ( shape )
	{
		return shape;
	}
	tls->addReference();
	return tls;
}

const hkShape* hkCollapseTransformsDeprecated::collapseTransformShape(const hkTransform& parentFromChild, const hkShape* childShape)
{

	// First case : transform is identity -> return child shape
	if (parentFromChild.isApproximatelyEqual(hkTransform::getIdentity()))
	{
		childShape->addReference();
		return childShape;
	}

	// Otherwise : do different stuff depending on the shape type
	const hkShapeType type = childShape->getType();

	switch (type)
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

				hkCapsuleShape* newCapsule = new hkCapsuleShape(parentA, parentB, radius);

				return newCapsule;

				break;
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

				hkCylinderShape* newCylinder = new hkCylinderShape(parentA, parentB, cylRadius, radius);

				return newCylinder;
			}

		case HK_SHAPE_CONVEX:
		case HK_SHAPE_CONVEX_VERTICES:
		case HK_SHAPE_BOX:
		case HK_SHAPE_SPHERE:
			{
				// Convex shapes : Use hkConvexTranslate or hkConvexTransform shapes
				const hkConvexShape* childConvexShape = static_cast<const hkConvexShape*> (childShape);

				// Is is just translation? (sphere's rotation can safely be ignored)
				const hkBool translateOnly = 
					( type == HK_SHAPE_SPHERE ) 
					|| 
					( parentFromChild.getRotation().isApproximatelyEqual(hkTransform::getIdentity().getRotation()) );

				if (translateOnly )
				{
					const hkConvexTranslateShape* newShape = new hkConvexTranslateShape(childConvexShape, parentFromChild.getTranslation());
					return newShape;
				}
				else
				{
					const hkConvexTransformShape* newShape = new hkConvexTransformShape(childConvexShape, parentFromChild);

					return newShape;
				}

				break;
			}

		case HK_SHAPE_TRANSFORM:
			{
				// Another transform shape : multiply both transforms together
				const hkTransformShape* childTransformShape = static_cast<const hkTransformShape*> (childShape);

				const hkTransform& childFromGrandchild = childTransformShape->getTransform();
				const hkShape* grandchildShape = childTransformShape->getChildShape();

				hkTransform parentFromGrandchild;
				parentFromGrandchild.setMul(parentFromChild, childFromGrandchild);

				hkTransformShape* newTransformShape = new hkTransformShape(grandchildShape, parentFromGrandchild);

				return newTransformShape;
			}

		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				// Another transform shape : multiply both transforms together
				const hkConvexTransformShape* childConvexTransformShape = static_cast<const hkConvexTransformShape*> (childShape);
				const hkTransform& childFromGrandchild = childConvexTransformShape->getTransform();
				const hkConvexShape* grandchildShape = childConvexTransformShape->getChildShape();

				hkTransform parentFromGrandchild;
				parentFromGrandchild.setMul(parentFromChild, childFromGrandchild);

				hkConvexTransformShape* newTransformShape = new hkConvexTransformShape(grandchildShape, parentFromGrandchild);

				return newTransformShape;
			}

		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				// Another transform shape : multiply both transforms together
				const hkConvexTranslateShape* childConvexTranslateShape = static_cast<const hkConvexTranslateShape*> (childShape);

				const hkVector4& childFromGrandchildTranslation = childConvexTranslateShape->getTranslation();
				const hkTransform childFromGrandchild (hkQuaternion::getIdentity(), childFromGrandchildTranslation);
				const hkConvexShape* grandchildShape = childConvexTranslateShape->getChildShape();


				hkTransform parentFromGrandchild;
				parentFromGrandchild.setMul(parentFromChild, childFromGrandchild);

				hkConvexTransformShape* newTransformShape = new hkConvexTransformShape(grandchildShape, parentFromGrandchild);

				return newTransformShape;
			}

		default:
			{
				// We couldn't collapse it
				// So return the original transform shape, adding one reference

				childShape->addReference();

				return childShape;

			}

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
