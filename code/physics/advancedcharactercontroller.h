//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/advancedcharactercontroller.h
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.25
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _ADVANCED_CHARACTER_CONTROL
#define _ADVANCED_CHARACTER_CONTROL

#include "config.h"
#include "havokincludes.h"

class CEntity;
class Character;
class CHierarchy;

#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"
#include "logicgroup.h"

#include <hkdynamics/phantom/hkPhantomOverlapListener.h>

// This is a relatively simple class to handle combat physics volumes
// At the moment they just push according to user spec vals, but could get a whole lot more funky soon
class hkShapePhantom;
class hkVector4;
class hkRigidBody;
class CombatPhysicsPushVolumeDescriptor
{
	HAS_INTERFACE( CombatPhysicsPushVolumeDescriptor )

	
public:
	CombatPhysicsPushVolumeDescriptor()
		: m_obBindToTransform(),
		m_fSphereRadius(0.0f),
		m_fCapsuleRadius(0.0f),
		m_fCapsuleLength(0.0f),
		m_fBoxHalfExtentX(0.0f),
		m_fBoxHalfExtentY(0.0f),
		m_fBoxHalfExtentZ(0.0f),
		m_fXAxisPositionOffset(0.0f),
		m_fYAxisPositionOffset(0.0f),
		m_fZAxisPositionOffset(0.0f),
		m_fXAxisRotationOffset(0.0f),
		m_fYAxisRotationOffset(0.0f),
		m_fZAxisRotationOffset(0.0f),
		m_bPushOnce( true ),
		m_bUseMassMultiplier( true ),
		m_bUseMagnitudeMultiplier( true ),
		m_bUseMovementOfVolume( true ),
		m_bUseFromCentreOfVolume( true ),
		m_bCanCollapseCollapsablesWhenNotAttacking( false ),
		m_bCanCollapseCollapsablesWhenAttacking( false ),
		m_bOnlyActiveWhenAttacking( false ),
		m_fPushFactorLargeInteractable(0.5f),
		m_fPushFactorSmallInteractable(0.4f),
		m_fPushFactorRagdoll(1.5f),
		m_fPushFactorCollapse(5.0f),
		m_bShouldStrikeCharactersWhenAttacking( false )
	{		
	}

	// Transform to bind to
	CHashedString m_obBindToTransform;
	
	// The first non-zero values in the following 3 sets of floats will decide which shape is wanted
	// E.g. if you want a box, leave sphere and capsule values at 0.0f and fill in box values
	float m_fSphereRadius;
	float m_fCapsuleRadius, m_fCapsuleLength;
	float m_fBoxHalfExtentX, m_fBoxHalfExtentY, m_fBoxHalfExtentZ;

	// Any offset along the XYZs of the transform
	float m_fXAxisPositionOffset, m_fYAxisPositionOffset, m_fZAxisPositionOffset;
	float m_fXAxisRotationOffset, m_fYAxisRotationOffset, m_fZAxisRotationOffset;

	// Modifiers for push calculation
	bool m_bPushOnce, m_bUseMassMultiplier, m_bUseMagnitudeMultiplier, m_bUseMovementOfVolume, m_bUseFromCentreOfVolume, m_bCanCollapseCollapsablesWhenNotAttacking, m_bCanCollapseCollapsablesWhenAttacking, m_bOnlyActiveWhenAttacking;

	// How much should this push?
	float m_fPushFactorLargeInteractable, m_fPushFactorSmallInteractable, m_fPushFactorRagdoll, m_fPushFactorCollapse;

	// For strike 2 generation
	bool m_bShouldStrikeCharactersWhenAttacking;	
};

// Data of ombatPhysicsPushVolume that must be instanced per each entity, so we have store them in CAttackComponent
class CombatPhysicsPushVolumeData
{
public:
	struct RBAgePair
	{
	public:
		RBAgePair()
		{
			pobRB = 0;
			iAge = 0;
		}

