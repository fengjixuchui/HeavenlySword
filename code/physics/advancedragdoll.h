#ifndef _ADVANCED_RAGDOLL
#define _ADVANCED_RAGDOLL

#include "config.h"
#include "havokincludes.h"
#include "collisionlistener.h"
#include "world.h"
#include "rigidbody.h"

class CEntity;
class CBindPose;
class hkSkeleton;
class hkString;
class hsRagdollInstance;
class hkSkeletonMapper;
class hkRagdollPoweredConstraintController;
class hkRagdollRigidBodyController;
class hkRigidBody;
class hkAllRayHitCollector;
class hkPhysicsSystem;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkbase/memory/hkLocalArray.h>
#include <hkmath/hkMath.h>
#include <hkdynamics/phantom/hkAabbPhantom.h>
#endif


namespace Physics {

	class RagdollAntiGravityAction;
	class RagdollUberGravityAction;
	class RagdollCollisionListener;
	class ActivatePhantom;

	enum RAGDOLL_STATE
		{
			DEACTIVATED,
			TRANSFORM_TRACKING,
			TRANSFORM_TRACKING_ANIMATED,
			ANIMATED,
			ANIMATED_ABSOLUTE,	
			DEAD
		};

	bool IsStateAnimated(RAGDOLL_STATE state);

	class AdvancedRagdoll
	{
	public:		

		// This is used purely for interfacing to allow individual bones to be specified
		// It is translated using the integers defined privately below
		enum RAGDOLL_BONES
		{
			PELVIS = 1 << 0,
			SPINE = 1 << 1,
			UPPER_ARM_LEFT = 1 << 2,
			UPPER_ARM_RIGHT = 1 << 3,
			LOWER_ARM_LEFT = 1 << 4,
			LOWER_ARM_RIGHT = 1 << 5,
			HEAD = 1 << 6,
			UPPER_LEG_LEFT = 1 << 7,
			UPPER_LEG_RIGHT = 1 << 8,
			LOWER_LEG_LEFT = 1 << 9,
			LOWER_LEG_RIGHT = 1 << 10,
			ALL_BONES = 1 << 11
		};

		hkRigidBody* GetRagdollBone( int index );

		// Fundamentals...
		AdvancedRagdoll( Character* p_entity, const char* p_filename );		//< Build a ragdoll.
		~AdvancedRagdoll();

		void	Activate(	RAGDOLL_STATE p_state );	//< Activate the ragdoll with a default state.
		void	Deactivate(bool bPermanently = false);
		bool	IsActive();
		void	PausePresenceInHavokWorld(bool pause);

		void	Update(	float deltaTime, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform );
		RAGDOLL_STATE	GetState( );

		void ResetEntireRagdoll(bool bPositions, bool bVelocities, bool squiz);
		void SetAnimatedBones(int iBoneFlags);
		void AddAnimatedBone(int iBoneFlag);
		int GetAnimatedBones() { return m_iAnimatedBoneFlags; };
		int GetBoneIndexFromFlag(int iFlag);
		void SetFullRagdoll();
		void ForceFreeze();
		//void CollideAtAll( bool bCollide );
		void SetBoneBoneCollision( bool bCollide );
		//void FeetCollide( bool mode );
		void CalculateMotionStatus( void );
		float GetMotionStatus( void ) 		{ if ( m_fCumulativeLinearVelocitySqr < 0.0f ) CalculateMotionStatus(); return m_fCumulativeLinearVelocitySqr; }
		bool IsMoving( void ) const			{ return m_bMoving; }

		void RegisterImpact()
		{
			m_bRegisterImpact = true;
			Activate( DEAD );
		}
		
		CDirection GetAngularVelocity();
		void SetAngularVelocity( const CDirection& obAngularVelocity );
		void SetLinearVelocity( const CDirection& obLinearVelocity );
		void ApplyAngularImpulse( const CDirection& obAngularImpulse );
		void ApplyLinearImpulse( const CDirection& obLinearImpulse );
		void SetPelvisLinearVelocity( const CDirection& obLinearVelocity );
		void AddLinearVelocity( const CDirection& obLinearVelocity );
		CDirection GetLinearVelocity();
		void ApplyLocalisedLinearImpulse (const CDirection& obImpulse,const CVector& obPoint);
		void ApplyImpulseToBody(int iHitArea, const CPoint& obPosition, const CDirection& obImpulse);
		void ApplyForce(const CDirection& obForce);
		CPoint GetPosition();

