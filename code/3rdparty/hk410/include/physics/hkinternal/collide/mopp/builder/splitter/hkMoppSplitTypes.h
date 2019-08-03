/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//
// Havok Memory Optimised Partial Polytope Binary Tree Node
// Each node represents a splitting plane of the original space into two half-spaces
// The left branch will always represent the half-space in front of the splitting plane
// (i.e. the plane normal is pointing INTO the half-space).
// Similarily, the right branch will always represent the half-space behind the splitting
// plane (i.e. the plane normal is pointing AWAY from the half-space).
// Note: If the pointer to the Mopp primitive is non-NULL, the branch represents a leaf, i.e.
//		 a terminal branch.
//

#ifndef HK_COLLIDE2_MOPP_SPLIT_TYPES_H
#define HK_COLLIDE2_MOPP_SPLIT_TYPES_H

enum hkMoppMeshType
{
	HK_MOPP_MT_LANDSCAPE,
	HK_MOPP_MT_INDOOR
};

typedef hkUint32 hkMoppPrimitiveId;
	//: an id, representing a primitive

//
// structure to hold a single extent of an object
//
struct hkMoppExtent 
{
	hkReal m_min;
	hkReal m_max;
};

struct hkMoppCompilerPrimitive 
{
	hkMoppPrimitiveId		        m_primitiveID;		// ID
	hkMoppPrimitiveId				m_primitiveID2;		// an extra primitive id which can be used for Mediator implementations
	hkMoppExtent					m_extent;			// set by the project functions, varys depending on direction
	inline hkBool operator <(const hkMoppCompilerPrimitive& b) const { return this->m_extent.m_min < b.m_extent.m_min; }
};

//
// structure to hold a splitting plane and its associated cost
//
struct hkMoppSplittingPlaneDirection 
{
	hkVector4	m_direction;
	hkReal		m_cost;
	int			m_index;
};


#define MOPP_DEBUG_COSTS


class hkMoppTreeNode
{
public:
	// 
	// some public classes
	//
	struct hkMopp3DOPExtents 
	{
		hkMoppExtent m_extent[3];
	};

	enum 
	{
		HK_MOPP_ASSEMBLER_DATA_SIZE = 32
	};

public:
	class hkMoppTreeInternalNode*		m_parent;
	hkBool					m_isTerminal;
	int						m_numPrimitives;			// the number of terminals in this subtree

	hkMopp3DOPExtents		m_extents;					// the extent of the space for this node

	hkMoppPrimitiveId	    m_minPrimitiveId;			// pointer to the primitive with the smallest ID for that node
	hkMoppPrimitiveId	    m_maxPrimitiveId;			// pointer to the primitive with the largest ID for that node

	// the number of properties in use
	int						m_numProperties;									
	// like the primitive IDs - these user IDs need to be re-offset
	hkPrimitiveProperty		m_minPropertyValue[hkMoppCode::MAX_PRIMITIVE_PROPERTIES];	
	hkPrimitiveProperty		m_maxPropertyValue[hkMoppCode::MAX_PRIMITIVE_PROPERTIES];

	// data which can be used by the assembler
	hkUint32				m_assemblerData[HK_MOPP_ASSEMBLER_DATA_SIZE];		

	/// safe cast of this node to hkMoppTreeInternalNode
	inline class hkMoppTreeInternalNode* toNode();

	/// safe cast of this node to hkMoppTree
	inline class hkMoppTreeTerminal* toTerminal();

	inline void init();
};

class hkMoppTreeInternalNode* hkMoppTreeNode::toNode()
{
	HK_ASSERT(0x72b4d18a,  !m_isTerminal );
	return reinterpret_cast<hkMoppTreeInternalNode*>(this);
}


class hkMoppTreeTerminal* hkMoppTreeNode::toTerminal()
{
	HK_ASSERT(0x3f08f90a,  m_isTerminal );
	return reinterpret_cast<hkMoppTreeTerminal*>(this);
}

void hkMoppTreeNode::init()
{
	for (int i =0; i < HK_MOPP_ASSEMBLER_DATA_SIZE; i++)
	{
		m_assemblerData[i] = 0;
	}
}





class hkMoppTreeTerminal: public hkMoppTreeNode
{
public:
	// pointer to the primitive list on left if bottom of the tree, otherwise NULL
	hkMoppCompilerPrimitive* m_primitive;	
};


class hkMoppBasicNode : public hkMoppTreeNode 
{
public:
	//
	//	public classes
	//
	struct hkMoppCostInfo 
	{
		hkReal	m_splitCost;			 
		hkReal	m_planeRightPositionCost;
		hkReal    m_primitiveIdSpread;
		hkReal	m_planeLeftPositionCost;	
		hkReal	m_numUnBalancedCost;	 
		hkReal	m_planeDistanceCost;
		hkReal	m_absoluteMin;
		hkReal	m_absoluteMax;
		hkReal	m_directionCost;
	};
	
public:
	const hkMoppSplittingPlaneDirection*	m_plane;
	hkReal									m_planeRightPosition;	
	hkReal									m_planeLeftPosition;	
	hkReal									m_bestOverallCost;			// stores the overal best cost

#ifdef MOPP_DEBUG_COSTS
	void printDebugInfo();
	hkMoppCostInfo			m_costInfo;
#endif
	const hkVector4& getPlaneNormal(){ return m_plane->m_direction; }
};

class hkMoppTreeInternalNode : public hkMoppBasicNode 
{

public:
	hkMoppTreeNode*			m_leftBranch;		// half space left of the  splitting plane
	hkMoppTreeNode*			m_rightBranch;		// half space right of the splitting plane
	int						m_numPrimitives;
};

#endif // HK_COLLIDE2_MOPP_SPLIT_TYPES_H

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