		hkRigidBody* pobRB;
		int iAge;
	};

	CombatPhysicsPushVolumeData() : 
	  m_pobPhantom(0),		
	  m_obPreviousPosition(0,0,0,0)
	  {
		  ResetAllPreviousRBs();
	  };


	int GetNumPreviousRBs()
	{
		return 8;
	}

	void DecrementRBAge(hkRigidBody* pobRB)
	{
		for (int i = 0; i < GetNumPreviousRBs(); i++)
		{
			if (pobRB == m_apobPreviousRBs[i].pobRB)
				m_apobPreviousRBs[i].iAge--;
		}
	}

	bool IsRBInPrevious(hkRigidBody* pobRB)
	{
		for (int i = 0; i < GetNumPreviousRBs(); i++)
		{
			if (pobRB == m_apobPreviousRBs[i].pobRB)
				return true;
		}

		return false;
	}

	void AddPreviousRB(hkRigidBody* pobBody)
	{
		m_apobPreviousRBs[m_iNextRBSpace].pobRB = pobBody;
		m_apobPreviousRBs[m_iNextRBSpace].iAge = 0;
		m_iNextRBSpace++;
		if (m_iNextRBSpace == GetNumPreviousRBs())
			m_iNextRBSpace = 0; // Start from the beginning
	}

	void ResetAllPreviousRBs()
	{
		m_iNextRBSpace = 0;
		for (int i = 0; i < GetNumPreviousRBs(); i++)
		{
			m_apobPreviousRBs[i].pobRB = 0;
			m_apobPreviousRBs[i].iAge = 0;
		}
	}

	void IncrementPreviousRBsAge()
	{
		for (int i = 0; i < GetNumPreviousRBs(); i++)
		{
			if (m_apobPreviousRBs[i].pobRB)
				m_apobPreviousRBs[i].iAge++;
		}
	}

	void ResetPreviousRBsWithAgeGreaterThan(int iAge)
	{
		bool bNextRBSpaceSet = false;
		m_iNextRBSpace = 0;
		for (int i = 0; i < GetNumPreviousRBs(); i++)
		{
			if (m_apobPreviousRBs[i].iAge > iAge)
			{
				if (!bNextRBSpaceSet)
				{
					m_iNextRBSpace = i;
					bNextRBSpaceSet = true;
				}

				m_apobPreviousRBs[i].pobRB = 0;
				m_apobPreviousRBs[i].iAge = 0;
			}
		}
	}
	
	hkShapePhantom* m_pobPhantom;
	hkVector4 m_obPreviousPosition;
	RBAgePair m_apobPreviousRBs[8];
	int m_iNextRBSpace;
};

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeMovementDescriptor
*
*	DESCRIPTION	Pseudo-abstract base class to describe combat strike volume movements
*
***************************************************************************************************/
class CombatPhysicsStrikeVolumeMovement;
class CombatPhysicsStrikeVolumeMovementDescriptor
{
	HAS_INTERFACE(CombatPhysicsStrikeVolumeMovementDescriptor)
public:
	enum CPSVM_MOVEMENT_TYPE { CPSVM_STRAIGHT, CPSVM_TRANSFORM, CPSVM_COUNT };

	CombatPhysicsStrikeVolumeMovementDescriptor();
	virtual CPSVM_MOVEMENT_TYPE GetType() { return CPSVM_COUNT; };
	virtual void Initialise(CEntity* pobParent) {};
	virtual float GetStartTime(CEntity* pobParent) { UNUSED(pobParent); return m_fStartTime; }
	float GetTimeout() const { return m_fTimeout; }
	CPoint GetStartPosition() const { return m_obStartPosition; }
	virtual ~CombatPhysicsStrikeVolumeMovementDescriptor();
protected:
	CPoint m_obStartPosition;
	float m_fStartTime;
	float m_fTimeout;
	float m_fXAxisRotationOffset, m_fYAxisRotationOffset, m_fZAxisRotationOffset;

