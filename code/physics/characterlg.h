  //---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/characterlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.09
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_CHARACTER_LG_INC
#define _DYNAMICS_CHARACTER_LG_INC

#include "config.h"

class CEntity;
class CColprimDesc;
class hkWorld;
class hkCapsuleShape;
class Transform;

#include "hsCharacterProxy.h"
#include "advancedcharactercontroller.h"
#include "collisionbitfield.h"
// Even though this is defined in the above include, it needs to be forward declared to work... no idea why - DGF
//class CombatPhysicsPushVolumeDescriptor;

// Typedef of chunked lists for easy reference
typedef ntstd::List<CombatPhysicsPushVolumeDescriptor*, Mem::MC_ENTITY> PushVolumeDescriptorList;
typedef ntstd::List<CombatPhysicsPushVolumeData*, Mem::MC_ENTITY> 		PushVolumeDataList;

namespace Physics
{
	class CVolumeData;
	class IKFootPlacement;
	class IKLookAt;
	class System;
	class CharacterAntiGravityAction;
	class CharacterUberGravityAction;
	class RigidCharacterInteractionListener;
	class CharacterInteractionListener;
	class MyFullCharacterControllerPhantomOverlapListener;
	class MyRaycastPhantomCharacterControllerOverlapListener;

	class CharacterController 
	{
	public:
		enum CharacterControllerType
		{
			FULL_CONTROLLER, // Expensive fully integrated phantom, can do everything we want
			RIGID_BODY, // Much cheaper rigid body, slides off slopes and bounces on stairs
			RAYCAST_PHANTOM, // Much cheaper phantom with no integration, just pushes any bodies it encounters
			DUMMY, // Dummy with no physics at all, will go wherever it's told
			CHARACTER_CONTROLLER_TYPE_COUNT
		};
		
		static CharacterController*	Construct( Character*, CColprimDesc*, CharacterControllerType );

		// Buiding...
		CharacterController( Character* p_entity, CColprimDesc* obColprim );
		virtual ~CharacterController( );

		virtual CharacterControllerType GetType() = 0;
		
		// Activation...
		virtual void					Activate( ) = 0; // Adds the character to the world
		virtual void					Deactivate( ) = 0; // Takes the chracter out of the world
		virtual bool					IsActive( );
		virtual void					PausePresenceInHavokWorld(bool pause);
		
		// Settings...
		virtual void					RegisterSystem( System* ) = 0;

		// Entity properties...
		virtual CPoint					GetEntityPosition() const;
		virtual void					GetEntityWorldMatrix(CMatrix& obMatrix);
		virtual CMatrix					GetEntityWorldMatrix();

		// Character phantom properties
		virtual void					SetPosition (const hkVector4& position) = 0;
		virtual void					SetLinearVelocity (const hkVector4& vel) = 0;
		virtual hkVector4				GetPosition() const = 0;
		virtual hkVector4				GetLinearVelocity( ) = 0;
		virtual void					SetApplyGravity(bool bGravity) = 0;
		virtual bool					GetApplyGravity() = 0;
		virtual void					SetDoMovementAbsolutely(bool bAbsolute) = 0;
		virtual bool					GetDoMovementAbsolutely()  = 0;
		virtual void					SetPositionOnNextActivation(CPoint& obPos);

		// Position calculation
		virtual CPoint					ComputeEntityPositionFromCharacter() const;	// Get where the entity should be from the physical position of the character phantom
		virtual hkVector4				ComputeCharacterPositionFromEntity(CPoint* pobPosition = 0) const; // Get where the phantom should be from the transform position of the entities root
		
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// Character Proxy accessors...
		virtual void Debug_RenderCollisionInfo() = 0;
#endif

		// Groundedness - will only be calculated once in a frame, then recycled
		virtual bool					IsCharacterControllerOnGround( ) = 0;

		// Collision status
		virtual void					SetCollidable( bool bCollideable ) = 0;
		virtual void					SetDynamicCollidable (bool isDynamicCollidable) = 0;
		virtual void					SetRagdollCollidable (bool bCollide) = 0;
		virtual void					SetFilterExceptionFlags (unsigned int uiFlags) = 0;
		virtual void					SetHoldingCapsule(bool bShape) = 0;
		virtual void					SetKOState(KO_States state) = 0;

		// Updates
		virtual void					Update(float deltaTime, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform) = 0;
		
