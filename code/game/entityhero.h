//------------------------------------------------------------------------------------------
//!
//!	\file game/entityhero.h
//!	Definition of the Hero entity object
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_HERO_H
#define	_ENTITY_HERO_H

#include "game/entityplayer.h"
#include "game/messages.h"


//------------------------------------------------------------------------------------------
//!
//! Class Hero.
//! Hero entity type
//!
//------------------------------------------------------------------------------------------
class Hero : public Player
{
	// Declare dataobject interface
	HAS_INTERFACE(Hero)

public:
	// Constructor
	Hero();

	//Destructor
	~Hero();

	// From CEntity
	virtual void OnLevelStart();

	void OnPostConstruct();
	void OnPostPostConstruct();

	// Overidden Virtuals
	virtual void Show();

	// Helpers 
	void SetMovementController();					// FIX ME - move to Player once archer is in C++?
	void ForwardInteractionMsg(MessageID obMsg);	// FIX ME - move to Player once archer is in C++?
	bool HasHeavenlySword( void ) { return m_bHasHeavenlySword; };

	// Public Variables (once part of attribute table in the script)
	bool m_bCombatRecovered;
	
	// Weapon Functions
	void LeftSword_Power();
	void RightSword_Power();
	void LeftSword_Range();
	void RightSword_Range();
	void LeftSword_Technique();
	void RightSword_Technique();
	void LeftSword_Basic();
	void LeftSword_Away();
	void RightSword_Away();
	void BasicSword_Away();

	// Health management
	virtual void	ChangeHealth( float fDelta, const char* );

	// LC recharge management
	void	RechargeLC( float fHeldTime, float fTimeStep );
	float   GetLifeclockTimeRemaining();

private:
	static void DeactivateWeapon(CEntity* pWeapon);
	u_int m_iSetMovementFrame;

protected:
	// Weapons
	CEntity* m_pBasicSword;
	CEntity* m_pBigSword;
	CEntity* m_pRanged_LHandle;
	CEntity* m_pRanged_LBlade;
	CEntity* m_pRanged_RHandle;
	CEntity* m_pRanged_RBlade;

	CHashedString m_sLeftSheathedTransform;
	CPoint        m_ptLeftSheathedPosition;
	CPoint        m_ptLeftSheathedYPR;
	CHashedString m_sRightSheathedTransform;
	CPoint        m_ptRightSheathedPosition;
	CPoint        m_ptRightSheathedYPR;
	CHashedString m_sBasicSheathedTransform;
	CPoint        m_ptBasicSheathedPosition;
	CPoint        m_ptBasicSheathedYPR;

	bool		  m_bHasHeavenlySword;
	enum          HS_MODE {POWER, RANGE, TECHNIQUE, AWAY};
	HS_MODE       m_eHeavenlySwordMode;



	// Recharging LC parameters
	bool			m_bInstantRefill;			// Should the LC refill instantly (renderable will handle blending)?  
												// Or recharge over time?
	float			m_fRequiredHeldTime;		// Time to hold grab before we can refill
	float			m_fRefillRate;				// Rate to recharge at
	float			m_fPartStyle;				// Hold onto remainer for recharging LC

	friend class Weapons;
	friend class HeroWeaponSetDef;

public:
	bool	m_bWaitingForFireComplete;
	bool	m_bWaitingForReloadComplete;
	bool	m_bFireRequested;
	// Timer used to allow the hero to queue fire requests
	float	m_fFireRequestTimer;
};

LV_DECLARE_USERDATA(Hero);

#endif //_ENTITY_HERO_H