	//CPSVM_MOVEMENT_TYPE m_eType;

	friend class CombatPhysicsStrikeVolumeMovement;
};

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeMovement
*
*	DESCRIPTION	Pseudo-abstract base class to execute combat strike volume movements
*
***************************************************************************************************/
class CombatPhysicsStrikeVolumeMovement
{
public:
	CombatPhysicsStrikeVolumeMovement(CombatPhysicsStrikeVolumeMovementDescriptor* pobDesc);
	const CombatPhysicsStrikeVolumeMovementDescriptor* GetDescriptor() { return m_pobDescriptor; };
	virtual CPoint GetNextPosition( const CEntity* pobOwner, const CEntity* pobTarget, CPoint& obCurrentPosition, float fTimeDelta);
	virtual hkTransform GetInitialTransform( const CEntity* pobOwner ) { return hkTransform::getIdentity(); };
	void SetDeleteDescriptor(bool bDel) { m_bDeleteDescriptor = bDel; };
	virtual ~CombatPhysicsStrikeVolumeMovement();
protected:
	bool m_bDeleteDescriptor;

	const CombatPhysicsStrikeVolumeMovementDescriptor* m_pobDescriptor;
};

/***************************************************************************************************
*
*	FUNCTION		StrikeVolumeStraightMovementDescriptor
*
*	DESCRIPTION		
*
***************************************************************************************************/
class CombatPhysicsStrikeVolumeStraightMovement;
class CStrike;
class CombatPhysicsStrikeVolumeStraightMovementDescriptor : public CombatPhysicsStrikeVolumeMovementDescriptor
{
	HAS_INTERFACE(CombatPhysicsStrikeVolumeStraightMovementDescriptor)
public:
	CombatPhysicsStrikeVolumeStraightMovementDescriptor();	
	virtual CPSVM_MOVEMENT_TYPE GetType() { return CPSVM_STRAIGHT; };
	virtual ~CombatPhysicsStrikeVolumeStraightMovementDescriptor();
protected:	
	// Exposed:
	float m_fAngle;
	CDirection m_obTravelVector;
	float m_fSpeed;
	float m_fMaxHomingRotationSpeed;
	float m_fHomeWithinAngle;

	friend class CombatPhysicsStrikeVolume;
	friend class CombatPhysicsStrikeVolumeStraightMovement;
};

class CombatPhysicsTransformBasedMovementDescriptor : public CombatPhysicsStrikeVolumeMovementDescriptor
{
	HAS_INTERFACE(CombatPhysicsTransformBasedMovementDescriptor)
public:
	CombatPhysicsTransformBasedMovementDescriptor();	
	virtual void Initialise(CEntity* pobParent);
	virtual CPSVM_MOVEMENT_TYPE GetType() { return CPSVM_TRANSFORM; };
	virtual float GetStartTime(CEntity* pobParent);
	virtual ~CombatPhysicsTransformBasedMovementDescriptor() {};
protected:	
	CDirection m_obReferenceVector;

	// Exposed:
	CHashedString m_obTransform;
	float m_fSpeed;
	float m_fAngleDeltaTrigger;

