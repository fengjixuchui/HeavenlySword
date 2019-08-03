//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/IKFootPlacement.cpp
//!	
//!	DYNAMICS COMPONENT:
//!		Hacking the HKA IK system to be used by our animation system.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.06.03
//!
//---------------------------------------------------------------------------------------------------------
#include "config.h"
#include "physics/havokincludes.h"
#include "maths_tools.h"
#include "hierarchy_tools.h"
#include "hsFootPlacementIkSolver.h"
#include "core/gatso.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>
#include <hkanimation/rig/hkPose.h>
#include <hkanimation/rig/hkSkeleton.h>
//#include <hkanimation/ik/footplacement/hkFootPlacementIkSolver.h>
#include <hkanimation/ik/lookat/hkLookAtIkSolver.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\collector\raycollector\hkClosestRayHitCollector.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/castutil/hkLinearCastInput.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkdynamics/entity/hkRigidBody.h>

#endif

#include "core/exportstruct_clump.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_anim.h"
#include "anim/transform.h"

#include "IKFootPlacement.h"
#include "physics/world.h"
#include "physics/havokthreadutils.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"
//#define DEBUG_DRAW_IK

namespace Physics
{
	bool IKFootPlacement::m_gIKEnable = false;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	static void DebugRay(  const hkVector4& fromWS, const hkVector4& toWS, const hkClosestRayHitCollector& collector )
	{
	#ifdef DEBUG_DRAW_IK
			if(!collector.hasHit())
			{
				// White Line for failed Raycast
				HK_DISPLAY_LINE(fromWS, toWS, hkColor::GREEN);

			} else {

				CPoint obFrom, obTo;
				MathsTools::hkVectorToCPoint(fromWS,obFrom);
				MathsTools::hkVectorToCPoint(toWS,obTo);

				CPoint obHit = CPoint::Lerp(	obFrom, obTo, 
												collector.getHit().m_hitFraction );
				hkVector4 vHit;
				MathsTools::CPointTohkVector(obHit,vHit);

				// Red line for successfull ones
				HK_DISPLAY_LINE(fromWS, vHit, hkColor::YELLOW);
		
			}
	#endif
	}

	static void DoCastRay( const hkVector4& fromWS, const hkVector4& toWS, hkClosestRayHitCollector& collector )
	{
		// 1 - Do Raycast -------------------------------------------------------------
		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		// [Mus] - What settings for this cast ?
		obFlag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT/*COLLISION_ENVIRONMENT_BIT*/;
		obFlag.flags.i_collide_with = (	Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);

		hkWorldRayCastInput input;
		input.m_from = fromWS;
		input.m_to = toWS;
		input.m_filterInfo = obFlag.base;

		{
			Physics::CastRayAccess mutex;
			CPhysicsWorld::Get().GetHavokWorldP()->castRay( input, collector );
		}
	}

	class IKFootRayCast: public hsRaycastInterface
	{
		
	public:	
		hkBool castRay ( const hkVector4& fromWS, const hkVector4& toWS, hkReal& hitFractionOut, hkVector4& normalWSOut )
		{
			//CGatso::Start("IKFootRayCastOneShoot::cast");
			// 1 - Do Raycast -------------------------------------------------------------
			hkClosestRayHitCollector collector;
			DoCastRay( fromWS, toWS, collector );
			
			// 2 - If no hit, exit ----------------------
			hkWorldRayCastOutput output = collector.getHit();
			hitFractionOut				= output.m_hitFraction;
			normalWSOut					= output.m_normal;
			bool hasHit					= collector.hasHit();
			DebugRay( fromWS, toWS, collector );

			// 3 - Keep the feet straight over rigid bodies
			if( collector.hasHit() )
			{
				hkRigidBody* rb = hkGetRigidBody( output.m_rootCollidable );
				if( (rb) && ( rb->getMotionType() != hkMotion::MOTION_FIXED ) )
				{
					normalWSOut(0) = 0.0f;
					normalWSOut(2) = 0.0f;
					normalWSOut.normalize3();
				}
			}

			// 4 - smoother.
			/*if( ankleFraction != -1.0f )
				hitFractionOut = ankleFraction + (( hitFractionOut - ankleFraction ) * 0.1f);*/
			

			//CGatso::Stop("IKFootRayCastOneShoot::cast");
			return hasHit; 
		}

	
	};

#endif

	//----------------------------------------------------------------------------------

