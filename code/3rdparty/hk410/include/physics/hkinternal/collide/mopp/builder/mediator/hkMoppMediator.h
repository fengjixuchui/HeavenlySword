/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// PrimitiveMediator definition and implementation

#ifndef HK_COLLIDE2_MOPP_MEDIATOR_H
#define HK_COLLIDE2_MOPP_MEDIATOR_H

/// the Mediator is the interface to set of convex objects
/// the mediator can use two integer ids to identify
///	each primitive
/// the main job of the mediator is to project a set of primitives
/// onto a straight and return the max and min value of this
/// projection
class hkMoppMediator : public hkReferencedObject
{
public:
	virtual ~hkMoppMediator(){;}

	/// tell mediator about the splitting planes
	virtual void setSplittingPlaneDirections(const hkMoppSplittingPlaneDirection* directions, int numDirections) = 0;

	/// returns the total number of primitives handles by the mediator
	virtual int  getNumPrimitives() = 0;

	/// sets the primitiveId and primitiveIi2 in the out array
	/// Each primitive can be represented by two integer numbers
	virtual void getPrimitives(hkMoppCompilerPrimitive *primitiveArrayOut) = 0;

	/// fill the m_extent members in the primitiveArray
	virtual void projectPrimitives(const hkVector4 &direction, int directionIndex, hkMoppCompilerPrimitive *primitiveArray, int numPrimitives, hkReal *absMinOut, hkReal *absMaxOut) = 0;

	/// only get the min max values
	virtual void findExtents(const hkVector4 &direction, int directionIndex, const hkMoppCompilerPrimitive *primitiveArray, int numPrimitives, hkReal *absMinOut, hkReal *absMaxOut) = 0;

	/// split a primitive and store the result into *primitiveOut
	virtual void splitPrimitive( const hkMoppCompilerPrimitive &primtiveIn, const hkVector4 &direction, hkReal planeOffset, int treeDepth, hkMoppCompilerPrimitive *primitiveOut ) = 0;

	///  get additional properties connected to the primitive
	///  returns the number of properties
	///  Note: the maximum number of properties is hkMoppCode::MAX_PRIMITIVE_PROPERTIES
	virtual int getPrimitiveProperties( const hkMoppCompilerPrimitive &primitiveIn, hkPrimitiveProperty propertiesOut[hkMoppCode::MAX_PRIMITIVE_PROPERTIES]) = 0;
};

#endif // HK_COLLIDE2_MOPP_MEDIATOR_H

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
