//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/hierarchy_tools.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.17
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"
#include "havokincludes.h"

#include "hierarchy_tools.h"
#include "maths_tools.h"

#include "anim/hierarchy.h"
#include "anim/transform.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "core/exportstruct_clump.h"
#include "core/exportstruct_anim.h"
#endif

namespace Physics {

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	static char *asRagdollBodyName[] = 
	{
		"_Invalid_Bone_",			//CHARACTER_BONE_ROOT = CHARACTER_BONE_HIGH_DETAIL_START,		//0	'root' 
		"Biped_Pelvis",				//CHARACTER_BONE_PELVIS,										//1	'pelvis'
		"Biped_Spine0",				//CHARACTER_BONE_SPINE_00,										//2	'spine_00'
		"Biped_Spine1",				//CHARACTER_BONE_SPINE_01,										//3	'spine_01'
		"Biped_Spine2",				//CHARACTER_BONE_SPINE_02,										//4	'spine_02'
		"_Invalid_Bone_",			//CHARACTER_BONE_NECK,											//5	'neck'
		"Biped_Head",				//CHARACTER_BONE_HEAD,											//6	'head'
		"_Invalid_Bone_",			//CHARACTER_BONE_HIPS,											//7	'hips'

		"_Invalid_Bone_",			//CHARACTER_BONE_L_SHOULDER,									//8	'l_shoulder'
		"Biped_L_UpperArm",			//CHARACTER_BONE_L_ARM,											//9	'l_arm'
		"Biped_L_ForeArm",			//CHARACTER_BONE_L_ELBOW,										//10	'l_elbow'
		"Biped_L_ForeArm",			//CHARACTER_BONE_L_WRIST,										//11	'l_wrist'
		"_Invalid_Bone_",			//CHARACTER_BONE_L_WEAPON,										//12	'l_weapon'

		"Biped_L_Thigh",			//CHARACTER_BONE_L_LEG,											//13	'l_leg'
		"Biped_L_Calf",				//CHARACTER_BONE_L_KNEE,										//14	'l_knee'

		"_Invalid_Bone_",			//CHARACTER_BONE_R_SHOULDER,									//15	'r_shoulder'
		"Biped_R_UpperArm",			//CHARACTER_BONE_R_ARM,											//16	'r_arm'
		"Biped_R_ForeArm",			//CHARACTER_BONE_R_ELBOW,										//17	'r_elbow'
		"Biped_R_ForeArm",			//CHARACTER_BONE_R_WRIST,										//18	'r_wrist'
		"_Invalid_Bone_",			//CHARACTER_BONE_R_WEAPON,										//19	'r_weapon'

		"Biped_R_Thigh",			//CHARACTER_BONE_R_LEG,											//20	'r_leg'
		"Biped_R_Calf",				//CHARACTER_BONE_R_KNEE,										//21	'r_knee'
	};
	static int asRagdollBodyNameSize = 22;

	//________________________________________________________________________________________________________________________________________________________________________________
	bool HierarchyTools::IsTransformInHierarchy( const CHierarchy* parentHierarchy, const Transform* curentTrf )
	{
		if( parentHierarchy == curentTrf->GetParentHierarchy() )
		{
			// then perform a hierarchy test
			int	iOffset = ( int )( curentTrf - parentHierarchy->GetRootTransform() );
			if( ( iOffset >= 0 ) && ( iOffset < parentHierarchy->GetTransformCount() ) )
			{
				return true;
			}
		};

		return false;
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::GetLocalPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform  )
	{
		GetLocalPoseRecursive( poseOut, parentHierarchy, currentTransform, -1 );
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::GetLocalPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------
			// Tranform the current node into the Havok format.
			hkVector4 translation = MathsTools::CPointTohkVector( currentTransform->GetLocalTranslation() );
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion( CQuat(currentTransform->GetLocalMatrix()) );

			// Set the hierarchy.
			poseOut[currentHierarchyIndex] = hkQsTransform( translation, rotation);
			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			

			if(currentTransform->GetFirstChild())
			{
				GetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				GetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				GetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	};

	//DGF version for normal arrays
	void HierarchyTools::GetLocalPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform  )
	{
		GetLocalPoseRecursive( poseOut, parentHierarchy, currentTransform, -1 );
	}

