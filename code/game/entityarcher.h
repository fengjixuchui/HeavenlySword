//------------------------------------------------------------------------------------------
//!
//!	\file game/entityarcher.h
//!	Definition of the Archer entity object
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_ARCHER_H
#define	_ENTITY_ARCHER_H

// Necesary includes
#include "game/entityplayer.h"
#include "game/messages.h"
#include "game/aimcontroller.h"
#include "editable/enumlist.h"
#include "editable/flipflop.h"
#include "game/eventlog.h"

class Archer;

//!------------------------------------------------------------------------------
//!  ArcherSettings
//!  Archer settings exposed to XML
//!
//!
//!  @author GavB @date 27/11/2006
//!------------------------------------------------------------------------------
class ArcherSettings
{
public:

	// Base sound bank for the archer. 
	CKeyString		m_MainSoundBank;

	/// Various sounds for the archer. 
	CKeyString		m_ChargeUpSFX, m_ChargedSFX;


	/// Numbers that control the shooting of her xbow
	float			m_fReloadTime;

	// 
	float			m_fChargeUpTime;

	//
	float			m_fDenyControlUntilCharge;

};

//------------------------------------------------------------------------------------------
//!
//! Class Archer.
//! Archer entity type
//!
//------------------------------------------------------------------------------------------
class Archer : public Player
{
	// Declare dataobject interface
	HAS_INTERFACE(Archer)

public:
	
	// How close to vaulting volumes should the archer be to enable the crouch
	static float m_sCrouchActivatingRange;

	// The Shooty mode the archer is currently in.. 
	enum SHOOTY_MODE
	{
		// This is the default shooting state everyone is familiar with
		DEFAULT,

		// Turret point - this is when the archer is fixed to the point. 
		TURRET,

		// The cable car turret mode could be different from the normal turret mode. 
		TURRET_CALBECAR,
	};


public:

	// Lua only exposed methods. 
	void Lua_ReactState(void);

	// Override 
	virtual void 	Update1stFiring			( float );

	// Lua bindings. 
	void			LogEvent				(ArcherEventType eEvent) const;
	CEntity*		Lua_CreateBolt			(CEntity*,CEntity*,const CDirection&,float fCharge) const;
	void			Lua_CanCrouch			( bool bState )						{ m_bCanCrouch = bState; }
	void			Lua_AlwaysCrouch		( bool bState )						{ m_bAlwaysCrouch = bState; }

	bool			CanCrouch				(void) const						{ return m_bCanCrouch; }
	bool			AlwaysCrouch			(void) const						{ return m_bAlwaysCrouch; }

	// Set the crouch set of the archer. 
	void			UseSmallCapsule			(bool) const;

	// Allow the foot ik to be enabled and disabled.
	void			UseFootIK				(bool) const;

	// Does the archer have a fully charged bow?
	bool			SuperShot				(void) const { return (m_afXbowArmCharges[0] >= 1.0f) && (m_afXbowArmCharges[1] >= 1.0f); }

	// Allow access to the shot charge. 
	float			GetXbowArmCharge		(u_int uiIndex) const { ntError( uiIndex < 2 ); return clamp( m_afXbowArmCharges[uiIndex], 0.0f, 1.0f ); }
	
public:
	// Constructor
	Archer();

	//Destructor
	virtual ~Archer();

	// From CEntity
	virtual void OnLevelStart();

	void OnPostConstruct();
	void OnPostPostConstruct();

	void				SetReturnState(CHashedString sState) {m_sReturnState = sState;}
	CHashedString		GetReturnState()                     {return m_sReturnState;}

	CHashedString		GetWalkingController() const {return IsCrouching() ? m_obCrouchedController : m_obWalkingController;}

	int					GetAimCamID() {return m_iAimCamID;}
	void				SetAimCamID(int iID) {m_iAimCamID = iID;}

	void				ForwardInteractionMsg(MessageID id);

	bool				VaultObjectIfAppropriate( bool bTestOnly = false );

	// Access the settings for the archer
	const ArcherSettings* GetSettings(void) const { return m_pSettings; }

	// Vaulting notification/state checking.
	bool	IsVaulting			()	const	{ return m_State == ARC_VAULTING; }
	bool	CanVault			()	const	{ return m_State == ARC_IDLE || m_State == ARC_VAULTING; }			// Only allow us to vault from the idle state for now.
	void	SetVaulting			()			{ /*ntError_p( CanVault(), ("Must be in a state that allows vaulting.") );*/ m_State = ARC_VAULTING; }
	void	Set1stPersonState	()			{ /*ntError_p( m_State == ARC_IDLE, ("Must be in a state that allows 1st person shooting.") );*/ m_State = ARC_1ST_AIMING; }
	bool	IsIn1stPersonState	()	const	{ return m_State == ARC_1ST_AIMING || m_State == ARC_1ST_FIRING; }
	void	VaultFinished		();
	bool	IsCrouching			()	const	{ return m_bCurrentlyCrouching; }
	void	SetCrouching		(bool bCrouching)	{ m_bCurrentlyCrouching = bCrouching; }