		void Debug_RenderCollisionInfo ();
		void Debug_RenderCollisionInfo_Animated ();
		
		// Gets the hierarchy root transfom world matrix from before any ragdoll animation was applied
        CMatrix& GetWorldMatrixBeforeUpdate() { return m_obWorldMatrixBeforeUpdate; }

		void SetTurnDynamicOnContact( bool bDynamic ) 	{ m_bTurnDynamicOnContact = bDynamic; };
		bool GetTurnDynamicOnContact( void ) 			{ return m_bTurnDynamicOnContact; };
		void SetRagdollTrajectory( float fDistMultiplier, float fAngleMultiplier );
		void SetRagdollThrown( bool bThrown );
		bool GetRagdollThrown( void )					{ return m_bRagdollIsThrown; }
		void AimAtPlayer( void );
		void SetRagdollHeld( bool bHeld );
		Character* GetEntity( ) const 					{ return m_entity; };	

		void SetSendMessageOnAtRest(const char* pcMsg)	{ m_pcAtRestMsg = pcMsg;}

		CMatrix GetIdealEntityWorldMatrix()				{ return m_obIdealEntityRootMatrix; }
		bool IsInContactWithGround();

		void SetBoneTransformTrackingMapping(int iBoneFlag, const Transform* pobTransform);

		float GetActiveTime() 							{ return m_fActiveTime; }

		void SetExemptFromCleanup(bool bExemption);
		bool GetExemptFromCleanup() const 				{ return m_bExemptFromDisposal; }

		void SetAntiGravity(bool bAntiGravity);
		bool GetAntiGravity();

		void SetUberGravity(bool bUberGravity);
		bool GetUberGravity();

		void Twitch();

		// Can be simulaiton swith from dynamical to static phantom
		bool CanBePhantom() const;
		bool IsPhantom() const {return m_bIsPhantom;}
		void DynamicToPhantom();
		void PhantomToDynamic();
		void PhantomToDynamicNextUpdate() {m_bPhantomToDynamicNextUpdate = true;};
		void UnfixBones(); // JML - Added for respawning, please tidy as necessary

		// can be removed from scene because of performance issues
		bool CanBeRemoved() ;

		void SetKOState( KO_States state );
		void SetCharacterDead(bool dead = true); //< character corresponding to ragdoll just died. 
		
		void UpdateCollisionFilter();

		bool CastRayOnAnimated(const CPoint &obSrc, const CPoint &obTarg, Physics::RaycastCollisionFlag obFlag, TRACE_LINE_QUERY& stQuery); 
	
	
	private:
		// Temporary measure, this is exposed to friends so they get direct access to ragdoll
		friend class SpearThrown;
		friend class RagdollCollisionListener;
		friend class ActivatePhantom;

		// Constraint data cache to allow uis to adjust the waist joint when being held.
		static float m_fWaistPlaneAngle;
		static float m_fWaistTwistAngle;
		
		CMatrix m_obIdealEntityRootMatrix;
		CMatrix m_obWorldMatrixBeforeUpdate; 
		
		const char* m_pcAtRestMsg;
		float m_fActiveTime;
		float m_fNonPhantomTime;
		
		// Send at-rest messages.
		void FlagAtRest( void );

		// Public interface should go through Activate
		void SetState(	RAGDOLL_STATE p_state );	//< Set the ragdoll state.

		struct BoneTransformTrackingMapping
		{
			bool m_bIsActive;
			const Transform* m_pobTransformToTrack;
		};
		BoneTransformTrackingMapping* m_aobBoneTransformTracking;
		void SetupBoneTransformTrackingArray();
		void ResetBoneTransformTrackingArray();

		// Indices of our bones
		int PELVIS_BONE;
		int L_LEG_BONE;
		int R_LEG_BONE;
		int SPINE_BONE;
		int L_KNEE_BONE;
		int R_KNEE_BONE;
		int HEAD_BONE;
		int R_FORARM_BONE;
		int L_FORARM_BONE;
		int R_ELBOW_BONE;
		int L_ELBOW_BONE;
		int WAIST_JOINT;
		