		// Combat push/collapse volumes
		void UpdateCombatPhysicsPushVolumes( float p_timestep );
		void SetCombatPhysicsPushVolumeDescriptors(	PushVolumeDescriptorList* pobCombatPhysicsPushVolumeDescriptors,
													PushVolumeDataList* pobCombatPhysicsPushVolumeData);
		
		// NB there looks to be a mismatch between name and type after recent changes...
		void CleanupCombatPhysicsPushVolumeDescriptors(PushVolumeDataList* pobCombatPhysicsPushVolumeDescriptors);
		
		float GetMaxSlopeCosine();
		float GetMaxSlope();

		void SetSoftParent(CEntity * entity);
		bool IsSoftParented() const {return m_softParent != NULL;};

		virtual void UpdateCollisionFilter() = 0;
		
	protected:
		// Tools...
		virtual void					SetupCharacterVolume() = 0;
		virtual void					AddToWorld (hkWorld* pWorld) = 0;
		void							AddCombatPhysicsPushVolumesToWorld();
		virtual void					RemoveFromWorld (hkWorld* pWorld) = 0;
		void							RemoveCombatPhysicsPushVolumesFromWorld();

		bool							m_isActive; // Am I active?
		Character*						m_entity; // Who do I represent?
		Transform*						m_pobRoot; // Root transform of who I represent
		bool							m_bDoneGroundCheckThisFrame;
		bool							m_bGrounded;
		bool							m_bGravity, m_bApplyMovementAbsolutely;
		CPoint							m_obPositionOnNextActivation;
		bool							m_bPositionOnNextActivationSet;
		CDirection						m_obDisplacementLastFrame;		
		hkWorld*						m_world; // The havok world in which I reside
		float							m_fCapsuleHalfLength;
		bool							m_bUseHolding;
		CColprimDesc*					m_pobColPrim;
		hkCapsuleShape					*m_standShape, *m_holdingShape;
		Physics::ChatacterControllerCollisionFlag    m_obEntityCFlag;
		
		// [scee_st] now uses chunked memory allocs because of CHUNKED_INTERFACE change
		PushVolumeDescriptorList*		m_pobCombatPhysicsPushVolumeDescriptors;
		PushVolumeDataList* 			m_pobCombatPhysicsPushVolumeData;

		MyFullCharacterControllerPhantomOverlapListener* m_pol; // A listener for my volumes	

		CEntity							*m_softParent;
		CMatrix                         *m_relToSoftParent;   

	};

	class RigidCharacterController : public CharacterController
	{
	public:
		virtual CharacterControllerType GetType() { return RIGID_BODY; };

		// Buiding...
		RigidCharacterController( Character* p_entity, CColprimDesc* obColprim );
		virtual ~RigidCharacterController( );
		
		// Activation...
		virtual void					Activate( ); // Adds the character to the world
		virtual void					Deactivate( ); // Takes the chracter out of the world		
		
		// Settings...
		virtual void					RegisterSystem( System* );

		// Character phantom properties
		virtual void					SetPosition (const hkVector4& position);
		virtual void					SetLinearVelocity (const hkVector4& vel);
		virtual hkVector4				GetPosition() const;
		virtual hkVector4				GetLinearVelocity( );
		virtual void					SetApplyGravity(bool bGravity) { m_bGravity = bGravity; };
		virtual bool					GetApplyGravity() { return m_bGravity; };
		virtual void					SetDoMovementAbsolutely(bool bAbsolute) { m_bApplyMovementAbsolutely = bAbsolute; };
		virtual bool					GetDoMovementAbsolutely() { return m_bApplyMovementAbsolutely; };

		virtual void Debug_RenderCollisionInfo();

		// Groundedness - will only be calculated once in a frame, then recycled
		virtual bool					IsCharacterControllerOnGround( );

		// Collision status
		virtual void					SetCollidable( bool bCollideable );
		virtual void					SetDynamicCollidable (bool isDynamicCollidable);
		virtual void					SetRagdollCollidable (bool bCollide);
		virtual void					SetFilterExceptionFlags (unsigned int uiFlags);
		virtual void					SetHoldingCapsule(bool bShape);
		virtual void					SetKOState(KO_States state);

		// Updates
		virtual void					Update(float deltaTime, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform);
	
		virtual void UpdateCollisionFilter();
	protected:
		// Tools...
		virtual void					SetupCharacterVolume();
		virtual void					AddToWorld (hkWorld* pWorld);
		virtual void					RemoveFromWorld (hkWorld* pWorld);

