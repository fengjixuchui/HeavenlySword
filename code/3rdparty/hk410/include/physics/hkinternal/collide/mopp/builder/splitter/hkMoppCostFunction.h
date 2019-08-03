/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// hkMopp Cost Function definition and implementation

#ifndef HK_COLLIDE2_MOPP_COST_FUNCTION_H
#define HK_COLLIDE2_MOPP_COST_FUNCTION_H




// cost function base class
class hkMoppCostFunction : public hkReferencedObject
{
public:
	//
	// public classes
	//
	struct hkMoppSplitCostParams 
	{
		hkReal    m_weightPrimitiveSplit;
		hkReal    m_weightPlaneDistance;
		hkReal    m_weightNumUnbalancedTriangles;
		hkReal    m_weightPlanePosition;
		hkReal	m_weightPrimitiveIdSpread;
		hkReal	m_queryErrorTolerance;

		hkMoppSplitCostParams(hkMoppMeshType meshType = HK_MOPP_MT_LANDSCAPE);
	};

	struct hkPlaneRightParams 
	{
		// the current depth of the tree
		int		m_currentDepth;	
		// the optimal depth of the tree
		int		m_optimalDepth;	

		hkReal	m_absoluteMin;
		hkReal	m_absoluteMax;
		hkReal    m_extentsInv;  // 1.0 / (max - min )
		int		m_currentNumPrimitivesRight;
		int     m_numPrimitives;
		hkReal	m_numPrimitivesInv; // 1.0 / numPrimitives
		hkReal	m_planeRightPosition;
		const hkMoppSplittingPlaneDirection* m_plane;
		// smallest primitive ID
		hkMoppPrimitiveId m_minPrimitiveId;	
		// highest primitive ID
		hkMoppPrimitiveId m_maxPrimitiveId;	
	};

	struct hkPlanesParams: public hkPlaneRightParams 
	{

		int		m_numSplitPrimitives;
		int		m_currentNumPrimitivesLeft;
		hkReal	m_planeLeftPosition;
	};


public:

	HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MOPP );

	hkMoppCostFunction(const hkMoppSplitCostParams& bp);

	~hkMoppCostFunction(){}

	/// update the current parameter settings
	void updateParams( const hkMoppSplitCostParams& bp );

	/// calculate the costs just for the plane, which limits the right child
	/// (note: all elements of this child are sitting right (= higher extents values) than this plane
	inline hkReal costPlaneRight(const hkPlaneRightParams& planeRightInfo);

	/// calculate the cost just for the plane which limits the left subtree
	inline hkReal extraCostPlaneLeft(const hkPlanesParams& planeLeftInfo);

	/// calculate the cost for the splitting plane direction
	inline hkReal directionCost(const hkMoppCostFunction::hkPlaneRightParams& planeRightInfo);

#ifdef MOPP_DEBUG_COSTS
	/// export the current costs
	inline void debugCosts ( const hkPlanesParams& p, hkMoppBasicNode::hkMoppCostInfo& costInfoOut );
#endif

protected:
	// costUnbalancedPlaneRight
	// returns 0.0 for optimal position
	inline hkReal costUnbalancedPlaneRight(const hkMoppCostFunction::hkPlaneRightParams& );

	inline hkReal costprimitiveIdSpread( const hkMoppCostFunction::hkPlaneRightParams& );

	// costUnbalancedPlaneLeft
	// retruns 0.0 for optimal position
	inline hkReal costUnbalancedPlaneLeft(const hkMoppCostFunction::hkPlanesParams& );

	// costPrimitiveSplits
	// Returns 0.0 if no primitives are split
	// The two splitting planes can have arbitrary positions.
	// If the two splitting planes both split a primitive, it is actually split and the cost is increased.
	inline hkReal costPrimitiveSplits(const hkMoppCostFunction::hkPlanesParams& );

	// costPlaneDistance
	// calculates the cost associated with the relative positions of the two splitting planes to each other
	inline hkReal costPlaneDistance(const hkMoppCostFunction::hkPlanesParams& );

	// costNumUnBalanced
	// Returns 0.0 if there are the same amount of primitives on each side
	inline hkReal costNumUnBalanced(const hkMoppCostFunction::hkPlanesParams& );


	hkMoppSplitCostParams m_userScaling;
};

#include <hkinternal/collide/mopp/builder/splitter/hkMoppCostFunction.inl>
#endif // HK_COLLIDE2_MOPP_COST_FUNCTION_H

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