	IKFootPlacement::IKFootPlacement(CHierarchy* pobHierarchy)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		:m_pobLeftIK(0),
		m_pobRightIK(0),
		m_pobHierarchy(pobHierarchy),
		m_pobSkeleton(0),
		m_pobAnimPose(0)
#endif
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_resetPelvisY = true;
		Create();
#else
		UNUSED( pobHierarchy );
#endif
	}

	IKFootPlacement::~IKFootPlacement()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		HK_DELETE( m_pobLeftIK );
		HK_DELETE( m_pobRightIK );
		if (m_pobSkeleton)
		{
			for(int i = 0; i < m_pobSkeleton->m_numBones; i++ )
			{
				NT_DELETE_ARRAY( m_pobSkeleton->m_bones[i]->m_name );
				HK_DELETE( m_pobSkeleton->m_bones[i] ); 	
			};

			HK_DELETE_ARRAY(m_pobSkeleton->m_parentIndices);
			HK_DELETE( m_pobSkeleton );
		}
		HK_DELETE( m_pobAnimPose );

		m_pobLeftIK		= 0;
		m_pobRightIK	= 0;
		m_pobHierarchy	= 0;
		m_pobSkeleton	= 0;

#endif
	}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	static hkVector4 kneeAxisLS( -1.0f, 0.0f, 0.0f );
	static hkVector4 footEndLS( 0.0f, 0.15f, 0.0f );	
	static hkVector4 worldUpDirectionWS( 0.0f, 1.0f, 0.0f );		
	static hkVector4 modelUpDirectionMS( 0.0f, 1.0f, 0.0f );	
		
	static float originalGroundHeightMS		= 0.0f;	
	static float footPlantedAnkleHeightMS	= 0.07f;  
	static float footRaisedAnkleHeightMS	= 0.15f;

	static float minAnkleHeightMS	= -0.7f;
	static float maxAnkleHeightMS	= 0.8f/*0.7f*/;

	static float cosineMaxKneeAngle	= -0.95f;		
	static float cosineMinKneeAngle	= 0.95f;	

	static float raycastDistanceUp		= 0.3f;		
	static float raycastDistanceDown	= 0.3f;	
#endif


	void IKFootPlacement::Reset()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// --------------
		m_pobLeftIK->m_setup.m_kneeAxisLS				= kneeAxisLS;
		m_pobLeftIK->m_setup.m_footEndLS					= footEndLS;
		m_pobLeftIK->m_setup.m_worldUpDirectionWS		= worldUpDirectionWS;		
		m_pobLeftIK->m_setup.m_modelUpDirectionMS		= modelUpDirectionMS;	
		m_pobLeftIK->m_setup.m_originalGroundHeightMS	= originalGroundHeightMS;	
		m_pobLeftIK->m_setup.m_footPlantedAnkleHeightMS	= footPlantedAnkleHeightMS;  
		m_pobLeftIK->m_setup.m_footRaisedAnkleHeightMS	= footRaisedAnkleHeightMS;	
		m_pobLeftIK->m_setup.m_minAnkleHeightMS			= minAnkleHeightMS;
		m_pobLeftIK->m_setup.m_maxAnkleHeightMS			= maxAnkleHeightMS;
		m_pobLeftIK->m_setup.m_cosineMaxKneeAngle		= cosineMaxKneeAngle;		
		m_pobLeftIK->m_setup.m_cosineMinKneeAngle		= cosineMinKneeAngle;	
		m_pobLeftIK->m_setup.m_raycastDistanceUp			= raycastDistanceUp;		
		m_pobLeftIK->m_setup.m_raycastDistanceDown		= raycastDistanceDown;	

		// --------------
		m_pobRightIK->m_setup.m_kneeAxisLS				= kneeAxisLS;
		m_pobRightIK->m_setup.m_footEndLS				= footEndLS;
		m_pobRightIK->m_setup.m_worldUpDirectionWS		= worldUpDirectionWS;		
		m_pobRightIK->m_setup.m_modelUpDirectionMS		= modelUpDirectionMS;	
		m_pobRightIK->m_setup.m_originalGroundHeightMS	= originalGroundHeightMS;	
		m_pobRightIK->m_setup.m_footPlantedAnkleHeightMS	= footPlantedAnkleHeightMS;  
		m_pobRightIK->m_setup.m_footRaisedAnkleHeightMS	= footRaisedAnkleHeightMS;		
		m_pobRightIK->m_setup.m_minAnkleHeightMS			= minAnkleHeightMS;
		m_pobRightIK->m_setup.m_maxAnkleHeightMS			= maxAnkleHeightMS;
		m_pobRightIK->m_setup.m_cosineMaxKneeAngle		= cosineMaxKneeAngle;		
		m_pobRightIK->m_setup.m_cosineMinKneeAngle		= cosineMinKneeAngle;	
		m_pobRightIK->m_setup.m_raycastDistanceUp		= raycastDistanceUp;		
		m_pobRightIK->m_setup.m_raycastDistanceDown		= raycastDistanceDown;