	// Register a ArcherEventLog of this archer component
	void RegisterArcherEventLog(ArcherEventLog* pobLog) { m_obEventLogManager.RegisterEventLog(pobLog); };

	float	GetAfterTouchStartDelay() const { return m_fAfterTouchStartDelay; }

	void SetVaultingSafe			()			{ m_bVaultingSafe = true; }
	void ClearVaultingSafe			()			{ m_bVaultingSafe = false; }
	bool IsVaultingSafe				() const	{ return m_bVaultingSafe; }

	// Method that allows the Archer to modifiy the holding capsule shape for that of a crouching capsule
	virtual void ModifyPhysicsHoldingShape( CPoint& rptA, CPoint& rptB, float& fRadius );

	// Access the after trouch enabled flag
	bool	AfterTouchEnabled		() const		{ return m_bAfterTouchEnabled; }
	void	AfterTouchEnabled		(bool bValue)	{ m_bAfterTouchEnabled = bValue; }


	// Return the expected Y value that camera should be focused on during vaulting.
	float	GetVaultingCameraY(void) const { return m_fVaultingCameraY; }

	float	WalkRunBlendInTime(void) const { return m_fWalkRunBlendInTime; }
	void	WalkRunBlendInTime(float fTime) { m_fWalkRunBlendInTime = fTime; }

	bool	AddChainedController(void) const { return m_bAddNewController; }
	void	AddChainedController(bool bValue) { m_bAddNewController = bValue; }

	// Get the charge of each of an xbow arm
	float GetXbowArmCharges( u_int uiIndex ) const { ntAssert( uiIndex < (u_int)(sizeof(m_afXbowArmCharges) / sizeof(float)) ); return m_afXbowArmCharges[ uiIndex ]; }

	// Clear out the charges.
	void  ClearXbowArmCharges( void ) { m_afXbowArmCharges[0] = m_afXbowArmCharges[1] = 0.0f; }

	// Obtain the firing charge. 
	float GetFiringCharge( void ) const { return m_fFiringCharge; }

	// Access the shooty mode variable. 
	SHOOTY_MODE	ShootyMode(void) const { return m_eShootyMode; }
	void ShootyMode(SHOOTY_MODE eMode) { m_eShootyMode = eMode; }

protected:
	CHashedString		m_obWalkingController;
	CHashedString		m_obCrouchedController;

	CHashedString		m_sReturnState;

	// When the archer enters 1st person shooting mode, the current int stores the id of the cool cam
	int					m_iAimCamID;

	// XML Settings for the archer. 
	const ArcherSettings* m_pSettings;

private:

	// Archer event log
	ArcherEventLogManager	m_obEventLogManager;


	// Keep a recond of the last an
	int						m_iLastVaultAnimPlayed;

	// watchdog
	bool					m_bVaultingSafe;

	// Can Crouch?
	bool					m_bCanCrouch;

	// temp debug binding to allow the archer to crouch
	bool					m_bAlwaysCrouch;

	// Is the after touch enabled for the archer?
	bool					m_bAfterTouchEnabled;

	// Are we currently crouching? (The movement controller sets this, so it should always be up to date with the visuals).
	bool					m_bCurrentlyCrouching;

	// Size of the physics capsule when the archer is crouched, (y units)
	float					m_fCrouchVolumeHeight, m_fCrouchVolumeHeight2;

	// During a vault - the camera should continue to look at the following Y value
	float					m_fVaultingCameraY;

	// Blend in time based for default walkrun movement
	float					m_fWalkRunBlendInTime;

	// Allow modification of the controller pushing during the initial state
	bool					m_bAddNewController;

	// Current charge for the arms on the archers xbow.. I've decided to place the
	// variables here to allow instant processing on button press. 
	float					m_afXbowArmCharges[2];

	// If there is a firing request - then there will be a normalised value here giving the charged power. 
	float					m_fFiringCharge; 

	// The shooty mode the archer is in. 
	SHOOTY_MODE				m_eShootyMode;
};

LV_DECLARE_USERDATA(Archer);

#endif //_ENTITY_ARCHER_H