	friend class CombatPhysicsStrikeVolume;
	friend class CombatPhysicsStrikeVolumeStraightMovement;
};

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeStraightMovement
*
*	DESCRIPTION	Concrete class to execute straight combat strike volume movements
*
***************************************************************************************************/
class CombatPhysicsStrikeVolumeStraightMovement : public CombatPhysicsStrikeVolumeMovement
{
public:
	CombatPhysicsStrikeVolumeStraightMovement(CombatPhysicsStrikeVolumeStraightMovementDescriptor* pobDesc);
	virtual CPoint GetNextPosition(const CEntity* pobOwner, const CEntity* pobTarget, CPoint& obCurrentPosition, float fTimeDelta);
	virtual hkTransform GetInitialTransform( const CEntity* pobOwner );
	virtual ~CombatPhysicsStrikeVolumeStraightMovement();
private:
	CDirection m_obTravelVector; // Copy of travel vector for each instance, calculated from descriptor
};

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeDescriptor
*
*	DESCRIPTION	Class to describe all aspects of a combat strike volume
*
***************************************************************************************************/
class CAttackData;
class CombatPhysicsStrikeVolumeDescriptor
{
	HAS_INTERFACE(CombatPhysicsStrikeVolumeDescriptor)
public:
	CombatPhysicsStrikeVolumeDescriptor();
	~CombatPhysicsStrikeVolumeDescriptor();
	CombatPhysicsStrikeVolumeMovementDescriptor* GetMovementDescriptor() { return m_pobMovementDescriptor; };
	void SetLastStrikePointer(const CStrike* pobStrike) { m_pobLastStrike = pobStrike; };
	const CStrike* GetLastStrikePointer() { return m_pobLastStrike; };
	CAttackData* GetAlternateAttackData() { return m_pobAlternateAttackData; };
	ntstd::String GetEffectsScript() { return m_obEffectsScript; };
	bool GetRattleInteractables() { return m_bRattleInteractables; };
	bool GetCollapseInteractables() { return m_bCollapseInteractables; };
	bool GetStopAtInteractables() { return m_bStopAtInteractables; };
	bool GetDoRaycastCollisionCheckForStrike() { return m_bDoRaycastCollisionCheckForStrike; };
	float GetCollapseCollapsableStrength() { return m_fCollapseInteractableStrength; };
protected:
	const CStrike* m_pobLastStrike; // Only to be used to compare to newer strike pointers, will be deleted by attacks quickly

	// The first non-zero values in the following 3 sets of floats will decide which shape is wanted
	// E.g. if you want a box, leave sphere and capsule values at 0.0f and fill in box values
	float m_fSphereRadius;
	float m_fCapsuleRadius, m_fCapsuleLength;
	float m_fBoxHalfExtentX, m_fBoxHalfExtentY, m_fBoxHalfExtentZ;
	float m_fConcentricWaveSweep, m_fConcentricWaveOuterRadius, m_fConcentricWaveInnerRadius, m_fConcentricWaveHeight, m_fConcentricWaveExpansionSpeed, m_fConcentricWaveAngle;

	CombatPhysicsStrikeVolumeMovementDescriptor* m_pobMovementDescriptor;

	bool m_bDoRaycastCollisionCheckForStrike;
	bool m_bStopAtInteractables;
	bool m_bCollapseInteractables;
	bool m_bRattleInteractables;
	float m_fCollapseInteractableStrength;

	CAttackData* m_pobAlternateAttackData;

	ntstd::String m_obEffectsScript;

	// Projectile audio control
	CHashedString	m_obLoopingSoundCue;
	CHashedString	m_obLoopingSoundBank;
	float			m_fLoopingSoundRange;
	CHashedString	m_obPassbySoundCue;
	CHashedString	m_obPassbySoundBank;
	float			m_fPassbySoundRange;

	friend class CombatPhysicsStrikeVolume;
	friend class CombatPhysicsStrikeVolumeStraightMovement;
};

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumePhantomOverlapListener
*
*	DESCRIPTION	Class to listen for target hits in combat strike volumes
*
***************************************************************************************************/
class hkSimpleShapePhantom;
class CombatPhysicsStrikeVolumePhantomOverlapListener : public hkPhantomOverlapListener 
{
public:
	CombatPhysicsStrikeVolumePhantomOverlapListener( hkSimpleShapePhantom* pobPhantom );
	virtual void collidableAddedCallback(   const hkCollidableAddedEvent& event );
	virtual void collidableRemovedCallback( const hkCollidableRemovedEvent& event );
    const ntstd::List< const CEntity* >& Update();
	void Reset() { if (m_obEntitesToStrike.size() > 0) m_obEntitesToStrike.clear(); if (m_obInteractableEntities.size() > 0) m_obInteractableEntities.clear(); };
	const ntstd::List< const CEntity* >& GetInteractableEntityList() { return m_obInteractableEntities; };
private:
	bool EntityIsInStrikeList( const CEntity* pobEntity );
	bool EntityIsInInteractableList( const CEntity* pobEntity );

