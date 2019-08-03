/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
//
//

#ifndef HK_COLLIDE2_MOPP_MACHINE_H
#define HK_COLLIDE2_MOPP_MACHINE_H

#include <hkbase/hkBase.h>
#include <hkvisualize/shape/hkDisplayGeometry.h>
#include <hkvisualize/type/hkColor.h>
#include <hkinternal/collide/mopp/machine/hk26Dop.h>

struct hkMoppPlanesQueryInput
{
	public:
		enum { HK_MAX_NUM_PLANES = 32 };
	
			/// The number of planes, a maximum of HK_MAX_NUM_PLANES
		int m_numPlanes;

			/// The planes. The distance to the plane is calculated using:<br>
			/// dist = m_planes[x].dot3( position ) + m_planes[x](3)<br>
			/// The planes are pointing away from the viewing frustum (they define a convex object)
			/// so they have the same direction as the planes in the hkConvexVerticesShape
		const hkVector4 *m_planes;
};

/// Output object for hkMoppKDopGeometriesVirtualMachine.  One of these is
/// created for each KDop that is found, according to the hkMoppKDopQuery.
struct hkMoppInfo
{
	/// The 26-plane bounded shape at each node of the mopp
	hk26Dop m_dop;

	/// Shapekey of a terminal, if m_isTerminal is true
	hkShapeKey m_shapeKey;

	/// The level this hkMoppInfo represents
	hkInt8 m_level;

	/// Specifies whether this info represents a terminal.  If it doesn't,
	/// it represents an intermediate 
	hkBool m_isTerminal;
};


/// Query object for hkMoppKDopGeometriesVirtualMachine
struct hkMoppKDopQuery
{
	/// Set true to exit after the first hit
	hkBool	m_earlyExit;

	/// Depth of display kdops to display (-1 to just display nodes, 0 to display all)
	int	m_kdopDepth;

	/// Set true to only save kdops that lead to the specified ID.
	hkBool	m_useSpecifiedID;
	unsigned int m_specifiedId;

	hkMoppKDopQuery()
	{
		m_earlyExit = false;

		m_kdopDepth = -1;
		m_useSpecifiedID = false;
		m_specifiedId = 0;
	}
};


class hkMoppModifier;

extern "C"
{
		/// Returns true if the obb hits a mopp leave node
	int HK_CALL hkMoppEarlyExitObbVirtualMachine_queryObb(const hkMoppCode* code, const hkTransform& BvToWorld, const hkVector4& extent, const float& radius);

		/// Return all the keys in a mopp. Note: the order of keys in a mopp is consistant.
		/// Please read hkMoppFitToleranceRequirements
	void HK_CALL hkMoppFindAllVirtualMachine_getAllKeys(   const hkMoppCode* code, hkArray<hkShapeKey>* primitives_out);

		/// Returns at least keys in a mopp which overlap with the given obb
	void HK_CALL hkMoppObbVirtualMachine_queryObb(const hkMoppCode* code, const hkTransform& BvToWorld, const hkVector4& halfExtent, const float radius, hkArray<hkShapeKey>* primitives_out);

		/// Returns at least keys in a mopp which overlap with the given aabb
	void HK_CALL hkMoppObbVirtualMachine_queryAabb(const hkMoppCode* code, const hkAabb& aabb, hkArray<hkShapeKey>* primitives_out);

			/// Returns at least keys in a mopp which overlap with the given sphere
	void HK_CALL hkMoppSphereVirtualMachine_querySphere(const hkMoppCode* code, const hkSphere &sphere, hkArray<hkShapeKey>* primitives_out);

		/// Query optimized for frustum checks. It reports all hits intersecting the planes (partialHitsOut).
		/// and all hits completely inside the convex object defined by the planes (fullyIncludedHitsOut).
	void HK_CALL hkMoppUsingFloatAabbVirtualMachine_queryPlanes( const hkMoppCode* code, const hkMoppPlanesQueryInput &query, hkArray<hkShapeKey>* partialHitsOut, hkArray<hkShapeKey>* fullyIncludedHitsOut);

		/// Same as hkMoppUsingFloatAabbVirtualMachine_queryPlanes but instead of returning all hits which are fully included
		/// it returns ranges of hits in fullyIncludedHitsOut. 
		/// You can use hkMoppFindAllVirtualMachine_getAllKeys to find about the ordering of keys and
		/// than either reorder your input or create two mapping arrays.
		/// Example:<br>
		/// If you call hkMoppFindAllVirtualMachine_getAllKeys you might get the following hits:<br>
		/// 1, 3, 2, 7, 5, 4, 8, 6<br>
		/// If hkMoppUsingFloatAabbVirtualMachine_queryPlanesOptimized returns the range [3,4], it means all hits between 3 and 4 inclusive,
		/// which is 3,2,7,5,4
	void HK_CALL hkMoppUsingFloatAabbVirtualMachine_queryPlanesOptimized( const hkMoppCode* code, const hkMoppPlanesQueryInput &query, hkArray<hkShapeKey>* partialHitsOut, hkArray<hkShapeKey>* fullyIncludedHitsOut);


		/// Same as hkMoppUsingFloatAabbVirtualMachine_queryPlanes, but using a sphere instead of a convex object
	void HK_CALL hkMoppUsingFloatAabbVirtualMachine_querySphere( const hkMoppCode* code, const hkSphere &query, hkArray<hkShapeKey>* partialHitsOut, hkArray<hkShapeKey>* fullyIncludedHitsOut);

		/// Same as hkMoppUsingFloatAabbVirtualMachine_queryPlanesOptimized, but using a sphere instead of a convex object
	void HK_CALL hkMoppUsingFloatAabbVirtualMachine_querySphereOptimized( const hkMoppCode* code, const hkSphere &query, hkArray<hkShapeKey>* partialHitsOut, hkArray<hkShapeKey>* fullyIncludedHitsOut);

			/// Queries the mopp and calls shouldTerminalBeRemoved on at least all nodes which overlap the input aabbs.
			/// modifierOut returns whether you want to remove a node or not.
			/// For every subtree which only has 'to-be-removed' nodes the address ADDR of the root node of this
			/// subtree is calculated and m_modifierOut->addTerminalRemoveInfo(ADDR) called. If you set the moppcode at
			/// this relative address to zero, you will effectively disable this subtree.<br>
			/// In short:
			///   - call queryAabb with an aabb containing all your nodes you want to remove
			///   - for every node you want to remove, return true in you implementation of hkMoppModifier::shouldTerminalBeRemoved
			///   - remember all mopp code addressed in hkMoppModifier::addTerminalRemoveInfo
			///   - change the mopp at all those addresses to zero to apply the mopp changes.
			///   - optional: if you remember all the places you have changed, than you can undo your changes
	void HK_CALL hkMoppModifyVirtualMachine_queryAabb( const hkMoppCode* code, const hkAabb& aabb, hkMoppModifier* modifierOut );

		/// 
	void HK_CALL hkMoppKDopGeometriesVirtualMachine_query( const hkMoppCode* code, const hkMoppKDopQuery &query, hkMoppInfo* kDopGeometries );
}



#endif // HK_COLLIDE2_MOPP_MACHINE_H

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