#endif
	}

	void IKFootPlacement::Create()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if( false == IKFootPlacement::m_gIKEnable)
			return;

		// The hkFootPlacementIkSolver need a setup.
		// The setup structure is actually composed of many informations.
		// It would make sense to be able to set some of theses values using the welder!
		hsFootPlacementIkSolver::Setup IKSetup;

		// The tricky bit is to build the skeleton from our anim system.
		if (m_pobAnimPose)		
			HK_DELETE( m_pobAnimPose );
		if (m_pobSkeleton)		
			HK_DELETE( m_pobSkeleton );
		

		m_pobSkeleton = HK_NEW hkSkeleton();		

		int l_hip; int l_knee; int l_ankle; int r_hip; int r_knee; int r_ankle;

		m_pobSkeleton->m_numParentIndices	= HierarchyTools::NumberOfTransform( m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild(), l_hip, l_knee, l_ankle, r_hip, r_knee, r_ankle );//IKNumberOfTransform(m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild(), -1 );
		m_pobSkeleton->m_numBones			= m_pobSkeleton->m_numParentIndices;
		m_pobSkeleton->m_parentIndices		= NT_NEW hkInt16[m_pobSkeleton->m_numParentIndices];

		/*SetIKSkeletonHierarchyRecursive(	m_pobHierarchy, m_pobSkeleton->m_parentIndices, 
											m_pobHierarchy->GetRootTransform()->GetFirstChild(), 
											-1);*/
		HierarchyTools::SetSkeletonRecursive( m_pobSkeleton->m_parentIndices, m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild() );

		m_pobSkeleton->m_bones = hkAllocate<hkBone*> (m_pobSkeleton->m_numParentIndices, HK_MEMORY_CLASS_UTILITIES );

		m_pobSkeleton->m_referencePose		= HK_NEW hkQsTransform[m_pobSkeleton->m_numParentIndices];
		m_pobSkeleton->m_numReferencePose	= m_pobSkeleton->m_numParentIndices;

		/*SetIKSkeletonBonesRecursive(	m_pobHierarchy, m_pobSkeleton->m_bones, 
										m_pobSkeleton->m_referencePose, 
										m_pobHierarchy->GetRootTransform()->GetFirstChild(),  
										-1);*/
		HierarchyTools::SetSkeletonBonesRecursive( m_pobSkeleton->m_bones, m_pobHierarchy, m_pobSkeleton->m_referencePose, m_pobHierarchy->GetRootTransform()->GetFirstChild() );

		m_pobAnimPose = HK_NEW hkPose( m_pobSkeleton );

		m_previousPlevisY = m_pobHierarchy->GetRootTransform()->GetFirstChild()->GetWorldMatrix().GetTranslation().Y();
		// Setup the parameters
		// [HACK] - There is more hardcoded values here than any sane programmer would enjoy letting in its code!

		// A skeleton matching our current animation rig
		IKSetup.m_skeleton	= m_pobSkeleton;
		
		// Index of the joint representing the hip (== bone representing the thigh)
		// Index of the joint representing the knee (== bone representing the calf)
		// Index of the joint representing the ankle (== bone representing the foot)

		
		IKSetup.m_hipIndex		= (hkInt16)l_hip;//CHARACTER_BONE_L_LEG - 1;//L_HIP;//m_pobHierarchy->GetCharacterBoneToIndexArray()[CHARACTER_BONE_L_LEG];					
		IKSetup.m_kneeIndex		= (hkInt16)l_knee;//CHARACTER_BONE_L_KNEE - 1;//L_KNEE;//m_pobHierarchy->GetCharacterBoneToIndexArray()[CHARACTER_BONE_L_LEG];						
		IKSetup.m_ankleIndex	= (hkInt16)l_ankle;//CHARACTER_BONE_L_ANKLE - 1;//L_ANKLE;//m_pobHierarchy->GetCharacterBoneToIndexArray()[CHARACTER_BONE_L_LEG];

		// Axis of rotation of the knee, in local space of the knee
		// The end of the foot, in the local space of the ankle/foot.				
		IKSetup.m_kneeAxisLS	= kneeAxisLS;
		IKSetup.m_footEndLS		= footEndLS;

		// The UP direction in the world (from ground to sky), in world space
		// The UP direction in the model (from feet to head), in model space
		IKSetup.m_worldUpDirectionWS	= worldUpDirectionWS;		
		IKSetup.m_modelUpDirectionMS	= modelUpDirectionMS;	
		
		// The height of the ground where the model was created/animated, in model space. 0 for us.
		// The height of the ankle below which the foot is considered planted.
		// The height of the ankle above which the foot is considered fully raised.
		// setupData.m_footPlantedAnkleHeightMS = 0.14f;	// At this height (and lower), we'll assume the foot is fully planted
		// setupData.m_footRaisedAnkleHeightMS = 0.5f;		// At this height (and higher), we'll asume the foot is fully raised
		IKSetup.m_originalGroundHeightMS	= originalGroundHeightMS;	
		IKSetup.m_footPlantedAnkleHeightMS	= footPlantedAnkleHeightMS;  
		IKSetup.m_footRaisedAnkleHeightMS	= footRaisedAnkleHeightMS;

		// Maximum height the ankle can reach (in model space)
		// Minimum height the ankle can reach (in model space)
		// setupData.m_minAnkleHeightMS = -0.1f;			// We won't allow our ankle to go any lower
		// setupData.m_maxAnkleHeightMS = 0.7f;			// We won't allow our ankle to go any higher			
		IKSetup.m_minAnkleHeightMS	= minAnkleHeightMS;
		IKSetup.m_maxAnkleHeightMS	= maxAnkleHeightMS;

		// Limit the knee angle (to avoid knee popping artifacts). Default is -1 (180 deg).
		// Limit the hinge angle (to avoid knee artifacts). Default is 1 (0 deg).
		// setupData.m_cosineMaxKneeAngle = -0.95f;		// Don't let the knee go too close to full extension
		// setupData.m_cosineMinKneeAngle = 0.95f;			// Don't let the knee go too close to full contraction
		IKSetup.m_cosineMaxKneeAngle	= cosineMaxKneeAngle;		
		IKSetup.m_cosineMinKneeAngle	= cosineMinKneeAngle;	

		// The (positive) distance, from the foot and in the up direction
		// The (positive) distance, from the foot and in the down direction
		IKSetup.m_raycastDistanceUp		= raycastDistanceUp;		
		IKSetup.m_raycastDistanceDown	= raycastDistanceDown;		

		// Create the IK solver
		HK_DELETE( m_pobLeftIK );
		m_pobLeftIK		= HK_NEW hsFootPlacementIkSolver(IKSetup);

		// Addapt the setup for the right leg
		IKSetup.m_hipIndex		= (hkInt16)r_hip;//CHARACTER_BONE_R_LEG - 1;//R_HIP;//m_pobHierarchy->GetCharacterBoneToIndexArray()[CHARACTER_BONE_R_LEG];					
		IKSetup.m_kneeIndex		= (hkInt16)r_knee;////CHARACTER_BONE_R_KNEE - 1;//R_KNEE;//m_pobHierarchy->GetCharacterBoneToIndexArray()[CHARACTER_BONE_R_KNEE];						
		IKSetup.m_ankleIndex	= (hkInt16)r_ankle;////CHARACTER_BONE_R_ANKLE - 1;//R_ANKLE;//m_pobHierarchy->GetCharacterBoneToIndexArray()[CHARACTER_BONE_R_ANKLE];

		// Create the IK solver
		HK_DELETE( m_pobRightIK );
		m_pobRightIK	= HK_NEW hsFootPlacementIkSolver(IKSetup);