	hkSimpleShapePhantom* m_pobPhantom;
	ntstd::List< const CEntity* > m_obEntitesToStrike, m_obInteractableEntities;
};

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolume
*
*	DESCRIPTION	Class to execute described combat strike volume behaviour
*
***************************************************************************************************/
class CAttackData;
class CombatPhysicsStrikeVolume
{
public:
	CombatPhysicsStrikeVolume( const CStrike* pobStrike, CombatPhysicsStrikeVolumeDescriptor* pobDescriptor );
	~CombatPhysicsStrikeVolume();
	const ntstd::List< const CEntity* >& Update(float fTimeDelta);
	const CStrike* GetStrikePointer() { return m_pobStrikePointer; };
	CombatPhysicsStrikeVolumeDescriptor* GetDescriptor() { return m_pobDescriptor; };
	float GetAge() { return m_fAge; };
	const CAttackData* GetAttackData() { return m_pobAttackData; };
	float GetDistanceFrom(CPoint& obPosition);
	void ResetEntityList();
	bool IsEntityInPreviouslyStruckList(const CEntity* pobEnt);
	Transform* GetTransform() { return m_pobTransform; };
	void SetCombatEffectsTrigger(u_int iEff) { m_uiCombatEffectsTrigger = iEff; };
	u_int GetCombatEffectsTrigger() { return m_uiCombatEffectsTrigger; };
	void DestroyNow() { m_fAge = m_pobMovement->GetDescriptor()->GetTimeout(); };
	float GetWidth() const;
	float GetHeight() const;
	float GetDepth() const;
	const CEntity* GetOwner() { return m_pobOwner; };
	CDirection GetDisplacementLastFrame() { return m_obDisplacementLastFrame; };
	const ntstd::List< const CEntity* >& GetInteractableEntityList() { if (m_pobListener) return m_pobListener->GetInteractableEntityList(); else return m_obInteractableEntities; };
private:
	void UpdateAudio();

	u_int m_uiCombatEffectsTrigger;

	CombatPhysicsStrikeVolumeDescriptor* m_pobDescriptor;
	CombatPhysicsStrikeVolumeMovement* m_pobMovement;

	const CEntity* m_pobOwner;
	const CEntity* m_pobTarget;
	const CStrike* m_pobStrikePointer; // After construction, this is assumed invalid, just used to see if we've already produced volumes for a strike
	const CAttackData* m_pobAttackData;
	hkSimpleShapePhantom* m_pobPhantom;
	CombatPhysicsStrikeVolumePhantomOverlapListener* m_pobListener;

	float m_fAge;

	Transform* m_pobTransform;

	// For non havok entity query based concentric wave type
	float m_fConcentricWaveSweep, m_fConcentricWaveInnerRadius, m_fConcentricWaveOuterRadius, m_fConcentricWaveHeight, m_fConcentricWaveExpansionSpeed, m_fConcentricWaveAngle;
	
	ntstd::List< const CEntity* > m_obEntitesToStrike, m_obEntitiesStruck, m_obInteractableEntities;

	// Audio control
	unsigned int			m_uiLoopingSoundId;	
	unsigned int			m_uiPassbySoundId;	
	bool					m_bAudio;	

	CDirection m_obDisplacementLastFrame;
};

// Need to be included after the above class defs/imps
#include "characterlg.h"
#include "advancedragdoll.h"

namespace Physics 
{
	class AdvancedCharacterController : public LogicGroup
	{
	public:

