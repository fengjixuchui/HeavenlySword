/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// hkMoppDefaultSplitter definition

#ifndef HK_COLLIDE2_MOPP_SPLITTER_H
#define HK_COLLIDE2_MOPP_SPLITTER_H

//
// Havok Memory Optimised Partial Polytope Tree
//

// forward definition
class hkMoppSplitParams; 


class hkMoppNodeMgr : public hkReferencedObject
{
public:
	virtual void releaseNode( class hkMoppTreeNode *nodeToRelease ) = 0;
	virtual int getFreeNodes() = 0;
};


class hkMoppSplitter: public hkMoppNodeMgr
{
public:
	//
	// some public classes
	//

	struct hkMoppScratchArea 
	{
		hkMoppCompilerPrimitive* m_primitives;
		hkMoppTreeInternalNode*				m_nodes;
		hkMoppTreeTerminal*	m_terminals;
	};

public:
	//
	// some public classes
	//
 
	/// parameters to the Mopp compile call
	struct hkMoppSplitParams
	{
		// set the essential parameters and initialize the rest with reasonable default values
		hkMoppSplitParams( hkMoppMeshType meshType = HK_MOPP_MT_LANDSCAPE );

		// the maximum error we allow the system to operate
		hkReal m_tolerance;				

		int	m_maxPrimitiveSplits;			// maximum number of split primitives in tree
		int	m_maxPrimitiveSplitsPerNode;	// maximum number of split primitives per node
		int	m_minRangeMaxListCheck;			// minimum number of elements which is checked in the max list 
		int	m_checkAllEveryN;				// all elements in the max list will be checked every N iterations

		// Flag that indicates whether 'interleaved building' is enabled or disabled.
		// For more information on 'interleaved building' see the respective parameter in hkMoppFitToleranceRequirements.
		hkBool m_interleavedBuildingEnabled;
	};


public:
	
	hkMoppSplitter() {}

	virtual	~hkMoppSplitter() {}
	
	virtual hkMoppTreeNode* buildTree(hkMoppMediator*, hkMoppCostFunction*, class hkMoppAssembler*, const hkMoppSplitParams&, hkMoppScratchArea&) = 0;
		// recursively build the tree
};


#endif // HK_COLLIDE2_MOPP_SPLITTER_H

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