#endif
	}

	void IKFootPlacement::DrawIKParameters( CMatrix& modelToWorld )
	{
#ifdef _PS3_RUN_WITHOUT_HAVOK_BUILD
		UNUSED( modelToWorld );
#endif
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef DEBUG_DRAW_IK
		// Debug draw for the principals parameters
		CPoint position		= modelToWorld.GetTranslation();
		float currentHeight = position.Y();

		float footPlantedHeight = currentHeight + m_pobLeftIK->m_setup.m_footPlantedAnkleHeightMS;
		float footRaisedHeight	= currentHeight + m_pobLeftIK->m_setup.m_footRaisedAnkleHeightMS;
		float maxAnkleHeight	= currentHeight + m_pobLeftIK->m_setup.m_maxAnkleHeightMS;
		float minAnkleHeight	= currentHeight + m_pobLeftIK->m_setup.m_minAnkleHeightMS;

		CVector start	= CVector(0.0f,	0.0f, 1.0f, 1.0f)  * modelToWorld;
		CVector end		= CVector(0.0f,	0.0f, -1.0f, 1.0f) * modelToWorld; 
		hkVector4 hkStart	= MathsTools::CVectorTohkVector(start);
		hkVector4 hkEnd		= MathsTools::CVectorTohkVector(end);

		hkStart(1) = footPlantedHeight; hkEnd(1) = footPlantedHeight;
		HK_DISPLAY_LINE( hkStart, hkEnd, hkColor::WHITE );

		hkStart(1) = footRaisedHeight; hkEnd(1) = footRaisedHeight;
		HK_DISPLAY_LINE( hkStart, hkEnd, hkColor::WHITE );

		hkStart(1) = maxAnkleHeight; hkEnd(1) = maxAnkleHeight;
		HK_DISPLAY_LINE( hkStart, hkEnd, hkColor::CYAN );

		hkStart(1) = minAnkleHeight; hkEnd(1) = minAnkleHeight;
		HK_DISPLAY_LINE( hkStart, hkEnd, hkColor::CYAN );
		
	#endif //DEBUG_DRAW_IK
#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD
	}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	void IKFootPlacement::DrawCurrentSkeletonPose( hkPose& animPose, CMatrix& modelToWorld, int color  )
	{

	#ifdef DEBUG_DRAW_IK
		for(int i=0; i < m_pobSkeleton->m_numBones; i++)
		{
			if(m_pobSkeleton->m_parentIndices[i] != -1)
			{
				hkQsTransform father	= animPose.getBoneModelSpace(m_pobSkeleton->m_parentIndices[i]);
				hkQsTransform son		= animPose.getBoneModelSpace(i);

				CVector father2 = MathsTools::hkVectorToCVector(father.getTranslation()) * modelToWorld ;
				CVector son2	= MathsTools::hkVectorToCVector(son.getTranslation()) * modelToWorld ;

				HK_DISPLAY_LINE( MathsTools::CVectorTohkVector(father2), MathsTools::CVectorTohkVector(son2), color );
			}
		}
	#endif //DEBUG_DRAW_IK

	}

	static float onOffGain				= 0.2f;
	static float groundAscendingGain	= 1.0f;
	static float groundDescendingGain	= 1.0f;
	static float footPlantedGain		= 1.0f;
	static float footRaisedGain			= 0.2f;