		// Logic Group interface...
		virtual const GROUP_TYPE			GetType( ) const;
		virtual void						Activate( bool activateInHavok = false );
		virtual void						Deactivate( );
		virtual bool						IsActive( );
		virtual void						PausePresenceInHavokWorld(bool pause);
		virtual void						AddedToSystem( System* sys );
		virtual void						RemovedFromSystem( System* sys );
		virtual RigidBody*					AddRigidBody( const BodyCInfo* p_info );
		virtual RigidBody*					AddRigidBody( RigidBody* body )	;
		virtual void						UpdateCollisionFilter( );
		virtual void						EntityRootTransformHasChanged();
		virtual void						Pause( bool bPause );
		virtual CDirection					GetLinearVelocity( );
		virtual CDirection					GetAngularVelocity( );
		virtual void						SetLinearVelocity( const CDirection& p_linearVelocity );
		virtual void						SetAngularVelocity( const CDirection& p_angularVelocity );		
		virtual void						Update( const float p_timeStep );
		
		// Constructors / Destructors...
		AdvancedCharacterController( Character* p_entity, CColprimDesc* p_CharacterColPrim, ntstd::String sRagdollClump );
		virtual ~AdvancedCharacterController();

		// General movement application, could be to the ragdoll or to the character
		void			SetMoveTo( const CPoint& obPosition );
		void			SetTurnTo( const CQuat& obOrientation );

		// Character controller stuff
		void			SetCharacterControllerCollidable( bool is );
		void			SetCharacterControllerRagdollCollidable( bool is );
		void			SetCharacterControllerDynamicCollidable( bool is );
		void			SetCharacterControllerSetFilterExceptionFlags( unsigned int uiFlags );
		void			SetCharacterControllerHoldingCapsule( bool is );
		void			GetRootWorldMatrix(CMatrix& obMatrix);
		bool			IsOnGround();
		float			onGroundTimeOut;
		bool			IsCharacterControllerActive();
		void			ActivateCharacterController();
		void			DeactivateCharacterController();
		void			SetApplyCharacterControllerGravity(bool bGravity);
		bool			GetApplyCharacterControllerGravity();
		void			SetDoCharacterControllerMovementAbsolutely(bool bAbsolute);
		bool			GetDoCharacterControllerMovementAbsolutely();
		CharacterController::CharacterControllerType GetCharacterControllerType();

		void SetKOState(KO_States state);
		void SetRagdollLinearVelocity(CDirection& obVel);
		void AddRagdollLinearVelocity(CDirection& obVel);
		void SetRagdollAnimated(int iBoneFlags, bool bAnimateFromRagdollBones = false);
		void SetRagdollAnimatedAbsolute(int iBoneFlags, bool bAnimateFromRagdollBones = false);
		void SetRagdollAnimatedBones(int iBoneFlags);
		void AddRagdollAnimatedBone(int iBoneFlag);
		void SetRagdollTransformTracking();
		void SetRagdollTransformTrackingAnimated();
		void SetRagdollBoneTransformTrackingMapping(int iBoneFlag, Transform* pobTransform);
		void SetRagdollDead();
		void SetRagdollDisabled();
		void SetRagdollTurnDynamicOnContact( bool bDynamic );
		void SetRagdollTrajectory( float fDistanceMultiplier = 1.0f, float fAngleMultiplier = 1.0f );
		void SetRagdollExemptFromCleanup( bool bExemption );
		void SetRagdollAntiGravity( bool bAntiGrav );
		void RagdollApplyLinearImpulse(const CDirection& obImp);
		void RagdollApplyAngularImpulse(const CDirection& obImp);
		void RagdollBodyApplyImpulse(int iHitArea, const CPoint& position, const CDirection& impulse);
		void RagdollTwitch();
		bool GetRagdollTurnDynamicOnContact();
		bool IsRagdollActive();
		RAGDOLL_STATE GetRagdollState();
		void DeactivateRagdoll();
		CDirection GetRagdollLinearVelocity();
		void SetSendRagdollMessageOnAtRest(const char* pcMsg);

