//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/hierarchy_tools.h
//!	
//!	DYNAMICS COMPONENT:
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.17
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_HIERARCHY_TOOLS_INC
#define _DYNAMICS_HIERARCHY_TOOLS_INC

#include "config.h"
#include "havokincludes.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

class CHierarchy;
class Transform;

#include <hkmath/hkmath.h>
#include <hkanimation/rig/hkPose.h>
class hkQsTransform;

namespace Physics
{
	// ---------------------------------------------------------------
	//	Tools class. Do not try to instanciate.
	// ---------------------------------------------------------------
	class HierarchyTools
	{
	public:

		static bool	IsTransformInHierarchy( const CHierarchy*, const Transform* );

		static int	NumberOfTransform( const CHierarchy*, Transform* pobCurrentHierarchyNode );
		static int	NumberOfTransform( const CHierarchy*, Transform* pobCurrentHierarchyNode, int& l_hip, int& l_knee, int& l_ankle, int& r_hip, int& r_knee, int& r_ankle   );

		static void GetLocalPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform );
		static void GetLocalPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform );
		static void GetWorldPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform );
		static void GetWorldPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform );
		static void GetBindPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform );
		static void GetBindPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform );

		static void SetLocalPoseRecursive( const hkArray<hkQsTransform>& transformHierarchy, const CHierarchy* parentHierarchy, Transform* startTransform , hkUint16 AnimatedRagdollBonesFlag, bool bRagdoll = true );
		static void SetAllLocalPoseRecursive( const hkArray<hkQsTransform>& transformHierarchy, const CHierarchy* parentHierarchy, Transform* startTransform );

		static void SetSkeletonRecursive( hkInt16* hierarchyPtr, const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode );
		static void SetSkeletonBonesRecursive(hkBone** hierarchyPtr, const CHierarchy* parentHierarchy, hkQsTransform* transform, Transform* pobCurrentHierarchyNode);

	private:

		static int	NumberOfTransform( const CHierarchy*, Transform* pobCurrentHierarchyNode, hkInt16 parentForThisHierarchyNode );
		static int	NumberOfTransform( const CHierarchy*, Transform* pobCurrentHierarchyNode, hkInt16 parentForThisHierarchyNode, int& l_hip, int& l_knee, int& l_ankle, int& r_hip, int& r_knee, int& r_ankle  );

		static void GetLocalPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );
		static void GetLocalPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );
		static void GetWorldPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );
		static void GetWorldPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );
		static void GetBindPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );
		static void GetBindPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );
		
		static void SetLocalPoseRecursive( const hkArray<hkQsTransform>& transformHierarchy, const CHierarchy* parentHierarchy, Transform* currentTransform, hkUint16 AnimatedRagdollBonesFlag, hkInt16 previousIndex, bool bRagdoll = true );
		static void SetAllLocalPoseRecursive( const hkArray<hkQsTransform>& transformHierarchy, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex );

		static void SetSkeletonRecursive( hkInt16* hierarchyPtr, const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode, hkInt16 parentForThisHierarchyNode );
		static void SetSkeletonBonesRecursive(hkBone** hierarchyPtr, const CHierarchy* parentHierarchy, hkQsTransform* transform, Transform* pobCurrentHierarchyNode, hkInt16 parentForThisHierarchyNode);

		HierarchyTools();
		~HierarchyTools();
	};

}

#endif
#endif // _DYNAMICS_HIERARCHY_TOOLS_INC
