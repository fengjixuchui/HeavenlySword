//--------------------------------------------------
//!
//!	\file game/entityinteractablethrown.h
//!	Definition of the Interactable Thrown entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_THROWN_H
#define	_ENTITY_INTERACTABLE_THROWN_H

#include "game/entityinteractable.h"

#include "fsm.h"

class CoolCam_AfterTouchDef;
class CoolCam_ChaseDef;
class CoolCam_AimDef;
class AftertouchControlParameters;
class MovementControllerDef;

//--------------------------------------------------
//!
//! Class Att_Thrown.
//! Interactable definition params
//!
//--------------------------------------------------

class Att_Thrown
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Thrown)

public:
	bool							m_bAIAvoid;						// Sets if the AI will try to avoid this object
	float							m_fAIAvoidRadius;				// Sets the radius that the AI will try to avoid the object by
	float							m_fMass;						// Sets the mass of the object effecting things like inertia and bounce
	CPoint							m_obCenterOfMass;				// Adjusts the centre of mass, offset from the root
	float							m_fRestitution;					// The rate at which energy is lost from impacts with other objects
	float							m_fFriction;					// The rate at which energy is lost from sliding against other objects
	float							m_fLinearDamping;				// The rate at which velocity is lost when thrown
	float							m_fAngularDamping;				// The rate at which spin is lost when thrown
	float							m_fMaxLinearVelocity;			// Defines the maximum velocity of the object
	float							m_fMaxAngularVelocity;			// Defines the maximum spin rate of the object
	CHashedString					m_obPhysicsSoundDef;			// Sound definitions for the physics stuff
	bool							m_bOrientateToVelocity;			// Sets the object orientation to be the same as the velocity, used for spears and swords
	float							m_fImpactThreshold;				// Magic number to indicate the threshold before an object will break on impact
	bool							m_bDamageOnChar;				// Flag if the object will break in contact with a character
	bool							m_bDamageOnEnv;					// Flag if the object will break in contact with the environment
	bool							m_bCollapseOnDamage;			// Do compound rigid collapse on damage
	bool							m_bRemoveOnDamage;				// Remove mesh on damage
	bool							m_bRebound;						// If the object will rebound for more fun, shields
	bool							m_bStickInChar;					// If the object will stick in characters, spears and swords
	bool							m_bStickInEnv;					// If the object will stick in the environment, spears and swords
	bool							m_bInstantKill;					// If the object will kill on impact
	bool							m_bImpaleKill;					// If the object will kill on impale
	CHashedString					m_obThrownAttackData;			// The definition that defines the strike data
	float							m_fThrowTime;					// Time offset from the start of the anim before the object is thrown			
	float							m_fDropTime;					// Time offset from the start of the anim before the object is dropped
	CPoint							m_obThrowTranslation;			// Translation offset from the root of the character where the object is released
	CQuat							m_obThrowOrientation;			// Orientation offset from the root of the character where the object is released
	CPoint							m_obThrowVelocity;				// The object velocity when thrown
	CPoint							m_obThrowAngularVelocity;		// The object spin rate when thrown
	CPoint							m_obDropVelocity;				// The objects direction when dropped
	CHashedString					m_obAnimPlayerMoveTo;			// Anim for the goto
	CHashedString					m_obAnimPlayerRunTo;			// Anim for the run goto
	CHashedString					m_obAnimPlayerPickup;			// Anim for the pickup
	CHashedString					m_obAnimPlayerRunPickup;		// Anim for the run pickup
	CHashedString					m_obAnimPlayerThrow;			// Anim for the throw
	CHashedString					m_obAnimPlayerDrop;				// Anim for the drop
	MovementControllerDef*			m_pobPlayerHoldingMovement;		// Definition for the partial WalkRun def
	AftertouchControlParameters*	m_pobAftertouchProperites;		// Aftertouch parameters for the object
	CoolCam_AfterTouchDef*			m_pobAftertouchCamProperties;	// Aftertouch camera paramters
	CHashedString					m_obSfxImpaleRagdoll;
	CHashedString					m_obSfxImpaleSolid;
	CHashedString					m_obSfxDestroy;
	CHashedString					m_obSfxCollapse;
	CHashedString					m_obPfxImpaleRagdoll;
	CHashedString					m_obPfxImpaleSolid;
	CHashedString					m_obPfxDestroy;
	CHashedString					m_obPfxCollapse;
	CHashedString					m_obOnAftertouchStart;
	CHashedString					m_obOnAftertouchEnd;


	/* // Damage meshes removed
	int m_iHitCount;
	ntstd::String m_obDamageMesh;		// FIX ME - seem to still need chars
	int m_iDamageMeshCount;	*/


	/* // First person throw aiming removed
	MovementControllerDef* m_pobPlayerAimingMovement;
	MovementControllerDef* m_pobPlayerThrowMovement;
	float m_fAimedThrowTime;
	CoolCam_ChaseDef* m_pobChasecamProperties;
	CoolCam_AimDef*  m_pobAimcamProperties;
	*/

	Att_Thrown()
	:	  m_bAIAvoid (false)
	,     m_fAIAvoidRadius (0.0f)
	,     m_fMass (10.0f)
	,     m_obCenterOfMass( CONSTRUCT_CLEAR )
	,     m_fRestitution( 0.0f )
	,     m_fFriction(0.75f)
	,     m_fLinearDamping(0.01f)
	,     m_fAngularDamping(0.75f)
	,     m_fMaxLinearVelocity(30.0f)
	,     m_fMaxAngularVelocity(0.1f)
	,     m_obPhysicsSoundDef("SwordPhysicsSoundDef")
	,     m_bOrientateToVelocity(true)
	,     m_fImpactThreshold(7.5f)
	,     m_bDamageOnChar(false)
	,     m_bDamageOnEnv(false)
	,     m_bCollapseOnDamage(false)
	,     m_bRemoveOnDamage(false)
	,     m_bRebound(false)
	,     m_bStickInChar(true)
	,     m_bStickInEnv(true)
	,     m_bInstantKill(true)
	,     m_bImpaleKill(false)
	,     m_obThrownAttackData("atk_generic_obj_strike")
	,     m_fThrowTime(0.25f)
	,     m_fDropTime(0.18f)
	,     m_obThrowTranslation(-0.065f,0.923f,1.041f)
	,     m_obThrowOrientation(0.0f,0.0f,0.0f,1.0f)
	,     m_obThrowVelocity(0.0f,0.0f,25.0f)
	,     m_obThrowAngularVelocity(-150.0f,0.0f,0.0f)
	,     m_obDropVelocity(1.0f,0.0f,-1.0f)
	,     m_obAnimPlayerMoveTo("Spear_MoveTo")
	,     m_obAnimPlayerRunTo("Sword_RunTo")
	,     m_obAnimPlayerPickup("Sword_Pickup")
	,     m_obAnimPlayerRunPickup("Sword_RunPickup")
	,     m_obAnimPlayerThrow("Sword_Throw")
	,     m_obAnimPlayerDrop("Sword_Drop")
	,     m_pobPlayerHoldingMovement( 0 )
	,     m_pobAftertouchProperites( 0 )
	,     m_pobAftertouchCamProperties( 0 )
	,     m_obSfxImpaleRagdoll("impale_ragdoll")
	,     m_obSfxImpaleSolid("")
	,     m_obSfxDestroy("")
	,     m_obSfxCollapse("")
	,     m_obPfxImpaleRagdoll("")
	,     m_obPfxImpaleSolid("")
	,     m_obPfxDestroy("")
	,     m_obPfxCollapse("")
	,     m_obOnAftertouchStart("aftertouchStartAudio")
	,     m_obOnAftertouchEnd("aftertouchEndAudio")

	/* // Damage meshes removed
	,     m_iHitCount( 0 )
	,     m_obDamageMesh("")
	,     m_iDamageMeshCount(0)*/
	
	/* // First person throw aiming removed
	,     m_fAimedThrowTime(0.27f)
	,     m_pobPlayerAimingMovement( 0 )
	,     m_pobPlayerThrowMovement( 0 )
	,     m_pobChasecamProperties( 0 )
	,     m_pobAimcamProperties( 0 ) */
	{}
}	
;