#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD

	
	void IKFootPlacement::Update( bool valid, float timestep )
	{
		//GATSO_PHYSICS_START("IKFootPlacement::Update");
		
#ifdef _PS3_RUN_WITHOUT_HAVOK_BUILD
		UNUSED( valid );
		UNUSED( walkingDir );
		UNUSED( timestep );
#endif
		//UpdateOld( 1.0f );
		//return;

		if( false == IKFootPlacement::m_gIKEnable)
		{
			//GATSO_PHYSICS_STOP("IKFootPlacement::Update");
			return;
		}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		
		
		//----------------------------------------------------------------------
		// GENERIC PARAMETERS
		//----------------------------------------------------------------------
		// Matrix to transform from world to model space
		CMatrix modelToWorld = m_pobHierarchy->GetRootTransform()->GetWorldMatrix();
		CMatrix worldToModel = modelToWorld.GetAffineInverse();

		DrawIKParameters(modelToWorld);

		//----------------------------------------------------------------------
		// SETUP IK INPUT
		//----------------------------------------------------------------------
		// To update the foot position, the solver need to know about some details
		hsFootPlacementIkSolver::Input ikInput;

		// The original position and orientation of the ankle (the one we base the foot placement on)
		Transform*	originalTrf				= m_pobHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_L_ANKLE);
		CMatrix		originalMat				= originalTrf->GetWorldMatrix() * worldToModel;
        hkQsTransform originalAnkleTransformLeftMS( Physics::MathsTools::CVectorTohkVector( CVector(originalMat.GetTranslation()) ),
													Physics::MathsTools::CMatrixTohkQuaternion( originalMat ));

		ikInput.m_originalAnkleTransformMS	= originalAnkleTransformLeftMS;

		// The transform that converts from model to world
		originalTrf					= m_pobHierarchy->GetRootTransform();
		originalMat					= originalTrf->GetWorldMatrix();
		ikInput.m_worldFromModel	= hkQsTransform(	Physics::MathsTools::CVectorTohkVector( CVector(originalMat.GetTranslation()) ),
														Physics::MathsTools::CMatrixTohkQuaternion( originalMat ));

		// If false, foot placement will be eased out/kept off, otherwise it will be eased in/kept on
		ikInput.m_footPlacementOn = valid;


		// Gain used when transitioning from foot placement on and off. Default value is 0.2f
		ikInput.m_onOffGain = onOffGain;

		// Gain used when the ground height is increasing (going up). Default value is 1.0f
		ikInput.m_groundAscendingGain = groundAscendingGain;

		// Gain used when the ground height is decreasing (going downp). Default value is 1.0f
		ikInput.m_groundDescendingGain = groundDescendingGain;

		// Gain used when the foot is fully planted (as defined in Setup::m_footPlantedAnkleHeightMS). Depending on the height of the ankle,
		// a value is interpolated between m_footPlantedGain and m_footRaisedGain and then multiplied by the ascending/descending gain to give
		// the final gain used. Default (and most common) value is 1.0f
		ikInput.m_footPlantedGain = footPlantedGain;

		// Gain used when the foot is fully raise (as defined in Setup::m_footRaisedAnkleHeightMS). Depending on the height of the ankle,
		// a value is interpolated between m_footPlantedGain and m_footRaisedGain and then multiplied by the ascending/descending gain to give
		// the final gain used. Default value is 1.0f.
		ikInput.m_footRaisedGain = footRaisedGain;
		
		//----------------------------------------------------------------------
		// GET THE CURRENT ANIMATION POSE
		//----------------------------------------------------------------------
		
		
		// Pose data
		const int numBones = m_pobSkeleton->m_numBones;
		hkLocalArray<hkQsTransform> animatedPose(numBones);
		animatedPose.setSizeUnchecked(numBones);

		// Set the current animated pose data
		HierarchyTools::GetLocalPoseRecursive(animatedPose, m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild());

		// Construct the pose
		m_pobAnimPose->setPoseLocalSpace(animatedPose); 

		// DrawCurrentSkeletonPose(animPose, modelToWorld, hkColor::BLUE);
		//----------------------------------------------------------------------
		// DO THE IK MAGIC
		//----------------------------------------------------------------------
		hsFootPlacementIkSolver::Output ikOutput;

		// So now, update the IK solver
        IKFootRayCast rayCast;
		ikInput.m_raycastInterface = &rayCast;
		m_pobLeftIK->startFootPlacement (ikInput, ikOutput, *m_pobAnimPose);
		float errorLeft = ikOutput.m_verticalError * ikOutput.m_footPlantWeight;
		int miss = 0;
		if( !ikOutput.m_hitSomething )
		{
			errorLeft = 0.0f;
			miss++;
		}


		originalTrf							= m_pobHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_R_ANKLE);
		originalMat							= originalTrf->GetWorldMatrix() * worldToModel;
		hkQsTransform originalAnkleTransformRightMS( Physics::MathsTools::CVectorTohkVector( CVector(originalMat.GetTranslation()) ),
													Physics::MathsTools::CMatrixTohkQuaternion( originalMat ));

		ikInput.m_originalAnkleTransformMS	= originalAnkleTransformRightMS;	
		ikInput.m_raycastInterface = &rayCast;
		
		m_pobRightIK->startFootPlacement (ikInput, ikOutput, *m_pobAnimPose);
		float errorRight = ikOutput.m_verticalError * ikOutput.m_footPlantWeight;
		if( !ikOutput.m_hitSomething )
		{
			errorRight = 0.0f;
			miss++;
		}
		float ntError = errorRight;
		if( errorLeft < errorRight )
		{
			ntError = errorLeft;
		}

		if( miss == 2 )
		{
			ikInput.m_footPlacementOn = false;
		}
		

		//----------------------------------------------------------------------
		// PELVIS CORRECTION
		//----------------------------------------------------------------------
		
		hkQsTransform pelvis		= m_pobAnimPose->getBoneModelSpace(0);
		hkVector4 pelvisTranslation	= pelvis.getTranslation();
		float animatedPelvisPos		= pelvisTranslation(1);
		float targetPelvisPos		= animatedPelvisPos;
		if( m_resetPelvisY )
		{
			m_previousPlevisY = targetPelvisPos;
			m_resetPelvisY = false;
		}
		if( valid )
			 targetPelvisPos		+= ntError;
		float difference			= targetPelvisPos - m_previousPlevisY;
		
		float pelvisCorrectionMaxDiff = 1.0f * timestep; // beware that max change speed is set 1 m/s
		if (pelvisCorrectionMaxDiff > fabs(difference))
			pelvisTranslation(1) = m_previousPlevisY + difference;
		else
			pelvisTranslation(1) = m_previousPlevisY + ((difference > 0) ? pelvisCorrectionMaxDiff : -pelvisCorrectionMaxDiff);

		pelvis.setTranslation( pelvisTranslation );
		m_pobAnimPose->setBoneModelSpace( 0, pelvis, hkPose::PROPAGATE );
		m_previousPlevisY = m_pobAnimPose->getBoneModelSpace(0).getTranslation()(1);

		//----------------------------------------------------------------------
		// IK CORRECTION
		//----------------------------------------------------------------------

		ikInput.m_originalAnkleTransformMS	= originalAnkleTransformRightMS;

		m_pobRightIK->finishFootPlacement (ikInput, *m_pobAnimPose);

		ikInput.m_originalAnkleTransformMS	= originalAnkleTransformLeftMS;

		m_pobLeftIK->finishFootPlacement (ikInput, *m_pobAnimPose);


		DrawCurrentSkeletonPose(*m_pobAnimPose, modelToWorld, hkColor::RED);

		HierarchyTools::SetAllLocalPoseRecursive( m_pobAnimPose->getPoseLocalSpace(), m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild() );

		//GATSO_PHYSICS_STOP("IKFootPlacement::Update");
		