	void HierarchyTools::GetLocalPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------
			// Tranform the current node into the Havok format.
			hkVector4 translation = MathsTools::CPointTohkVector( currentTransform->GetLocalTranslation() );
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion( CQuat(currentTransform->GetLocalMatrix()) );

			// Set the hierarchy.
			poseOut[currentHierarchyIndex] = hkQsTransform( translation, rotation);
			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			

			if(currentTransform->GetFirstChild())
			{
				GetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				GetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				GetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	};

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::GetWorldPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform  )
	{
		GetWorldPoseRecursive( poseOut, parentHierarchy, currentTransform, -1 );
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::GetWorldPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------
			// Tranform the current node into the Havok format.
			hkVector4 translation = MathsTools::CPointTohkVector(		currentTransform->GetWorldMatrix().GetTranslation() );
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion( CQuat(currentTransform->GetWorldMatrix()) );

			// Set the hierarchy.
			poseOut[currentHierarchyIndex] = hkQsTransform( translation, rotation );
			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(currentTransform->GetFirstChild())
			{
				GetWorldPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				GetWorldPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				GetWorldPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	};

	//DGF version for normal arrays
	void HierarchyTools::GetWorldPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform  )
	{
		GetWorldPoseRecursive( poseOut, parentHierarchy, currentTransform, -1 );
	}

	void HierarchyTools::GetWorldPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------
			// Tranform the current node into the Havok format.
			hkVector4 translation = MathsTools::CPointTohkVector(		currentTransform->GetWorldMatrix().GetTranslation() );
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion( CQuat(currentTransform->GetWorldMatrix()) );

			// Set the hierarchy.
			poseOut[currentHierarchyIndex] = hkQsTransform( translation, rotation );
			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(currentTransform->GetFirstChild())
			{
				GetWorldPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				GetWorldPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				GetWorldPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	};

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::GetBindPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform  )
	{
		GetBindPoseRecursive( poseOut, parentHierarchy, currentTransform, -1 );
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::GetBindPoseRecursive( hkLocalArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------

			int	iOffset = ( int )( currentTransform - parentHierarchy->GetRootTransform() );
			CMatrix m;
			m.SetFromQuat(		parentHierarchy->GetBindPoseJointRotation( iOffset ) );
			m.SetTranslation(	parentHierarchy->GetBindPoseJointTranslation( iOffset ) );

			// Set the initial transform
			hkVector4 translation = MathsTools::CPointTohkVector(		m.GetTranslation()	);
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion(	CQuat( m )			);

			poseOut[currentHierarchyIndex] = hkQsTransform( translation, rotation );

			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(currentTransform->GetFirstChild())
			{
				GetBindPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				GetBindPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				GetBindPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	};

	// DGF version for normal arrays
	void HierarchyTools::GetBindPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform  )
	{
		GetBindPoseRecursive( poseOut, parentHierarchy, currentTransform, -1 );
	}

	void HierarchyTools::GetBindPoseRecursive( hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------

			int	iOffset = ( int )( currentTransform - parentHierarchy->GetRootTransform() );
			CMatrix m;
			m.SetFromQuat(		parentHierarchy->GetBindPoseJointRotation( iOffset ) );
			m.SetTranslation(	parentHierarchy->GetBindPoseJointTranslation( iOffset ) );

			// Set the initial transform
			hkVector4 translation = MathsTools::CPointTohkVector(		m.GetTranslation()	);
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion(	CQuat( m )			);

			poseOut[currentHierarchyIndex] = hkQsTransform( translation, rotation );

			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(currentTransform->GetFirstChild())
			{
				GetBindPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				GetBindPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				GetBindPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	};

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::SetLocalPoseRecursive( const hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform , hkUint16 AnimatedRagdollBonesFlag, bool bRagdoll )
	{
		SetLocalPoseRecursive( poseOut, parentHierarchy, startTransform, AnimatedRagdollBonesFlag, -1, bRagdoll );
	}

	void HierarchyTools::SetAllLocalPoseRecursive( const hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* startTransform  )
	{
		SetAllLocalPoseRecursive( poseOut, parentHierarchy, startTransform, -1 );
	}

	void HierarchyTools::SetAllLocalPoseRecursive( const hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkInt16 previousIndex )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------

			
				CMatrix result = MathsTools::hkQuaternionToCMatrix(  poseOut[currentHierarchyIndex].getRotation() );
				result.SetTranslation( MathsTools::hkVectorToCPoint( poseOut[currentHierarchyIndex].getTranslation()) );
				
				currentTransform->SetLocalMatrix(result);
			

			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(currentTransform->GetFirstChild())
			{
				SetAllLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				SetAllLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

			

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				SetAllLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), previousIndex);
			};

		};
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::SetLocalPoseRecursive( const hkArray<hkQsTransform>& poseOut, const CHierarchy* parentHierarchy, Transform* currentTransform, hkUint16 AnimatedRagdollBonesFlag, hkInt16 previousIndex, bool bRagdoll )
	{
		static hkInt16 currentHierarchyIndex;

		// I've moved this here from the old RagdollLG as this is the only place this enum is used.
		enum
		{
			RAGDOLL_HEAD_BIT		= ( 1 << 0 ),
			RAGDOLL_SPINE_00_BIT	= ( 1 << 1 ),
			RAGDOLL_PELVIS_BIT		= ( 1 << 2 ),
			RAGDOLL_L_ARM_BIT		= ( 1 << 3 ),
			RAGDOLL_R_ARM_BIT		= ( 1 << 4 ),
			RAGDOLL_L_ELBOW_BIT		= ( 1 << 5 ),
			RAGDOLL_R_ELBOW_BIT		= ( 1 << 6 ),
			RAGDOLL_L_LEG_BIT		= ( 1 << 7 ),
			RAGDOLL_R_LEG_BIT		= ( 1 << 8 ),
			RAGDOLL_L_KNEE_BIT		= ( 1 << 9 ),
			RAGDOLL_R_KNEE_BIT		= ( 1 << 10 ),
		};

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, currentTransform ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------

			bool skip = false;
		
			if (bRagdoll)
			{
				// Do not update keyframed bones
				if((AnimatedRagdollBonesFlag & RAGDOLL_HEAD_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_HEAD) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_SPINE_00_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_SPINE_00) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_PELVIS_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_PELVIS) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_L_ARM_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_L_ARM) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_R_ARM_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_R_ARM) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_L_ELBOW_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_L_ELBOW) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_R_ELBOW_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_R_ELBOW) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_L_LEG_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_L_LEG) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_R_LEG_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_R_LEG) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_L_KNEE_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_L_KNEE) == currentTransform))
					skip = true;
				if((AnimatedRagdollBonesFlag & RAGDOLL_R_KNEE_BIT) && (parentHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_R_KNEE) == currentTransform))
					skip = true;
			}

			if( false == skip )
			{
				CMatrix result = MathsTools::hkQuaternionToCMatrix(  poseOut[currentHierarchyIndex].getRotation() );
				result.SetTranslation( MathsTools::hkVectorToCPoint( poseOut[currentHierarchyIndex].getTranslation()) );
				
				currentTransform->SetLocalMatrix(result);
			};

			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(currentTransform->GetFirstChild())
			{
				SetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetFirstChild(), AnimatedRagdollBonesFlag, currentHierarchyIndex);		
			};

			if(currentTransform->GetNextSibling())
			{
				SetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), AnimatedRagdollBonesFlag, previousIndex);
			};

			

			

		} else {

			if(currentTransform->GetNextSibling())
			{
				SetLocalPoseRecursive(poseOut, parentHierarchy, currentTransform->GetNextSibling(), AnimatedRagdollBonesFlag, previousIndex);
			};

		};
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	int HierarchyTools::NumberOfTransform( const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode )
	{
		return NumberOfTransform( parentHierarchy, pobCurrentHierarchyNode, -1 );
	}

	int HierarchyTools::NumberOfTransform( const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode, hkInt16 previousIndex )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, pobCurrentHierarchyNode ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			// Recurse.
			if(pobCurrentHierarchyNode->GetFirstChild())
			{
				NumberOfTransform(parentHierarchy, pobCurrentHierarchyNode->GetFirstChild(), currentHierarchyIndex);		
			};

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				NumberOfTransform(parentHierarchy, pobCurrentHierarchyNode->GetNextSibling(), previousIndex);
			};

			

			

		} else {

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				NumberOfTransform(parentHierarchy, pobCurrentHierarchyNode->GetNextSibling(), previousIndex);
			};

		};

		return (currentHierarchyIndex + 1);
	}

	int HierarchyTools::NumberOfTransform( const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode, int& l_hip, int& l_knee, int& l_ankle, int& r_hip, int& r_knee, int& r_ankle  )
	{
		return NumberOfTransform( parentHierarchy, pobCurrentHierarchyNode, -1,   l_hip,  l_knee,  l_ankle,  r_hip,  r_knee,  r_ankle );
	}

	int HierarchyTools::NumberOfTransform( const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode, hkInt16 previousIndex, int& l_hip, int& l_knee, int& l_ankle, int& r_hip, int& r_knee, int& r_ankle  )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, pobCurrentHierarchyNode ) )
		{
			// Increment the index.
			if(-1 == previousIndex)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			if( pobCurrentHierarchyNode == parentHierarchy->GetCharacterBoneTransform( CHARACTER_BONE_L_LEG ) )
				l_hip = currentHierarchyIndex;
			if( pobCurrentHierarchyNode == parentHierarchy->GetCharacterBoneTransform( CHARACTER_BONE_L_KNEE ) )
				l_knee = currentHierarchyIndex;
			if( pobCurrentHierarchyNode == parentHierarchy->GetCharacterBoneTransform( CHARACTER_BONE_L_ANKLE ) )
				l_ankle = currentHierarchyIndex;
			if( pobCurrentHierarchyNode == parentHierarchy->GetCharacterBoneTransform( CHARACTER_BONE_R_LEG ) )
				r_hip = currentHierarchyIndex;
			if( pobCurrentHierarchyNode == parentHierarchy->GetCharacterBoneTransform( CHARACTER_BONE_R_KNEE ) )
				r_knee = currentHierarchyIndex;
			if( pobCurrentHierarchyNode == parentHierarchy->GetCharacterBoneTransform( CHARACTER_BONE_R_ANKLE ) )
				r_ankle = currentHierarchyIndex;

			// Recurse.
			if(pobCurrentHierarchyNode->GetFirstChild())
			{
				NumberOfTransform(parentHierarchy, pobCurrentHierarchyNode->GetFirstChild(), currentHierarchyIndex, l_hip,  l_knee,  l_ankle,  r_hip,  r_knee,  r_ankle);		
			};

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				NumberOfTransform(parentHierarchy, pobCurrentHierarchyNode->GetNextSibling(), previousIndex, l_hip,  l_knee,  l_ankle,  r_hip,  r_knee,  r_ankle);
			};

			

			

		} else {

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				NumberOfTransform(parentHierarchy, pobCurrentHierarchyNode->GetNextSibling(), previousIndex, l_hip,  l_knee,  l_ankle,  r_hip,  r_knee,  r_ankle);
			};

		};

		return (currentHierarchyIndex + 1);
	}
	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::SetSkeletonRecursive( hkInt16* hierarchyPtr, const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode )
	{
		SetSkeletonRecursive( hierarchyPtr, parentHierarchy, pobCurrentHierarchyNode, -1 );
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::SetSkeletonRecursive( hkInt16* hierarchyPtr, const CHierarchy* parentHierarchy, Transform* pobCurrentHierarchyNode, hkInt16 parentForThisHierarchyNode )
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, pobCurrentHierarchyNode ) )
		{
			// Increment the index.
			if(-1 == parentForThisHierarchyNode)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------

			hkInt16 currentNodeIndex = currentHierarchyIndex;
			hierarchyPtr[currentNodeIndex] = parentForThisHierarchyNode;

			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(pobCurrentHierarchyNode->GetFirstChild())
			{
				SetSkeletonRecursive(hierarchyPtr, parentHierarchy, pobCurrentHierarchyNode->GetFirstChild(), currentHierarchyIndex);		
			};

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				SetSkeletonRecursive(hierarchyPtr, parentHierarchy, pobCurrentHierarchyNode->GetNextSibling(), parentForThisHierarchyNode);
			};

			

			

		} else {

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				SetSkeletonRecursive(hierarchyPtr, parentHierarchy, pobCurrentHierarchyNode->GetNextSibling(), parentForThisHierarchyNode);
			};

		};
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::SetSkeletonBonesRecursive(hkBone** hierarchyPtr, const CHierarchy* parentHierarchy, hkQsTransform* transform, Transform* pobCurrentHierarchyNode)
	{
		SetSkeletonBonesRecursive( hierarchyPtr, parentHierarchy, transform, pobCurrentHierarchyNode, -1 );
	}

	//________________________________________________________________________________________________________________________________________________________________________________
	void HierarchyTools::SetSkeletonBonesRecursive(hkBone** hierarchyPtr, const CHierarchy* parentHierarchy, hkQsTransform* transform, Transform* pobCurrentHierarchyNode, hkInt16 parentForThisHierarchyNode)
	{
		static hkInt16 currentHierarchyIndex;

		if(  HierarchyTools::IsTransformInHierarchy( parentHierarchy, pobCurrentHierarchyNode ) )
		{
			// Increment the index.
			if(-1 == parentForThisHierarchyNode)
				currentHierarchyIndex = 0;
			else
				currentHierarchyIndex++;

			//---------------------- SPECIFIC WORK ----------------------------------------------------------------------

			hkBone* newBone = HK_NEW  hkBone;
			hierarchyPtr[currentHierarchyIndex] = newBone;

			// Lock the translation component as needed
			if(-1 == parentForThisHierarchyNode)
				hierarchyPtr[currentHierarchyIndex]->m_lockTranslation = false;
			else
				hierarchyPtr[currentHierarchyIndex]->m_lockTranslation = true;

			// Give a name (I should do that at some point...)
			// [scee_st] this is totalling over 200K on some levels, I need to look at this
			hierarchyPtr[currentHierarchyIndex]->m_name = NT_NEW_CHUNK ( MC_PHYSICS ) char[30];
			
			strcpy(hierarchyPtr[currentHierarchyIndex]->m_name, asRagdollBodyName[0]);
			for(int i = 0; i < asRagdollBodyNameSize; i++)
			{
				if(pobCurrentHierarchyNode->GetParentHierarchy()->GetCharacterBoneTransform( (CHARACTER_BONE_ID) i ) == pobCurrentHierarchyNode)
				{
					strcpy(hierarchyPtr[currentHierarchyIndex]->m_name,asRagdollBodyName[i]);
					i = asRagdollBodyNameSize;
				};
			};

			int	iOffset = ( int )( pobCurrentHierarchyNode - pobCurrentHierarchyNode->GetParentHierarchy()->GetRootTransform() );
			CMatrix m;// = m_BindPose[i].GetLocalMatrix();
			m.SetFromQuat(pobCurrentHierarchyNode->GetParentHierarchy()->GetBindPoseJointRotation( iOffset ) );
			m.SetTranslation(pobCurrentHierarchyNode->GetParentHierarchy()->GetBindPoseJointTranslation( iOffset ) );
			
			// Set the initial transform
			//CVector trans(pobCurrentHierarchyNode->GetLocalTranslation());
			CVector trans(m.GetTranslation());
			hkVector4 translation( trans.X(), trans.Y(), trans.Z(), trans.W());

			//CQuat rot(pobCurrentHierarchyNode->GetLocalMatrix());
			CQuat rot(m);
			hkQuaternion rotation = MathsTools::CQuatTohkQuaternion(rot);

			transform[currentHierarchyIndex] = hkQsTransform( translation, rotation);

			//---------------------- !SPECIFIC WORK ----------------------------------------------------------------------

			// Recurse.
			if(pobCurrentHierarchyNode->GetFirstChild())
			{
				SetSkeletonBonesRecursive(hierarchyPtr, parentHierarchy, transform, pobCurrentHierarchyNode->GetFirstChild(), currentHierarchyIndex);		
			};

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				SetSkeletonBonesRecursive(hierarchyPtr, parentHierarchy, transform, pobCurrentHierarchyNode->GetNextSibling(), parentForThisHierarchyNode);
			};

			

			

		} else {

			if(pobCurrentHierarchyNode->GetNextSibling())
			{
				SetSkeletonBonesRecursive(hierarchyPtr, parentHierarchy, transform, pobCurrentHierarchyNode->GetNextSibling(), parentForThisHierarchyNode);
			};

		};
	}

#endif

}