		// Thrown ragdoll stuff.
		CPoint 	m_ptThrowOrigin;				// Origin of ragdoll trajectory.
		float 	m_fForceDistanceMultiplier;		// For adjusting the trajectory of thrown ragdolls.
		float 	m_fAngleDistanceMultiplier;		// For adjusting the trajectory of thrown ragdolls.
		float 	m_fCumulativeLinearVelocitySqr;	// Movement state.
		
		// Flags.
		bool 	m_bFirstTime:1;
		bool 	m_bPermanentlyDeactivated:1;
		bool 	m_bRegisterImpact:1;
		bool 	m_bRagdollIsThrown:1;			// Is this ragdoll being thrown at the player?
		bool 	m_bTumbleContact:1;				// Has the ragdoll hit the landscape this frame?
		bool 	m_bTurnDynamicOnContact:1; 		// Go all ragdolly if we hit something?
		bool	m_bRagdollIsHeld:1;				// Is the ragdoll being held?
		bool	m_bIsActive:1;
		bool    m_bIsPhantom:1; 				// If yes ragdoll is replaced by phantom. 
		bool	m_bMoving:1;
		bool 	m_bExemptFromDisposal:1;
		bool 	m_bSendRagdollFloored:1;
		bool 	m_bSetDeadNextUpdate:1;
		bool 	m_bDoParticleEffectNextUpdate:1;
		bool 	m_bDoSoundNextUpdate:1;
		bool	m_bPhantomToDynamicNextUpdate:1;
		
		
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// Flags for which bones we should be dragging to whereever the animation is
		int m_iAnimatedBoneFlags;
		hkArray<int> m_aiBonesToAnimate;
		hkArray<KeyframedMotionParams> m_lastPose; // used for animated ragdolls. 

		RAGDOLL_STATE	m_currentState;		//< Ragdoll State.

		hkSkeleton*						m_AnimatedSkeleton;		// This skeleton represents the high-res animated skeleton.
		hkSkeleton*						m_RagdollSkeleton;		// And this one the low res physical one
		hsRagdollInstance*				m_RagdollInstance;		// This is our extended Havok ragdoll container
		hkSkeletonMapper*				m_AnimatedToRagdollMapper; // This maps from the high-res skeleton to the low-res
		hkSkeletonMapper*				m_RagdollToAnimatedMapper; // ...and vice versa
		
		RagdollAntiGravityAction*		m_pobAntiGravityAction;
		RagdollUberGravityAction*		m_pobUberGravityAction;

		RagdollCollisionListener*		m_listener;
		ActivatePhantom*				m_phantom;

		PhysicsDataRef				m_loaderDataRef;    //!< Reference to data used to create lg

		/*static float maxPosPowerHinge;
		static float maxPosPowerTwist;
		static float maxPosPowerCone;
		static float maxPosPowerPlane;
		static float maxNegPowerHinge;
		static float maxNegPowerTwist;
		static float maxNegPowerCone;
		static float maxNegPowerPlane;
		static float m_maxSpringConstant;
		static float m_maxSpringDamping;*/
		
		void SetupRagdoll( const char* p_filename );
		void LoadRagdoll(  const char*  filename );
		void SetupAnimatedSkeleton();
		void SetupRigidBodies();
		void SetupMappers();

		void ResetRagdollPoseToMatchCurrentHierarchy();
		void TransformTrackingAnimation( hkArray<hkQsTransform>& ragdollWorldPoseIn, hkArray<hkQsTransform>& animatedWorldPoseOut );
		void FullyAnimatedRagdollAnimation( hkArray<hkQsTransform>& ragdollWorldPoseIn, hkArray<hkQsTransform>& animatedWorldPoseOut );
		void AnimationPelvisMappedAsRoot( hkArray<hkQsTransform>& ragdollWorldPoseIn, hkArray<hkQsTransform>& animatedWorldPoseOut );
		
		void DriveRagdollBonesToAnimation( const hkArray<hkQsTransform>& aobPoseToDriveTo, hkArray<int>& aiBonesToAnimate, float fFrameDelta );
		void ApplyDriveToPoseAsynchronously( const hkVector4& nextPosition, const hkQuaternion& nextOrientation, hkReal invFrameTime, hkReal invPhysicsTime, hkRigidBody* body);