//--------------------------------------------------
//!
//! Class Interactable_Thrown.
//! Base interactable thrown entity type
//!
//--------------------------------------------------
class Interactable_Thrown : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Thrown)

public:
	// Constructor
	Interactable_Thrown();

	// Destructor
	~Interactable_Thrown();

	// Post Construct
	void OnPostConstruct();

	// Check to see if the thrown object is in use
	virtual bool InteractableInUse( void ) const;

	virtual void Reset();

	void Attach();

	Att_Thrown* GetSharedAttributes(void) { return m_pSharedAttributes; }

	// Public Variables (once part of attribute table in the script)
	Character*	m_pOther;
	CEntity*	m_pAttacker;
	//bool		m_bAiming;
	//int		m_nHits;
	int			m_nThrownCameraHandle;
	//int		m_nLastMesh;
	bool		m_bDoQuickThrow;
	bool		m_bCheckQuickThrow;
	bool		m_bMovementDone;
	bool		m_bImpaled;
	bool		m_bThrown;
	bool		m_bBeingUsed;

	//void		OnDamage(int nDamage);

	void		Aftertouch_Power_Off(void);
	void		ReparentObject(CEntity* pParent);

protected:
	// Object description
	CHashedString	m_Description;

	// Initial State
	CHashedString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;

	Att_Thrown* m_pSharedAttributes;	

	bool m_bAttached;
};


#endif // _ENTITY_INTERACTABLE_THROWN_H