#endif
		
	}





	//! Constructors / Destructors 
	IKLookAt::IKLookAt(CHierarchy* pobHierarchy, CEntity* current)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		:m_pobHierarchy(pobHierarchy),
		m_pobSkeleton(0)
#endif
	{
#ifdef _PS3_RUN_WITHOUT_HAVOK_BUILD
		UNUSED( current );
		UNUSED( pobHierarchy );
#endif
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_currentEntity = current;
		m_lookAtEntity = 0;
		m_lookAtPosition(3) = -1.0f;

		ntError( pobHierarchy != NULL );
		ntError( current != NULL );
		ntError( pobHierarchy->GetRootTransform() != NULL );

		m_pobSkeleton = HK_NEW hkSkeleton();
		m_pobSkeleton->m_numParentIndices	= HierarchyTools::NumberOfTransform( m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild() );// IKNumberOfTransform(m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild(), -1 );
		m_pobSkeleton->m_numBones			= m_pobSkeleton->m_numParentIndices;
		m_pobSkeleton->m_parentIndices		= NT_NEW hkInt16[m_pobSkeleton->m_numParentIndices];

		/*SetIKSkeletonHierarchyRecursive(	m_pobHierarchy, m_pobSkeleton->m_parentIndices, 
											m_pobHierarchy->GetRootTransform()->GetFirstChild(), 
											-1);*/
		HierarchyTools::SetSkeletonRecursive( m_pobSkeleton->m_parentIndices, m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild() );

		m_pobSkeleton->m_bones = hkAllocate<hkBone*> (m_pobSkeleton->m_numParentIndices, HK_MEMORY_CLASS_UTILITIES );

		m_pobSkeleton->m_referencePose		= HK_NEW hkQsTransform[m_pobSkeleton->m_numParentIndices];
		m_pobSkeleton->m_numReferencePose	= m_pobSkeleton->m_numParentIndices;

		HierarchyTools::SetSkeletonBonesRecursive( m_pobSkeleton->m_bones, m_pobHierarchy, m_pobSkeleton->m_referencePose, m_pobHierarchy->GetRootTransform()->GetFirstChild() );

		m_lookAtLastTargetWS.setZero4();
		m_lookAtWeight = 0.0f;
#else
		UNUSED( pobHierarchy );
#endif
	}

	IKLookAt::~IKLookAt()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		NT_DELETE_ARRAY( m_pobSkeleton->m_parentIndices );
		HK_DELETE_ARRAY( m_pobSkeleton->m_referencePose );

		for(int i = 0; i < m_pobSkeleton->m_numBones; i++ )
		{
			NT_DELETE_ARRAY( m_pobSkeleton->m_bones[i]->m_name );
			HK_DELETE( m_pobSkeleton->m_bones[i] );
		};

		HK_DELETE( m_pobSkeleton );
		m_pobSkeleton = 0;

		m_pobHierarchy = 0;