		hkRigidBody*					GetBodyP () const;

		// My rigid bodies
		hkRigidBody*					m_pobBody;
		hkRigidBody*					m_pobHoldingBody;

		CharacterAntiGravityAction* GetAntiGravityAction();
		CharacterAntiGravityAction *m_pobAntiGravityAction, *m_pobAntiGravityActionHolding;

		CharacterUberGravityAction* GetUberGravityAction();
		CharacterUberGravityAction *m_pobUberGravityAction, *m_pobUberGravityActionHolding;	

		RigidCharacterInteractionListener* m_pobCharacterInteractionListener;
	};

	// This listener will take care of character and invisibler wall interaction... 
	// If a wall is one side it let character pass from one side, if the character is in KO
	// it check if he whould be killed by wall
	class CharacterContactListener : public hsCharacterProxyContactListener
	{
	public:
		void contactsFoundCallback(hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector);
	protected:
		hkVector4 GetTriangleNormal(const hkShape * shape, const hkShapeKey& key) const;
		bool IgnoreContact(hkRootCdPoint& pt, bool &kill) const;	
	};

	class FullCharacterController : public CharacterController
	{
	public:
		virtual CharacterControllerType GetType() { return FULL_CONTROLLER; };

		// Buiding...
		FullCharacterController( Character* p_entity, CColprimDesc* obColprim );
		virtual ~FullCharacterController( );
		
		// Activation...
		virtual void					Activate( ); // Adds the character to the world
		virtual void					Deactivate( ); // Takes the chracter out of the world
		
		// Settings...
		virtual void					RegisterSystem( System* );

		// Character phantom properties
		virtual void					SetPosition (const hkVector4& position);
		virtual void					SetLinearVelocity (const hkVector4& vel);
		virtual hkVector4				GetPosition() const;
		virtual hkVector4				GetLinearVelocity( );
		virtual void					SetApplyGravity(bool bGravity) { m_bGravity = bGravity; };
		virtual bool					GetApplyGravity() { return m_bGravity; };
		virtual void					SetDoMovementAbsolutely(bool bAbsolute) { m_bApplyMovementAbsolutely = bAbsolute; }
		virtual bool					GetDoMovementAbsolutely() { return m_bApplyMovementAbsolutely; }

		virtual void Debug_RenderCollisionInfo();

		// Groundedness - will only be calculated once in a frame, then recycled
		virtual bool					IsCharacterControllerOnGround( );
		virtual hkSurfaceInfo			GetCurrentCharacterControllerGround( );

		// Collision status
		virtual void					SetCollidable( bool bCollideable );
		virtual void					SetDynamicCollidable (bool isDynamicCollidable);
		virtual void					SetRagdollCollidable (bool bCollide);
		virtual void					SetFilterExceptionFlags (unsigned int uiFlags);
		virtual void					SetHoldingCapsule(bool bShape);
		virtual void					SetKOState(KO_States state);

		// Updates
		virtual void					Update(float deltaTime, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform);
	
		virtual void UpdateCollisionFilter();
	protected:
		// Tools...
		virtual void			SetupCharacterVolume();
		virtual void			AddToWorld (hkWorld* pWorld);
		virtual void			RemoveFromWorld (hkWorld* pWorld);

		hkVector4				CheckWallAndCorrectVelocityAsNeccessary(hkVector4& flat_velocity);
		hkSurfaceInfo			GetCurrentCharacterControllerWall( const hkVector4& dir, hkVector4& norm );

		hsCharacterProxy*				m_characterProxy; // My Havok character controller
		CharacterInteractionListener*	m_listener; // A listener to push bodies I collide with
		CharacterContactListener*		m_wallContactListener; // Listen the collision with AI Walls
		hkShapePhantom*					m_phantom; // My phantom which is added to the world when I'm active

		hkSurfaceInfo m_obGroundInfo;	

		hkRigidBody *m_pobRigidBody;
	};

	class RaycastPhantomCharacterController : public CharacterController
	{
	public:
		virtual	CharacterControllerType GetType() { return RAYCAST_PHANTOM; };

		// Buiding...
		RaycastPhantomCharacterController( Character* p_entity, CColprimDesc* obColprim );
		virtual ~RaycastPhantomCharacterController( );
		
		// Activation...
		virtual void					Activate( ); // Adds the character to the world
		virtual void					Deactivate( ); // Takes the chracter out of the world
		