		//bool TestPenetrations();
		//void CheckBetweenBonesAndApplyCorrection(hkRigidBody* pobBone1, hkRigidBody* pobBone2, hkAabbPhantom* pobPhantom);
		//bool TestBonePenetration(hkRigidBody* pobBone);
		//void CheckBetweenEachBoneAndApplyCorrection();
		//bool CheckBonePenetrationsAndApplyCorrection(hkRigidBody* rbody);
		//bool CheckAllBonePenetrationsAndApplyCorrection();
		//bool CheckForStaticGeometry(hkAllRayHitCollector& obRayCollector, hkVector4& obRayContactNormal, float& fDistanceFrom);
		bool CheckBoneInContactWithWorld(hkRigidBody* rbody);

		Character*	m_entity;			//!< Associated entity.
		CPoint m_obPelvisPositionLastFrame;

		void DoSound();
		void DoParticleEffect();

		CMatrix m_obParticleEffectMatrix;
		float m_fSoundProjectedVelocity;
		void* m_pSparksDef;

		void CalculateRagdollFromHierarchy(hkPhysicsSystem* pobRagdollSystem);
		/*CMatrix MultiplyBindPoseToRoot(int iIndex, CMatrix& obBindPoseBoneMatrix);
		CMatrix GetBindPoseAsCMatrix(const CBindPose& obBone);*/

		enum StateBeforePause
		{
			Undefined = 0,
			NotInWorld,
			Deactivated,
			Activated
		};

		StateBeforePause m_stateBeforePause; 
#endif	

	};

#define USUAL_DYNAMIC_RAGDOLLS 4 // try to keep maximaly USUAL_DYNAMIC_RAGDOLLS ragdolls in scene simulated as dynamic
#define MAX_DYNAMIC_RAGDOLLS 4 // allow simulation of maximaly MAX_DYNAMIC_RAGDOLLS ragdolls. Number of simulated ragdolls 
	                           // can exceed this number but if it is possible it tries to keep it under this number
#define MAX_RAGDOLLS 8 			// try to keep maximaly MAX_RAGDOLLS ragdolls in scene
    /** The ragdolls simulation is quite CPU intensive. We want to reduce number of ragdolls simulating at once. 
	  * That is a role of this Manager. 
	  * 
	  * Ragdolls can be simulated:
	  * - as a dynamic body -- full simulation.
	  * - as a phantom -- they are not simulated. They will be switched back to dynamic state, only if:
	  *                        -- phantom collides with person capsule
	  *                        -- phantom collides with dynamic rigid body and there is some power left for simulation. 
	  *                            (less than MAX_DYNAMIC_RAGDOLLS are currently in scene). 
	  */
	class RagdollPerformanceManager : public Singleton<RagdollPerformanceManager> 
	{
	public:
		RagdollPerformanceManager();

		void Clear();
		void Update();		

		// do we have power to change phantom to dynamic in optional case
		bool CanPhantomToDynamic() const;		
		void AddDynamicRagdoll(AdvancedRagdoll * ragdoll);
		void AddPhantomRagdoll(AdvancedRagdoll * ragdoll);		
		void AddAnimatedRagdoll(AdvancedRagdoll * ragdoll);
		
		void RemoveDynamicRagdoll(AdvancedRagdoll * ragdoll);		
		void RemovePhantomRagdoll(AdvancedRagdoll * ragdoll);
		void RemoveAnimatedRagdoll(AdvancedRagdoll * ragdoll);
		
		void PhantomToDynamicRagdoll(AdvancedRagdoll * ragdoll);
		void DynamicToPhantomRagdoll(AdvancedRagdoll * ragdoll);	
		void AnimatedToDynamicRagdoll(AdvancedRagdoll * ragdoll);
		void PhantomToAnimatedRagdoll(AdvancedRagdoll * ragdoll);
		void DynamicToAnimatedRagdoll(AdvancedRagdoll * ragdoll);

		bool CanAnimated();		

	protected:
		ntstd::Vector<AdvancedRagdoll *> m_dynamicRagdolls;
		ntstd::Vector<AdvancedRagdoll *> m_phantomRagdolls;
		int m_nAnimatedRagdolls; // we do not need to know pointers to animated ragdolls only their number.
	};

} // namespace Physics

#endif // _ADVANCED_RAGDOLL