#endif
	}

	void IKLookAt::SetLookAtEntity( const CEntity* entity )
	{
#ifdef _PS3_RUN_WITHOUT_HAVOK_BUILD
		UNUSED( entity );
#endif
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if( m_currentEntity != entity)
		{
			m_lookAtEntity = entity;
			m_lookAtPosition(3) = -1.0f;
		} else {
			m_lookAtEntity = 0;
			m_lookAtPosition(3) = -1.0f;
		}
#endif
	}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	void IKLookAt::SetLookAtPosition( const hkVector4& pos )
	{
		m_lookAtEntity = 0;
		m_lookAtPosition = pos;
		m_lookAtPosition(3) = 1.0f;

	}
#endif

		//! Public methods
	void IKLookAt::Update()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if( ( 0 == m_lookAtEntity ) && ( m_lookAtPosition(3) == -1.0f) )
			return;


	
		static hkInt16 HEAD	= 5;

		// Matrix to transform from world to model space
		ntError( m_pobHierarchy != NULL );
		ntError( m_pobHierarchy->GetRootTransform() != NULL );
		CMatrix modelToWorld = m_pobHierarchy->GetRootTransform()->GetWorldMatrix();
		CMatrix worldToModel = modelToWorld.GetAffineInverse();

		// --------------------------------------------------------------
		// Let's get the current pose
		const int numBones = m_pobSkeleton->m_numBones;
		hkLocalArray<hkQsTransform> animatedPose(numBones);
		animatedPose.setSizeUnchecked(numBones);

		// Set the current animated pose data
		//SetIKPoseRecursive(m_pobHierarchy, animatedPose, m_pobHierarchy->GetRootTransform()->GetFirstChild(), -1);
		HierarchyTools::GetLocalPoseRecursive(animatedPose, m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild());

		// Construct the pose
		hkPose thePose(m_pobSkeleton);
		thePose.setPoseLocalSpace(animatedPose); 

		// --------------------------------------------------------------
		Transform*	originalTrf		= m_pobHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_HEAD);
		ntError( originalTrf != NULL );
		CMatrix		HeadModelSpace	= originalTrf->GetWorldMatrix() * worldToModel;

		originalTrf					= m_pobHierarchy->GetCharacterBoneTransform(CHARACTER_BONE_NECK);
		ntError( originalTrf != NULL );
		CMatrix		NeckModelSpace	= originalTrf->GetWorldMatrix() * worldToModel;

		// Current transforms (MS) of the head and neck
		const hkQsTransform headMS	= hkQsTransform(	Physics::MathsTools::CVectorTohkVector( CVector(HeadModelSpace.GetTranslation()) ),
														Physics::MathsTools::CMatrixTohkQuaternion( HeadModelSpace ));

		const hkQsTransform neckMS	= hkQsTransform(	Physics::MathsTools::CVectorTohkVector( CVector(NeckModelSpace.GetTranslation()) ),
														Physics::MathsTools::CMatrixTohkQuaternion( NeckModelSpace ));
		
		// Position of the head (MS)
		const hkVector4& headPosMS = headMS.getTranslation();

		// Forward-looking direction (MS)
		hkVector4 forwardDirMS; forwardDirMS.setRotatedDir(headMS.getRotation(), hkVector4(0,0,1));
		
		const hkQsTransform worldFromModel	= hkQsTransform(	Physics::MathsTools::CVectorTohkVector( CVector(modelToWorld.GetTranslation()) ),
																Physics::MathsTools::CMatrixTohkQuaternion( modelToWorld ));

		hkVector4 targetPosWS;
		if( m_lookAtEntity )  
		{
			if( ( m_lookAtEntity->IsEnemy() ) || ( m_lookAtEntity->IsPlayer() ) )
			{
				originalTrf		= m_lookAtEntity->GetHierarchy()->GetCharacterBoneTransform(CHARACTER_BONE_SPINE_01);
				CMatrix	PlayerPosition	= originalTrf->GetWorldMatrix();
				targetPosWS =  Physics::MathsTools::CVectorTohkVector( CVector(PlayerPosition.GetTranslation()));
			} 
			else
			{
				targetPosWS =  Physics::MathsTools::CPointTohkVector( m_lookAtEntity->GetPosition() );
			}
		} else
		{ 
			targetPosWS = m_lookAtPosition;
		}

		bool lookAtOn = true;
		// We want to look at something; but we shouldn't try to look at something behind the character
		if (lookAtOn)
		{
			hkVector4 targetPosMS; targetPosMS.setTransformedInversePos(worldFromModel, targetPosWS);

			hkVector4 headToTargetDirMS; headToTargetDirMS.setSub4(targetPosMS, headPosMS);
			if (headToTargetDirMS.dot3(forwardDirMS)<0.0f)
			{
				// Target is behind us, ignore it
				lookAtOn = false;
			}
		}

		// Still we want to look at something
		if (lookAtOn)
		{
			// If we were doing look at before, blend the new position
			if (m_lookAtWeight>0.0f)
			{
				const hkReal targetGain = 0.4f;
				m_lookAtLastTargetWS.setInterpolate4(m_lookAtLastTargetWS, targetPosWS, targetGain);
			}
			else
			{
				// Otherwise use this new position straight away
				m_lookAtLastTargetWS = targetPosWS;
			}
		} 

		// Ease in/out the weight depending on whether there is something we want to look at
		const hkReal desiredWeight = lookAtOn ? 1.0f : 0.0f;
		m_lookAtWeight += (desiredWeight - m_lookAtWeight) * 0.3f;

		// If weight gets under threshold, do nothing
		if (m_lookAtWeight < 0.01f)
		{
			m_lookAtWeight = 0.0f;
		}
		else
		{
			// Use the LookAt Ik solver to modify the orientation of the head
			// Use the weight the gain, so we move smoothly over time
			hkLookAtIkSolver::Setup setup;
			setup.m_fwdLS.set(0,0,1);
			setup.m_limitAxisMS.setRotatedDir(neckMS.getRotation(), hkVector4(0, 0.34f ,1.0f));
			setup.m_limitAxisMS.normalize3();
			setup.m_limitAngle = 0.7853f;

			hkVector4 targetMS; targetMS.setTransformedInversePos(worldFromModel, m_lookAtLastTargetWS);

			// By using hkPose and the PROPAGATE flag we automatically update the descendant bones of the head
			hkLookAtIkSolver::solve(setup, targetMS, m_lookAtWeight, thePose.accessBoneModelSpace(HEAD, hkPose::PROPAGATE));
		}

		// Update the hierarchy
		HierarchyTools::SetLocalPoseRecursive( thePose.getPoseLocalSpace(), m_pobHierarchy, m_pobHierarchy->GetRootTransform()->GetFirstChild(), 0 );
#endif
	}
}
