/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//
// Havok Memory Optimised Partial Polytope Assembler
// This class generates the binary BV code for the VM to execute
//

#ifndef HK_COLLIDE2_MOPP_ASSEMBLER_H
#define HK_COLLIDE2_MOPP_ASSEMBLER_H

class hkMoppAssembler : public hkReferencedObject
{
public:
	//
	// some public classes
	//
	struct hkMoppAssemblerParams
		{
		public:
			hkMoppAssemblerParams() :
											m_relativeFitToleranceOfInternalNodes(0.5f),
											m_absoluteFitToleranceOfInternalNodes(0.2f),
											m_absoluteFitToleranceOfTriangles (1.0f),
											m_groupLevels(4)
											{
												m_absoluteFitToleranceOfAxisAlignedTriangles.set( 0.2f, 0.2f, 0.05f );
												m_interleavedBuildingEnabled = true;
											}
			
			/// the maximum relative size of the unused space
			float m_relativeFitToleranceOfInternalNodes;

			/// the minimum width of a chopped off slice
			float m_absoluteFitToleranceOfInternalNodes;

			/// the tightness of the MOPP on a terminal level.
			/// The Mopp compiler tries to create a virtual proxy AABB node around each terminal
			/// where the distance between this proxy node and the hkReal AABB node is
			/// at most m_absoluteFitToleranceOfTriangles
			float m_absoluteFitToleranceOfTriangles;

			/// the tightness for flat triangles for a given direction
			hkVector4 m_absoluteFitToleranceOfAxisAlignedTriangles;

			/// In order to optimize cache utilizations for the virtual machines
			/// the assembler should organize the tree accordingly:
			/// A node X and all nodes N in the subtree of X with a maximum pathlengths
			/// of m_groupLevels between X and N should be assembled into one continues
			/// piece of memory.
			/// Note: to archieve best performance, the following formula should be true:
			/// (2^m_groupLevels) ~ cacheLineSizeOfCPU
			/// e.g. for PIII  2^5 ~ 32
			int m_groupLevels;

			/// Flag that indicates whether 'interleaved building' is enabled or disabled.
			///
			/// For more information on 'interleaved building' see the respective parameter in hkMoppFitToleranceRequirements.
			hkBool m_interleavedBuildingEnabled;
		};


public:
	HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MOPP);

	hkMoppAssembler(){}

	virtual ~hkMoppAssembler(){}

	/// returns the number of splitting plane directions, the assembler can handle
	virtual int   getNumSplittingPlaneDirections() const = 0;

	/// returns a pointer to a static table to an [] of possible splitting planes 
	/// including a cost for each plane
	virtual const hkMoppSplittingPlaneDirection* getSplittingPlaneDirections() const = 0;

	/// try to assemble a partial tree (the tree might not be complete) 
	/// Once a node is fully assembled, the assembler should call
	/// hkMoppNodeMgr::releaseNode(node) to tell the splitter, that a node can be reused
	/// At least minNodesToAssemble must be assembled
	/// the result of this assemble() call is implementation specific

	// assembles the tree into BV machine code
	virtual void assemble(hkMoppTreeNode* rootNode, class hkMoppNodeMgr* mgr, int minNodesToAssemble) = 0;	

	/// gets the scale information for the tree
	virtual void getScaleInfo( hkMoppTreeNode* rootNode, hkMoppCode::CodeInfo* scaleInfoOut  ) = 0;
};

#endif // HK_COLLIDE2_MOPP_ASSEMBLER_H

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