		void			SetWorldSpaceSynchronised( WS_SYNC_TYPE status );

		virtual void Debug_RenderCollisionInfo ();

		void StartBlendFromRagdollToAnimation(hkArray<hkQsTransform>* aobRagdollTransforms, CMatrix& obRoot);

		bool GetCharacterControllerWasActiveLastFrame();
		bool GetRagdollWasActiveLastFrame();
		CPoint GetCharacterControllerPhantomPositionLastFrame();

		void SetCharacterCombatPhysicsPushVolumeDescriptors(	PushVolumeDescriptorList* 	pobCombatPhysicsPushVolumeDescriptors, 
																PushVolumeDataList* 		pobCombatPhysicsPushVolumeData);
		void CleanupCharacterCombatPhysicsPushVolumeDescriptors(	PushVolumeDataList* pobCombatPhysicsPushVolumeData);

		void SwitchCharacterControllerType(CharacterController::CharacterControllerType eTo);

		CharacterController::CharacterControllerType GetActiveCharacterControllerType() { return m_eCurrentCharacterControllerType; };

		// Need to get rid of this heinous accessor
		AdvancedRagdoll* GetAdvancedRagdoll() { return m_advRagdoll; };

		// SOFT PARENTING
		void SetSoftParent(CEntity * parent);
		bool IsSoftParented() const; 

		// Access the Enabled state
		void FootIKEnabled( bool bValue )	{ m_bFootIKEnabled = bValue; }
		bool FootIKEnabled( ) const			{ return m_bFootIKEnabled; }

		// Do raycast on character ragdoll, character is simulated by controller at the moment
		bool CastRayOnRagdoll(const CPoint &obSrc, const CPoint &obTarg, Physics::RaycastCollisionFlag obFlag, TRACE_LINE_QUERY& stQuery);

	protected:		
		// Temporary measure, this is exposed to friends so they get direct access to ragdoll		
		friend class RagdollFlooredTransition;
		friend class SpearThrown;
		friend class SpearLG;
		// End temporary friendliness.

		// - EXTERNAL COMPONENTS --------------------------------------
		Character*			m_entity;
		CHierarchy*			m_hierarchy;

		// - CONTROLLED COMPONENTS ------------------------------------
		AdvancedRagdoll*	m_advRagdoll;	// [Mem::MC_HAVOK]

		//IKHand*			m_handIK;		// - Hand IK
		IKFootPlacement*	m_footIK;		// - Foot IK [Mem::MC_HAVOK]
		//IKLookAt*			m_headIK;		// - Head IK

		bool				m_bFootIKEnabled;

		// - REAL TIME PARAMETERS -----------------------------------
		CPoint			m_GoalTranslation;
		CQuat			m_GoalOrientation;

		float			m_PreviousGravityFactor;
		float			m_FallingTime;
		float			m_PelvisSpring;

		CPoint			m_synchronisedReference;
		CPoint			m_computedLocalReference;
		CPoint			m_lastGoal;
		WS_SYNC_TYPE	m_Synchronised;

		bool m_bBlendingFromRagdollToAnimation;
		hkArray<hkQsTransform> m_aobRagdollBlendFromHierarchy;
		float m_fRagdollToHierarchyBlendAmount;
		CMatrix m_obRagdollBlendFromRoot;
		float m_fTimeSpentBlendingFromRagdoll;

		bool m_bCharacterControllerWasActiveLastFrame, m_bRagdollWasActiveLastFrame;
		CPoint m_obCharacterControllerPhantomPositionLastFrame;

		CharacterController* m_apobCharacterControllers[CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT];
		CharacterController::CharacterControllerType m_eCurrentCharacterControllerType;
	};

} // Physics

#endif // _ADVANCED_CHARACTER_CONTROL