		// Settings...
		virtual void					RegisterSystem( System* );

		// Character phantom properties
		virtual void					SetPosition (const hkVector4& position);
		virtual void					SetLinearVelocity (const hkVector4& vel);
		virtual hkVector4				GetPosition() const;
		virtual hkVector4				GetLinearVelocity( );
		virtual void					SetApplyGravity(bool bGravity) { m_bGravity = bGravity; };
		virtual bool					GetApplyGravity() { return m_bGravity; };
		virtual void					SetDoMovementAbsolutely(bool bAbsolute) { m_bApplyMovementAbsolutely = bAbsolute; };
		virtual bool					GetDoMovementAbsolutely() { return m_bApplyMovementAbsolutely; };

		virtual void Debug_RenderCollisionInfo();

		// Groundedness - will only be calculated once in a frame, then recycled
		virtual bool					IsCharacterControllerOnGround( );

		// Collision status
		virtual void					SetCollidable( bool bCollideable );
		virtual void					SetDynamicCollidable (bool isDynamicCollidable);
		virtual void					SetRagdollCollidable (bool bCollide);
		virtual void					SetFilterExceptionFlags (unsigned int uiFlags);
		virtual void					SetHoldingCapsule(bool bShape);
		virtual void					SetKOState(KO_States state);

		// Updates
		virtual void					Update(float deltaTime, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform);
	
		virtual void UpdateCollisionFilter();
	protected:
		virtual void					SetupCharacterVolume();
		virtual void					AddToWorld (hkWorld* pWorld);
		virtual void					RemoveFromWorld (hkWorld* pWorld);

		hkVector4						RaycastGetGroundPosition(hkVector4* pobPosition = 0);
		hkVector4						ShortRaycastGetGroundPosition(hkVector4* pobPosition = 0);

		hkShapePhantom*					m_pobPhantom;
		MyRaycastPhantomCharacterControllerOverlapListener* m_pol;
		hkVector4						m_obGroundPosition;		
		bool							m_bCollideWithStatic;
	};

	class DummyCharacterController : public CharacterController
	{
	public:
		virtual	CharacterControllerType GetType() { return DUMMY; };

		// Buiding...
		DummyCharacterController( Character* p_entity, CColprimDesc* obColprim ) 
			: CharacterController(p_entity,obColprim) 
		{
			SetupCharacterVolume();
		};
		virtual ~DummyCharacterController( ) {};
		
		// Activation...
		virtual void					Activate( ); // Adds the character to the world
		virtual void					Deactivate( ); // Takes the chracter out of the world		
		
		// Settings...
		virtual void					RegisterSystem( System* );

		// Character phantom properties
		virtual void					SetPosition (const hkVector4& position) { m_obPosition = position; };
		virtual void					SetLinearVelocity (const hkVector4& vel) {};
		virtual hkVector4				GetPosition() const { return m_obPosition; };
		virtual hkVector4				GetLinearVelocity() { return hkVector4::getZero(); };
		virtual void					SetApplyGravity(bool bGravity) { m_bGravity = bGravity; };
		virtual bool					GetApplyGravity() { return m_bGravity; };
		virtual void					SetDoMovementAbsolutely(bool bAbsolute) { m_bApplyMovementAbsolutely = bAbsolute; };
		virtual bool					GetDoMovementAbsolutely() { return m_bApplyMovementAbsolutely; };

		virtual void Debug_RenderCollisionInfo() {};

		// Groundedness - will only be calculated once in a frame, then recycled
		virtual bool					IsCharacterControllerOnGround( ) { return false; };

		// Collision status
		virtual void					SetCollidable( bool bCollideable ) {};
		virtual void					SetDynamicCollidable (bool isDynamicCollidable) {};
		virtual void					SetRagdollCollidable (bool bCollide) {};
		virtual void					SetFilterExceptionFlags (unsigned int uiFlags) {};
		virtual void					SetHoldingCapsule(bool bShape){};
		virtual void					SetKOState(KO_States state) {};

		// Updates
		virtual void					Update(float deltaTime, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform);
	
		virtual void UpdateCollisionFilter() {};
	protected:
		virtual void					SetupCharacterVolume();
		virtual void					AddToWorld (hkWorld* pWorld);
		virtual void					RemoveFromWorld (hkWorld* pWorld);

		hkVector4						m_obPosition;
	};
}
#endif // _DYNAMICS_CHARACTER_LG_INC
